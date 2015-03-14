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

More information about corenlp here:
http://nlp.stanford.edu/software/corenlp.shtml

Stats for 401 reviews:
    - Total number of words:        35498
    - Number of unique word:        376
    - Aprox. number of sentences:   2403
