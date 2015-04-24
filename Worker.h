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
#define MAX_BEST_NGRAMS 50
// The number of candidates considered when requesting sentiment information
// from CoreNLP in order to identify the best ngrams.
#define MAX_CANDIDATES 1000

// Number of words that we use to create bigrams (we select them based on their
// occurence frequency in the reviews)
#define MAX_WORDS_NUMBER 1500

// The minimum number of occurences a word must have in order for it t be
// considered for further creating bigrams containing it.
#define MIN_WORD_OCCURENCES 5

// Number of bigrams that we use to create (n+1)grams from ngrams. They are
// selected based on their scores and bonuses.
#define MAX_BIGRAM_NUMBER 2000

#define NGRAM_COUNT_LIMIT 5

// Used to compute representativeness
#define WINDOW_SIZE 10

// Worse value a representativeness score can have
#define LOW_REP -1234567

// Used by replaceWithBestPermutation
#define GATHER 0
#define PROCESS 1

class NgramEntry;
struct BigramEntry;

using namespace std;

class Worker {
    private:
        // Structure in which we keep al the ngrams, with a size greater than 3,
        // that we created in the while running the programs. From this vector
        // we will find the best ngrams for the final part of the project
        std::vector <NgramEntry*> vect_best_ngrams;

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

    public:
        Worker(IO*);
        ~Worker();

        // Initializes the members of the | Worker | class.
        void init();

        // Converts the unigrams into bigrams (the seed for the generation of
        // the summary).
        void initBigrams();

        // Used to traverse the solution space in a DFS manner.
        void generateCandidate();

        // Calls generateCandidate until we have reached the desired limit
        void generateLoop();

        // Calls generateCandidate until the user hits space key
        void generateInteractiveLoop();

        // Computes the representativeness of this ngram
        float computeRepresentativeness(NgramEntry*);

        // This functions computes the readability scores for all the permutations
        // formed with the ngram's words and replaces the current ngram with the one
        // that has the best score.
        void replaceWithBestPermutation(NgramEntry*, int);

        void printNgrams(std::ostream&);

        void printBestNgrams(std::ostream&);

        WordInfo getWordInfo(std::string word);

        // Computes n! for all natural values of n less than 10.
        static unsigned int factorial(unsigned int n) {
            switch(n) {
                case 0: return 1;
                case 1: return 1;
                case 2: return 2;
                case 3: return 6;
                case 4: return 24;
                case 5: return 120;
                case 6: return 720;
                case 7: return 5040;
                case 8: return 40320;
                case 9: return 362880;
                case 10: return 3628800;
                default: return -1;
            }
        }
};

#endif // __WORKER_H__
