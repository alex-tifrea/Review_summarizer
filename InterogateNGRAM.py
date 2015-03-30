#! /usr/bin/python

import MicrosoftNgram
import sys, io, os, time

E_VALUE = 2.71828
MAX_LENGTH = 55000

lookup = MicrosoftNgram.LookupService()

fdreq = int(sys.argv[1])
fdres = int(sys.argv[2])
#print 'py am primit fd-urile pipurilor anonime: ' + str(fdreq) + ' ' + str(fdres)

requestInput = os.fdopen(fdreq, "r")
resultOutput = os.fdopen(fdres, "w")
#print 'py am deschis fd-urile'

out_deb = open("InterogateNGRAM.out", "w", 0)
out_deb.write('am deschis iesirea\n')
#print 'py am deschis iesirea\n'

buffer = []
buffering = False
buffer_len = 0
result = ""

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
            buffer_len = 0
            result = ""
            continue
        elif (line == 'Interogate please send queries\n'):
            #out_deb.write('Am terminat de facut buffer\n')
            """
            Check if we have previous results
            """
            if result != "":
                result += '\n'
            if buffer != []:
                result += lookup.GetJointProbabilities(buffer)
            result_final = ""
            for nr in result.split('\n'):
                result_final = result_final + str(E_VALUE**(float(nr))) + "\n"
            #out_deb.write("result " + result_final)
            resultOutput.write(result_final)
            resultOutput.flush()
            #out_deb.write('am trimis rezultat final\n')
            buffer = []
            buffering = False
            buffer_len = 0
            result = ""
            continue

        if buffering:
            '''remove the '\n' from the end of the line'''
            buffer.append(line[:-1])
            buffer_len += len(line[:-1])
            #out_deb.write("dimensiunea bufferului: " + str(buffer_len) + "\n")
            if buffer_len > MAX_LENGTH:
                out_deb.write("am atins dimensiunea maxima, fac req\n")
                """
                Reached maximum size of html requests
                """
                if result != "":
                    result += '\n'
                result += lookup.GetJointProbabilities(buffer)
                buffer = []
                buffer_len = 0
            continue
        result = str(E_VALUE**lookup.GetJointProbability(line))
        #out_deb.write('am calculat rezultatul ' + result + ' pentru ' + line + '\n')
        resultOutput.write(result + '\n')
        resultOutput.flush()
    else:
        time.sleep(1)


requestInput.close()
resultOutput.close()
