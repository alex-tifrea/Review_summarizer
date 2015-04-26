#ifndef __TOPICS_H__
#define __TOPICS_H__

#include <vector>
#include <string>

/*
 * This class contains a hardcoded matrix of words,
 * each line containing words from a unique topic
 *
 * getTopic method returns the id of the topic which
 * is closest to the input n-gram
 */
class Topics
{
public:
    //uses a file with the binary output of the
    //word2vec training tool
    static void Init(std::string);

    static void Finalize();

    //get the id of the topic
    static int getTopic(std::string );

public: //for testing purposes
//private:
    static float getDistance(const std::string, const std::string);
    static float getMultipleDistances(std::string, std::string);


private:
    //matrix with hardcoded words
    //each line contains words from a unique topic
    static std::vector<std::vector<std::string> > topics;

};

#endif // __TOPICS_H__
