#include "NgramEntry.h"
#include "Interogate.h"
#include <string>
#include <vector>

NgramEntry::NgramEntry(std::vector<std::string> bigram) {
    this->ngram = bigram;
    this->computeScores();
}

NgramEntry::~NgramEntry() {}

NgramEntry* NgramEntry::mergeNgrams(NgramEntry *bigram) {
    std::vector<std::string> bigram_text = bigram->getNgram();

    // Compare the last word of the n-gram with the first word of the bigram.
    if (this->ngram[this->ngram.size()-1].compare(bigram_text[0]) != 0) {
        std::cerr << "First and final words don't match." << std::endl;
        return NULL;
    }
    // Check if the n-gram is a mirror of the bigram.
    if (this->ngram.size() == 2 &&
        this->ngram[0].compare(bigram_text[1]) == 0 &&
        this->ngram[1].compare(bigram_text[0]) == 0) {
        std::cerr << "The n-grams are mirrors." << std::endl;
        return NULL;
    }

    std::vector<std::string> new_ngram_text = this->getNgram();
    new_ngram_text.push_back(bigram_text[1]);

    NgramEntry *ret = new NgramEntry(new_ngram_text);

    if (ret->getReadability() < SIGMA_READ ||
        ret->getRepresentativeness() < SIGMA_REP ||
        ret->getNgram().size() > MAX_NGRAM_LENGTH) {
        std::cerr << "SCORURI NASOALE " << ret->getReadability() << " " <<
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

void NgramEntry::computeScores() {
    this->readability = Interogate::getJointProbability(this->ngram);
    // TODO: complete this function
    // On it
    this->representativeness = 0;
}

std::pair<float, float> NgramEntry::getScore ()
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
