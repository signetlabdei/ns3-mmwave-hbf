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
                                            'dRecPktsDl',
                                            'dRecPktsUl'
                                            ])
for filename in filenames:

    f=open(filename)
    dRecPktsDl={}
    dRecPktsUl={}
    for ln in f:
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

    resultsDic[filename]=ResDicStruct(  dRecPktsDl,
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

## PER SIMULATION BAR PLOTS
plt.figure(1)
plt.bar(listOfLabels,[np.sum(resultsDic[x].dRecPktsDl.values()) for x in filenames],)
plt.figure(2)
plt.bar(listOfLabels,[np.sum(resultsDic[x].dRecPktsUl.values()) for x in filenames],)

# plt.figure(5)
# plt.bar([x+barWidth*markCtr for x in resultsDic[file].dAvgBlerUL.keys()],resultsDic[file].dAvgBlerUL.values(), width=barWidth)

plt.figure(1)
plt.ylabel('Received Data (bits)')
plt.gca().set_xticklabels(listOfLabels)
plt.savefig('PKTS_DL_plot%s.eps' %(outputFileTag), format='eps')
plt.figure(2)
plt.ylabel('Received Data (bits)')
plt.gca().set_xticklabels(listOfLabels)
plt.savefig('PKTS_UL_plot%s.eps' %(outputFileTag), format='eps')


# plt.show()
