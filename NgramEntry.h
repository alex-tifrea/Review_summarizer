#ifndef __NGRAMENTRY_H__
#define __NGRAMENTRY_H__

#include <string>
#include <vector>
#include <iostream>
#include "Worker.h"

class Worker;

// threshholds used to prune undesired ngrams
#define SIGMA_REP 0.075
#define SIGMA_READ 0.0001
#define SIGMA_SIM 0.5 // TODO: maybe change this

// bonuses for ngrams that contain nouns and/or adjectives.
#define NOUN_BONUS 0.001
#define ADJ_BONUS 0.01
#define VERY_POS_NEG_BONUS 0.05
#define POS_NEG_BONUS 0.01

#define MAX_NGRAM_LENGTH 7

enum Sentiment {
    VERY_NEGATIVE,
    NEGATIVE,
    NEUTRAL,
    POSITIVE,
    VERY_POSITIVE,
    UNDEFINED
};

class NgramEntry {
private:
    std::vector<std::string> ngram;
    std::string text;
    float readability, representativeness,
          pos_bonus, sentiment_bonus;
    Worker *worker;
    Sentiment sentiment;

public:
    NgramEntry(std::vector<std::string>, Worker*);
    NgramEntry(NgramEntry*);
    ~NgramEntry();

    // Returns the merged NgramEntry
    NgramEntry* mergeNgrams(NgramEntry*);
    NgramEntry* mergeNgrams(NgramEntry*, float);

    // Computes the readability score.
    void computeReadability();

    // Computes the representativeness score.
    void computeRepresent();

    // Computes the bonuses for ngrams that contain nouns or adjectives.
    void computePOSBonuses();

    // Computes bonuses for ngrams that have a strong opinion.
    void computeSentimentBonuses();

    // Setter for the readability score;
    void setReadability(float);

    // Computes similarity between the two given ngrams using the Jaccard
    // distance
    float computeSimilarity(NgramEntry*);

    // Returns the readability and representativeness score.
    std::pair<float, float> getScore();

    void setSentiment(Sentiment s) {
        this->sentiment = s;
    }

    Sentiment getSentiment() {
        return sentiment;
    }

    float getReadability() const {
        return readability;
    }

    float getRepresentativeness() const {
        return representativeness;
    }

    void setRepresentativeness(float rep_value){
        representativeness = rep_value;
    }

    float getPOSBonus() {
        return this->pos_bonus;
    }

    void setPOSBonus(float _pos_bonus) {
        this->pos_bonus = _pos_bonus;
    }

    float getSentimentBonus() {
        return this->sentiment_bonus;
    }

    void setSentimentBonus(float _sentiment_bonus) {
        this->sentiment_bonus = _sentiment_bonus;
    }

    int getNgramSize(){
        return (int)this->ngram.size();
    }

    std::vector<std::string> getNgram() const {
        return ngram;
    }

    std::string getText() {
        return this->text;
    }

    friend std::ostream&
    operator<<(std::ostream&, const NgramEntry&) override;

    bool operator>(const NgramEntry& ne) const {
        float lhs = this->readability + this->representativeness +
                    this->pos_bonus + this->sentiment_bonus;
        float rhs = ne.readability + ne.representativeness +
                    ne.pos_bonus + ne.sentiment_bonus;
        return lhs > rhs;
    }

    bool operator<(const NgramEntry& ne) const {
        float lhs = this->readability + this->representativeness +
                    this->pos_bonus + this->sentiment_bonus;
        float rhs = ne.readability + ne.representativeness +
                    ne.pos_bonus + ne.sentiment_bonus;
        return lhs < rhs;
    }

    struct DereferenceGreaterComparator {
        bool operator() (NgramEntry *lhs, NgramEntry *rhs) {
            return *(lhs) > *(rhs);
        }
    };
};

#endif // __NGRAMENTRY_H__
