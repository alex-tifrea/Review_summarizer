#include "NgramEntry.h"
#include "InterogateNGRAM.h"
#include <string>
#include <vector>
#include "POS.h"

const float NgramEntry::sim_min_values[] = {SIGMA_SIM_3,SIGMA_SIM_4,
                                            SIGMA_SIM_5,SIGMA_SIM_6,
                                            SIGMA_SIM_7,SIGMA_SIM_8};

const float NgramEntry::rep_min_values[] = {SIGMA_REP_3,SIGMA_REP_4,
                                            SIGMA_REP_5,SIGMA_REP_6,
                                            SIGMA_REP_7};

const float NgramEntry::read_min_values[] = {SIGMA_READ_3,SIGMA_READ_4,
                                             SIGMA_READ_5,SIGMA_READ_6,
                                             SIGMA_READ_7};

NgramEntry::NgramEntry(std::vector<std::string> _ngram, Worker *_worker) :
    ngram(_ngram),
    readability(0),
    pos_bonus(0),
    sentiment_bonus(0),
    worker(_worker),
    sentiment(UNDEFINED)
{
    // Update the text field.
    this->updateText();

    this->computePOSBonusesAndPenalties();
    this->computeRepresent();
}

NgramEntry::NgramEntry(NgramEntry *ne) :
    ngram(ne->ngram),
    text(ne->text),
    readability(ne->readability),
    representativeness(ne->representativeness),
    pos_bonus(ne->pos_bonus),
    sentiment_bonus(ne->sentiment_bonus),
    sentiment(ne->sentiment)
{
    this->worker = ne->worker;
}

NgramEntry::~NgramEntry() {}

void NgramEntry::updateText() {
    if (this->ngram.size() == 0) {
        this->text = "";
        return;
    }

    std::vector<std::string>::iterator it = this->ngram.begin();
    this->text = *it++;
    for (; it != this->ngram.end(); ++it) {
        this->text = this->text + " " + *(it);
    }
}

NgramEntry* NgramEntry::mergeNgrams(NgramEntry *bigram) {
    std::vector<std::string> bigram_text = bigram->getNgram();

    // Compare the last word of the n-gram with the first word of the bigram.
    if (this->ngram[this->ngram.size()-1].compare(bigram_text[0]) != 0) {
        std::cout<< "First and final words don't match." << std::endl;
        return NULL;
    }
    // Check if the n-gram is a mirror of the bigram.
    if (this->ngram.size() == 2 &&
        this->ngram[0].compare(bigram_text[1]) == 0 &&
        this->ngram[1].compare(bigram_text[0]) == 0) {
//         std::cout<< "The n-grams are mirrors." << std::endl;
        return NULL;
    }

    std::vector<std::string> new_ngram_text = this->getNgram();
    new_ngram_text.push_back(bigram_text[1]);

    NgramEntry *ret = new NgramEntry(new_ngram_text, this->worker);

    if (ret->getReadability() < NgramEntry::read_min_values[ret->getNgramSize()] ||
        ret->getRepresentativeness() < SIGMA_REP ||
        ret->getNgram().size() > MAX_NGRAM_LENGTH) {
//         std::cout<< "SCORURI NASOALE\n";
//         std::cout<< "SCORURI NASOALE " << ret->getReadability() << " " <<
//                 ret->getRepresentativeness() << std::endl;
        return NULL;
    }

    return ret;
}

