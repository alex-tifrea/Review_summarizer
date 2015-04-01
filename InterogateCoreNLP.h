#ifndef __INTEROGATECORENLP_H__
#define __INTEROGATECORENLP_H__

#include <iostream>
#include "IO.h"
#include "NgramEntry.h"
#include <vector>
#include <string>

template<class T> struct Alloc {};

class InterogateCoreNLP {
public:
    static void init(std::unordered_map<std::string, WordInfo>&);
    static void getPartOfSpeech(std::unordered_map<std::string, WordInfo>&);

    template<typename T, template <typename, typename> class Container>
    static void getSentiment(Container<T, std::allocator<T> >&);

    static void finalize();
};
#endif // __INTEROGATECORENLP_H__
