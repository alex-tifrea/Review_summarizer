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

buffer = []
buffering = False

while (1):
    '''print "am citit ceva de la input"'''
    line = input.readline(50)
    if line:
        '''print 'am primit ' + line'''
        if (line == 'Interogate please exit\n'):
            input.close()
            out.close()
            exit(0)
        elif (line == 'Interogate please start buffering\n'):
            buffer = []
            buffering = True
            continue
        elif (line == 'Interogate please send queries\n'):
            result = lookup.GetJointProbabilities(buffer)
            for nr in result.split('\n'):
                nr = str(E_VALUE**(float(nr)))
            buffer = []
            buffering = False
            '''TODO send back the results'''
            continue

        if buffering:
            '''remove the '\n' from the end of the line'''
            buffer.append(line[:-1])
            continue
        result = str(E_VALUE**lookup.GetJointProbability(line))
        print 'am calculat rezultatul ' + result + ' pentru ' + line
        out.write(result + '\n')
        out.flush()
    else:
        time.sleep(1)


out.close()
