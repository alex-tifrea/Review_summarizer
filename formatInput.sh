#!/bin/bash
# Separates each point from the adiacent words using spaces.
# Converts all words to lowercase.
# @args the file which need to be formatted and the destination file

python xml_input.py -f $1
old_reviews=$(cat reviews.in)

formatted_reviews=$(echo "$old_reviews" | tr -d "():;,\"'[]{}*&%\$\#@~\`\\></=+_-")
formatted_reviews=$(echo "$formatted_reviews" | tr -s " ")

formatted_reviews=$(sed 's/\./ \. /g' <<< "$formatted_reviews")
formatted_reviews=$(sed 's/\?/ \? /g' <<< "$formatted_reviews")
formatted_reviews=$(sed 's/\!/ \! /g' <<< "$formatted_reviews")
formatted_reviews=$(echo "$formatted_reviews" | tr '[:upper:]' '[:lower:]')
echo "$formatted_reviews" > tmp
mv tmp $2
