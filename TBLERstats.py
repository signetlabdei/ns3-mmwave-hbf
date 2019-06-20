#!/usr/bin/python

f=open('log.txt')

dAvgBler={}
dNBler={}
for ln in f:
    if ln.find('TBLER')>=0:
        rnti=int(ln[ln.find('RNTI')+5:ln.find(' ',ln.find('RNTI')+5)])
        tbler=float(ln[ln.find('TBLER')+6:ln.find(' ',ln.find('TBLER')+6)])
        if dAvgBler.has_key(rnti):
            dAvgBler[rnti] = (dAvgBler[rnti] * dNBler[rnti] + tbler)* 1.0 / (dNBler[rnti] + 1)
            dNBler[rnti] = dNBler[rnti] + 1
        else:
            dNBler[rnti]=1
            dAvgBler[rnti]=tbler

for usr in dAvgBler:
    print "User %d TBLER %.3f"%(usr,dAvgBler[usr])
