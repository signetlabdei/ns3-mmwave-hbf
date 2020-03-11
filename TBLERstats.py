#!/usr/bin/python

import sys
import matplotlib.pyplot as plt
import numpy as np

if len(sys.argv)>1:
    filename = sys.argv[1]
else:
    filename = 'log.txt'

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

for usr in dAvgBlerDL:
    print "User %d downlink TBLER %.3f AvgSINR %.2f dB CRPT %.2f MGTR %.2f"%(usr,dAvgBlerDL[usr],10.0*np.log10(dAvgSINRDL[usr]),dAvgCorrDL[usr],dAvgGtrDL[usr])
    lAux=sorted(dlAllSINRDL[usr])
    plt.plot(lAux,np.arange(0.0,float(len(lAux)),1)/len(lAux),label='RNTI %s'%usr)
plt.legend()
plt.figure()
for usr in dAvgBlerUL:
    print "User %d uplink   TBLER %.3f AvgSINR %.2f dB CRPT %.2f MGTR %.2f"%(usr,dAvgBlerUL[usr],10.0*np.log10(dAvgSINRUL[usr]),dAvgCorrUL[usr],dAvgGtrUL[usr])

for usr in dlAllMCSreqDl:
    plt.plot(range(0,len(dlAllMCSreqDl[usr])),dlAllMCSreqDl[usr],marker='o')
    plt.plot(range(0,len(dlAllMCSactDl[usr])),dlAllMCSactDl[usr],marker='x')
for usr in dlAllMCSreqUl:
    plt.plot(range(0,len(dlAllMCSreqUl[usr])),dlAllMCSreqUl[usr],marker='o')
    plt.plot(range(0,len(dlAllMCSactUl[usr])),dlAllMCSactUl[usr],marker='x')

plt.figure()
for key in dlAllBFGain:
    lAux=sorted(dlAllBFGain[key])
    print "BF tuple %s uses %d E[G]= %.2f dB"%(key,len(dlAllBFGain[key]),10*np.log10(sum(dlAllBFGain[key])/len(dlAllBFGain[key])))
    plt.plot(range(0,len(lAux)),10*np.log10(lAux))
plt.show()
