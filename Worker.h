#ifndef __WORKER_H__
#define __WORKER_H__

#include <iostream>
#include <string>
#include <string.h>
#include <algorithm>
#include <vector>
#include <map>
#include <deque>
#include <queue>
#include <unordered_map>
#include <algorithm>

#include "NgramEntry.h"
#include "IO.h"
#include "InterogateCoreNLP.h"
#include "InterogateNGRAM.h"

// Number of ngrams that we consider for the best candidate
#define MAX_BEST_NGRAMS 20

// Number of words that we use to create bigrams (we select them based on their
// occurence frequency in the reviews)
#define MAX_WORDS_NUMBER 1500

// Number of bigrams that we use to create (n+1)grams from ngrams. They are
// selected based on their scores and bonuses.
#define MAX_BIGRAM_NUMBER 2000

#define NGRAM_COUNT_LIMIT 5

// Used to compute representativeness
#define WINDOW_SIZE 10

// Worse value a representativeness score can have
#define LOW_REP -1234567

class NgramEntry;
struct BigramEntry;

using namespace std;

class Worker {
    private:
        // Structure in whidw we keep al the ngrams, with a size greater than 3,
        // that we created in the while running the programs. From this vector
        // we will find the best ngrams for the final part of the project
        std::vector <NgramEntry *> vect_best_ngrams;

        // Structure in which we keep the best ngrams.
        // It is a bounded priority queue with maximum MAX_BEST_NGRAMS elements.
        // Even though it keeps the best ngrams (so the ones that have the
        // highest scores), we declare it with the reversed comparator, so that
        // we can bound its size.
//         priority_queue<NgramEntry*, std::vector<NgramEntry*>, NgramEntry::ReverseLess> best_ngrams;

        std::ofstream log;
        // Holds the number of occurences for each word.
        std::unordered_map<std::string, WordInfo> wordInfo;

        // A hashtable in which the key is a word and the value is a vector in
        // which we have both the review in which we can find that word and at
        // which position the word is in the review. The frequency of the word
        // in the text can also be found in this data structure by evaluating
        // the size of the vector associated with a word.
        std::unordered_map<std::string, std::vector<WordPosition> > wordPos;

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
        std::unordered_multimap<std::string, NgramEntry*> bigrams_map;

        // This is the initial review, the one that needs to be summerized. Each
        // element of the vector is a word of the original review (or a '\n' if
        // necessary). We need this to compute the representativeness of a given
        // n-gram.
        unsigned int current_review;

        int total_sentences_nr;

        // Contains all the reviews.
        std::vector<std::vector<std::string> > all_reviews;

        // Reads the input and writes back the output.
        IO *io;

        // Keep the values for the minimum representativenes for ngrams of
        // size: 3, 4 and 5.
        float *rep_min_values;

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

        void printBestNgrams(std::ostream&);

        WordInfo getWordInfo(std::string word);
};

#endif // __WORKER_H__
