#!/bin/bash
# returns stats about the input (i.e. the number of words, unique words,
# sentences etc.)
input="reviews.in"

cat $input | tr " " "\n" | sort -f > out
cat $out | sort -fu > unic
