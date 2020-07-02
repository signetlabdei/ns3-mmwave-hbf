import sem
import numpy as np
import pandas as pd
import math

sem.parallelrunner.MAX_PARALLEL_PROCESSES = 32

script = 'mmwave-hbf'
ns_path = '../ns3-mmwave-hbf'
campaign_dir = "./campaign-2/"

nruns = 20

###############################################################################

# Returns the average bler and sinr for UL and DL
def compute_phy_metrics (rxPacketTraceFile):
    data = pd.read_csv (rxPacketTraceFile, delimiter = "\t", index_col=False, usecols = [0, 7, 10, 12, 14], names = ['mode', 'rnti', 'mcs', 'sinr', 'tbler'], 
                        dtype = {'mode' : 'object', 'rnti' : 'int64', 'mcs' : 'int64', 'sinr' : 'float64', 'tbler' : 'float64'}, engine='python', header=0)
    
    sinrUlLinear = data.where ((data ['mode'] == 'UL')) ['sinr'].dropna ().apply (lambda x : pow (10.0, x/10.0))
    avgSinrUl = 10.0*math.log10 (sinrUlLinear.mean ())
    
    sinrDlLinear = data.where ((data ['mode'] == 'DL')) ['sinr'].dropna ().apply (lambda x : pow (10.0, x/10.0))
    avgSinrDl = 10.0*math.log10 (sinrDlLinear.mean ())
    
    avgBlerUl = data.where ((data ['mode'] == 'UL')) ['tbler'].mean ()
    avgBlerDl = data.where ((data ['mode'] == 'DL')) ['tbler'].mean ()

    return avgSinrUl, avgSinrDl, avgBlerUl, avgBlerDl

# Returns the average delay in ns and the amount of received data in bytes
def compute_pdcp_metrics (pdcpTraceFile):
    data = pd.read_csv (pdcpTraceFile, delimiter = " ", index_col=False, usecols = [0, 1, 4, 5, 6], names = ['mode', 'time', 'drbid', 'size', 'delay'], engine='python', header=0)
    
    txData = data.where ((data ['mode'] == 'Tx') & (data ['drbid'] >= 3)) ['size'].sum (axis=0)
    rxData = data.where ((data ['mode'] == 'Rx') & (data ['drbid'] >= 3)) ['size'].sum (axis=0)
    avgDelay = data.where ((data ['mode'] == 'Rx') & (data ['drbid'] >= 3)) ['delay'].mean (axis=0)

    return txData, rxData, avgDelay

###############################################################################

campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, check_repo=False, overwrite=False, runner_type = "ParallelRunner")

bf_comparison_sigle_layer = {
'RngRun' : list (range (nruns)),
'numEnb' : 1,
'numUe' : 7,
'simTime' : 1.2,
'interPacketInterval' : [150, 1500],
'harq' : [False, True],
'rlcAm' : [True, False],
'fixedTti' : False,
'sched' : 'ns3::MmWaveFlexTtiMacScheduler',
'bfmod' : ['ns3::MmWaveDftBeamforming', 'ns3::MmWaveFFTCodebookBeamforming'],
'nLayers' : 1,
'useTCP' : False
}

bf_comparison_multi_layer = {
'RngRun' : list (range (nruns)),
'numEnb' : 1,
'numUe' : 7,
'simTime' : 1.2,
'interPacketInterval' : [150, 1500],
'harq' : [False, True],
'rlcAm' : [True, False],
'fixedTti' : False,
'sched' : 'ns3::MmWavePaddedHbfMacScheduler',
'bfmod' : ['ns3::MmWaveDftBeamforming', 'ns3::MmWaveFFTCodebookBeamforming', 'ns3::MmWaveMMSEBeamforming', 'ns3::MmWaveMMSESpectrumBeamforming'],
'nLayers' : 4,
'useTCP' : False
}

tcp_sched_comparison_sigle_layer = {
'RngRun' : list (range (nruns)),
'numEnb' : 1,
'numUe' : 7,
'simTime' : 1.2,
'interPacketInterval' : 0, # not used in tcp app
'harq' : [False, True],
'rlcAm' : [True, False],
'fixedTti' : False,
'sched' : ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler'],
'bfmod' : 'ns3::MmWaveDftBeamforming',
'nLayers' : 1,
'useTCP' : True
}

