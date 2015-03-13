#ifndef __INTEROGATECORENLP_H__
#define __INTEROGATECORENLP_H__

#include <iostream>

class InterogateCoreNLP {
public:
    static void Init();
    static void Finalize();
private:
    FILE *words_input, *xml_output, *parsed_output;
};
#endif // __INTEROGATECORENLP_H__
