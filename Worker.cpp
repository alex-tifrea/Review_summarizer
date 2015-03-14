#include "Worker.h"
#include <math.h>

Worker::Worker(IO *_io) {
    io = new IO(_io);
}

Worker::~Worker() {}

struct comp_frequencies
{
    typedef pair <string, WordInfo> type;
    bool operator()(type const& a, type const& b) const
    {
        return a.second.frequency > b.second.frequency;
    }
};

void Worker::init() {
    // use IO::readReviews to populate wordInfo.frequency and original_review;

    io->readReviews(this->wordInfo, all_reviews, wordPos);
    map<string, vector<WordPosition> >::iterator it;
    for (it = wordPos.begin(); it != wordPos.end(); ++it)
    {
        cout << it->first << " ";
        for (unsigned int i = 0; i < it->second.size(); i++)
            cout << all_reviews[it->second[i].review_nr][it->second[i].word_nr] << " ";
        cout << endl;
    }

    // Start aquiring part-of-speech information for all the words in the
    // reviews, using CoreNLP
    InterogateCoreNLP::init(this->wordInfo);
    InterogateCoreNLP::getPartOfSpeech(this->wordInfo);
    InterogateCoreNLP::finalize();
    // At this point, this->wordInfo should be contain both the frequency and
    // the part-of-speech of each word appearing in the reviews.

    fstream words;
    words.open("wordInfo.out", std::fstream::out);
    std::map<std::string, WordInfo>::iterator iter;
    for (iter = wordInfo.begin(); iter != wordInfo.end(); ++iter) {
        words << iter->first << " " << iter->second.frequency << " " <<
                 iter->second.partOfSpeech << std::endl;
    }
    words.close();

    current_review = 0;
    sentences_count= 5;
}

void Worker::initBigrams() {
    // convert frequency to a std::vector<std::pair<std::string, int> >;
    // sort the resulting vector and then use either the first half or the first
    // MIN_BIGRAM_NUMBER (?) (depending on which is the least) to generate all the possible
    // bigrams. Keep a bigram only if it meets the readability and
    // representativeness requirements and if there is no other bigram similar to
    // the newly created one.

    vector <pair <string, WordInfo> > wordInfo_copy(this->wordInfo.begin(),
        this->wordInfo.end());
    sort (wordInfo_copy.begin(), wordInfo_copy.end(), comp_frequencies());
    int remove_from = ((int)wordInfo_copy.size() >> 1) + 1;

    if (remove_from < MIN_BIGRAM_NUMBER)
    {
        unsigned int last_value = wordInfo_copy[remove_from-1].second.frequency;
        while (remove_from < (int)wordInfo_copy.size() &&
               wordInfo_copy[remove_from].second.frequency == last_value)
        {
            remove_from++;
        }
        wordInfo_copy.erase(wordInfo_copy.begin()+remove_from,
                wordInfo_copy.end());
    }
    else
    {
        wordInfo_copy.erase(wordInfo_copy.begin()+MIN_BIGRAM_NUMBER, wordInfo_copy.end());
    }

    this->wordInfo.clear();
    for (int i = 0; i < (int)wordInfo_copy.size(); i++)
        this->wordInfo[wordInfo_copy[i].first] = wordInfo_copy[i].second;

    // newBigrams is used to store all the possible bigrams, until we can decide
    // which ones we want to keep
    std::vector<NgramEntry*> newBigrams;
    for (int i = 0; i < (int)wordInfo_copy.size(); i++)
    {
        for (int j = 0; j < (int)wordInfo_copy.size(); j++)
        {
            if (i != j)
            {
                vector <string> bigram_text;
                bigram_text.push_back(wordInfo_copy[i].first);
                bigram_text.push_back(wordInfo_copy[j].first);
                newBigrams.push_back(new NgramEntry(bigram_text));
            }
        }
    }

    // TODO: here we should call getJointProbabilities for all the bigrams at
    // once

    for (unsigned int i = 0; i < newBigrams.size(); i++)
    {
        pair<float, float> read_rep = newBigrams[i]->getScore();
        // Check the readability and the representativeness score.
        if (read_rep.first >= SIGMA_READ && read_rep.second >= SIGMA_REP)
        {
            this->bigrams.push_back(newBigrams[i]);
        }
    }

    // TODO: sort bigrams alphabetically after the first word so that we can use
    // binary search when looking for a certain bigram in
    // Worker::generateCandidate

    // when done with bigrams, copy the | bigrams | vector into the | ngrams |
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
    NgramEntry *curr_ngram = ngrams.front();
    ngrams.pop_front();

    // TODO: here use Worker::binarySearch instead
    for (unsigned int i = 0; i < bigrams.size(); i++) {

        std::vector<std::string> bigram_text = bigrams[i]->getNgram(),
                                 ngram_text  = curr_ngram->getNgram();
        std::cout << "About to merge " << bigram_text[0] << " " <<
                     ngram_text[ngram_text.size()-1] << std::endl;
        if (bigram_text[0] == ngram_text[ngram_text.size()-1]) {
            NgramEntry *new_ngram = curr_ngram->mergeNgrams(bigrams[i]);
            if (!new_ngram) {
//                 std::cerr << "Merge did not go smoothly." << std::endl;
            } else {
                // Check if the newly created ngram is similar to any of the
                // other ngrams
                bool is_unique = true;
                // TODO: this might need to be changed. I think it may slow us
                // down
                for (unsigned int i = 0; i < ngrams.size(); i++) {
                    if (ngrams[i]->computeSimilarity(new_ngram) > SIGMA_SIM) {
                        is_unique = false;
                        break;
                    }
                }

                if (is_unique) {
                    // Add the newly created (n+1)-gram to the deque
                    ngrams.push_back(new_ngram);
                }
            }
        }
    }
}

