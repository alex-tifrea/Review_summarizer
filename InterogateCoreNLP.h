#ifndef __INTEROGATECORENLP_H__
#define __INTEROGATECORENLP_H__

#include <iostream>
#include "IO.h"

class InterogateCoreNLP {
public:
    static void init(std::unordered_map<std::string, WordInfo>&);
    static void getPartOfSpeech(std::unordered_map<std::string, WordInfo>&);
    static void getSentiment(void);
    static void finalize();
private:
    static FILE *words_input,
                *xml_output,
                *parsed_output;
};
#endif // __INTEROGATECORENLP_H__
