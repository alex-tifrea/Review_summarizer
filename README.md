Review_summarizer
=================

Summarizes the reviews

In order to make crawler.py work, install lxml and requests (i.e. 'pip install
lxlm' and 'pip install requests')

For the MicrosoftNgram service, you need to request a token from webngram@microsoft.com
and place is in you .bashrc as following:
```bash
export NGRAM_TOKEN=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
```
More information about MicrosoftNgram here:
http://weblm.research.microsoft.com/info/


To run corenlp with input.txt run:
```bash
java -cp "*" -Xmx2g edu.stanford.nlp.pipeline.StanfordCoreNLP -annotators tokenize,ssplit,pos,lemma,ner,parse,dcoref -file input.txt
```
You can delete some annotators:
```bash
java -cp "*" -Xmx2g edu.stanford.nlp.pipeline.StanfordCoreNLP -annotators tokenize,ssplit,pos -file input.txt
```

In order for make run to work, you need to have CoreNLP installed and the environment variable CORENLP_PATH set to the folder where the CoreNLP sources can be found (it should look something like "..."/stanford-corenlp-full-...)

Add the CORENLP_MEMORY environment variable which is the size with which java will run
export CORENLP_MEMORY=2g


More information about corenlp here:
http://nlp.stanford.edu/software/corenlp.shtml

List with POS tags used by CoreNLP: https://www.ling.upenn.edu/courses/Fall_2003/ling001/penn_treebank_pos.html
