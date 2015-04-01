#include "NgramEntry.h"
#include "InterogateNGRAM.h"
#include <string>
#include <vector>

NgramEntry::NgramEntry(std::vector<std::string> _ngram, Worker *_worker) :
    ngram(_ngram),
    worker(_worker),
    sentiment(UNDEFINED)
{
    // Update the text field.
    std::vector<std::string>::iterator it = this->ngram.begin();
    text = *it++;
    for (; it != this->ngram.end(); ++it) {
        text = text + " " + *(it);
    }
    // TODO: solutie temporara; va trebui sa nu calculam scorul de
    // readability in constructor pentru niciun tip de ngrama, nu doar pentru
    // bigrame
    if (this->ngram.size() > 2) {
//         this->computeReadability();
        this->readability = 0;
        this->computeRepresent();
    } else {
        this->readability = 0;
        this->computeRepresent();
    }
}

NgramEntry::NgramEntry(NgramEntry *ne) :
    ngram(ne->ngram),
    text(ne->text),
    readability(ne->readability),
    representativeness(ne->representativeness),
    sentiment(ne->sentiment)
{
    this->worker = ne->worker;
}

NgramEntry::~NgramEntry() {}

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
        std::cout<< "The n-grams are mirrors." << std::endl;
        return NULL;
    }

    std::vector<std::string> new_ngram_text = this->getNgram();
    new_ngram_text.push_back(bigram_text[1]);

    NgramEntry *ret = new NgramEntry(new_ngram_text, this->worker);

    if (ret->getReadability() < SIGMA_READ ||
        ret->getRepresentativeness() < SIGMA_REP ||
        ret->getNgram().size() > MAX_NGRAM_LENGTH) {
        std::cout<< "SCORURI NASOALE\n";
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
        std::cout<< "The n-grams are mirrors." << std::endl;
        return NULL;
    }

    std::vector<std::string> new_ngram_text = this->getNgram();
    new_ngram_text.push_back(bigram_text[1]);

    NgramEntry *ret = new NgramEntry(new_ngram_text, this->worker);
    // Set readability score.
    ret->setReadability(readability);

    if (ret->getReadability() < SIGMA_READ ||
        ret->getRepresentativeness() < SIGMA_REP ||
        ret->getNgram().size() > MAX_NGRAM_LENGTH) {
//         std::cout<< "SCORURI NASOALE\n";
        std::cout<< "SCORURI NASOALE " << ret->getReadability() << " " <<
                ret->getRepresentativeness() << std::endl;
        return NULL;
    }

    return ret;
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
        dist = (float) intersection_count / (intersection_count +
                                             ne_text.size() +
                                             this->ngram.size());
    }

    std::cout << "SIMILARITATE " << dist << std::endl;

    return dist;
}

void NgramEntry::computeReadability() {
    this->readability = InterogateNGRAM::getJointProbability(this->ngram);
}

void NgramEntry::computeRepresent() {
    // Compute the representativeness score.
    this->representativeness = this->worker->
                                     computeRepresentativeness(this);
    // Adjust the representativeness score by adding bonuses if the ngram
    // contains at least a noun and at least an adjective.
    bool hasNoun = false, hasAdjective = false;
    for (auto it = ngram.begin(); it != ngram.end(); ++it) {
        WordInfo wi = this->worker->getWordInfo(*it);
        auto adj = wi.partOfSpeech.find("JJ");
        auto noun = wi.partOfSpeech.find("NN");
        if (adj != string::npos) {
            hasAdjective = true;
        }

        if (noun != string::npos) {
            hasNoun = true;
        }

        if (hasNoun && hasAdjective) {
            break;
        }
    }

    if (hasNoun) {
        this->representativeness += NOUN_BONUS;
    }

    if (hasAdjective) {
        this->representativeness += ADJ_BONUS;
    }
}

void NgramEntry::setReadability(float _read) {
    this->readability = _read;
}

std::pair<float, float> NgramEntry::getScore()
{
    return std::make_pair(this->readability, this->representativeness);
}

std::ostream &operator<<(std::ostream &out, const NgramEntry &ne) {
    out << "[";
    std::vector<std::string> _ngram = ne.getNgram();
    for (unsigned int i = 0; i < _ngram.size()-1; i++) {
        out << _ngram[i] << " ";
    }
    out << _ngram[_ngram.size()-1] << ", ";
    out << ne.getReadability() << ", " << ne.getRepresentativeness() << "] ";
    return out;
}