// Pass the readability of the new ngram as a parameter, to be set before
// evaluating the scores of the ngram.
NgramEntry* NgramEntry::mergeNgrams(NgramEntry *bigram, float readability) {
    std::vector<std::string> bigram_text = bigram->getNgram();

    // Compare the last word of the n-gram with the first word of the bigram.
    if (this->ngram[this->ngram.size()-1].compare(bigram_text[0]) != 0) {
        std::cout<< "First and final words don't match." << std::endl;
        return NULL;
    }
    // Check if the n-gram is a mirror of the bigram.
    if (this->ngram.size() == 2 &&
        this->ngram[0].compare(bigram_text[1]) == 0 &&
        this->ngram[1].compare(bigram_text[0]) == 0) {
//         std::cout<< "The n-grams are mirrors." << std::endl;
        return NULL;
    }

    std::vector<std::string> new_ngram_text = this->getNgram();
    new_ngram_text.push_back(bigram_text[1]);

    NgramEntry *ret = new NgramEntry(new_ngram_text, this->worker);
    // Set readability score.
    ret->setReadability(readability);

    if (ret->getReadability() < NgramEntry::read_min_values[ret->getNgramSize()] ||
        ret->getRepresentativeness() < SIGMA_REP ||
        ret->getNgram().size() > MAX_NGRAM_LENGTH) {
//         std::cout<< "SCORURI NASOALE\n";
//         std::cout<< "SCORURI NASOALE " << ret->getReadability() << " " <<
//                 ret->getRepresentativeness() << std::endl;
        return NULL;
    }

    return ret;
}

void NgramEntry::computeReadability() {
    this->readability = InterogateNGRAM::getJointProbability(this->ngram);
}

void NgramEntry::computeRepresent() {
    // Compute the representativeness score.
    this->representativeness = this->worker->
                                     computeRepresentativeness(this);
}

void NgramEntry::computePOSBonusesAndPenalties() {
    // Set bonuses for ngrams that contain at least a noun or
    // at least an adjective.
    bool hasNoun = false, hasAdjective = false,
         hasInterjection = false, hasDeterminer = false, hasPreposition = false,
         hasNumber = false;
    for (auto it = this->ngram.begin(); it != this->ngram.end(); ++it) {
        WordInfo wi = this->worker->getWordInfo(*it);
        auto adj = wi.partOfSpeech.find(POS_t::name[JJ]);
        auto noun = wi.partOfSpeech.find(POS_t::name[NN]);
        auto interjection = wi.partOfSpeech.find(POS_t::name[UH]);
        auto determiner = wi.partOfSpeech.find(POS_t::name[DT]);
        auto preposition = wi.partOfSpeech.find(POS_t::name[IN]);
        auto number = wi.partOfSpeech.find(POS_t::name[CD]);

        if (adj != string::npos) {
            hasAdjective = true;
        }

        if (noun != string::npos) {
            hasNoun = true;
        }
        if (interjection != string::npos) {
            hasInterjection = true;
        }
        if (determiner != string::npos) {
            hasDeterminer = true;
        }
        if (preposition != string::npos) {
            hasPreposition = true;
        }
        if (number != string::npos) {
            hasNumber = true;
        }
    }

    if (hasNoun) {
        this->pos_bonus += NOUN_BONUS;
    }
    if (hasNoun && hasAdjective) {
        // Only give the adjective bonus if the ngram also has a noun.
        this->pos_bonus += ADJECTIVE_BONUS;
    } else {
        // If the ngram has NO adjectives, then apply a penalty.
        this->pos_bonus -= ADJECTIVE_BONUS;
    }
    if (hasInterjection) {
        this->pos_bonus += INTERJECTION_PENALTY;
    }
    if (hasDeterminer) {
        this->pos_bonus += DETERMINER_PENALTY;
    }
    if (hasPreposition) {
        this->pos_bonus += PREPOSITION_PENALTY;
    }
    if (hasNumber) {
        this->pos_bonus += NUMBER_PENALTY;
    }
}

// Computes bonuses for ngrams that have a strong opinion.
void NgramEntry::computeSentimentBonuses() {
    if (this->sentiment == VERY_NEGATIVE ||
        this->sentiment == VERY_POSITIVE) {
        this->sentiment_bonus += VERY_POS_NEG_BONUS;
    }
    if (this->sentiment == POSITIVE ||
        this->sentiment == NEGATIVE) {
        this->sentiment_bonus += POS_NEG_BONUS;
    }
}

