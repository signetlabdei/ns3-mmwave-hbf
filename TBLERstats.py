#!/usr/bin/python

import sys
import matplotlib.pyplot as plt
import numpy as np
import collections as cl

if len(sys.argv)>1:
    if (sys.argv[1].find('-t=')==0):
        firstFileName=2
        outputFileTag=sys.argv[1][ sys.argv[1].find('-t=')+3: ]
    if (sys.argv[2].find('-l=')==0):
        firstFileName=3
        listOfLabels=sys.argv[2][ sys.argv[2].find('-l=')+3: ].split(',')
        print(listOfLabels)
    else:
        outputFileTag=""
    filenames = sys.argv[firstFileName:]
else:
    filenames = ['logHBF.txt']
    outputFileTag=""

resultsDic={}
ResDicStruct=cl.namedtuple('ResDicStruct',[
                                            'dAvgBlerDL',
                                            'dAvgBlerUL',
                                            'dAvgCorrDL',
                                            'dAvgCorrUL',
                                            'dAvgGtrDL',
                                            'dAvgGtrU',
                                            'dNBlerDL',
                                            'dNBlerUL',
                                            'dAvgSINRDL',
                                            'dAvgSINRUL',
                                            'dlAllBFGain',
                                            'dlAllSINRDL',
                                            'dlAllSINRUL',
                                            'dlAllMCSreqDl',
                                            'dlAllMCSreqUl',
                                            'dlAllMCSactDl',
                                            'dlAllMCSactUl',
                                            'dRecPktsDl',
                                            'dRecPktsUl'
                                            ])
