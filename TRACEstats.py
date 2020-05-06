#!/usr/bin/python

import sys
import matplotlib.pyplot as plt
import numpy as np
import collections as cl

if len(sys.argv)>1:
    if (sys.argv[1].find('-t=')==0):
        outputFileTag=sys.argv[1][ sys.argv[1].find('-t=')+3: ]
        if len(sys.argv)>2:
            if (sys.argv[2].find('-l=')==0):
                firstFileName=3
                listOfLabels=sys.argv[2][ sys.argv[2].find('-l=')+3: ].split(',')
                print(listOfLabels)
            else:
                firstFileName=2
                listOfLabels=sys.argv[firstFileName:]
    else:
        firstFileName=1
        outputFileTag=""
        listOfLabels=sys.argv[firstFileName:]
    filenames = sys.argv[firstFileName:]
else:
    filenames = ['RxPacketTrace.txt']
    outputFileTag=""

resultsDic={}
ResDicStruct=cl.namedtuple('ResDicStruct',[
                                            'dAvgBlerDL',
                                            'dAvgBlerUL',
                                            'dAvgCorrDL',
                                            'dAvgCorrUL',
                                            'dNBlerDL',
                                            'dNBlerUL',
                                            'dAvgSINRDL',
                                            'dAvgSINRUL',
                                            'dlAllSINRDL',
                                            'dlAllSINRUL',
                                            'dlAllMCSactDl',
                                            'dlAllMCSactUl',
                                            ])
for filename in filenames:

    f=open(filename)

    dAvgBlerDL={}
    dAvgBlerUL={}
    dAvgCorrDL={}
    dAvgCorrUL={}
    dNBlerDL={}
    dNBlerUL={}
    dAvgSINRDL={}
    dAvgSINRUL={}
    dlAllSINRDL={}
    dlAllSINRUL={}
    dlAllMCSactDl={}
    dlAllMCSactUl={}
    bFirstLine = True
    for ln in f:
        if bFirstLine:
            bFirstLine=False
            continue
        vals=ln.split('\t')
        rnti=int(vals[7])
        size=int(vals[9])
        mcsact=int(vals[10])
        sinrdB=float(vals[12])
        corrupted=int(vals[13])
        tbler=float(vals[14])
        if (size>200):
            if vals[0].find('DL')>=0:
                if dAvgBlerDL.has_key(rnti):
                    dAvgBlerDL[rnti] = (dAvgBlerDL[rnti] * dNBlerDL[rnti] + tbler)* 1.0 / (dNBlerDL[rnti] + 1)
                    dAvgCorrDL[rnti] = (dAvgCorrDL[rnti] * dNBlerDL[rnti] + corrupted)* 1.0 / (dNBlerDL[rnti] + 1)
                    dAvgSINRDL[rnti] = (dAvgSINRDL[rnti] * dNBlerDL[rnti] + 10.0**(sinrdB/10.0))* 1.0 / (dNBlerDL[rnti] + 1)
                    dlAllSINRDL[rnti].append(sinrdB)
                    dlAllMCSactDl[rnti].append(mcsact)
                    dNBlerDL[rnti] = dNBlerDL[rnti] + 1
                else:
                    dNBlerDL[rnti]=1
                    dAvgBlerDL[rnti]=tbler
                    dAvgCorrDL[rnti]=corrupted
                    dAvgSINRDL[rnti]=10.0**(sinrdB/10.0)
                    dlAllSINRDL[rnti]=[sinrdB]
                    dlAllMCSactDl[rnti]=[mcsact]
            elif vals[0].find('UL')>=0:
                if dAvgBlerUL.has_key(rnti):
                    dAvgBlerUL[rnti] = (dAvgBlerUL[rnti] * dNBlerUL[rnti] + tbler)* 1.0 / (dNBlerUL[rnti] + 1)
                    dAvgCorrUL[rnti] = (dAvgCorrUL[rnti] * dNBlerUL[rnti] + corrupted)* 1.0 / (dNBlerUL[rnti] + 1)
                    dAvgSINRUL[rnti] = (dAvgSINRUL[rnti] * dNBlerUL[rnti] + 10.0**(sinrdB/10.0))* 1.0 / (dNBlerUL[rnti] + 1)
                    dlAllSINRUL[rnti].append(sinrdB)
                    dlAllMCSactUl[rnti].append(mcsact)
                    dNBlerUL[rnti] = dNBlerUL[rnti] + 1
                else:
                    dNBlerUL[rnti]=1
                    dAvgBlerUL[rnti]=tbler
                    dAvgCorrUL[rnti]=corrupted
                    dAvgSINRUL[rnti]=10.0**(sinrdB/10.0)
                    dlAllSINRUL[rnti]=[sinrdB]
                    dlAllMCSactUl[rnti]=[mcsact]
            else:
                print "found a TBLER with unknown DL/UL attribution"

    resultsDic[filename]=ResDicStruct(  dAvgBlerDL,
                                        dAvgBlerUL,
                                        dAvgCorrDL,
                                        dAvgCorrUL,
                                        dNBlerDL,
                                        dNBlerUL,
                                        dAvgSINRDL,
                                        dAvgSINRUL,
                                        dlAllSINRDL,
                                        dlAllSINRUL,
                                        dlAllMCSactDl,
                                        dlAllMCSactUl)

