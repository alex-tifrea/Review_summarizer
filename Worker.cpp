#include "Worker.h"

Worker::Worker(IO _io) {
  io = new IO(_io);
}

Worker::~Worker() {}

Worker::init() {
  // TODO: use IO::readReviews to populate frequency and original_review;
}

Worker::initBigrams() {
  // TODO: convert frequency to a std::vector<std::pair<std::string, int> >;
  // sort the resulting vector and then use either the first half or the first
  // 500 (?) (depending on which is the least) to generate all the possible
  // bigrams. Keep a bigram only if it meets the readability and
  // representativeness requirements and if there is no other bigram similar to
  // the newly created one.

  // TODO: when done with ^, copy the | bigrams | vector into the | ngrams |
  // vector (because initially the n-grams are the bigrams);
}

Worker::generateCandidate() {
  // TODO: pop from the ngrams queue (let n be the popped n-gram) and find a
  // bigram that starts with the last word of n (this test is done by
  // NgramEntry::mergeNgrams); merge the two and push the newly created
  // (n+1)-gram at the and of the queue.
}