for filename in filenames:

    f=open(filename)

    dAvgBlerDL={}
    dAvgBlerUL={}
    dAvgCorrDL={}
    dAvgCorrUL={}
    dAvgGtrDL={}
    dAvgGtrUL={}
    dNBlerDL={}
    dNBlerUL={}
    dAvgSINRDL={}
    dAvgSINRUL={}
    dlAllBFGain={}
    dlAllSINRDL={}
    dlAllSINRUL={}
    dlAllMCSreqDl={}
    dlAllMCSreqUl={}
    dlAllMCSactDl={}
    dlAllMCSactUl={}
    dRecPktsDl={}
    dRecPktsUl={}
    for ln in f:
        if ln.find('BF Gain')>=0:
            txid=int(ln[ln.find('TxId')+5:ln.find(' ',ln.find('TxId')+5)])
            rxid=int(ln[ln.find('RxId')+5:ln.find(' ',ln.find('RxId')+5)])
            txbm=int(ln[ln.find('TxBeam')+7:ln.find(' ',ln.find('TxBeam')+7)])
            rxbm=int(ln[ln.find('RxBeam')+7:ln.find(' ',ln.find('RxBeam')+7)])
            gain=float(ln[ln.find('g=')+3:])
            key=(txid,rxid,txbm,rxbm)
            if dlAllBFGain.has_key(key):
                dlAllBFGain[key].append(gain)
            else:
                dlAllBFGain[key]=[gain]
        if ln.find('TBLER')>=0:
            size=int(ln[ln.find('size')+5:ln.find(' ',ln.find('size')+5)])
            rnti=int(ln[ln.find('RNTI')+5:ln.find(' ',ln.find('RNTI')+5)])
            tbler=float(ln[ln.find('TBLER')+6:ln.find(' ',ln.find('TBLER')+6)])
            sinrdB=float(ln[ln.find('SINR')+5:ln.find(' ',ln.find('SINR')+5)])
            corrupted=int(ln[ln.find('corrupted')+10:ln.find('.',ln.find('corrupted')+10)])
            mcsreq=int(ln[ln.find('should be')+10:ln.find(' ',ln.find('should be')+10)])
            mcsact=int(ln[ln.find('mcs')+4:ln.find(' ',ln.find('mcs')+4)])
            if (mcsreq>=mcsact):
                gtr=0;
                if (corrupted>0):
                    print(ln)
            else:
                gtr=1;
            if (size>200):
                if ln.find(' DL size')>=0:
                    if dAvgBlerDL.has_key(rnti):
                        dAvgBlerDL[rnti] = (dAvgBlerDL[rnti] * dNBlerDL[rnti] + tbler)* 1.0 / (dNBlerDL[rnti] + 1)
                        dAvgCorrDL[rnti] = (dAvgCorrDL[rnti] * dNBlerDL[rnti] + corrupted)* 1.0 / (dNBlerDL[rnti] + 1)
                        dAvgGtrDL[rnti] = (dAvgGtrDL[rnti] * dNBlerDL[rnti] + gtr)* 1.0 / (dNBlerDL[rnti] + 1)
                        dAvgSINRDL[rnti] = (dAvgSINRDL[rnti] * dNBlerDL[rnti] + 10.0**(sinrdB/10.0))* 1.0 / (dNBlerDL[rnti] + 1)
                        dlAllSINRDL[rnti].append(sinrdB)
                        dlAllMCSreqDl[rnti].append(mcsreq)
                        dlAllMCSactDl[rnti].append(mcsact)
                        dNBlerDL[rnti] = dNBlerDL[rnti] + 1
                    else:
                        dNBlerDL[rnti]=1
                        dAvgBlerDL[rnti]=tbler
                        dAvgCorrDL[rnti]=corrupted
                        dAvgGtrDL[rnti]=gtr
                        dAvgSINRDL[rnti]=10.0**(sinrdB/10.0)
                        dlAllSINRDL[rnti]=[sinrdB]
                        dlAllMCSreqDl[rnti]=[mcsreq]
                        dlAllMCSactDl[rnti]=[mcsact]
                elif ln.find(' UL size')>=0:
                    if dAvgBlerUL.has_key(rnti):
                        dAvgBlerUL[rnti] = (dAvgBlerUL[rnti] * dNBlerUL[rnti] + tbler)* 1.0 / (dNBlerUL[rnti] + 1)
                        dAvgCorrUL[rnti] = (dAvgCorrUL[rnti] * dNBlerUL[rnti] + corrupted)* 1.0 / (dNBlerUL[rnti] + 1)
                        dAvgGtrUL[rnti] = (dAvgGtrUL[rnti] * dNBlerUL[rnti] + gtr)* 1.0 / (dNBlerUL[rnti] + 1)
                        dAvgSINRUL[rnti] = (dAvgSINRUL[rnti] * dNBlerUL[rnti] + 10.0**(sinrdB/10.0))* 1.0 / (dNBlerUL[rnti] + 1)
                        dlAllSINRUL[rnti].append(sinrdB)
                        dlAllMCSreqUl[rnti].append(mcsreq)
                        dlAllMCSactUl[rnti].append(mcsact)
                        dNBlerUL[rnti] = dNBlerUL[rnti] + 1
                    else:
                        dNBlerUL[rnti]=1
                        dAvgBlerUL[rnti]=tbler
                        dAvgCorrUL[rnti]=corrupted
                        dAvgGtrUL[rnti]=gtr
                        dAvgSINRUL[rnti]=10.0**(sinrdB/10.0)
                        dlAllSINRUL[rnti]=[sinrdB]
                        dlAllMCSreqUl[rnti]=[mcsreq]
                        dlAllMCSactUl[rnti]=[mcsact]
                else:
                    print "found a TBLER with unknown DL/UL attribution"
        if ln.find('The number of DL received bytes for UE ')>=0:
            rnti=int(ln[ln.find('UE')+3:ln.find(':',ln.find('UE')+3)])
            pkts=int(ln[ln.find(':')+2:])
            if dRecPktsDl.has_key(rnti):
                dRecPktsDl[rnti]+=pkts#should never reach this point
            else:
                dRecPktsDl[rnti]=pkts
        if ln.find('The number of UL received bytes for UE ')>=0:
            rnti=int(ln[ln.find('UE')+3:ln.find(':',ln.find('UE')+3)])
            pkts=int(ln[ln.find(':')+2:])
            if dRecPktsUl.has_key(rnti):
                dRecPktsUl[rnti]+=pkts#should never reach this point
            else:
                dRecPktsUl[rnti]=pkts

    resultsDic[filename]=ResDicStruct(  dAvgBlerDL,
                                        dAvgBlerUL,
                                        dAvgCorrDL,
                                        dAvgCorrUL,
                                        dAvgGtrDL,
                                        dAvgGtrUL,
                                        dNBlerDL,
                                        dNBlerUL,
                                        dAvgSINRDL,
                                        dAvgSINRUL,
                                        dlAllBFGain,
                                        dlAllSINRDL,
                                        dlAllSINRUL,
                                        dlAllMCSreqDl,
                                        dlAllMCSreqUl,
                                        dlAllMCSactDl,
                                        dlAllMCSactUl,
                                        dRecPktsDl,
                                        dRecPktsUl)

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
plt.bar(listOfLabels,[np.sum(resultsDic[x].dRecPktsDl.values()) for x in filenames],)
plt.figure(3)
plt.bar(listOfLabels,[np.mean(resultsDic[x].dAvgBlerUL.values()) for x in filenames],)
plt.figure(4)
plt.bar(listOfLabels,[np.sum(resultsDic[x].dRecPktsUl.values()) for x in filenames],)

