#!/usr/bin/python

f=open('log.txt')

dAvgBlerDL={}
dAvgBlerUL={}
dNBlerDL={}
dNBlerUL={}
for ln in f:
    if ln.find('TBLER')>=0:
        rnti=int(ln[ln.find('RNTI')+5:ln.find(' ',ln.find('RNTI')+5)])
        tbler=float(ln[ln.find('TBLER')+6:ln.find(' ',ln.find('TBLER')+6)])
        if ln.find(' DL size')>=0:
            if dAvgBlerDL.has_key(rnti):
                dAvgBlerDL[rnti] = (dAvgBlerDL[rnti] * dNBlerDL[rnti] + tbler)* 1.0 / (dNBlerDL[rnti] + 1)
                dNBlerDL[rnti] = dNBlerDL[rnti] + 1
            else:
                dNBlerDL[rnti]=1
                dAvgBlerDL[rnti]=tbler
        elif ln.find(' UL size')>=0:
            if dAvgBlerUL.has_key(rnti):
                dAvgBlerUL[rnti] = (dAvgBlerUL[rnti] * dNBlerUL[rnti] + tbler)* 1.0 / (dNBlerUL[rnti] + 1)
                dNBlerUL[rnti] = dNBlerUL[rnti] + 1
            else:
                dNBlerUL[rnti]=1
                dAvgBlerUL[rnti]=tbler
        else:
            print "found a TBLER with unknown DL/UL attribution"

for usr in dAvgBlerDL:
    print "User %d downlink TBLER %.3f"%(usr,dAvgBlerDL[usr])
for usr in dAvgBlerUL:
    print "User %d uplink   TBLER %.3f"%(usr,dAvgBlerUL[usr])
