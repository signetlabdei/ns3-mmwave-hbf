# !/bin/sh

################################################################################
# NS3 mmWave Previous Scheduler

# echo "Geometric BF previous SISO scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=false --ns3::MmWavePhyRxTrace::OutputFilename=GBF-PREV-UNRL.trace" 2> GBF-PREV-UNRL.log
# echo "Geometric BF previous SISO scheduler  HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=true  --ns3::MmWavePhyRxTrace::OutputFilename=GBF-PREV-HARQ.trace" 2> GBF-PREV-HARQ.log
#
# echo "Codebook BF previous SISO scheduler  noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=false  --ns3::MmWavePhyRxTrace::OutputFilename=ABF-PREV-UNRL.trace" 2> ABF-PREV-UNRL.log
# echo "Codebook BF previous SISO scheduler  HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=true  --ns3::MmWavePhyRxTrace::OutputFilename=ABF-PREV-HARQ.trace" 2> ABF-PREV-HARQ.log

# ################################################################################
# # 1-layer SISO but with our padding scheduler
#
# echo "Geometric BF 1-layer padding-scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=false --nLayers=1   --ns3::MmWavePhyRxTrace::OutputFilename=GBF-1LAY-UNRL.trace" 2> GBF-1LAY-UNRL.log
# echo "GeometricBF 1-layer padding-scheduler HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=true --nLayers=1   --ns3::MmWavePhyRxTrace::OutputFilename=GBF-1LAY-HARQ.trace" 2> GBF-1LAY-HARQ.log
#
# echo "Codebook BF 1-layer padding-scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=false --nLayers=1   --ns3::MmWavePhyRxTrace::OutputFilename=ABF-1LAY-UNRL.trace" 2> ABF-1LAY-UNRL.log
# echo "Codebook BF 1-layer padding-scheduler HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=true --nLayers=1   --ns3::MmWavePhyRxTrace::OutputFilename=ABF-1LAY-HARQ.trace" 2> ABF-1LAY-HARQ.log

# ################################################################################
# # 4-layer HBF MIMO SDMA with our padding scheduler
#
# echo "Geometric BF 4-layer padding-scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=false  --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=GBF-PAD4-UNRL.trace" 2> GBF-PAD4-UNRL.log
# echo "Geometric BF 4-layer padding-scheduler HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=true --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=GBF-PAD4-HARQ.trace" 2> GBF-PAD4-HARQ.log
#
# echo "Codebook BF 4-layer padding-scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=false --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=ABF-PAD4-UNRL.trace" 2> ABF-PAD4-UNRL.log
# echo "Codebook BF 4-layer padding-scheduler HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=true --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=ABF-PAD4-HARQ.trace" 2> ABF-PAD4-HARQ.log
#
# echo "Single-subcarrier MMSE BF 4-layer padding-scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSEBeamforming --harq=false --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=HBF-PAD4-UNRL.trace" 2> HBF-PAD4-UNRL.log
# echo "Single-subcarrier MMSE BF 4-layer padding-scheduler HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSEBeamforming --harq=true --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=HBF-PAD4-HARQ.trace" 2> HBF-PAD4-HARQ.log
#
# echo "Full-spectrum MMSE B 4-layer padding-scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=false --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=SBF-PAD4-UNRL.trace" 2> SBF-PAD4-UNRL.log
# echo "Full-spectrum MMSE BF 4-layer padding-scheduler HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=true --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=SBF-PAD4-HARQ.trace" 2> SBF-PAD4-HARQ.log

################################################################################
# 4-layer HBF MIMO SDMA with asynchronous Scheduler

#TODO: The asynchronous scheduling scheme does not guarantee that slotbundles work well in MMSE, but in many particular cases it does work

# # echo "Geometric BF 4-layer paddingasynchronous-scheduler noHARQ"
# # time ./waf --run "mmwave-hbf --sched=ns3::MmWaveAsyncHbfMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=false  --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=GBF-ASC4-UNRL.trace" 2> GBF-ASC4-UNRL.log
# # echo "Geometric BF 4-layer asynchronous-scheduler HARQ"
# # time ./waf --run "mmwave-hbf --sched=ns3::MmWaveAsyncHbfMacScheduler --bfmod=ns3::MmWaveDftBeamforming --harq=true --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=GBF-ASC4-HARQ.trace" 2> GBF-ASC4-HARQ.log
#
# echo "Codebook BF 4-layer asynchronous-scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveAsyncHbfMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=false --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=ABF-ASC4-UNRL.trace" 2> ABF-ASC4-UNRL.log
# echo "Codebook BF 4-layer asynchronous-scheduler HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveAsyncHbfMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=true --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=ABF-ASC4-HARQ.trace" 2> ABF-ASC4-HARQ.log
#
# # echo "Single-subcarrier MMSE BF 4-layer asynchronous-scheduler noHARQ"
# # time ./waf --run "mmwave-hbf --sched=ns3::MmWaveAsyncHbfMacScheduler --bfmod=ns3::MmWaveMMSEBeamforming --harq=false --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=HBF-ASC4-UNRL.trace" 2> HBF-ASC4-UNRL.log
# # echo "Single-subcarrier MMSE BF 4-layer asynchronous-scheduler HARQ"
# # time ./waf --run "mmwave-hbf --sched=ns3::MmWaveAsyncHbfMacScheduler --bfmod=ns3::MmWaveMMSEBeamforming --harq=true --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=HBF-ASC4-HARQ.trace" 2> HBF-ASC4-HARQ.log
#
# echo "Full-spectrum MMSE B 4-layer asynchronous-scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveAsyncHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=false --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=SBF-ASC4-UNRL.trace" 2> SBF-ASC4-UNRL.log
# echo "Full-spectrum MMSE BF 4-layer asynchronous-scheduler HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveAsyncHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=true --nLayers=4  --ns3::MmWavePhyRxTrace::OutputFilename=SBF-ASC4-HARQ.trace" 2> SBF-ASC4-HARQ.log