for file in filenames:
    ## PER SIMULATION STATISTICS
    plt.figure(10)
    lAux = sorted(sum(resultsDic[file].dlAllSINRDL.values(), []))
    plt.plot(lAux,np.arange(0.0,float(len(lAux)),1)/len(lAux),marker=allMarkers[markCtr],label='%s'%(file))
    plt.legend()
    plt.figure(11)
    lAux = sorted(sum(resultsDic[file].dlAllSINRUL.values(), []))
    plt.plot(lAux,np.arange(0.0,float(len(lAux)),1)/len(lAux),marker=allMarkers[markCtr],label='%s'%(file))
    plt.legend()
    # plt.figure(3)
    # for key in resultsDic[file].dlAllBFGain:
    #     lAux=sorted(resultsDic[file].dlAllBFGain[key])
    #     if len(lAux)>0:
    #         #print "BF tuple %s uses %d E[G]= %.2f dB"%(key,len(dlAllBFGain[key]),10*np.log10(1e-20+sum(dlAllBFGain[key])/len(dlAllBFGain[key])))
    #         plt.plot(np.arange(0.0,len(lAux),1.0)/len(lAux),10*np.log10([max(1e-4,x) for x in lAux]),marker=allMarkers[markCtr],label='%s beams %s'%(file,key))
    # plt.legend()
    ## PER USER STATISTICS
    plt.figure(20)
    for usr in resultsDic[file].dAvgBlerDL:
        print "File %s User %d downlink TBLER %.3f AvgSINR %.2f dB CRPT %.2f MGTR %.2f"%(file,usr,resultsDic[file].dAvgBlerDL[usr],10.0*np.log10(resultsDic[file].dAvgSINRDL[usr]),dAvgCorrDL[usr],dAvgGtrDL[usr])
        lAux=sorted(resultsDic[file].dlAllSINRDL[usr])
        plt.plot(lAux,np.arange(0.0,float(len(lAux)),1)/len(lAux),marker=allMarkers[markCtr],label='%s RNTI %s'%(file,usr))
    # plt.figure(2)
    for usr in resultsDic[file].dAvgBlerUL:
        print "File %s User %d uplink   TBLER %.3f AvgSINR %.2f dB CRPT %.2f MGTR %.2f"%(file,usr,resultsDic[file].dAvgBlerUL[usr],10.0*np.log10(resultsDic[file].dAvgSINRUL[usr]),resultsDic[file].dAvgCorrUL[usr],dAvgGtrUL[usr])

    # for usr in resultsDic[file].dlAllMCSreqDl:
    #     plt.plot(range(0,len(resultsDic[file].dlAllMCSreqDl[usr])),resultsDic[file].dlAllMCSreqDl[usr],marker='o')
    #     plt.plot(range(0,len(resultsDic[file].dlAllMCSactDl[usr])),resultsDic[file].dlAllMCSactDl[usr],marker='x')
    # for usr in resultsDic[file].dlAllMCSreqUl:
    #     plt.plot(range(0,len(resultsDic[file].dlAllMCSreqUl[usr])),resultsDic[file].dlAllMCSreqUl[usr],marker='o')
    #     plt.plot(range(0,len(resultsDic[file].dlAllMCSactUl[usr])),resultsDic[file].dlAllMCSactUl[usr],marker='x')


    markCtr+=1

# plt.figure(5)
# plt.bar([x+barWidth*markCtr for x in resultsDic[file].dAvgBlerUL.keys()],resultsDic[file].dAvgBlerUL.values(), width=barWidth)

plt.figure(1)
plt.ylabel('BLER')
plt.gca().set_xticklabels(listOfLabels)
plt.savefig( 'BLER_DL_plot%s.eps' %(outputFileTag), format='eps')
plt.figure(2)
plt.ylabel('Received Data (bits)')
plt.gca().set_xticklabels(listOfLabels)
plt.savefig('PKTS_DL_plot%s.eps' %(outputFileTag), format='eps')
plt.figure(3)
plt.ylabel('BLER')
plt.gca().set_xticklabels(listOfLabels)
plt.savefig('BLER_UL_plot%s.eps' %(outputFileTag), format='eps')
plt.figure(4)
plt.ylabel('Received Data (bits)')
plt.gca().set_xticklabels(listOfLabels)
plt.savefig('PKTS_UL_plot%s.eps' %(outputFileTag), format='eps')

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

# plt.show()
