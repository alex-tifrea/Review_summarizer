#include "Worker.h"
#include <math.h>

Worker::Worker(IO *_io) {
    this->io = new IO(_io);
    this->rep_min_values =  new float[3];
    this->rep_min_values[0] = 0.35;
    this->rep_min_values[1] = 0.52;
    this->rep_min_values[2] = 0.7;
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
//     InterogateCoreNLP::finalize();

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
    float res = a->getReadability() - b->getReadability();
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

    /*
    if (remove_from < MAX_WORDS_NUMBER)
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
        wordInfo_copy.erase(wordInfo_copy.begin()+MAX_WORDS_NUMBER, wordInfo_copy.end());
    }
    */

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

    /*
    // TODO: interogate CoreNLP for sentiment.
    InterogateCoreNLP::getSentiment<NgramEntry*, std::vector>(this->bigrams);
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
    float minimum_rep;
    if (this->ngrams.size() > 0)
        minimum_rep = rep_min_values[this->ngrams[0]->getNgramSize()-2];
    else
        minimum_rep = SIGMA_REP;

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
        std::cout << "Pentru \"" << curr_ngram->getText() << "\" am BIGRAMELE astea:";
        int measure_size = 0;
        for (; iter != matching_bigrams_range.second; ++iter, measure_size++) {
            tmpNgram = curr_ngram->getText() + " " + iter->second->getNgram()[1];
            newNgrams.push_back(tmpNgram);
            std::cout << "\"" << iter->second->getText() << "\" ";
        }
        size_vector.push_back(measure_size);
        std::cout << std::endl;
    }

    if (newNgrams.size() == 0) {
        return;
    }

    std::vector<float> allReadabilities =
        InterogateNGRAM::getJointProbabilities(newNgrams);

    int current_poz = 0;
    int count = 0;
    float mean_rep = 0;
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
//             std::cout << "About to merge \"" << curr_ngram->getText() << "\" with \"" <<
//                          iter->second->getText() << "\"" << std::endl;
//             std::cout << "\"" << newNgrams[i] << "\" are scorul " << allReadabilities[i] << std::endl;
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
                        // TODO: Keep the ngram with the highest scores.
//                         if (new_ngram < this->ngrams[i]) {
                            is_unique = false;
//                         } else {
//                             // Remove the old ngram.
//                             this->ngrams.erase(this->ngrams.begin() + i);
//                         }
                        break;
                    }
                }

                // TODO: poate verifica si similaritatea cu ce avem in
                // this->vect_best_ngrams

                if (is_unique && new_ngram->getRepresentativeness() > minimum_rep) {
                    mean_rep += new_ngram->getRepresentativeness();
                    count ++;
                    // Add the newly created (n+1)-gram to the deque
                    this->ngrams.push_back(new_ngram);
                    this->vect_best_ngrams.push_back(new_ngram);
//                     this->best_ngrams.push(new_ngram);
//                     if (this->best_ngrams.size() > MAX_BEST_NGRAMS) {
//                         this->best_ngrams.pop();
//                     }
                    this->printNgrams(log);
                }
            }
            ++iter;
        }
    }

    cout << "mean rep value " << (float)(mean_rep / (float)count) << endl;

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
    //cout << "Ngram::";
    //for (unsigned int i = 0; i < ngram.size(); i++)
    //    cout << ngram[i] << " ";
    //cout << endl;
    for (unsigned int i = 0; i < ngram.size()-1; i++)
    {
        float pmi_local = 0;
        char end_words[] = ".!?;";
        vector <WordPosition> current_word_pos = wordPos[ngram[i]];
        vector <float> mutual_p(ngram.size()-i-1, 0);
        vector <float> mutual_c(ngram.size()-i-1, 0);
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
            //pmi_local += (float)(mutual_p[k-i-1] * mutual_c[k-i-1]) /
            //            (float)((float)wordPos[ngram[k]].size() *
            //                (float)wordPos[ngram[i]].size());
            float aux_pmi = (float)(mutual_p[k-i-1] * mutual_c[k-i-1] *
                    total_sentences_nr) / (float)(wordPos[ngram[k]].size() *
                        wordPos[ngram[i]].size());
            if (aux_pmi == 0 || ngram[i].compare(ngram[k]) == 0)
                return LOW_REP;
            pmi_local += log2(aux_pmi);
        }
        if (pmi_local == 0)
            return LOW_REP;
        srep += (pmi_local) / (2 * WINDOW_SIZE);
    }
    return (float)(srep / ngram.size());
}

void Worker::printNgrams(ostream &fout) {
    fout << ngrams.size() << std::endl;
    for (unsigned int i = 0; i < ngrams.size(); i++) {
        fout << *(ngrams[i]) << " ";
    }
}

void Worker::printBestNgrams(ostream &fout) {
//     fout << this->best_ngrams.size() << " ";
//     while (this->best_ngrams.size() != 0) {
//         fout << *(this->best_ngrams.top()) << " ";
//         this->best_ngrams.pop();
//     }

    NgramEntry::DereferenceGreaterComparator comp;
    fout << "\nBest n-grams are: " << this->vect_best_ngrams.size() << "\n";
    sort(this->vect_best_ngrams.begin(), this->vect_best_ngrams.end(), comp);
    for (unsigned int i = 0;
         i < MAX_BEST_NGRAMS && i < this->vect_best_ngrams.size(); i++) {
        fout << *(this->vect_best_ngrams[i]) << std::endl;
    }
    fout << std::endl;
}

WordInfo Worker::getWordInfo(std::string word) {
    return this->wordInfo[word];
}
