#include "Worker.h"
#include <math.h>

Worker::Worker(IO *_io) {
    io = new IO(_io);
    log.open("log.file", std::ofstream::out);
}

Worker::~Worker() {
    log.close();
}

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
    this->total_sentences_nr = io->get_sentences_nr();
    unordered_map<string, vector<WordPosition> >::iterator it;
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
    std::unordered_map<std::string, WordInfo>::iterator iter;
    for (iter = wordInfo.begin(); iter != wordInfo.end(); ++iter) {
        words << iter->first << " " << iter->second.frequency << " " <<
                 iter->second.partOfSpeech << std::endl;
    }
    words.close();

    current_review = 0;
    sentences_count= 5;
}

// Comparator used to get trim this->bigrams;
bool desc_comp(NgramEntry *a, NgramEntry *b) {
    float res = a->getReadability() - b->getReadability();
    // The representativeness score is more important, so give it a more
    // significant weight (x 10).
    res = res + 10 * (a->getRepresentativeness() - b->getRepresentativeness());
    return (res > 0);
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
    std::vector<std::string> allBigrams;
    for (int i = 0; i < (int)wordInfo_copy.size(); i++)
    {
        for (int j = 0; j < (int)wordInfo_copy.size(); j++)
        {
            if (i != j)
            {
                vector <string> bigram_text;
                bigram_text.push_back(wordInfo_copy[i].first);
                bigram_text.push_back(wordInfo_copy[j].first);
                NgramEntry *aux_ngram = new NgramEntry(bigram_text, this);
                // Keep only the bigrams that have a rep val higher than SIGMA_REP
                if (aux_ngram->getRepresentativeness() > SIGMA_REP)
                {
                    newBigrams.push_back(aux_ngram);
                    std::string concat_bigram = bigram_text[0] + " " + bigram_text[1];
                    allBigrams.push_back(concat_bigram);
                }
            }
        }
    }

    // We call getJointProbabilities for all the bigrams at once.
    std::cout << "Aici fac REQUEST cu lungimea " << allBigrams.size() << std::endl;
    std::vector<float> allReadabilities =
        InterogateNGRAM::getJointProbabilities(allBigrams);
    allBigrams.clear();

    for (unsigned int i = 0; i < newBigrams.size(); i++)
    {
        // Set the readability score.
        newBigrams[i]->setReadability(allReadabilities[i]);
        pair<float, float> read_rep = newBigrams[i]->getScore();
        // Check the readability and the representativeness score.
        if (read_rep.first >= SIGMA_READ && read_rep.second >= SIGMA_REP)
        {
            this->bigrams.push_back(newBigrams[i]);
            std::string concat_bigram = newBigrams[i]->getText()[0] +
                                        " " +
                                        newBigrams[i]->getText()[1];
            allBigrams.push_back(concat_bigram);
        }
    }

    // TODO: interogate CoreNLP for sentiment.
    InterogateCoreNLP::getSentiment<NgramEntry*, std::vector>(this->bigrams);

    // Select the top 500 bigrams, if there are more than 500 in this->bigrams.
    if (this->bigrams.size() > MIN_BIGRAM_NUMBER) {
        std::nth_element (this->bigrams.begin(),
                          this->bigrams.begin()+MIN_BIGRAM_NUMBER,
                          this->bigrams.end(),
                          desc_comp);
        this->bigrams.erase(this->bigrams.begin()+MIN_BIGRAM_NUMBER,
                            this->bigrams.end());
    }

    for (unsigned int i = 0; i < this->bigrams.size(); i++) {
        this->ngrams.push_back(this->bigrams[i]);

        this->bigrams_t.insert(std::make_pair(this->bigrams[i]->getNgram()[0],
                                              this->bigrams[i]));
    }
}

void Worker::generateCandidate() {
    // TODO: pop from the ngrams queue (let n be the popped n-gram) and find a
    // bigram that starts with the last word of n (this test is done by
    // NgramEntry::mergeNgrams); merge the two and push the newly created
    // (n+1)-gram at the and of the queue.
    NgramEntry *curr_ngram = ngrams.front();
    ngrams.pop_front();

    auto matching_bigrams_range =
        this->bigrams_t.equal_range(curr_ngram->getNgram().back());

    std::vector<std::string> newNgrams;
    std::unordered_multimap<std::string, NgramEntry*>::iterator iter;
    std::string tmpNgram;
    iter = matching_bigrams_range.first;
    std::cout << "Pentru \"" << curr_ngram->getText() << "\" am BIGRAMELE astea:";
    for (; iter != matching_bigrams_range.second; ++iter) {
        tmpNgram = curr_ngram->getText() + " " + iter->second->getNgram()[1];
        newNgrams.push_back(tmpNgram);
        std::cout << "\"" << iter->second->getText() << "\" ";
    }
    std::cout << std::endl;

    if (newNgrams.size() == 0) {
        return;
    }

    std::vector<float> allReadabilities =
        InterogateNGRAM::getJointProbabilities(newNgrams);

    iter = matching_bigrams_range.first;
    for (unsigned int i = 0; i < allReadabilities.size(); i++) {
        std::cout << "About to merge \"" << curr_ngram->getText() << "\" with \"" <<
                     iter->second->getText() << "\"" << std::endl;
        std::cout << "\"" << newNgrams[i] << "\" are scorul " << allReadabilities[i] << std::endl;
        NgramEntry *new_ngram =
            curr_ngram->mergeNgrams(iter->second, allReadabilities[i]);

        if (new_ngram != NULL) {
            // Check if the newly created ngram is similar to any of the
            // other ngrams
            bool is_unique = true;
            // TODO: this might need to be changed. I think it may slow us
            // down
            for (unsigned int i = 0; i < ngrams.size(); i++) {
                if (ngrams[i]->computeSimilarity(new_ngram) > SIGMA_SIM) {
                    // TODO: pastreaza ngrama cu cele mai bune scoruri
                    is_unique = false;
                    break;
                }
            }

            if (is_unique) {
                // Add the newly created (n+1)-gram to the deque
                ngrams.push_back(new_ngram);
                this->printNgrams(log);
            }
        }
        ++iter;
    }

    /*
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
                        // TODO: pastreaza ngrama cu cele mai bune scoruri
                        is_unique = false;
                        break;
                    }
                }

                if (is_unique) {
                    // Add the newly created (n+1)-gram to the deque
                    ngrams.push_back(new_ngram);
                    this->printNgrams(log);
                }
            }
        }
    }
*/
}

