#ifndef __WORKER_H__
#define __WORKER_H__

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <deque>
#include <unordered_map>

#include "NgramEntry.h"
#include "IO.h"
#include "InterogateCoreNLP.h"
#include "InterogateNGRAM.h"

#define MIN_BIGRAM_NUMBER 500 // TODO: maybe change this
#define NGRAM_COUNT_LIMIT 5
#define WINDOW_SIZE 10

class NgramEntry;
struct BigramEntry;

using namespace std;

class Worker {
    private:
        std::ofstream log;
        // Holds the number of occurences for each word.
        std::map<std::string, WordInfo> wordInfo;

        // A hashtable in which the key is a word and the value is a vector in
        // which we have both the review in which we can find that word and at
        // which position the word is in the review. The frequency of the word
        // in the text can also be found in this data structure by evaluating
        // the size of the vector associated with a word.
        std::map<std::string, std::vector<WordPosition> > wordPos;

        // The n-grams at a given time. Initially they are all bigrams. At each step,
        // we try to merge an n-gram with a bigram to obtain an (n+1)-gram.
        std::deque<NgramEntry*> ngrams;

        // The bigrams. They are used as seed in the generation of the n-grams and are
        // also used at each step in the generation process (a bigram will be
        // appended at the end of a given n-gram at each step).
        // They are all saved in an unordered map with the key being the first
        // word in every bigram and the value being a BigramEntry object
        std::vector<NgramEntry*> bigrams;
        // TODO remove this ^ line
        std::unordered_map<std::string, BigramEntry*> bigrams_t;

        // This is the initial review, the one that needs to be summerized. Each
        // element of the vector is a word of the original review (or a '\n' if
        // necessary). We need this to compute the representativeness of a given
        // n-gram.
        unsigned int current_review;

        // Contains all the reviews.
        std::vector<std::vector<std::string> > all_reviews;

        // Reads the input and writes back the output.
        IO *io;

        // TODO We need to find the total number of sentences in the reviews in
        // order to find the corresponding joint probability, which is the
        // number of times two words are in the same sentence over the total
        // number of sentences. This variable will be 5 for the moment.
        int sentences_count;

    public:
        Worker(IO*);
        ~Worker();

        // Initializes the members of the | Worker | class.
        void init();

        // Converts the unigrams into bigrams (the seed for the generation of
        // the summary).
        void initBigrams();

        // TODO: searches for the bigrams that start with the last word of the
        // given bigram
        std::vector<int> binarySearch(NgramEntry*);

        // Used to traverse the solution space in a DFS manner.
        void generateCandidate();

        // Calls generateCandidate until we have reached the desired limit
        void generateLoop();

        // Calls generateCandidate until the user hits space key
        void generateInteractiveLoop();

        // Computes the representativeness of this ngram
        float computeRepresentativeness(NgramEntry *current_ngram);

        void printNgrams(std::ostream&);
};

#endif // __WORKER_H__
