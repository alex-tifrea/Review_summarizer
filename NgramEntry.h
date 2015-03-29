#ifndef __NGRAMENTRY_H__
#define __NGRAMENTRY_H__

#include <string>
#include <vector>
#include <iostream>
#include "Worker.h"

class Worker;

// threshholds used to prune undesired ngrams
#define SIGMA_REP 0 //TODO: change this
#define SIGMA_READ 0.0005
#define SIGMA_SIM 0.5 // TODO: maybe change this

#define MAX_NGRAM_LENGTH 7

typedef struct BigramEntry {
    std::string second_word;
    float readability, representativeness;
} BigramEntry;

class NgramEntry {
private:
    std::vector<std::string> ngram;
    float readability, representativeness;
    Worker *worker;

public:
    NgramEntry(std::vector<std::string>, Worker*);
    ~NgramEntry();

    // Returns true if merging is possible, and false otherwise.
    NgramEntry* mergeNgrams(NgramEntry*);

    // Computes the readability score.
    void computeReadability();

    // Computes the representativeness score.
    void computeRepresent();

    // Setter for the readability score;
    void setReadability(float);

    // Computes similarity between the two given ngrams using the Jaccard
    // distance
    float computeSimilarity(NgramEntry*);

    // Returns the readability and representativeness score.
    std::pair<float, float> getScore();

    float getReadability() const {
        return readability;
    }

    float getRepresentativeness() const {
        return representativeness;
    }

    void setRepresentativeness(float rep_value){
        representativeness = rep_value;
    }

    std::vector<std::string> getNgram() const {
        return ngram;
    }

    friend std::ostream &operator<<(std::ostream&, const NgramEntry&);
};

#endif // __NGRAMENTRY_H__
