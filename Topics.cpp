#include "Topics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <string>
using namespace std;

#define FAIL 0.0f
#define BUFSIZE 255
#define TOPICS_NUM 10

vector<vector<string> > Topics::topics(TOPICS_NUM);

static const long long max_size = 2000;         // max length of strings
static const long long N = 40;                  // number of closest words that will be shown
static const long long max_w = 50;              // max length of vocabulary entries

static long long words, size;
static float *M;
static char *vocab;

void Topics::Init(string file)
{
    FILE *f;
    char file_name[max_size];
    float len;
    long long a, b;// c, d;
    //char ch;
    strcpy(file_name, file.c_str());
    f = fopen(file_name, "rb");
    if (f == NULL) {
        printf("Input file not found\n");
        return;
    }
    fscanf(f, "%lld", &words);
    fscanf(f, "%lld", &size);
    vocab = (char *)malloc((long long)words * max_w * sizeof(char));
    M = (float *)malloc((long long)words * (long long)size * sizeof(float));
    if (M == NULL) {
        printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
        return;
    }
    for (b = 0; b < words; b++) {
        a = 0;
        while (1) {
            vocab[b * max_w + a] = fgetc(f);
            if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
            if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
        }
        vocab[b * max_w + a] = 0;
        for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);
        len = 0;
        for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
        len = sqrt(len);
        for (a = 0; a < size; a++) M[a + b * size] /= len;
    }
    /*
       for (b = 0; b < words; b++)
       {
       printf("%s\n", &vocab[b * max_w]);
       for (a = 0; a < size; a++)
       {
       printf("%f ", M[a + b * size]);
       }
       printf("\n");
       }
       */
    fclose(f);


    //TODO populate topics
    //kitchen
    topics[0].push_back("kitchen");
    topics[0].push_back("breakfast");
    topics[0].push_back("dinner");
    topics[0].push_back("meal");
    topics[0].push_back("lunch");
    topics[0].push_back("restaurant");
    topics[0].push_back("food");
    topics[0].push_back("coffe");
    topics[0].push_back("wine");
    topics[0].push_back("snaks");
    topics[0].push_back("bar");
    topics[0].push_back("dish");
    
    //room
    topics[1].push_back("room");
    topics[1].push_back("bedroom");
    topics[1].push_back("bet");
    topics[1].push_back("sheets");
    topics[1].push_back("bathroom");
    topics[1].push_back("internet");
    topics[1].push_back("curtains");
    topics[1].push_back("tv");
    topics[1].push_back("clean");
    topics[1].push_back("dirty");
    topics[1].push_back("dusty");
    topics[1].push_back("spacious");
    topics[1].push_back("small");
    topics[1].push_back("design");
    topics[1].push_back("confortable");
    topics[1].push_back("shower");

    //staff
    topics[2].push_back("staff");
    topics[2].push_back("service");
    topics[2].push_back("roomservice");
    topics[2].push_back("helpful");
    topics[2].push_back("friendly");

    //noise
    topics[3].push_back("noise");
    topics[3].push_back("noisey");
    topics[3].push_back("quite");
    topics[3].push_back("disturbing");
    topics[3].push_back("street");
    topics[3].push_back("road");
    topics[3].push_back("wake");

    //price
    topics[4].push_back("price");
    topics[4].push_back("money");
    topics[4].push_back("reasonable");
    topics[4].push_back("expensive");
    topics[4].push_back("value");
    topics[4].push_back("moderate");
    topics[4].push_back("pricey");

    //location
    topics[5].push_back("city");
    topics[5].push_back("center");
    topics[5].push_back("walk");
    topics[5].push_back("minutes");
    topics[5].push_back("subway");
    topics[5].push_back("underground");
    topics[5].push_back("metro");
    topics[5].push_back("bus");
    topics[5].push_back("old");
    topics[5].push_back("station");
    topics[5].push_back("airport");
    topics[5].push_back("location");
    topics[5].push_back("town");
    topics[5].push_back("close");
    topics[5].push_back("away");
    topics[5].push_back("far");
    topics[5].push_back("distance");
}

void Topics::Finalize()
{
    //TODO free M and other stuff
}

//get the id of the topic
int Topics::getTopic(string ngram)
{
    float max_score = 0.0f;
    int max_idx = -1;
    unsigned int i, j;
    
    for(i = 0; i < topics.size(); i++)
    {
        float aux = 0.0f;
        for(j = 0; j < topics[i].size(); j++)
        {
            aux += Topics::getMultipleDistances(topics[i][j], ngram);
        }
        aux /= topics[i].size();
        if(aux > max_score)
        {
            max_score = aux;
            max_idx = i;
        }
    }
    return max_idx;
}

float Topics::getDistance(const std::string word1, const std::string word2)
{
    char *bestw[N];
    char st[100][max_size];
    float dist, len, bestd[N], vec[max_size];
    long long a, b, c, d, cn, bi[100];
    //char ch;
    cn = 0;
    b = 0;
    c = 0;
    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    while (1) {
        st[cn][b] = word1[c];
        b++;
        c++;
        st[cn][b] = 0;
        if (word1[c] == 0) break;
        if (word1[c] == ' ') {
            cn++;
            b = 0;
            c++;
        }
    }
    cn++;
    for (a = 0; a < cn; a++) {
        for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
        if (b == words) b = -1;
        bi[a] = b;
        if (b == -1) {
            break;
        }
    }
    if (b == -1)
        return FAIL;
    //printf("\n                                              Word       Cosine distance\n------------------------------------------------------------------------\n");
    for (a = 0; a < size; a++) vec[a] = 0;
    for (b = 0; b < cn; b++) {
        if (bi[b] == -1) continue;
        for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size];
    }
    len = 0;
    for (a = 0; a < size; a++) len += vec[a] * vec[a];
    len = sqrt(len);
    for (a = 0; a < size; a++) vec[a] /= len;
    for (a = 0; a < N; a++) bestd[a] = -1;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    for (c = 0; c < words; c++) {
        a = 0;
        for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
        if (a == 1) continue;
        dist = 0;
        for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
        for (a = 0; a < N; a++) {
            if (dist > bestd[a]) {
                for (d = N - 1; d > a; d--) {
                    bestd[d] = bestd[d - 1];
                    strcpy(bestw[d], bestw[d - 1]);
                }
                bestd[a] = dist;
                strcpy(bestw[a], &vocab[c * max_w]);
                break;
            }
        }
    }
    for (a = 0; a < N; a++)
    {
        if(strcmp(word2.c_str(), bestw[a]) == 0)
        {
            return bestd[a];
        }
        //printf("%50s\t\t%f\n", bestw[a], bestd[a]);
    }
    return FAIL;
}

float Topics::getMultipleDistances(std::string word, string ngram)
{
    float sum = 0.0f;
    char buffer[BUFSIZE];
    char *delim = (char*)" ";

    strcpy(buffer, ngram.c_str());
    char *token = strtok(buffer, delim);
    while(token != NULL)
    {
        sum += Topics::getDistance(word, token);
        token = strtok(NULL, delim);
    }
    return sum;
}
