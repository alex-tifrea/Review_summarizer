#include "Worker.h"
#include <math.h>

const char *Worker::topicVect[] = {"food", "room", "staff", "noise", "price", "location"};

Worker::Worker(IO *_io) {
    this->io = new IO(_io);
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

    // Start aquiring part-of-speech information for all the words in the
    // reviews, using CoreNLP
    InterogateCoreNLP::init(this->wordInfo);
    InterogateCoreNLP::getPartOfSpeech(this->wordInfo);
//     InterogateCoreNLP::finalizePOS();

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
}

// Comparator used to get trim this->bigrams;
bool desc_comp(NgramEntry *a, NgramEntry *b) {
    double res = a->getReadability() - b->getReadability();
    // The representativeness score is more important, so give it a more
    // significant weight (x 10).
    res = res + 10 * (a->getRepresentativeness() - b->getRepresentativeness());
    return (res > 0);
}

void Worker::initBigrams() {
    // convert frequency to a std::vector<std::pair<std::string, int> >;
    // sort the resulting vector and then use either the first half or the first
    // MAX_BIGRAM_NUMBER (?) (depending on which is the least) to generate all the possible
    // bigrams. Keep a bigram only if it meets the readability and
    // representativeness requirements and if there is no other bigram similar to
    // the newly created one.

    vector <pair <string, WordInfo> > wordInfo_copy(this->wordInfo.begin(),
        this->wordInfo.end());

    sort (wordInfo_copy.begin(), wordInfo_copy.end(), comp_frequencies());
//     int remove_from = ((int)wordInfo_copy.size() >> 1) + 1;

    // This part removes _words_ until only MAX_WORDS_NUMBER are left; it also
    // keeps into account the fact that only words that occur for at least
    // MIN_WORD_OCCURENCES times are being considered.
    int remove_from = wordInfo_copy.size() - 1;
    if (remove_from > MAX_WORDS_NUMBER) {
        remove_from = MAX_WORDS_NUMBER;
    }

    for (unsigned int i = remove_from; i >= 0; i--) {
        if (wordInfo_copy[i].second.frequency >= MIN_WORD_OCCURENCES) {
            break;
        } else {
            remove_from--;
        }
    }

    // Erase the undesired words.
    wordInfo_copy.erase(wordInfo_copy.begin()+remove_from, wordInfo_copy.end());

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
    std::vector<double> allReadabilities =
        InterogateNGRAM::getJointProbabilities(allBigrams);
    allBigrams.clear();

    for (unsigned int i = 0; i < newBigrams.size(); i++)
    {
        // Set the readability score.
        newBigrams[i]->setReadability(allReadabilities[i]);
        pair<double, double> read_rep = newBigrams[i]->getScore();
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

    /*
    // TODO: interogate CoreNLP for sentiment.
    InterogateCoreNLP::getSentiment<NgramEntry*, std::vector>(this->bigrams);
    InterogateCoreNLP::finalizeSentiment();
    */

    // Select the top 500 bigrams, if there are more than 500 in this->bigrams.
    if (this->bigrams.size() > MAX_BIGRAM_NUMBER) {
        std::nth_element (this->bigrams.begin(),
                          this->bigrams.begin()+MAX_BIGRAM_NUMBER,
                          this->bigrams.end(),
                          desc_comp);
        this->bigrams.erase(this->bigrams.begin()+MAX_BIGRAM_NUMBER,
                            this->bigrams.end());
    }

    for (unsigned int i = 0; i < this->bigrams.size(); i++) {
        this->ngrams.push_back(this->bigrams[i]);

        this->bigrams_map.insert(std::make_pair(this->bigrams[i]->getNgram()[0],
                                              this->bigrams[i]));
    }
}

void Worker::generateCandidate() {
    // TODO: pop from the ngrams queue (let n be the popped n-gram) and find a
    // bigram that starts with the last word of n (this test is done by
    // NgramEntry::mergeNgrams); merge the two and push the newly created
    // (n+1)-gram at the and of the queue.
    double minimum_rep;
    int current_ngram_size;
    if (this->ngrams.size() > 0)
    {
        current_ngram_size = this->ngrams[0]->getNgramSize() + 1;
        minimum_rep =
            NgramEntry::rep_min_values[this->ngrams[0]->getNgramSize()-2];
    }
    else
    {
        current_ngram_size = 0;
        minimum_rep = SIGMA_REP;
    }
    std::vector<std::string> newNgrams;
    int loop_size = ngrams.size();
    std::vector<int> size_vector;
    for (int i = 0; i < loop_size; i++)
    {
        NgramEntry *curr_ngram = ngrams[i];

        auto matching_bigrams_range =
            this->bigrams_map.equal_range(curr_ngram->getNgram().back());

        std::unordered_multimap<std::string, NgramEntry*>::iterator iter;
        std::string tmpNgram;
        iter = matching_bigrams_range.first;
//         std::cout << "Pentru \"" << curr_ngram->getText() << "\" am BIGRAMELE astea:";
        int measure_size = 0;
        for (; iter != matching_bigrams_range.second; ++iter, measure_size++) {
            tmpNgram = curr_ngram->getText() + " " + iter->second->getNgram()[1];
            newNgrams.push_back(tmpNgram);
//             std::cout << "\"" << iter->second->getText() << "\" ";
        }
        size_vector.push_back(measure_size);
//         std::cout << std::endl;
    }

    if (newNgrams.size() == 0) {
        return;
    }

    std::vector<double> allReadabilities =
        InterogateNGRAM::getJointProbabilities(newNgrams);

    int current_poz = 0;
    int count = 0;
    double mean_rep = 0, mean_read = 0;
    int new_ngrams_start = loop_size - 1;
    std::cout << "incerc sa creez ingrame de dim " << new_ngrams_start << std::endl;
    int count_sim = 0;
    for (int k = 0; k < loop_size; k++, current_poz++)
    {
        NgramEntry *curr_ngram = ngrams.front();
        ngrams.pop_front();

        // Get the bigrams matching the given key.
        auto matching_bigrams_range =
            this->bigrams_map.equal_range(curr_ngram->getNgram().back());

        std::unordered_multimap<std::string, NgramEntry*>::iterator iter;
        iter = matching_bigrams_range.first;
        int i = current_poz;
        for (int j = 0; j < size_vector[k]; j++, i++)
        {
            NgramEntry *new_ngram =
                curr_ngram->mergeNgrams(iter->second, allReadabilities[i]);

            if (new_ngram != NULL)
                count_sim++;
            if (new_ngram != NULL && new_ngram->getRepresentativeness() > minimum_rep) {
                // Check if the newly created ngram is similar to any of the
                // other ngrams
                std::vector<NgramEntry*> similar_ngrams;
                for (int i = new_ngrams_start; i < (int)ngrams.size(); i++) {
                    if (ngrams[i]->computeSimilarity(new_ngram) > SIGMA_SIM) {
                        similar_ngrams.push_back(this->ngrams[i]);
                        this->ngrams.erase(this->ngrams.begin() + i);
                    }
                }

                if (similar_ngrams.size() == 0) {
                    // If new_ngram is similar with no other ngram in
                    // this->ngrams.
                    mean_rep += new_ngram->getRepresentativeness();
                    count ++;
                    mean_read += new_ngram->getReadability();
                    // Add the newly created (n+1)-gram to the deque
                    this->ngrams.push_back(new_ngram);
                    this->vect_best_ngrams.push_back(new_ngram);
                    this->printNgrams(log);
                } else {
                    // If new_ngram is similar to more than just one ngram,
                    // then we should drop new_ngram. We can assume that at any
                    // given time, all the ngrams in this->ngrams are not
                    // similar to one another. So new_ngram being similar to
                    // more ngrams means that new_ngram contains information
                    // that can be found in all these ngrams. Since we do not
                    // want to mix up more than one topic into a summary, we
                    // should drop the ngram that contains more heterogeneous
                    // pieces of information.
                    if (similar_ngrams.size() == 1) {
                        // If new_ngram is similar to just another ngram, than
                        // we decide to keep only the one with the highest
                        // readability score.
                        if (similar_ngrams[0]->getReadability() >
                                new_ngram->getReadability()) {
                            this->ngrams.push_back(similar_ngrams[0]);
                        } else {
                            this->ngrams.push_back(new_ngram);
                        }
                    }
                }
            }
            ++iter;
        }
        new_ngrams_start--;
    }
    std::cout << "am " << count_sim << "ngrame care sunt similare cu altele si "
        << loop_size - count_sim << "care nu sunt" << std::endl;

    cout << "mean rep value " << (double)(mean_rep / (double)count) << endl;
    cout << "read value " << (double)(mean_read / (double)count) << endl;
    cout << "there are " << ngrams.size() << " ngrams of size "
        << current_ngram_size << endl;
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

double Worker::computeRepresentativeness(NgramEntry *current_ngram) {
    double srep = 0;
    vector<string> ngram = current_ngram->getNgram();
    for (unsigned int i = 0; i < ngram.size()-1; i++)
    {
        double pmi_local = 0;
        char end_words[] = ".!?;";
        vector <WordPosition> current_word_pos = wordPos[ngram[i]];
        vector <double> mutual_p(ngram.size()-i-1, 0);
        vector <double> mutual_c(ngram.size()-i-1, 0);
        for (unsigned int j = 0; j < current_word_pos.size(); j++)
        {
            bool over_WindowSize = false;
            int review_number = current_word_pos[j].review_nr;
            int current_pos = current_word_pos[j].word_nr - 1;
            while (current_pos >= 0)
            {
                if (all_reviews[review_number][current_pos].size() == 1)
                {
                    char aux_end = all_reviews[review_number][current_pos][0];
                    if (strchr(end_words, aux_end) != NULL)
                        break;
                }
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
            while (current_pos < (int)all_reviews[review_number].size())
            {
                if (all_reviews[review_number][current_pos].size() == 1)
                {
                    char aux_end = all_reviews[review_number][current_pos][0];
                    if (strchr(end_words, aux_end) != NULL)
                        break;
                }
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
            //pmi_local += (double)(mutual_p[k-i-1] * mutual_c[k-i-1]) /
            //            (double)((double)wordPos[ngram[k]].size() *
            //                (double)wordPos[ngram[i]].size());
            double aux_pmi = (double)(mutual_p[k-i-1] * mutual_c[k-i-1] *
                    total_sentences_nr) / (double)(wordPos[ngram[k]].size() *
                        wordPos[ngram[i]].size());
            if (aux_pmi == 0 || ngram[i].compare(ngram[k]) == 0)
                return LOW_REP;
            pmi_local += log2(aux_pmi);
        }
        if (pmi_local == 0)
            return LOW_REP;
        srep += (pmi_local) / (2 * WINDOW_SIZE);
    }
    return (double)(srep / ngram.size());
}


// This functions computes the readability scores for all the permutations
// formed with the ngram's words and replaces the current ngram with the one
// that has the best score.
// @mode    it tells whether to keep gathering ngrams (i.e. if mode == GATHER)
// or to start processing the gathered ngrams by interogating Microsoft Ngram
// service (i.e. if mode == PROCESS). If mode is PROCESS, then we ignore the
// value of ne (it can even be NULL).
void Worker::replaceWithBestPermutation(NgramEntry *ne, int mode) {
    static std::vector<std::string> permutations_text;
    static std::vector<std::vector<std::string> > permutations_ngram;
    static std::vector<double> permutations_read;
    static std::vector<NgramEntry*> results;

    if (mode == GATHER) {
        std::vector<std::string> ngram = ne->getNgram();
        std::string text;

        results.push_back(ne);

        // Sort the ngram alphabetically before iterating through the permutations.
        std::sort(ngram.begin(), ngram.end());

        do {
            text = NgramEntry::ngram2string(ngram);
            permutations_text.push_back(text);
            permutations_ngram.push_back(ngram);
        } while (std::next_permutation(ngram.begin(), ngram.end()));
    }

    if (mode == PROCESS) {
        // The query itself.
        permutations_read = InterogateNGRAM::getJointProbabilities(permutations_text);

        for (unsigned int i = 0; i < results.size(); i++) {
            unsigned int curr_perm_end =
                Worker::factorial(results[i]->getNgram().size());

            auto max_pos = std::max_element(permutations_read.begin(),
                                            permutations_read.begin() + curr_perm_end);

            unsigned int index = max_pos - permutations_read.begin();

            // All other members of the ngram stay unchanged. We only need to
            // update the following:
            results[i]->setReadability(*max_pos);
            results[i]->setText(*(permutations_text.begin() + index));
            results[i]->setNgram(*(permutations_ngram.begin() + index));

            // Remove the elements coresponding to the processed result from
            // permutations_text, permutations_read, and permutations_read.
            permutations_text.erase(permutations_text.begin(),
                                    permutations_text.begin() + curr_perm_end);
            permutations_ngram.erase(permutations_ngram.begin(),
                                     permutations_ngram.begin() + curr_perm_end);
            permutations_read.erase(permutations_read.begin(),
                                    permutations_read.begin() + curr_perm_end);
        }
    }
}

void Worker::printNgrams(ostream &fout) {
    fout << ngrams.size() << std::endl;
    for (unsigned int i = 0; i < ngrams.size(); i++) {
        fout << *(ngrams[i]) << " ";
    }
}

void Worker::printBestNgrams(ostream &fout) {

    NgramEntry::DereferenceGreaterComparator comp;
    fout << "\nBest n-grams are: " << this->vect_best_ngrams.size() << "\n";
    sort(this->vect_best_ngrams.begin(), this->vect_best_ngrams.end(), comp);
    unsigned int count = 0; // the number of unique ngrams printed so far
    for (unsigned int i = 0; i < this->vect_best_ngrams.size(); i++) {
        // Check for similarity in the best selected ngrams.
        bool is_unique = true;
        for (unsigned int j = 0; j < i; j++) {
            if (this->vect_best_ngrams[i]->
                    computeSimilarity(this->vect_best_ngrams[j]) > SIGMA_SIM) {
                is_unique = false;
                break;
            }
        }
        if (is_unique) {
            this->replaceWithBestPermutation(this->vect_best_ngrams[i], GATHER);
            count++;
        }
        if (count >= MAX_BEST_NGRAMS) {
            break;
        }
    }

    // Interogate Microsoft Ngram for all the permutations of all the ngrams at
    // once.
    this->replaceWithBestPermutation(NULL, PROCESS);

    // Interogate CoreNLP for sentiment information for the candidate ngrams.
    std::vector<NgramEntry*> arg_ngrams;
    if (this->vect_best_ngrams.size() < MAX_CANDIDATES) {
        arg_ngrams = this->vect_best_ngrams;
    } else {
        arg_ngrams = std::vector<NgramEntry*>(this->vect_best_ngrams.begin(),
            this->vect_best_ngrams.begin()+MAX_CANDIDATES);
    }
    InterogateCoreNLP::getSentiment<NgramEntry*, std::vector>(arg_ngrams);
    //InterogateCoreNLP::finalizeSentiment();

    /*
     * XXX: Noticed that some useful ngrams are tagged as NEUTRAL, so I decided
     * to comment out these lines for now. If they will prove necessary, they
     * can always be uncommented.
    // Erase the ngrams that are not conveying a sentiment.
    for (auto it = this->vect_best_ngrams.begin();
            it != this->vect_best_ngrams.end();) {
        if ((*it)->getSentiment() == NEUTRAL) {
            it = this->vect_best_ngrams.erase(it);
        } else {
            ++it;
        }
    }
    */

    // Refine the ngrams (basically, trim the redundant words at the beginning
    // and at the end of a ngram).
    for (unsigned int i = 0; i < count; i++) {
        this->vect_best_ngrams[i]->refineNgram();
    }

    // It is possible that after all these changes, some ngrams became similar
    // with others. We should check for that.
    std::vector<NgramEntry*> candidates =
        std::vector<NgramEntry*>(this->vect_best_ngrams.begin(),
                                 this->vect_best_ngrams.begin() + count);
    for (auto it = candidates.begin(); it != candidates.end();) {
        bool is_unique = true;
        unsigned int i = it - candidates.begin();
        for (unsigned int j = 0; j < i; j++) {
            if (candidates[i]->computeSimilarity(candidates[j]) > SIGMA_SIM) {
                is_unique = false;
                break;
            }
        }
        if (is_unique) {
            ++it;
        } else {
            it = candidates.erase(it);
        }
    }

    // Print the best ngrams.
    /*
    std::string bin_file = "vectors.bin";
    Topics::Init(bin_file);*/
    for (unsigned int i = 0; i < candidates.size(); i++) {
        /*
        pair<int, float> topicNumber = Topics::getTopic(candidates[i]->getText());
        string topic;
        if (topicNumber.first >= 0)
        {
            topic = topicVect[topicNumber.first];
        }
        else
        {
            topic = "none";
        }*/
        fout << *(candidates[i]) << std::endl;
    }
    fout << std::endl;
    Topics::Finalize();
}

WordInfo Worker::getWordInfo(std::string word) {
    return this->wordInfo[word];
}
