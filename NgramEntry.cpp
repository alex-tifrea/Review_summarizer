#include "NgramEntry.h"
#include <string>
#include <vector>

NgramEntry::NgramEntry(std::vector<std::string> bigram) {
  this->ngram = bigram;
  this->computeScores();
}

NgramEntry::~NgramEntry() {}

bool NgramEntry::mergeNgrams(std::vector<std::string> bigram) {
  // Compare the last word of the n-gram with the first word of the bigram.
  if (this->ngram[this->ngram.size()-1].compare(bigram[0]) != 0) {
    return false;
  }
  // Check if the n-gram is a mirror of the bigram.
  if (this->ngram.size() == 2 &&
      this->ngram[0].compare(bigram[1]) == 0 &&
      this->ngram[1].compare(bigram[0]) == 0) {
    return false;
  }
  this->ngram.push_back(bigram[1]);
  this->computeScores();
  return true;
}

void NgramEntry::computeScores() {
  // TODO: complete this function
  this->readability = this->representativeness = 0;
}

std::pair<float, float> NgramEntry::getScore ()
{
  return std::make_pair(this->readability, this->representativeness);
}