allMarkers=['o',
            '*',
            's',
            'x',
            'd',
            '+',
            '^',
            'v',
            '<',
            '>',
            'p',
            '.']
markCtr = 0
barWidth=1.0/(1+len(resultsDic));

## PER SIMULATION BAR PLOTS
plt.figure(1)
plt.bar(listOfLabels,[np.mean(resultsDic[x].dAvgBlerDL.values()) for x in filenames],)
plt.figure(2)
plt.bar(listOfLabels,[np.mean(resultsDic[x].dAvgBlerUL.values()) for x in filenames],)

for file in filenames:
    ## PER SIMULATION STATISTICS
    plt.figure(10)
    lAllSINRDl = sum(resultsDic[file].dlAllSINRDL.values(), [])
    lAux = sorted(lAllSINRDl)
    plt.plot(lAux,np.arange(0.0,float(len(lAux)),1)/len(lAux),marker=allMarkers[markCtr],label='%s'%(file))
    plt.legend()
    plt.figure(11)
    lAllSINRUl = sum(resultsDic[file].dlAllSINRUL.values(), [])
    lAux = sorted(lAllSINRUl)
    plt.plot(lAux,np.arange(0.0,float(len(lAux)),1)/len(lAux),marker=allMarkers[markCtr],label='%s'%(file))
    plt.legend()
    # ## PER USER STATISTICS
    # plt.figure(20)
    # for usr in resultsDic[file].dAvgBlerDL:
    #     print "File %s User %d downlink TBLER %.3f AvgSINR %.2f dB CRPT %.2f"%(file,usr,resultsDic[file].dAvgBlerDL[usr],10.0*np.log10(resultsDic[file].dAvgSINRDL[usr]),resultsDic[file].dAvgCorrDL[usr])
    #     lAux=sorted(resultsDic[file].dlAllSINRDL[usr])
    #     plt.plot(lAux,np.arange(0.0,float(len(lAux)),1)/len(lAux),marker=allMarkers[markCtr],label='%s RNTI %s'%(file,usr))
    # # plt.figure(21)
    # for usr in resultsDic[file].dAvgBlerUL:
    #     print "File %s User %d uplink   TBLER %.3f AvgSINR %.2f dB CRPT %.2f"%(file,usr,resultsDic[file].dAvgBlerUL[usr],10.0*np.log10(resultsDic[file].dAvgSINRUL[usr]),resultsDic[file].dAvgCorrUL[usr])

    # for usr in resultsDic[file].dlAllMCSreqDl:
    #     plt.plot(range(0,len(resultsDic[file].dlAllMCSreqDl[usr])),resultsDic[file].dlAllMCSreqDl[usr],marker='o')
    #     plt.plot(range(0,len(resultsDic[file].dlAllMCSactDl[usr])),resultsDic[file].dlAllMCSactDl[usr],marker='x')
    # for usr in resultsDic[file].dlAllMCSreqUl:
    #     plt.plot(range(0,len(resultsDic[file].dlAllMCSreqUl[usr])),resultsDic[file].dlAllMCSreqUl[usr],marker='o')
    #     plt.plot(range(0,len(resultsDic[file].dlAllMCSactUl[usr])),resultsDic[file].dlAllMCSactUl[usr],marker='x')

    plt.figure(12)
    lAllMCSDl = sum(resultsDic[file].dlAllMCSactDl.values(), [])
    plt.plot(lAllMCSDl,lAllSINRDl,linestyle='',marker=allMarkers[markCtr],label='%s'%(file))
    plt.legend()
    plt.figure(13)
    lAllMCSUl = sum(resultsDic[file].dlAllMCSactUl.values(), [])
    plt.plot(lAllMCSUl,lAllSINRUl,linestyle='',marker=allMarkers[markCtr],label='%s'%(file))
    plt.legend()

    markCtr+=1

# plt.figure(5)
# plt.bar([x+barWidth*markCtr for x in resultsDic[file].dAvgBlerUL.keys()],resultsDic[file].dAvgBlerUL.values(), width=barWidth)

plt.figure(1)
plt.ylabel('BLER')
plt.gca().set_xticklabels(listOfLabels)
plt.savefig( 'BLER_DL_plot%s.eps' %(outputFileTag), format='eps')
plt.figure(2)
plt.ylabel('BLER')
plt.gca().set_xticklabels(listOfLabels)
plt.savefig('BLER_UL_plot%s.eps' %(outputFileTag), format='eps')

plt.figure(10)
plt.legend(listOfLabels)
plt.xlabel('SINR (dB)')
plt.ylabel('C.D.F.')
plt.savefig('SINR_DL_plot%s.eps' %(outputFileTag), format='eps')
plt.figure(11)
plt.legend(listOfLabels)
plt.xlabel('SINR (dB)')
plt.ylabel('C.D.F.')
plt.savefig('SINR_UL_plot%s.eps' %(outputFileTag), format='eps')
plt.figure(12)
plt.legend(listOfLabels)
plt.xlabel('MCS')
plt.ylabel('SINR (dB)')
plt.savefig('MCS_DL_plot%s.eps' %(outputFileTag), format='eps')
plt.figure(13)
plt.legend(listOfLabels)
plt.xlabel('MCS')
plt.ylabel('SINR (dB)')
plt.savefig('MCS_UL_plot%s.eps' %(outputFileTag), format='eps')

# plt.show()
