#include "Worker.h"

Worker::Worker(IO *_io) {
    io = new IO(_io);
}

Worker::~Worker() {}

template <typename T1, typename T2>
struct comp_frequencies
{
    typedef pair <T1, T2> type;
    bool operator ()(type const& a, type const& b) const
    {
        return a.second > b.second;
    }
};

void Worker::init() {
    // TODO: use IO::readReviews to populate frequency and original_review;
    io->readReviews(this->frequency, all_reviews);
    current_review = 0;
}

void Worker::initBigrams() {
    // TODO: convert frequency to a std::vector<std::pair<std::string, int> >;
    // sort the resulting vector and then use either the first half or the first
    // 500 (?) (depending on which is the least) to generate all the possible
    // bigrams. Keep a bigram only if it meets the readability and
    // representativeness requirements and if there is no other bigram similar to
    // the newly created one.
    vector <pair <string, int> > frequency_copy(this->frequency.begin(),
        this->frequency.end());
    sort (frequency_copy.begin(), frequency_copy.end(), comp_frequencies<string,int>());
    int remove_from = ((int)frequency_copy.size() >> 1) + 1;

    if (remove_from < 500)
    {
        float last_value = frequency_copy[remove_from-1].second;
        while (remove_from < (int)frequency_copy.size() &&
               frequency_copy[remove_from].second == last_value)
        {
            remove_from++;
        }
        frequency_copy.erase(frequency_copy.begin()+remove_from,
                frequency_copy.end());
    }
    else
    {
        frequency_copy.erase(frequency_copy.begin()+500, frequency_copy.end());
    }
    this->frequency.clear();
    for (int i = 0; i < (int)frequency_copy.size(); i++)
        this->frequency[frequency_copy[i].first] = frequency_copy[i].second;

    for (int i = 0; i < (int)frequency_copy.size(); i++)
    {
        for (int j = 0; j < (int)frequency_copy.size(); j++)
        {
            if (i != j)
            {
                vector <string> bigram_text;
                bigram_text.push_back(frequency_copy[i].first);
                bigram_text.push_back(frequency_copy[j].first);
                NgramEntry *new_bigram = new NgramEntry(bigram_text);
                pair<float, float> read_rep = new_bigram->getScore();
                // Check the readability and the representativeness score.
                if (read_rep.first >= SIGMA_READ && read_rep.second >= SIGMA_REP)
                {
                    this->bigrams.push_back(new_bigram);
                }
            }
        }
    }

    // TODO: when done with ^, copy the | bigrams | vector into the | ngrams |
    // vector (because initially the n-grams are the bigrams);
    for (unsigned int i = 0; i < bigrams.size(); i++) {
        ngrams.push_back(bigrams[i]);
    }
}

void Worker::generateCandidate() {
    // TODO: pop from the ngrams queue (let n be the popped n-gram) and find a
    // bigram that starts with the last word of n (this test is done by
    // NgramEntry::mergeNgrams); merge the two and push the newly created
    // (n+1)-gram at the and of the queue.
}

void Worker::printNgrams() {
    std::cout << ngrams.size() << std::endl;
    for (unsigned int i = 0; i < ngrams.size(); i++) {
        std::cout << *(ngrams[i]) << " ";
    }
    std::cout << std::endl;
}
