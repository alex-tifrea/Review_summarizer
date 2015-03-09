#ifndef __NGRAMENTRY_H__
#define __NGRAMENTRY_H__

#include <string>
#include <vector>
#include <iostream>

class NgramEntry {
private:
    std::vector<std::string> ngram;
    float readability, representativeness;

public:
    NgramEntry(std::vector<std::string>);
    ~NgramEntry();

    // Returns true if merging is possible, and false otherwise.
    bool mergeNgrams(std::vector<std::string>);

    // Computes the readability and representativeness scores.
    void computeScores();

    // Returns the readability and representativeness score.
    std::pair<float, float> getScore();

    float getReadability() const {
        return readability;
    }

    float getRepresentativeness() const {
        return representativeness;
    }

    std::vector<std::string> getNgram() const {
        return ngram;
    }

    friend std::ostream &operator<<(std::ostream&, const NgramEntry&);
};

#endif // __NGRAMENTRY_H__
