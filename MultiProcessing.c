
//Rawand 1221054

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>

#define WORD_SIZE 100
#define MAX_WORDS 280000
#define MAX_PROCESSES 4

struct WordFreq
{
    char word[WORD_SIZE];
    int freq;
};

typedef struct WordFreq Word;

char *OpenFileAndReadFromIt(const char *fileName)
{
    FILE *file = fopen(fileName, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *fileContent = (char *)malloc(fileSize + 1);
    if (fileContent == NULL)
    {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    fread(fileContent, 1, fileSize, file);
    fileContent[fileSize] = '\0';
    fclose(file);
    return fileContent;
}

void MergeResults(Word *sharedWords, int *sharedWordCount, Word *localWords, int localWordCount)
{
    for (int i = 0; i < localWordCount; i++)
    {
        int found = 0;
        for (int j = 0; j < *sharedWordCount; j++)
        {
            if (strcmp(sharedWords[j].word, localWords[i].word) == 0)
            {
                sharedWords[j].freq += localWords[i].freq;
                found = 1;
                break;
            }
        }
        if (!found)
        {
            strcpy(sharedWords[*sharedWordCount].word, localWords[i].word);
            sharedWords[*sharedWordCount].freq = localWords[i].freq;
            (*sharedWordCount)++;
        }
    }
}

void RemoveAnyThingOtherThanChars(char *word)
{
    int length = strlen(word);
    int idx = 0;
    for (int i = 0; i < length; i++)
    {
        if (isalnum(word[i]))
        { // Keep only alphanumeric characters
            word[idx++] = tolower(word[i]);
        }
    }
    word[idx] = '\0';
}

void CountFreqOfTheWordsInTheFile(char *chunk, Word *localWords, int *localWordCount, Word *sharedWords, int *sharedWordCount, sem_t *lock)
{
    char *word = strtok(chunk, " \n\t.,;:!?\"'()[]{}<>-");
    while (word != NULL)
    {
        RemoveAnyThingOtherThanChars(word);
        if (strlen(word) > 0)
        {
            int found = 0;
            for (int i = 0; i < *localWordCount; i++)
            {
                if (strcmp(localWords[i].word, word) == 0)
                {
                    localWords[i].freq++;
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                strcpy(localWords[*localWordCount].word, word);
                localWords[*localWordCount].freq = 1;
                (*localWordCount)++;
            }

            if (*localWordCount >= 5000)
            {
                sem_wait(lock);
                MergeResults(sharedWords, sharedWordCount, localWords, *localWordCount);
                sem_post(lock);
                *localWordCount = 0;
            }
        }
        word = strtok(NULL, " \n\t.,;:!?\"'()[]{}<>-");
    }

    if (*localWordCount > 0)
    {
        sem_wait(lock);
        MergeResults(sharedWords, sharedWordCount, localWords, *localWordCount);
        sem_post(lock);
    }
}

void Heapify(Word *words, int n, int i)
{
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && words[left].freq > words[largest].freq)
    {
        largest = left;
    }

    if (right < n && words[right].freq > words[largest].freq)
    {
        largest = right;
    }

    if (largest != i)
    {
        Word temp = words[i];
        words[i] = words[largest];
        words[largest] = temp;

        Heapify(words, n, largest);
    }
}

void HeapSort(Word *words, int n)
{
    for (int i = n / 2 - 1; i >= 0; i--)
    {
        Heapify(words, n, i);
    }

    for (int i = n - 1; i >= 0; i--)
    {
        Word temp = words[0];
        words[0] = words[i];
        words[i] = temp;

        Heapify(words, i, 0);
    }
}

int main()
{
    const char *fileName = "text8.txt";
    clock_t start, end;
    double cpu_time_used;
    struct timeval wall_start, wall_end;
    double wall_time_used;
    clock_t Stimestart, Stimeend;

    start = clock();
    gettimeofday(&wall_start, NULL);
    char *fileContent = OpenFileAndReadFromIt(fileName);
    if (fileContent == NULL)
    {
        return 1;
    }
    sem_t lock;
    sem_init(&lock, 1, 1);

    Word *sharedWords = mmap(NULL, sizeof(Word) * MAX_WORDS, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sharedWords == MAP_FAILED)
    {
        perror("mmap failed for sharedWords");
        free(fileContent);
        return 1;
    }

    int *sharedWordCount = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sharedWordCount == MAP_FAILED)
    {
        perror("mmap failed for sharedWordCount");
        free(fileContent);
        return 1;
    }
    *sharedWordCount = 0;

    int numProcesses = MAX_PROCESSES;
    int chunkSize = strlen(fileContent) / numProcesses;
    pid_t pids[MAX_PROCESSES];

    for (int i = 0; i < numProcesses; i++)
    {
        int startIdx = i * chunkSize;
        int endIdx = (i == numProcesses - 1) ? strlen(fileContent) : startIdx + chunkSize;

        while (!isspace(fileContent[endIdx]) && fileContent[endIdx] != '\n' && fileContent[endIdx] != '\0')
        {
            endIdx++;
        }

        if ((pids[i] = fork()) == 0)
        {
            Word *localWords = malloc(sizeof(Word) * MAX_WORDS);
            int localWordCount = 0;
            char *chunk = strndup(fileContent + startIdx, endIdx - startIdx);
            Stimestart = clock();
            CountFreqOfTheWordsInTheFile(chunk, localWords, &localWordCount, sharedWords, sharedWordCount, &lock);
            Stimeend = clock();
            double executionTime = (double)(Stimeend - Stimestart) / CLOCKS_PER_SEC;
            printf("Execution time of serial part: %f seconds\n", executionTime);
            free(chunk);
            free(localWords);
            exit(0);
        }
    }

    for (int i = 0; i < numProcesses; i++)
    {
        waitpid(pids[i], NULL, 0);
    }

    HeapSort(sharedWords, *sharedWordCount);

    printf("Top 10 Most Frequent Words:\n");
    for (int i = *sharedWordCount - 1; i > (*sharedWordCount - 10); i--)
    {
        printf("%s: %d\n", sharedWords[i].word, sharedWords[i].freq);
    }

    munmap(sharedWords, sizeof(Word) * MAX_WORDS);
    munmap(sharedWordCount, sizeof(int));
    free(fileContent);

    end = clock();

    // Calculate execution time in seconds
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    gettimeofday(&wall_end, NULL);
    wall_time_used = (wall_end.tv_sec - wall_start.tv_sec) +
                     (wall_end.tv_usec - wall_start.tv_usec) / 1e6;

    printf("Execution time: %f seconds\n", cpu_time_used);
    printf("Wall-clock time: %f seconds\n\n", wall_time_used);

    return 0;
}
