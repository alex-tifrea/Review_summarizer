#! /usr/bin/python

import MicrosoftNgram
import sys
import io
import time

E_VALUE = 2.71828

lookup = MicrosoftNgram.LookupService()

fdreq = int(sys.argv[1])
fdres = int(sys.argv[2])
print 'am primit fd-urile pipurilor anonime: ' + str(fdreq) + ' ' + str(fdres)

#input = fdopen(fdreq, "r");
#out = fdopen(fdres, "w");

out_deb = open("Interogate.out", "w", 0);
out_deb.write('am deschis iesirea\n')
print 'am deschis iesirea\n'

input = open("/tmp/ngramfiforeq", "r")
out_deb.write('am deschis input\n')

out = open("/tmp/ngramfifo", "w", 0);
out_deb.write('am deschis output\n')

buffer = []
buffering = False

while (1):
    line = input.readline(50)
    out_deb.write('am citit ceva de la input\n')
    if line:
        out_deb.write('am primit ' + line)
        if (line == 'Interogate please exit\n'):
            input.close()
            out.close()
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
            out.write(result_final)
            out.flush()
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
        out.write(result + '\n')
        out.flush()
    else:
        time.sleep(1)


out.close()
