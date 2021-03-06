#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "InterogateCoreNLP.h"

#define CORENLP_POS_INPUT         "corenlpPOS.in"
#define CORENLP_POS_OUTPUT        "corenlpPOS.out"
#define CORENLP_SENTIMENT_INPUT   "corenlpSentiment.in"
#define CORENLP_SENTIMENT_OUTPUT  "corenlpSentiment.out"

void InterogateCoreNLP::init(std::unordered_map<std::string, WordInfo> &wordInfo) {
    std::fstream nlp_in;
    nlp_in.open(CORENLP_POS_INPUT, std::ofstream::out);
    std::unordered_map<std::string, WordInfo>::iterator iter;
    for (iter = wordInfo.begin(); iter != wordInfo.end(); ++iter) {
        nlp_in << iter->first << std::endl;
    }
    nlp_in.close();
}

void InterogateCoreNLP::getPartOfSpeech(std::unordered_map<std::string,WordInfo> &wordInfo) {
    // Call CoreNLP
    char command[1000];
    sprintf(command, "java -cp \"$CORENLP_PATH/*\" -Xmx\"$CORENLP_MEMORY\" edu.stanford.nlp.pipeline.StanfordCoreNLP -annotators tokenize,ssplit,pos -file %s",
            CORENLP_POS_INPUT);
    system(command);

    // Parse the XML created by CoreNLP
    sprintf(command,
            "python coreNLP_output_parser.py --pos %s.xml %s",
            CORENLP_POS_INPUT,
            CORENLP_POS_OUTPUT);
    system(command);

    // Add the part-of-speech information to the wordInfo structure
    std::fstream nlp_out;
    nlp_out.open(CORENLP_POS_OUTPUT, std::fstream::in);
    std::string word, part_of_speech;
    while (nlp_out >> word >> part_of_speech) {
        wordInfo[word].partOfSpeech = part_of_speech;
    }
    nlp_out.close();
}

template<typename T, template <typename, typename> class Container>
void InterogateCoreNLP::getSentiment(Container<T, std::allocator<T> > &ngrams) {
    std::fstream nlp_in;
    nlp_in.open(CORENLP_SENTIMENT_INPUT, std::fstream::out);
    for (unsigned int i = 0; i < ngrams.size(); i++) {
        nlp_in << ngrams[i]->getText() << std::endl;
    }
    nlp_in.close();

    char command[300];
    // Call CoreNLP
    sprintf(command, "java -cp \"$CORENLP_PATH/*\" -Xmx\"$CORENLP_MEMORY\" edu.stanford.nlp.pipeline.StanfordCoreNLP -annotators tokenize,ssplit,parse,sentiment -ssplit.eolonly -file %s",
            CORENLP_SENTIMENT_INPUT);
    system(command);

    // Parse the XML created by CoreNLP
    sprintf(command,
            "python coreNLP_output_parser.py --sentiment %s.xml %s",
            CORENLP_SENTIMENT_INPUT,
            CORENLP_SENTIMENT_OUTPUT);
    system(command);

    // Add the sentiment information to the NgramEntry structure
    std::fstream nlp_out;
    nlp_out.open(CORENLP_SENTIMENT_OUTPUT, std::fstream::in);
    // XXX: maybe provide a way to check if we have the coresponding sentiment
    // or not
    int i = 0;
    int temp_sent;
    while (nlp_out >> temp_sent) {
        ngrams[i]->setSentiment(static_cast<Sentiment>(temp_sent));
        i++;
    }
    nlp_out.close();
}

void InterogateCoreNLP::finalizePOS() {
    remove(CORENLP_POS_INPUT);
    std::string temp = CORENLP_POS_INPUT;
    temp += ".xml";
    remove(temp.c_str());
    remove(CORENLP_POS_OUTPUT);
}

void InterogateCoreNLP::finalizeSentiment() {
    remove(CORENLP_SENTIMENT_INPUT);
    std::string temp = CORENLP_SENTIMENT_INPUT;
    temp += ".xml";
    remove(temp.c_str());
    remove(CORENLP_SENTIMENT_OUTPUT);
}

template void
InterogateCoreNLP::getSentiment<NgramEntry*, std::vector>(std::vector<NgramEntry*>&);

template void
InterogateCoreNLP::getSentiment<NgramEntry*, std::deque>(std::deque<NgramEntry*>&);
