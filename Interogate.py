#! /usr/bin/python

import MicrosoftNgram
import sys
import io
import time

E_VALUE = 2.71828

lookup = MicrosoftNgram.LookupService()

input = open("/tmp/ngramfiforeq", "r")
'''print 'am deschis input' '''

out = open("/tmp/ngramfifo", "w", 0);
'''print 'am deschis output' '''

while (1):
    '''print "am citit ceva de la input"'''
    line = input.readline(25)
    if line:
        '''print 'am primit ' + line'''
        if (line == 'Interogate please exit\n'):
            out.close()
            exit(0)
        result = str(lookup.GetJointProbability(line))
        result = str(E_VALUE**(float(result)))
        print 'am calculat rezultatul ' + result + ' pentru ' + line
        out.write(result + '\n')
        out.flush()
    else:
        time.sleep(1)


out.close()
