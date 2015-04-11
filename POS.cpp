#include "POS.h"

const std::vector<std::string> POS_t::name = {
        "CC", "CD", "DT", "EX", "FW", "IN", "JJ", "JJR", "JJS", "LS", "MD",
        "NN", "NNS", "NNP", "NNPS", "PDT", "POS", "PRP", "PRP$", "RB", "RBR",
        "RBS", "RP", "SYM", "TO", "UH", "VB", "VBD", "VBG", "VBN", "VBP",
        "VBZ", "WDT", "WP", "WP$", "WRB"
    };

bool POS_t::isAdjective(unsigned int pos_code) {
    return pos_code == JJ || pos_code == JJR || pos_code == JJS;
}

bool POS_t::isNoun(unsigned int pos_code) {
    return pos_code == NN || pos_code == NNS || pos_code == NNP ||
           pos_code == NNPS;
}

bool POS_t::isAdverb(unsigned int pos_code) {
    return pos_code == RB || pos_code == RBR || pos_code == RBS;
}

bool POS_t::isToBeRemoved(unsigned int pos_code) {
    return pos_code == IN || pos_code == FW || pos_code == UH || pos_code == DT ||
           pos_code == CC || pos_code == PRP || pos_code == WDT || pos_code == PRP$ ||
           pos_code == TO || pos_code == CD;
}
