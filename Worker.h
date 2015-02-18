#ifndef __WORKER_H__
#define __WORKER_H__

#include <string>
#include <vector>
#include "NgramEntry.h"
#include "IO.h"
#include <map>
#include <queue>

// XXX: vezi ca metodele deocamdata nu primesc niciun argument; nu m-am gandit
// foarte bine ce ar trebui sa primeasca; feel free to make any changes :)

class Worker {
private:
  // Holds the number of occurences for each word.
  std::map<std::string, int> frequency;

  // The n-grams at a given time. Initially they are all bigrams. At each step,
  // we try to merge an n-gram with a bigram to obtain an (n+1)-gram.
  std::queue<NgramEntry*> ngrams;

  // The bigrams. They are used as seed in the generation of the n-grams and are
  // also used at each step in the generation process (a bigram will be
  // appended at the end of a given n-gram at each step).
  std::vector<NgramEntry*> bigrams;

  // This is the initial review, the one that needs to be summerized. Each
  // element of the vector is a word of the original review (or a '\n' if
  // necessary). We need this to compute the representativeness of a given
  // n-gram.
  std::vector<string> original_review;

  // Reads the input and writes back the output.
  IO *io;

public:
  Worker(IO);
  ~Worker();

  // Initializes the members of the | Worker | class.
  void init();

  // Converts the unigrams into bigrams (the seed for the generation of the
  // summary).
  void initBigrams();

  // Used to traverse the solution space in a DFS manner.
  void generateCandidate();
};

#endif // __WORKER_H__
