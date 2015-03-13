#include "Interogate.h"

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

#define PATHSIZE 255

FILE* Interogate::resultPipe;
FILE* Interogate::requestPipe;

float Interogate::getJointProbability(std::vector<std::string> &phrase)
{
    std::string strPhrase = "";
    for(unsigned int i = 0; i < phrase.size() - 1; i++)
    {
        strPhrase += phrase[i] + " ";
    }
    strPhrase += phrase[phrase.size() - 1];

    return Interogate::getJointProbability(strPhrase);
}

float Interogate::getJointProbability(std::string phrase)
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

std::vector<float> Interogate::getJointProbabilities(std::vector<std::string> &phrases)
{
    std::vector<float> result;
    char buffer[100];

    /* Protocol:
     * send a unique string: "Interogate please start buffering\n"
     * send all ngrams separated by "\n"
     * send a unique string: "Interogate please send queries"
     */
    sprintf(buffer, "Interogate please start buffering\n");
    fputs(buffer, requestPipe);
    printf("am trimis interogate pls start\n");
    for(unsigned int i = 0; i < phrases.size(); i++)
    {
        strcpy(buffer, (phrases[i] + "\n").c_str());
        printf("sending %s\n", buffer);
        fputs(buffer, requestPipe);
    }
    fputs("Interogate please send queries\n", requestPipe);
    printf("am trimis interogate pls send\n");

    result.clear();
    for(unsigned int i = 0; i < phrases.size(); i++)
    {
        printf("incep fgets\n");
        fgets(buffer, 100, resultPipe);
        printf("termin fgets\n");
        printf("deb %s\n", buffer);
        result.push_back(atof(buffer));
    }

    return result;
}

void Interogate::Init()
{
    //printf("init 1\n");
    mkfifo("/tmp/ngramfifo", 0666);
    mkfifo("/tmp/ngramfiforeq", 0666);
    //pid_t pidParent = getpid();
    //printf("1 pid %d\n", pidParent);
    pid_t pid = fork();
    //pid_t pid = getpid();
    //printf("2 pid %d\n", pid);
    //if(pid != pidParent)

    //unnamed pipes
    //each pipe is: input | output
    int preq[2], pres[2];
    pipe(preq);
    pipe(pres);

    if(pid == -1)
    {
        printf("ERROR starting the Interogate.py process\n");
        exit(0);
    }
    if(pid == 0)
    {
        //child process
        /*
        char *path = (char*) malloc(PATHSIZE*sizeof(char));
        getcwd(path, PATHSIZE);
        strcat(path, "/Interogate.py");
        int ret = execlp(path, "Interogate.py", NULL);
        */
        close(preq[0]);
        close(pres[1]);
        char spreq[10], spres[10];
        sprintf(spreq, "%d", preq[1]);
        sprintf(spres, "%d", pres[0]);
        int ret = execlp("python", "python", "Interogate.py", spreq, spres, NULL);
        printf("execlp a returnat %d\n", ret);
    }
    //printf("init 2\n");
    requestPipe = fopen("/tmp/ngramfiforeq", "w");
    //requestPipe = fdopen(preq[0], "w");
    if(requestPipe == NULL)
    {
        printf("ERROR opening request pipe\n");
    }
    //printf("init 3\n");
    resultPipe = fopen("/tmp/ngramfifo", "r");
    //resultPipe = fdopen(pres[1], "r");
    if(resultPipe == NULL)
    {
        printf("ERROR opening result pipe\n");
    }
    //printf("init 4\n");
}

void Interogate::Finalize()
{
    fputs("Interogate please exit\n", requestPipe);
    fclose(resultPipe);
    fclose(requestPipe);
}