// Use Jaccard distance to compute the similarity
float NgramEntry::computeSimilarity(NgramEntry *ne) {
    std::vector<std::string> ne_text = ne->getNgram();
    unsigned int intersection_count = 0;
    // It's ok to use for's because NgramEntry.ngram.size() is MAX_NGRAM_LENGTH
    // at most.
    for (unsigned int i = 0; i < ne_text.size(); i++) {
        for (unsigned int j = 0; j < this->ngram.size(); j++) {
            if (ne_text[i] == this->ngram[j]) {
                intersection_count++;
            }
        }
    }

    // the distance between the two ngrams is:
    // d = |intersection(A, B)| / |union(A, B)|, where |N| = card(N)
    float dist = 1; // if both ngrams have 0 words, then the distance is 1
    if (ne_text.size() + this->ngram.size() != 0) {
        dist = (float) intersection_count / (ne_text.size() +
                                             this->ngram.size() -
                                             intersection_count);
    }

    return dist;
}

void NgramEntry::refineNgram() {
    std::vector<unsigned int> ngram_pos(0);
    for (unsigned int i = 0; i < this->ngram.size(); i++) {
        for (unsigned int j = 0; j < POS_t::name.size(); j++) {
            // Get word info for current word.
            WordInfo wi = this->worker->getWordInfo(this->ngram[i]);
            if (wi.partOfSpeech.compare(POS_t::name[j]) == 0) {
                ngram_pos.push_back(j);
                break;
            }
        }
    }

    // Make sure that we don't have NN-JJ constructions (noun followed by an
    // adjective).
    for (unsigned int i = 0; i < ngram_pos.size() - 1; i++) {
        if (POS_t::isNoun(ngram_pos[i]) && POS_t::isAdjective(ngram_pos[i+1])) {
            swap(this->ngram[i], this->ngram[i+1]);
        }
    }

    /*
     * XXX: Uncomment this if needed.
    // Make sure that we don't have RB-VB constructions (adverb followed by a
    // verb).
    for (unsigned int i = 0; i < ngram_pos.size() - 1; i++) {
        if (POS_t::isAdverb(ngram_pos[i]) && POS_t::isVerb(ngram_pos[i+1])) {
            swap(this->ngram[i], this->ngram[i+1]);
        }
    }
    */

    for (unsigned int i = 0; i < ngram_pos.size(); i++) {
        if (!POS_t::isToBeRemoved(ngram_pos[i])) {
            break;
        }

        // Remove the word.
        this->ngram.erase(this->ngram.begin() + i);
        ngram_pos.erase(ngram_pos.begin() + i);
        i--;
    }

    if (this->ngram.size() != 0) {
        unsigned int i = ngram_pos.size() - 1;
        while (i >= 0) {
            // XXX: (Maybe) Also replace any adjectives at the end of the ngram
            if (!POS_t::isToBeRemoved(ngram_pos[i])) {
                break;
            }

            // Remove the word.
            this->ngram.erase(this->ngram.begin() + i);
            ngram_pos.erase(ngram_pos.begin() + i);

            i--;
        }
    }

    // Update the text field after the changes.
    // !!! This should be the last line in refineNgram.
    this->updateText();
}

void NgramEntry::setNgram(std::vector<std::string> _ngram) {
    this->ngram = _ngram;
}

void NgramEntry::setText(std::string _text) {
    this->text = _text;
}

void NgramEntry::setReadability(float _read) {
    this->readability = _read;
}

std::pair<float, float> NgramEntry::getScore()
{
    return std::make_pair(this->readability, this->representativeness);
}

std::ostream &operator<<(std::ostream &out, const NgramEntry &ne) {
    if (ne.ngram.size() == 0) {
        return out;
    }
    out << "[";
    std::vector<std::string> _ngram = ne.getNgram();
    for (unsigned int i = 0; i < _ngram.size()-1; i++) {
        out << _ngram[i] << " ";
    }
    out << _ngram[_ngram.size()-1] << ", ";
    out << ne.getReadability() << ", " << ne.getRepresentativeness() << ", "
        << ne.getPOSBonus() << " " << ne.sentimentToString(ne.getSentiment()) << "] ";
    return out;
}
