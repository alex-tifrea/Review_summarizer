#! /usr/bin/python

import MicrosoftNgram
import sys, io, os, time

E_VALUE = 2.71828

lookup = MicrosoftNgram.LookupService()

fdreq = int(sys.argv[1])
fdres = int(sys.argv[2])
#print 'py am primit fd-urile pipurilor anonime: ' + str(fdreq) + ' ' + str(fdres)

requestInput = os.fdopen(fdreq, "r")
resultOutput = os.fdopen(fdres, "w")
#print 'py am deschis fd-urile'

out_deb = open("Interogate.out", "w", 0)
out_deb.write('am deschis iesirea\n')
#print 'py am deschis iesirea\n'

#requestInput = open("/tmp/ngramfiforeq", "r")
#out_deb.write('am deschis requestInput\n')

#out = open("/tmp/ngramfifo", "w", 0);
#out_deb.write('py am deschis output\n')

buffer = []
buffering = False

while (1):
    #print 'py astept sa primesc ceva de la requestInput'
    line = requestInput.readline()
    out_deb.write('am citit ceva de la requestInput\n')
    #print 'py am citit ceva de la requestInput\n'
    if line:
        out_deb.write('am primit ' + line)
        #print 'py am primit ' + line
        if (line == 'Interogate please exit\n'):
            requestInput.close()
            resultOutput.close()
            exit(0)
        elif (line == 'Interogate please start buffering\n'):
            out_deb.write('Incep sa fac buffer\n')
            buffer = []
            buffering = True
            continue
        elif (line == 'Interogate please send queries\n'):
            out_deb.write('Am terminat de facut buffer\n')
            result = lookup.GetJointProbabilities(buffer)
            result_final = ""
            for nr in result.split('\n'):
                result_final = result_final + str(E_VALUE**(float(nr))) + "\n"
            out_deb.write("result " + result_final)
            resultOutput.write(result_final)
            resultOutput.flush()
            out_deb.write('am trimis rezultat final\n')
            buffer = []
            buffering = False
            continue

        if buffering:
            '''remove the '\n' from the end of the line'''
            buffer.append(line[:-1])
            continue
        result = str(E_VALUE**lookup.GetJointProbability(line))
        out_deb.write('am calculat rezultatul ' + result + ' pentru ' + line + '\n')
        resultOutput.write(result + '\n')
        resultOutput.flush()
    else:
        time.sleep(1)


requestInput.close()
resultOutput.close()
