#!/bin/sh

echo "Geometric BF SISO noHARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=false" 2> logGSISO.txt
echo "Geometric BF SISO HARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=true" 2> logGSISOh.txt

echo "Codebook BF SISO noHARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=false" 2> logSISO.txt
echo "Codebook BF SISO HARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=true" 2> logSISOh.txt

echo "Geometric BF MIMO noHARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=false" 2> logGBF.txt
echo "Geometric BF MIMO HARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=true" 2> logGBFh.txt

echo "Codebook BF MIMO noHARQ"
# ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=false" 2> logABF.txt
echo "Codebook BF MIMO HARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=true" 2> logABFh.txt

echo "Single-subcarrier MMSE BF MIMO noHARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSEBeamforming --harq=false" 2> logHBF.txt
echo "Single-subcarrier MMSE BF MIMO HARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSEBeamforming --harq=true" 2> logHBFh.txt

echo "Full-spectrum MMSE BF MIMO noHARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=false" 2> logSBF.txt
echo "Full-spectrum MMSE BF MIMO HARQ"
./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=true" 2> logSBFh.txt