################################################################################
# TCP tests

# echo "TCP Codebook BF previous SISO scheduler  noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=false --useTCP=true --ns3::MmWavePhyRxTrace::OutputFilename=TCP-ABF-PREV-UNRL.trace" 2> TCP-ABF-PREV-UNRL.log
# echo "TCP Codebook BF previous SISO scheduler  HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=true --useTCP=true --ns3::MmWavePhyRxTrace::OutputFilename=TCP-ABF-PREV-HARQ.trace" 2> TCP-ABF-PREV-HARQ.log
#
# echo "TCP Full-spectrum MMSE B 4-layer padding-scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=false --nLayers=4 --useTCP=true --ns3::MmWavePhyRxTrace::OutputFilename=TCP-SBF-PAD4-UNRL.trace" 2> TCP-SBF-PAD4-UNRL.log
# echo "TCP Full-spectrum MMSE BF 4-layer padding-scheduler HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=true --nLayers=4 --useTCP=true --ns3::MmWavePhyRxTrace::OutputFilename=TCP-SBF-PAD4-HARQ.trace" 2> TCP-SBF-PAD4-HARQ.log
################################################################################
# MMTC tests

# echo "MMTC Codebook BF previous SISO scheduler  noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=false --numUe=20 --ns3::MmWavePhyRxTrace::OutputFilename=MMTC-ABF-PREV-UNRL.trace" 2> MMTC-ABF-PREV-UNRL.log
# echo "MMTC Codebook BF previous SISO scheduler  HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=true --numUe=20 --ns3::MmWavePhyRxTrace::OutputFilename=MMTC-ABF-PREV-HARQ.trace" 2> MMTC-ABF-PREV-HARQ.log
#
# echo "MMTC Full-spectrum MMSE B 4-layer padding-scheduler noHARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=false --nLayers=4 --numUe=20 --ns3::MmWavePhyRxTrace::OutputFilename=MMTC-SBF-PAD4-UNRL.trace" 2> MMTC-SBF-PAD4-UNRL.log
# echo "MMTC Full-spectrum MMSE BF 4-layer padding-scheduler HARQ"
# time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=true --nLayers=4 --numUe=20 --ns3::MmWavePhyRxTrace::OutputFilename=MMTC-SBF-PAD4-HARQ.trace" 2> MMTC-SBF-PAD4-HARQ.log
################################################################################
# High Throughput tests

echo "HighThroughput Codebook BF previous SISO scheduler  noHARQ"
time ./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=false --interPacketInterval=1000 --ns3::MmWavePhyRxTrace::OutputFilename=FAST-ABF-PREV-UNRL.trace" 2> FAST-ABF-PREV-UNRL.log
echo "HighThroughput Codebook BF previous SISO scheduler  HARQ"
time ./waf --run "mmwave-hbf --sched=ns3::MmWaveFlexTtiMacScheduler --bfmod=ns3::MmWaveFFTCodebookBeamforming --harq=true --interPacketInterval=1000 --ns3::MmWavePhyRxTrace::OutputFilename=FAST-ABF-PREV-HARQ.trace" 2> FAST-ABF-PREV-HARQ.log

echo "HighThroughput Full-spectrum MMSE B 4-layer padding-scheduler noHARQ"
time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=false --nLayers=4 --interPacketInterval=1000 --ns3::MmWavePhyRxTrace::OutputFilename=FAST-SBF-PAD4-UNRL.trace" 2> FAST-SBF-PAD4-UNRL.log
echo "HighThroughput Full-spectrum MMSE BF 4-layer padding-scheduler HARQ"
time ./waf --run "mmwave-hbf --sched=ns3::MmWavePaddedHbfMacScheduler --bfmod=ns3::MmWaveMMSESpectrumBeamforming --harq=true --nLayers=4 --interPacketInterval=1000 --ns3::MmWavePhyRxTrace::OutputFilename=FAST-SBF-PAD4-HARQ.trace" 2> FAST-SBF-PAD4-HARQ.log
