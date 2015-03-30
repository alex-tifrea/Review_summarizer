#include <sstream>
#include "IO.h"

IO::IO(std::string _in_name, std::string _out_name) {
    this->in_name = _in_name;
    this->out_name = _out_name;
    in.open (this->in_name.c_str(), std::fstream::in);
    out.open (this->out_name.c_str(), std::fstream::out);
    this->total_sentences_nr = 0;
}


IO::IO(IO *_io) {
    this->in_name = _io->in_name;
    this->out_name = _io->out_name;
    in.open (this->in_name.c_str(), std::fstream::in);
    out.open (this->out_name.c_str(), std::fstream::out);
    this->total_sentences_nr = 0;
}

IO::~IO() {
    in.close();
    out.close();
}

void IO::readReviews(std::unordered_map<std::string, WordInfo> &wordInfo,
                     std::vector<std::vector<std::string> > &reviews) {
    int nr_reviews, i = 0;
    std::string line;
    this->in >> nr_reviews;
    this->total_sentences_nr = 0;
    while (i < nr_reviews && std::getline(this->in, line)) {
        // Separator between two consecutive reviews
        if (line.compare("-----") == 0) {
            i++;
            continue;
        }

        // Update hashtable
        std::istringstream iss(line);
        std::string word;
        std::vector<std::string> newWords;

        while (iss >> word) {
            // Check if it is a word (it can't be a "," or ":" or something
            // else because we deleted those characters when formatting the
            // input). However, it can still be ".", "?", "!" or "-". If it is
            // ".", "?" or "!", then put it in the reviews vector.
            if (word.compare("-") != 0 &&
                word.compare(".") != 0 &&
                word.compare("?") != 0 &&
                word.compare("!") != 0) {
                wordInfo[word].frequency++;
                total_sentences_nr++;
            }
            if (word.compare("-") != 0) {
                newWords.push_back(word);
            }
        }

        // Update the array
        // If there are any empty reviews this will be a problem
        if (i == (int)reviews.size())
            reviews.push_back(newWords);
        else
            reviews[i].insert(reviews[i].end(), newWords.begin(), newWords.end());
    }
}

void IO::readReviews(std::unordered_map<std::string, WordInfo> &wordInfo,
                     std::vector<std::vector<std::string> > &reviews,
                     std::unordered_map<std::string, std::vector<WordPosition> > &word_positions) {
    int nr_reviews, i = 0, k = 0;
    this->in >> nr_reviews;
    std::string line;
    while (i < nr_reviews && std::getline(this->in, line)) {
        // Separator between two consecutive reviews
        if (line.compare("-----") == 0) {
            i++;
            k = 0;
            continue;
        }

        // Update hashtable
        std::istringstream iss(line);
        std::string word;
        std::vector<std::string> newWords;

        while (iss >> word) {
            // Check if it is a word (it can't be a "," or ":" or something
            // else because we deleted those characters when formatting the
            // input). However, it can still be ".", "?", "!" or "-". If it is
            // ".", "?" or "!", then put it in the reviews vector.
            if (word.compare("-") != 0 &&
                word.compare(".") != 0 &&
                word.compare("?") != 0 &&
                word.compare("!") != 0) {
                wordInfo[word].frequency++;
                total_sentences_nr++;
            }
            if (word.compare("-") != 0) {
                newWords.push_back(word);
                word_positions[word].push_back(WordPosition(i, k));
                k++;
            }
        }

        // Update the array
        // If there are any empty reviews this will be a problem
        if (i == (int)reviews.size())
            reviews.push_back(newWords);
        else
            reviews[i].insert(reviews[i].end(), newWords.begin(), newWords.end());
    }
}