tcp_sched_comparison_multi_layer = {
'RngRun' : list (range (nruns)),
'numEnb' : 1,
'numUe' : 7,
'simTime' : 1.2,
'interPacketInterval' : 0, # not used in tcp app
'harq' : [False, True],
'rlcAm' : [True, False],
'fixedTti' : False,
'sched' : ['ns3::MmWaveAsyncHbfMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler'],
'bfmod' : 'ns3::MmWaveMMSESpectrumBeamforming',
'nLayers' : 4,
'useTCP' : True
}

udp_sched_comparison_sigle_layer = {
'RngRun' : list (range (nruns)),
'numEnb' : 1,
'numUe' : 7,
'simTime' : 1.2,
'interPacketInterval' : [150, 1500],
'harq' : [False, True],
'rlcAm' : [True, False],
'fixedTti' : False,
'sched' : ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler'],
'bfmod' : 'ns3::MmWaveDftBeamforming',
'nLayers' : 1,
'useTCP' : False
}

udp_sched_comparison_multi_layer = {
'RngRun' : list (range (nruns)),
'numEnb' : 1,
'numUe' : 7,
'simTime' : 1.2,
'interPacketInterval' : [150, 1500],
'harq' : [False, True],
'rlcAm' : [True, False],
'fixedTti' : False,
'sched' : ['ns3::MmWaveAsyncHbfMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler'],
'bfmod' : 'ns3::MmWaveMMSESpectrumBeamforming',
'nLayers' : 4,
'useTCP' : False
}

campaign.run_missing_simulations(sem.list_param_combinations(bf_comparison_sigle_layer))
campaign.run_missing_simulations(sem.list_param_combinations(bf_comparison_multi_layer))
campaign.run_missing_simulations(sem.list_param_combinations(tcp_sched_comparison_sigle_layer))
campaign.run_missing_simulations(sem.list_param_combinations(tcp_sched_comparison_multi_layer))
campaign.run_missing_simulations(sem.list_param_combinations(udp_sched_comparison_sigle_layer))
campaign.run_missing_simulations(sem.list_param_combinations(udp_sched_comparison_multi_layer))

campaign = sem.CampaignManager.load(campaign_dir, runner_type = "ParallelRunner", check_repo = False)

# create a data frame to store the results of each simulation
metrics = ['avgSinrUl', 'avgSinrDl', 'avgBlerUl', 'avgBlerDl', 'ulTxPdcpData', 'ulRxPdcpData', 'ulPdcpDelay', 'dlTxPdcpData', 'dlRxPdcpData', 'dlPdcpDelay']
columns = list (bf_comparison_sigle_layer.keys ()) + metrics
result_df = pd.DataFrame (columns = columns)

for r in campaign.db.get_results ():

    # get the output files
    available_files = campaign.db.get_result_files (r)

    # compute the metrics of interest
    (avgSinrUl, avgSinrDl, avgBlerUl, avgBlerDl) = compute_phy_metrics (available_files ['RxPacketTrace.txt'])
    (ulTxPdcpData, ulRxPdcpData, ulPdcpDelay) = compute_pdcp_metrics (available_files ['UlPdcpStats.txt'])
    (dlTxPdcpData, dlRxPdcpData, dlPdcpDelay) = compute_pdcp_metrics (available_files ['DlPdcpStats.txt'])

    # insert a new row in the result df
    new_row = r['params']
    new_row.update ({'avgSinrUl' : avgSinrUl, 'avgSinrDl' : avgSinrDl,
                     'avgBlerUl' : avgBlerUl, 'avgBlerDl' : avgBlerDl,
                     'ulTxPdcpData' : ulTxPdcpData, 'ulRxPdcpData' : ulRxPdcpData, 'ulPdcpDelay' : ulPdcpDelay,
                     'dlTxPdcpData' : dlTxPdcpData, 'dlRxPdcpData' : dlRxPdcpData, 'dlPdcpDelay' : dlPdcpDelay
                    })
    print (new_row)

    result_df = result_df.append (new_row, ignore_index=True)

result_df.to_csv (campaign_dir + '/parsed_results.csv', encoding='utf-8', index=False)
