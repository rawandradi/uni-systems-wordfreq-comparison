//Rawand 1221054

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>

#define WORD_SIZE 100
#define MAX_WORDS 280000

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
        printf("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *fileContent = (char *)malloc(fileSize + 1);
    if (fileContent == NULL)
    {
        printf("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    fread(fileContent, 1, fileSize, file);
    fileContent[fileSize] = '\0';
    fclose(file);
    return fileContent;
}

void RemoveAnyThingOtherThanChars(char *word)
{
    int length = strlen(word);
    int index = 0;
    for (int i = 0; i < length; i++)
    {
        if (isalnum(word[i]))
        {
            word[index++] = tolower(word[i]);
        }
    }
    word[index] = '\0';
}

void CountFreqOfTheWordsInTheFile(char *FileContent, Word *localWords, int *localWordCount)
{
    char *word = strtok(FileContent, " \n\t.,;:!?\"'()[]{}<>-");
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
        }
        word = strtok(NULL, " \n\t.,;:!?\"'()[]{}<>-");
    }
}

void Heapify(Word *words, int n, int i)
{
    int BigRoot = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && words[left].freq > words[BigRoot].freq)
    {
        BigRoot = left;
    }

    if (right < n && words[right].freq > words[BigRoot].freq)
    {
        BigRoot = right;
    }

    if (BigRoot != i)
    {
        Word temp = words[i];
        words[i] = words[BigRoot];
        words[BigRoot] = temp;

        Heapify(words, n, BigRoot);
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
    clock_t start, end;
    clock_t Stimestart, Stimeend;

    double cpu_time_used;
    struct timeval wall_start, wall_end; // For wall-clock time
    double wall_time_used;
    start = clock();
    gettimeofday(&wall_start, NULL);
    const char *fileName = "text8.txt"; // Change to your file path
    char *fileContent = OpenFileAndReadFromIt(fileName);
    if (fileContent == NULL)
    {
        return 1;
    }
    int UniqueWords = 0;

    Word *words = (Word *)malloc(MAX_WORDS * sizeof(struct WordFreq));
    Stimestart = clock();
    CountFreqOfTheWordsInTheFile(fileContent, words, &UniqueWords);
    Stimeend = clock();
    double executionTime = (double)(Stimeend - Stimestart) / CLOCKS_PER_SEC;
    printf("Execution time of serial part: %f seconds\n", executionTime);
    HeapSort(words, UniqueWords);

    printf("Top 10 Most Frequent Words:\n");
    for (int i = UniqueWords - 1; i > (UniqueWords - 10); i--)
    {
        printf("%s: %d\n", words[i].word, words[i].freq);
    }

    free(fileContent);

    end = clock();

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    gettimeofday(&wall_end, NULL);
    wall_time_used = (wall_end.tv_sec - wall_start.tv_sec) +
                     (wall_end.tv_usec - wall_start.tv_usec) / 1e6;

    printf("Execution time: %f seconds\n", cpu_time_used);
    printf("Wall-clock time: %f seconds\n\n", wall_time_used);

    return 0;
}
