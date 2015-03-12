import sys
import logging
import xml.etree.ElementTree as ET

def main():
    logging.basicConfig(level=logging.ERROR)

    if len(sys.argv) == 1:
        logging.error("Add at least one commad line argument, the input file. \
                The second command line argoment is optional and is an output \
                file")
        sys.exit(1)

    if len(sys.argv) > 2:
        sys.stdout = open(sys.argv[2], 'w')

    try:
        tree = ET.parse(sys.argv[1])
    except Exception:
        logging.error("Wrong input file")
        sys.exit(2)
    try:
        # Finds the path from root to tokens
        root = tree.getroot()
        document = root[0]
        sentences = document[0]
        for sentence in sentences:
            tokens = sentence[0]
            for token in tokens:
                print token[0].text, token[3].text
                # In case of emargency comment the line above and decoment 
                # the ones below
                """
                if token[0].tag == 'word' and token[3].tag == 'POS':
                    print token[0].text, token[3].text
                else:
                    word = ''
                    POS = ''
                    for pos in token:
                        if poz.tag == 'word':
                            word = poz.text
                        elif poz.tag == 'POS':
                            POS = poz.text
                    print word, POS
                """
    except IndexError:
        logging.error("The file has an unusual format")
        sys.exit(3)

if __name__ == "__main__":
    main()
