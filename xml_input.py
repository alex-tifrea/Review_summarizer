import sys
import logging
import argparse
from os import listdir
from os.path import isfile, join, exists

def main():
    logging.basicConfig(level=logging.ERROR,)

    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--filename", action="store", dest="filename", required = True,
            help="add the path to the file in which the reviews are: ./File/")
    pars = parser.parse_args()

    dirPath = pars.filename
    if not exists(dirPath):
        logging.error("The file send as argument does not exist")
        sys.exit(1)

    files = [ f for f in listdir(dirPath) if isfile(join(dirPath, f)) ]

    with open("reviews.out", "w") as out_file:
        out_file.write(str(len(files)))
        out_file.write('\n')
        
        for fname in files:
            if not fname.endswith(".xml"):
                logging.error("found an non xml file")
            else:
                with open(join(dirPath, fname), "r") as f:
                    review = ""
                    continue_reading = False

                    for line in f:
                        if len(line.split("<entry>")) > 1:
                            review = line.split("<entry>")[1]
                            continue_reading = True
                        if len(review.split("</entry>")) > 1:
                            review = review.split("</entry>")[0]
                            break
                        if len(line.split("</entry>")) > 1:
                            review += line.split("</entry>")[0]
                            continue_reading = False
                            break
                        elif not line.find("<entry>"):
                            review += line
                    if len(review) > 1 and review[len(review)-1] == '\n':
                        review = review[:(len(review)-1)]
                    out_file.write(review)
                    out_file.write("\n-----\n")

if __name__ == "__main__":
    main()
