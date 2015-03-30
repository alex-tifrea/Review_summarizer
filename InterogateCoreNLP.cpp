#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "InterogateCoreNLP.h"

void InterogateCoreNLP::init(std::unordered_map<std::string, WordInfo> &wordInfo) {
    std::fstream words_input;
    words_input.open("corenlpInput.in", std::ofstream::out);
    std::unordered_map<std::string, WordInfo>::iterator iter;
    for (iter = wordInfo.begin(); iter != wordInfo.end(); ++iter) {
        words_input << iter->first << std::endl;
    }
    words_input.close();
}

void InterogateCoreNLP::getPartOfSpeech(std::unordered_map<std::string,WordInfo> &wordInfo) {
    // Call CoreNLP
    system("java -cp \"$CORENLP_PATH/*\" -Xmx2g edu.stanford.nlp.pipeline.StanfordCoreNLP -annotators tokenize,ssplit,pos -file corenlpInput.in");
    // Parse the XML created by CoreNLP
    system("python coreNLP_output_parser.py corenlpInput.in.xml corenlpOutput.out");

    // Add the part-of-speech information to the wordInfo structure
    std::fstream fin;
    fin.open("corenlpOutput.out", std::fstream::in);
    std::string word, part_of_speech;
    while (fin >> word >> part_of_speech) {
        wordInfo[word].partOfSpeech = part_of_speech;
    }
    fin.close();
}

void InterogateCoreNLP::finalize() {
    remove("corenlpInput.in");
    remove("corenlpInput.in.xml");
    remove("corenlpOutput.out");
}
