/*********************************************************
This program takes a given filename, and sentence length
and processes it to find the overall complexity of the
articles language.

@author: Collin Beaudoin
@version: October 2020
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>
#include <time.h>

/*********************************************************
This section is used to declare the methods that shall be
used throughout the program.
*********************************************************/

void processFile();
void checkMatches();
void createCSV(int *matchArr, int mainArrLen, int compareArrLen);

/*********************************************************
This is the main function of the code, it is used to
accept the parameters that shall be used throughout the
program.

@parameter filename: This is used to select the file that
will be checked for complexity
@parameter sentenceLen: This is used to figure out the
length to test for local complexity
*********************************************************/

int main()
{
    //DECLARE VARS
    char mainFile[100];
    char compareFile[100];
    omp_set_num_threads(4);

    //FLUSH INPUT AND READ FILE NAME
    fflush(stdin);
    printf("Enter a file name to upload: ");
    gets(mainFile);

    //FLUSH INPUT AND READ OUTPUT FILE NAME
    fflush(stdin);
    printf("Enter a file name to compare similarity to: ");
    gets(compareFile);

    //BEGIN FILE PROCESSING
    processFile(mainFile, compareFile);

    return 0;
}

/*********************************************************
This function is used to take a given file and process
the data into a usable array.

@parameter mainFile: The name of the file to process.
@parameter compareFile: used to name desired output file
@return: none
*********************************************************/

void processFile(char *mainFile, char *compareFile)
{
   //DECLARE VARS
   FILE *mainFp;
   FILE *compareFp;
   int mainArrLen = 255;
   int compareArrLen = 255;
   char *mainBuff = NULL;
   char *compareBuff = NULL;
   int i = 0;
   int mainTotalLen = 0;
   char ch = ' ';

   //OPEN FILE AND ALLOCATE MEMORY FOR READING FILE
   mainFp = fopen(mainFile, "r");
   mainBuff = realloc(mainBuff, sizeof(char) * mainArrLen);

   //DO UNTIL THE END OF MAIN FILE
    while (ch != EOF)
    {
        //GET THE CHARACTER
        ch = fgetc(mainFp);

        //CHECK IF THERE IS ENOUGH MEMORY TO WRITE TO MAIN BUFFER
        //IF NOT ALLOCATE MORE
        if (i >= mainArrLen)
            {
                mainArrLen = mainArrLen * 2;
                mainBuff = realloc(mainBuff, sizeof(char) * mainArrLen);
            }

        //FILTER OUT UNWANTED CHARACTERS
        if (isalpha(ch) || ch == '?')
        {
            mainBuff[i++] = tolower(ch);
        }
    }
    //END STRING FOR MAIN BUFFER
    mainBuff[i] = '\0';

    //RESET THE CHARACTER AND RESET THE INCREMENTER
    mainTotalLen = i;
    ch = ' ';
    i = 0;

    //OPEN FILE AND ALLOCATE MEMORY FOR READING FILE
    compareFp = fopen(compareFile, "r");
    compareBuff = realloc(compareBuff, sizeof(char) * compareArrLen);

    //DO UNTIL THE END OF COMPARE FILE
    while (ch != EOF)
    {
        //GET THE CHARACTER
        ch = fgetc(compareFp);

        //CHECK IF THERE IS ENOUGH MEMORY TO WRITE TO COMPARE BUFFER
        //IF NOT ALLOCATE MORE
        if (i >= compareArrLen)
            {
                compareArrLen = compareArrLen * 2;
                compareBuff = realloc(compareBuff, sizeof(char) * compareArrLen);
            }

        //FILTER OUT UNWANTED CHARACTERS
        if (isalpha(ch) || ch == '?')
        {
            compareBuff[i++] = tolower(ch);
        }
    }
    //END STRING OF COMPARE BUFFER
    compareBuff[i] = '\0';

    //CLOSE FILE READ
    fclose(mainFp);
    fclose(compareFp);

    //CHECK THE COMPLEXITY OF OUR INFORMATION
    checkMatches(mainBuff, mainTotalLen, compareBuff, i);

    //FREE MEMORY
    free(mainBuff);
    free(compareBuff);
}

/*********************************************************
This function is used to iterate over the file's data and
process the local & general complexity of the whole file.

@parameter buff: The file data to be processed.
@parameter arrLen: The overall size of the file
@parameter sentenceLen: The length to check local
complexity.
@return: none
*********************************************************/