void Worker::generateInteractiveLoop() {
    std::string s;
    while (s != "gata") {
        generateCandidate();
        std::cin >> s;
    }
}

void Worker::generateLoop() {
    while (NGRAM_COUNT_LIMIT < this->ngrams.size()) {
        generateCandidate();
    }
}

void Worker::computeRepresentativeness(NgramEntry *current_ngram, int C) {
    float srep = 0;
    vector<string> ngram = current_ngram->getNgram();
    for (unsigned int i = 0; i < ngram.size(); i++)
    {
        float pmi_local = 0;
        string end_word = ".";
        vector <WordPosition> current_word_pos = wordPos[ngram[i]];
        vector <float> mutual_p(ngram.size()-i-1, 0);
        vector <float> mutual_c(ngram.size()-i-1, 0);
        for (unsigned int j = 0; j < current_word_pos.size(); j++)
        {
            bool over_C = false;
            int review_number = current_word_pos[j].review_nr;
            int current_pos = current_word_pos[j].word_nr - 1;
            while (current_pos > 0 &&
                    all_reviews[review_number][current_pos].compare(end_word) != 0)
            {
                for (unsigned int k = i+1; k < ngram.size(); k++)
                {
                    if (!over_C && all_reviews[review_number][current_pos].compare(ngram[k]) == 0)
                    {
                        mutual_c[k-i-1]++;
                        mutual_p[k-i-1]++;
                    }
                    else if (all_reviews[review_number][current_pos].compare(ngram[k]) == 0)
                        mutual_p[k-i-1]++;
                    current_pos--;
                    if (!over_C && current_word_pos[j].word_nr - current_pos > C)
                        over_C = true;
                }
            }
            current_pos = current_word_pos[j].word_nr + 1;
            over_C = false;
            while (current_pos < (int)all_reviews[review_number].size() &&
                    all_reviews[review_number][current_pos].compare(end_word) != 0)
            {
                for (unsigned int k = i+1; k < ngram.size(); k++)
                {
                    if (!over_C && all_reviews[review_number][current_pos].compare(ngram[k]) == 0)
                    {
                        mutual_c[k-i-1]++;
                        mutual_p[k-i-1]++;
                    }
                    else if (all_reviews[review_number][current_pos].compare(ngram[k]) == 0)
                        mutual_p[k-i-1]++;
                    current_pos++;
                    if (!over_C && current_pos - current_word_pos[j].word_nr > C)
                        over_C = true;
                }
            }
        }
        for (unsigned int k = i+1; k < ngram.size(); k++)
        {
            // It might also require the multiplication with sentences_count
            pmi_local += log2((float)(mutual_p[k-i-1] * mutual_c[k-i-1]) /
                        (float)((float)wordPos[ngram[k]].size() *
                            (float)wordPos[ngram[i]].size()));
        }
        srep += (pmi_local / (2 * C));
    }
    current_ngram->setRepresentativeness ((float)(srep / ngram.size()));
}

void Worker::printNgrams() {
    std::cout << ngrams.size() << std::endl;
    for (unsigned int i = 0; i < ngrams.size(); i++) {
        std::cout << *(ngrams[i]) << " ";
    }
    std::cout << std::endl;
}
