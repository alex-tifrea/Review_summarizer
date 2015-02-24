#include <sstream>
#include "IO.h"
#include <string>

IO::IO(std::string _in_name, std::string _out_name) {
  this->in_name = _in_name;
  this->out_name = _out_name;
  in.open (this->in_name.c_str(), std::fstream::in);
  out.open (this->out_name.c_str(), std::fstream::out);
}


IO::IO(IO *_io) {
  this->in_name = _io->in_name;
  this->out_name = _io->out_name;
  in.open (this->in_name.c_str(), std::fstream::in);
  out.open (this->out_name.c_str(), std::fstream::out);
}

void IO::readReviews(std::map<std::string, int> &frequency,
                     std::vector<std::vector<std::string> > reviews) {
  int nr_reviews, i = 0;
  this->in >> nr_reviews;
  std::string line;
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
      frequency[word]++;
      newWords.push_back(word);
    }

    // Update the array
    // If there are any empty reviews this will be a problem
    if (i == (int)reviews.size())
      reviews.push_back(newWords);
    else
      reviews[i].insert(reviews[i].end(), newWords.begin(), newWords.end());
  }
}