void checkMatches(char *mainBuff, int mainArrLen, char *compareBuff, int compareArrLen)
{
    //DECLARE VARS
    int *matchArr = {0};
    int i = 0, j = 0, k = 0, m = 0;

    //ALLOCATE MEMORY FOR COUNT
    matchArr = realloc(matchArr, sizeof(int*) * mainArrLen * compareArrLen * 2);

    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);

    for (i = 0; i < mainArrLen + 1; i++)
    {
        k = i;
        m = 0;

        int upUntil = i + 1;
        if (upUntil > compareArrLen + 1)
        {
            upUntil = compareArrLen + 1;
        }

        #pragma omp parallel for private(j)
        for (j = 0; j < upUntil; j++)
        {
            if (k == 0)
            {
                matchArr[m] = 0;
            } else if (m == 0)
            {
                matchArr[k * (compareArrLen + 1)] = 0;
            } else
            {
                if ((compareBuff[m-1] == mainBuff[k-1] || compareBuff[m-1] == '?' || mainBuff[m-1] == '?'))
                {
                    matchArr[(k * (compareArrLen + 1)) + m] = matchArr[((k - 1) * (compareArrLen + 1)) + (m - 1)] + 1;
                } else
                {
                    //CHECK LEFT
                    int leftSide =  matchArr[(k * (compareArrLen + 1)) + (m - 1)] - 2;
                    //CHECK UP
                    int upSide =  matchArr[((k - 1) * (compareArrLen + 1)) + m] - 2;
                    //CHECK UP LEFT
                    int upLeftSide = matchArr[((k - 1) * (compareArrLen + 1)) + (m - 1)] - 1;

                    if (leftSide > 0 || upSide > 0 || upLeftSide > 0)
                    {
                        if (leftSide > upSide)
                        {
                            if (leftSide > upLeftSide)
                            {
                                matchArr[(k * (compareArrLen + 1)) + m] = leftSide;
                            } else
                            {
                                matchArr[(k * (compareArrLen + 1)) + m] = upLeftSide;
                            }
                        } else if (upSide > upLeftSide)
                        {
                            matchArr[(k * (compareArrLen + 1)) + m] = upSide;
                        } else
                        {
                            matchArr[(k * (compareArrLen + 1)) + m] = upLeftSide;
                        }
                    } else
                    {
                        matchArr[(k * (compareArrLen + 1)) + m] = 0;
                    }
                }
            }

            //#pragma omp atomic
            m++;
            //#pragma omp atomic
            k--;
        }
    }

    for (i = 1; i < compareArrLen + 1; i++)
    {
        k = mainArrLen;
        m = i;

        #pragma omp parallel for private(j) schedule(dynamic,1)
        for (j = i; j < compareArrLen + 1; j++)
        {
                if (compareBuff[m - 1] == mainBuff[k-1] || compareBuff[m - 1] == '?' || mainBuff[m-1] == '?')
                {
                    matchArr[(k * (compareArrLen + 1)) + m] = matchArr[((k - 1) * (compareArrLen + 1)) + (m - 1)] + 1;
                } else
                {
                    //CHECK LEFT
                    int leftSide =  matchArr[(k * (compareArrLen + 1)) + (m - 1)] - 2;
                    //CHECK UP
                    int upSide =  matchArr[((k - 1) * (compareArrLen + 1)) + m] - 2;
                    //CHECK UP LEFT
                    int upLeftSide = matchArr[((k - 1) * (compareArrLen + 1)) + (m - 1)] - 1;

                    if (leftSide > 0 || upSide > 0 || upLeftSide > 0)
                    {
                        if (leftSide > upSide)
                        {
                            if (leftSide > upLeftSide)
                            {
                                matchArr[(k * (compareArrLen + 1)) + m] = leftSide;
                            } else
                            {
                                matchArr[(k * (compareArrLen + 1)) + m] = upLeftSide;
                            }
                        } else if (upSide > upLeftSide)
                        {
                            matchArr[(k * (compareArrLen + 1)) + m] = upSide;
                        } else
                        {
                            matchArr[(k * (compareArrLen + 1)) + m] = upLeftSide;
                        }
                    } else
                    {
                        matchArr[(k * (compareArrLen + 1)) + m] = 0;
                    }
                }
            //#pragma omp atomic
            m++;
            //#pragma omp atomic
            k--;
        }
    }

    clock_gettime(CLOCK_REALTIME, &end);
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    double elapsed = seconds + nanoseconds*1e-9;

    printf("time taken %f\n",elapsed);

    createCSV(matchArr, mainArrLen, compareArrLen);

    free(matchArr);
}

/*********************************************************
This function is used to output the processed complexity
to a given file.

@parameter complexityArr: The file data to be output.
@parameter arrLen: The overall size of the file.
@parameter outputFile: The name of the output file
@parameter genComplexity: The article complexity
@return: none
*********************************************************/
void createCSV(int *matchArr, int mainArrLen, int compareArrLen)
{
    //DECLARE VARS
    FILE *filep;
    int i = 0, j = 0;

    //OPEN FILE
    filep = fopen("test.txt", "w+");

    //WRITE RESULTS TO FILE
    for(i = 0; i < mainArrLen +1; i++)
    {
        for (j = 0; j < compareArrLen +1; j++)
        {
            fprintf(filep,"%d,", matchArr[j+(i*(compareArrLen+1))]);
        }
        fprintf(filep,"\n");
    }

    //CLOSE FILE
    fclose(filep);

    //LET USER KNOW PROGRAM IS DONE
    printf("file created \n");
}
