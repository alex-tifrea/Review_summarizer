#include "Interogate.h"

#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <python2.7/Python.h>

//mkfifo
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

FILE* Interogate::resultPipe;
FILE* Interogate::requestPipe;

float Interogate::getJointProbability(std::vector<std::string> phrase)
{
    std::string strPhrase = "";
    for(int i = 0; i < phrase.size() - 1; i++)
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

std::vector<float> Interogate::getJointProbabilities(std::vector<std::string> phrase)
{
    //TODO
    
    std::vector<float> result;
    
    return result;
}

void Interogate::Init()
{
    //printf("init 1\n");
    mkfifo("/tmp/ngramfifo", S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    mkfifo("/tmp/ngramfiforeq", S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    pid_t pidParent = getpid();
    //printf("1 pid %d\n", pidParent);
    fork();
    pid_t pid = getpid();
    //printf("2 pid %d\n", pid);
    if(pid != pidParent)
    {
        int ret = execlp("/home/mihai/git/Review_summarizer/Interogate.py", "Interogate.py", NULL);
        //printf("execlp a returnat %d\n", ret);
    }
    //printf("init 2\n");
    requestPipe = fopen("/tmp/ngramfiforeq", "w"); 
    //printf("init 3\n");
    resultPipe = fopen("/tmp/ngramfifo", "r"); 
    //printf("init 4\n");
}

void Interogate::Finalize()
{
    fputs("Interogate please exit\n", requestPipe);
    fclose(resultPipe);
    fclose(requestPipe);
}
