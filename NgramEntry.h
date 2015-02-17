#ifndef __NGRAMENTRY_H__
#define __NGRAMENTRY_H__

#include <string>
#include <vector>

class NgramEntry {
private:
  std::vector<std::string> ngram;
  float readability, representativeness;

public:
  NgramEntry(std::vector<std::string>);
  ~NgramEntry();

  // Returns true is merging is possible, and false otherwise.
  bool mergeNgrams(std::vector<std::string>);

  // Computes the readability and representativeness scores.
  void computeScores();
};

#endif // __NGRAMENTRY_H__