void Worker::generateInteractiveLoop() {
    std::string s;
    this->printNgrams(log);
    while (s != "gata") {
        generateCandidate();
        std::cin >> s;
    }
}

void Worker::generateLoop() {
    this->printNgrams(log);
    while (NGRAM_COUNT_LIMIT < this->ngrams.size()) {
        generateCandidate();
    }
}

float Worker::computeRepresentativeness(NgramEntry *current_ngram) {
    float srep = 0;
    vector<string> ngram = current_ngram->getNgram();
    cout << "Ngram::";
    for (unsigned int i = 0; i < ngram.size(); i++)
        cout << ngram[i] << " ";
    cout << endl;
    for (unsigned int i = 0; i < ngram.size()-1; i++)
    {
        float pmi_local = 0;
        string end_word = ".";
        vector <WordPosition> current_word_pos = wordPos[ngram[i]];
        vector <float> mutual_p(ngram.size()-i-1, 0);
        vector <float> mutual_c(ngram.size()-i-1, 0);
        for (unsigned int j = 0; j < current_word_pos.size(); j++)
        {
            bool over_WindowSize = false;
            int review_number = current_word_pos[j].review_nr;
            int current_pos = current_word_pos[j].word_nr - 1;
            while (current_pos >= 0 &&
                    all_reviews[review_number][current_pos].compare(end_word) != 0)
            {
                for (unsigned int k = i+1; k < ngram.size(); k++)
                {
                    if (current_pos < 0)
                        break;
                    if (!over_WindowSize && all_reviews[review_number][current_pos].compare(ngram[k]) == 0)
                    {
                        mutual_c[k-i-1]++;
                        mutual_p[k-i-1]++;
                        if (!over_WindowSize && current_word_pos[j].word_nr - (current_pos-1) > WINDOW_SIZE)
                            over_WindowSize = true;
                        break;
                    }
                    else if (all_reviews[review_number][current_pos].compare(ngram[k]) == 0)
                    {
                        mutual_p[k-i-1]++;
                        if (!over_WindowSize && current_word_pos[j].word_nr - (current_pos-1) > WINDOW_SIZE)
                            over_WindowSize = true;
                        break;
                    }
                }
                current_pos--;
            }
            current_pos = current_word_pos[j].word_nr + 1;
            over_WindowSize = false;
            while ((current_pos < (int)all_reviews[review_number].size()) &&
                    all_reviews[review_number][current_pos].compare(end_word) != 0)
            {
                for (unsigned int k = i+1; k < ngram.size(); k++)
                {
                    if (current_pos >= (int)all_reviews[review_number].size())
                        break;
                    if (!over_WindowSize && all_reviews[review_number][current_pos].compare(ngram[k]) == 0)
                    {
                        mutual_c[k-i-1]++;
                        mutual_p[k-i-1]++;
                        if (!over_WindowSize && (current_pos+1) - current_word_pos[j].word_nr > WINDOW_SIZE)
                            over_WindowSize = true;
                        break;

                    }
                    else if (all_reviews[review_number][current_pos].compare(ngram[k]) == 0)
                    {
                        mutual_p[k-i-1]++;
                        if (!over_WindowSize && (current_pos+1) - current_word_pos[j].word_nr > WINDOW_SIZE)
                            over_WindowSize = true;
                        break;
                    }
                }
                current_pos++;
            }
        }
        for (unsigned int k = i+1; k < ngram.size(); k++)
        {
            // It might also require the multiplication with sentences_count
            //pmi_local += (float)(mutual_p[k-i-1] * mutual_c[k-i-1]) /
            //            (float)((float)wordPos[ngram[k]].size() *
            //                (float)wordPos[ngram[i]].size());
            pmi_local += (float)(mutual_p[k-i-1] * mutual_c[k-i-1] *
                    total_sentences_nr) / (float)(wordPos[ngram[k]].size() *
                        wordPos[ngram[i]].size());
        }
        if (pmi_local == 0)
            return LOW_REP;
        srep += (log2(pmi_local) / (2 * WINDOW_SIZE));
    }
    return (float)(srep / ngram.size());
}

void Worker::printNgrams(ostream &fout) {
    fout << ngrams.size() << std::endl;
    for (unsigned int i = 0; i < ngrams.size(); i++) {
        fout << *(ngrams[i]) << " ";
    }
    fout << std::endl;
}

WordInfo Worker::getWordInfo(std::string word) {
    return this->wordInfo[word];
}
