#ifndef __INTEROGATE_NGRAM_H__
#define __INTEROGATE_NGRAM_H__

#include <vector>
#include <string>

//static methods used to interogate
//the MicrosoftNgram service
//
//the user needs to have the token
//set in the NGRAM_TOKEN environment variable
//

class InterogateNGRAM
{
public:
    //receives one ngram as an array of words
    static float getJointProbability(std::vector<std::string> &);
    //receives one ngram as a string of words
    static float getJointProbability(std::string);

    //receives multiple ngrams as an array of
    //strings (each with multiple words)
    static std::vector<float> getJointProbabilities(std::vector<std::string> &);

    static void Init();
    static void Finalize();
private:
    static FILE *requestPipe,
                *resultPipe;

};

#endif // __INTEROGATE_NGRAM_H__
