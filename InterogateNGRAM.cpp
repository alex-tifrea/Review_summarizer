#include "InterogateNGRAM.h"

#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//mkfifo
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

//if you run this program on a very busy machine
//consider incresing this value
#define FD_SIZE 5

FILE* InterogateNGRAM::resultPipe;
FILE* InterogateNGRAM::requestPipe;

float InterogateNGRAM::getJointProbability(std::vector<std::string> &phrase)
{
    std::string strPhrase = "";
    for(unsigned int i = 0; i < phrase.size() - 1; i++)
    {
        strPhrase += phrase[i] + " ";
    }
    strPhrase += phrase[phrase.size() - 1];

    return InterogateNGRAM::getJointProbability(strPhrase);
}

float InterogateNGRAM::getJointProbability(std::string phrase)
{
    //trimit request
    phrase += "\n";
    fputs(phrase.c_str(), requestPipe);
    fflush(requestPipe);

    //primesc rezultatul
    char buffer[20];
    fgets(buffer, 20, resultPipe);
    //puts(buffer);

    float result = atof(buffer);
    return result;
}

std::vector<float> InterogateNGRAM::getJointProbabilities(std::vector<std::string> &phrases)
{
    std::vector<float> result;
    char buffer[100];

    /* Protocol:
     * send a unique string: "Interogate please start buffering\n"
     * send all ngrams separated by "\n"
     * send a unique string: "Interogate please send queries"
     */
    fputs("Interogate please start buffering\n", requestPipe);
    //printf("am trimis interogate pls start\n");
    for(unsigned int i = 0; i < phrases.size(); i++)
    {
        strcpy(buffer, (phrases[i] + "\n").c_str());
        //printf("sending %s\n", buffer);
        fputs(buffer, requestPipe);
        //
        //  ATTENTION
        //
        //if InterogateNGRAM.py locks on readline
        //you should decomment this line
        //fflush(requestPipe);
    }
    fputs("Interogate please send queries\n", requestPipe);
    fflush(requestPipe);
    //printf("am trimis interogate pls send\n");

    result.clear();
    for(unsigned int i = 0; i < phrases.size(); i++)
    {
        //printf("incep fgets\n");
        fgets(buffer, 100, resultPipe);
        //printf("termin fgets\n");
        //printf("deb %s\n", buffer);
        result.push_back(atof(buffer));
    }

    return result;
}

void InterogateNGRAM::Init()
{
    //printf("init 1\n");
    //mkfifo("/tmp/ngramfifo", 0666);
    //mkfifo("/tmp/ngramfiforeq", 0666);

    //unnamed pipes
    //each pipe is: input | output
    int requestPipeFd[2], resultPipeFd[2];
    if(pipe(requestPipeFd) == -1)
    {
        printf("ERROR creating the annonymus request pipe\n");
        exit(0);
    }
    if(pipe(resultPipeFd) == -1)
    {
        printf("ERROR creating the annonymus result pipe\n");
        exit(0);
    }
    //printf("pipe: am creat pipe-urile: %d %d %d %d\n", requestPipeFd[0], requestPipeFd[1], resultPipeFd[0], resultPipeFd[1]);

    pid_t pid = fork();
    if(pid == -1)
    {
        printf("ERROR starting the InterogateNGRAM.py process\n");
        exit(0);
    }
    if(pid == 0)
    {
        //child process
        close(requestPipeFd[1]);
        close(resultPipeFd[0]);
        char requestPipeFdStr[FD_SIZE], resultPipeFdStr[FD_SIZE];
        sprintf(requestPipeFdStr, "%d", requestPipeFd[0]);
        sprintf(resultPipeFdStr, "%d", resultPipeFd[1]);
        int ret = execlp("python", "python", "InterogateNGRAM.py", requestPipeFdStr, resultPipeFdStr, NULL);
        if(ret == -1)
        {
            printf("ERROR at execlp\n");
        }
    }
    //parent process
    //printf("init 2\n");
    close(requestPipeFd[0]);
    close(resultPipeFd[1]);
    //requestPipe = fopen("/tmp/ngramfiforeq", "w");
    requestPipe = fdopen(requestPipeFd[1], "w");
    if(requestPipe == NULL)
    {
        printf("ERROR opening request pipe\n");
    }
    //printf("init 3\n");
    //resultPipe = fopen("/tmp/ngramfifo", "r");
    resultPipe = fdopen(resultPipeFd[0], "r");
    if(resultPipe == NULL)
    {
        printf("ERROR opening result pipe\n");
    }
    //printf("init 4\n");
}

void InterogateNGRAM::Finalize()
{
    fputs("Interogate please exit\n", requestPipe);
    fclose(resultPipe);
    fclose(requestPipe);
}
