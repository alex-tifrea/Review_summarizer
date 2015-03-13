#ifndef __IO_H__
#define __IO_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

struct word_pos{
    int review_nr, word_nr;
    word_pos(int a, int b)
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
    void readReviews(std::map<std::string, int> &frequency,
                     std::vector<std::vector<std::string> > &reviews);

    void readReviews(std::map<std::string, int> &frequency,
                     std::vector<std::vector<std::string> > &reviews,
                     std::map<std::string, std::vector<word_pos> > &word_positions);
};

#endif // __IO_H__
