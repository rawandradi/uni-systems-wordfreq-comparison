//Rawand1221054

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>

#define WORD_SIZE 100
#define MAX_WORDS 280000
#define NUM_THREADS 8 

// Word frequency structure
struct WordFreq
{
    char word[WORD_SIZE];
    int freq;
};

typedef struct WordFreq Word;

typedef struct
{
    char *fileChunk;
    Word *localWords;
    int *localWordCount;
    pthread_mutex_t *mutex;
    int ThreadNumber;
} ThreadData;

pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;
Word sharedWords[MAX_WORDS];
int sharedWordCount = 0;

void RemoveAnyThingOtherThanChars(char *word)
{
    int length = strlen(word);
    int idx = 0;
    for (int i = 0; i < length; i++)
    {
        if (isalnum(word[i]))
        {
            word[idx++] = tolower(word[i]);
        }
    }
    word[idx] = '\0';
}

void MergeResults(ThreadData *data)
{
    pthread_mutex_lock(&globalMutex);

    for (int i = 0; i < *data->localWordCount; i++)
    {

        int found = 0;
        for (int j = 0; j < sharedWordCount; j++)
        {
            if (strcmp(sharedWords[j].word, data->localWords[i].word) == 0)
            {
                sharedWords[j].freq += data->localWords[i].freq;
                found = 1;
                break;
            }
        }

        if (!found && sharedWordCount < MAX_WORDS)
        {
            strcpy(sharedWords[sharedWordCount].word, data->localWords[i].word);
            sharedWords[sharedWordCount].freq = data->localWords[i].freq;
            sharedWordCount++;
        }
    }

    pthread_mutex_unlock(&globalMutex);
}

void CountFreqOfTheWordsInTheFile(ThreadData *data)
{
    char *saveptr;
    char *word = strtok_r(data->fileChunk, " \n\t.,;:!?\"'()[]{}<>-", &saveptr);
    while (word != NULL)
    {
        RemoveAnyThingOtherThanChars(word);
        if (strlen(word) > 0)
        {
            int found = 0;
            for (int i = 0; i < *data->localWordCount; i++)
            {
                if (strcmp(data->localWords[i].word, word) == 0)
                {
                    data->localWords[i].freq++;
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                strcpy(data->localWords[*data->localWordCount].word, word);
                data->localWords[*data->localWordCount].freq = 1;
                (*data->localWordCount)++;
            }

            MergeResults(data);
            *data->localWordCount = 0;
        }

        word = strtok_r(NULL, " \n\t.,;:!?\"'()[]{}<>-", &saveptr);
    }
}

void *ThreadProcess(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    *data->localWordCount = 0;
    CountFreqOfTheWordsInTheFile(data);
    free(data->fileChunk);
    return NULL;
}

void Heapify(Word *words, int n, int i)
{
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && words[left].freq > words[largest].freq)
        largest = left;

    if (right < n && words[right].freq > words[largest].freq)
        largest = right;

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
        Heapify(words, n, i);

    for (int i = n - 1; i > 0; i--)
    {
        Word temp = words[0];
        words[0] = words[i];
        words[i] = temp;

        Heapify(words, i, 0);
    }
}

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

int main()
{
    const char *fileName = "text8.txt";
    clock_t start, end;
    double cpu_time_used;
    struct timeval wall_start, wall_end; // For wall-clock time
    double wall_time_used;
    start = clock();
    gettimeofday(&wall_start, NULL);
    char *fileContent = OpenFileAndReadFromIt(fileName);
    if (!fileContent)
        return 1;

    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];
    Word *localWords[NUM_THREADS];
    int localWordCounts[NUM_THREADS];
    pthread_mutex_t threadMutexes[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_mutex_init(&threadMutexes[i], NULL);
    }

    long fileSize = strlen(fileContent);
    int chunkSize = fileSize / NUM_THREADS;
    long endIdx = chunkSize;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        localWords[i] = malloc(MAX_WORDS * sizeof(Word));
        localWordCounts[i] = 0;

        long startIdx = (i == 0) ? 0 : endIdx;
        endIdx = (i == NUM_THREADS - 1) ? fileSize : (startIdx + chunkSize);

        while (endIdx < fileSize && fileContent[endIdx] != ' ' && fileContent[endIdx] != '\n')
            endIdx--;

        threadData[i].fileChunk = strndup(fileContent + startIdx, endIdx - startIdx);
        threadData[i].localWords = localWords[i];
        threadData[i].localWordCount = &localWordCounts[i];
        threadData[i].mutex = &threadMutexes[i];
        threadData[i].ThreadNumber = i;

        pthread_create(&threads[i], NULL, ThreadProcess, &threadData[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
        pthread_mutex_destroy(&threadMutexes[i]);
        free(localWords[i]);
    }

    HeapSort(sharedWords, sharedWordCount);
    printf("Top 10 Most Frequent Words:\n");
    for (int i = sharedWordCount - 1; i >= sharedWordCount - 10 && i >= 0; i--)
    {
        printf("%s: %d\n", sharedWords[i].word, sharedWords[i].freq);
    }

    end = clock();

    // Calculate execution time in seconds
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    gettimeofday(&wall_end, NULL);
    wall_time_used = (wall_end.tv_sec - wall_start.tv_sec) +
                     (wall_end.tv_usec - wall_start.tv_usec) / 1e6;

    printf("Execution time: %f seconds\n", cpu_time_used);
    printf("Wall-clock time: %f seconds\n\n", wall_time_used);
    free(fileContent);
    return 0;
}