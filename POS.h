#ifndef __POS_H__
#define __POS_H__
#include <string>
#include <vector>

#define CC 0
#define CD 1
#define DT 2
#define EX 3
#define FW 4
#define IN 5
#define JJ 6
#define JJR 7
#define JJS 8
#define LS 9
#define MD 10
#define NN 11
#define NNS 12
#define NNP 13
#define NNPS 14
#define PDT 15
// #define POS 16
#define PRP 17
#define PRP$ 18
#define RB 19
#define RBR 20
#define RBS 21
#define RP 22
#define SYM 23
#define TO 24
#define UH 25
#define VB 26
#define VBD 27
#define VBG 28
#define VBN 29
#define VBP 30
#define VBZ 31
#define WDT 32
#define WP 33
#define WP$ 34
#define WRB 35


// Contains definitions for all the possible part-of-speech tags.
struct POS_t {
    static const std::vector<std::string> name;

    static bool isAdjective(unsigned int pos_code);
    static bool isNoun(unsigned int pos_code);
    static bool isAdverb(unsigned int pos_code);
};
#endif // __POS_H__
