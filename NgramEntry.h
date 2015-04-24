#ifndef __NGRAMENTRY_H__
#define __NGRAMENTRY_H__

#include <string>
#include <vector>
#include <iostream>
#include "Worker.h"

class Worker;

// Threshholds used for pruning representativeness
#define SIGMA_REP_3 0.30
#define SIGMA_REP_4 0.52
#define SIGMA_REP_5 0.70
#define SIGMA_REP_6 0.82
#define SIGMA_REP_7 1.00

// Threshholds used for pruning readability
#define SIGMA_READ_3 0.0001
#define SIGMA_READ_4 0.000001
#define SIGMA_READ_5 0.0000001
#define SIGMA_READ_6 0.00000001
#define SIGMA_READ_7 0.000000001

// Thresholds used to erase ngrams that are similar to other ngrams.
#define SIGMA_SIM_0 1
#define SIGMA_SIM_1 1
#define SIGMA_SIM_2 1
#define SIGMA_SIM_3 0.5
#define SIGMA_SIM_4 0.33
#define SIGMA_SIM_5 0.43
#define SIGMA_SIM_6 0.33
#define SIGMA_SIM_7 0.4
#define SIGMA_SIM_8 0.33

// Thresholds for scores. Still used for bigrams.
#define SIGMA_REP 0.075
#define SIGMA_READ 0.0001
#define SIGMA_SIM 0.3

// bonuses for ngrams that contain nouns and/or adjectives.
#define NOUN_BONUS 0.01
#define ADJECTIVE_BONUS 0.1
// bonuses for ngrams that convey an opinion
#define VERY_POS_NEG_BONUS 0.05
#define POS_NEG_BONUS 0.01
// penalties for ngrams that contain certain parts of speech
// (i.e UH, DT, IN, CD)
#define INTERJECTION_PENALTY (-0.1)
#define DETERMINER_PENALTY (-0.001)
#define PREPOSITION_PENALTY (-0.001)
#define NUMBER_PENALTY (-0.01)

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
    static const float sim_min_values[];
    static const float rep_min_values[];
    static const float read_min_values[];

    NgramEntry(std::vector<std::string>, Worker*);
    NgramEntry(NgramEntry*);
    ~NgramEntry();

    static std::string ngram2string(std::vector<std::string> ngram) {
        std::string text;

        if (ngram.size() == 0) {
            text = "";
            return text;
        }

        std::vector<std::string>::iterator it = ngram.begin();
        text = *it++;
        for (; it != ngram.end(); ++it) {
            text = text + " " + *(it);
        }

        return text;
    }

    // Generate the concatenated ngram from this->ngram.
    void updateText();

    // Returns the merged NgramEntry
    NgramEntry* mergeNgrams(NgramEntry*);
    NgramEntry* mergeNgrams(NgramEntry*, float);

    // Computes the readability score.
    void computeReadability();

    // Computes the representativeness score.
    void computeRepresent();

    // Computes the bonuses and penalties for ngrams based on the pos of the
    // words that form them.
    void computePOSBonusesAndPenalties();

    // Computes bonuses for ngrams that have a strong opinion.
    void computeSentimentBonuses();

    // Computes similarity between the two given ngrams using the Jaccard
    // distance
    float computeSimilarity(NgramEntry*);

    // This function is called at the end in order to rephrase the ngram so that
    // we get the best readability score for it.
    void refineNgram();

    // Returns the readability and representativeness score.
    std::pair<float, float> getScore();

    void setSentiment(Sentiment s) {
        this->sentiment = s;
    }

    Sentiment getSentiment() const {
        return sentiment;
    }

    std::string sentimentToString(Sentiment s) const {
        switch (s) {
        case VERY_NEGATIVE: return "VERY_NEGATIVE";
        case VERY_POSITIVE: return "VERY_POSITIVE";
        case NEGATIVE: return "NEGATIVE";
        case POSITIVE: return "POSITIVE";
        case NEUTRAL: return "NEUTRAL";
        default: return "bai mare";
        }

        return "bai mare";
    }

    // Setter for the readability score;
    void setReadability(float);

    float getReadability() const {
        return readability;
    }

    float getRepresentativeness() const {
        return representativeness;
    }

    void setRepresentativeness(float rep_value){
        representativeness = rep_value;
    }

    float getPOSBonus() const {
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

    void setNgram(std::vector<std::string>);

    std::vector<std::string> getNgram() const {
        return ngram;
    }

    void setText(std::string);

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
