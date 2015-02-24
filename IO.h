#ifndef __IO_H__
#define __IO_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

class IO {
private:
  std::string in_name, out_name;
  std::fstream in;
  std::fstream out;

public:
  IO(std::string _in_name, std::string _out_name);
  IO(IO*);

  ~IO() {}

  /*
   * Read reviews from file.
   */
  void readReviews(std::map<std::string, int> &frequency,
                   std::vector<std::vector<std::string> > reviews);
};

#endif // __IO_H__
