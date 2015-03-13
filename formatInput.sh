#!/bin/bash
# TODO: decomenteaza liniile de jos
# python xml_input.py -f "Review_Bucharest_Hotel_Christina"
# old_reviews=cat reviews.in
# TODO: sterge linia asta
old_reviews=$(cat input_example)
echo $old_reviews
formatted_reviews=${old_reviews/"."/" . "}
echo $formatted_reviews > tmp
