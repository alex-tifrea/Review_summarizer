#ifndef __IO_H__
#define __IO_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

struct WordInfo_t {
    unsigned int frequency;
    std::string partOfSpeech;
};
typedef struct WordInfo_t WordInfo;

struct WordPosition{
    int review_nr, word_nr;
    WordPosition(int a, int b)
    {
        review_nr = a;
        word_nr = b;
    }
};

class IO {
private:
    std::string in_name, out_name;
    std::fstream in;
    std::fstream out;

public:
    IO(std::string _in_name, std::string _out_name);
    IO(IO*);

    ~IO();

    /*
     * Read reviews from file.
     */
    void readReviews(std::unordered_map<std::string, WordInfo> &wordInfo,
                     std::vector<std::vector<std::string> > &reviews);

    void readReviews(std::unordered_map<std::string, WordInfo> &wordInfo,
                     std::vector<std::vector<std::string> > &reviews,
                     std::unordered_map<std::string, std::vector<WordPosition> > &word_positions);
};

#endif // __IO_H__
