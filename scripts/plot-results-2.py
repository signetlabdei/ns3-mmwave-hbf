import sem
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

script = 'mmwave-hbf'
ns_path = '../ns3-mmwave-hbf'
campaign_dir = "./campaign/"

nruns = 20
n_bins = 100;

campaign = sem.CampaignManager.load(campaign_dir, runner_type = "ParallelRunner", check_repo = False)

def get_sinr_data (params):
    sinr_dl_data = pd.Series ()
    sinr_ul_data = pd.Series ()
    bler_ul_data = pd.Series ()
    bler_dl_data = pd.Series ()
    
    for r in campaign.db.get_results (params):
        available_files = campaign.db.get_result_files (r)
        
        data = pd.read_csv (available_files ['RxPacketTrace.txt'], delimiter = "\t", index_col=False, usecols = [0, 7, 10, 12, 14], names = ['mode', 'rnti', 'mcs', 'sinr', 'tbler'], 
                            dtype = {'mode' : 'object', 'rnti' : 'int64', 'mcs' : 'int64', 'sinr' : 'float64', 'tbler' : 'float64'}, engine='python', header=0)
        
        sinr_run_ul = data.loc [data ['mode'] == 'UL'] ['sinr'].replace ([np.inf, -np.inf], np.nan).dropna (how="all")
        sinr_ul_data = sinr_ul_data.append (sinr_run_ul, ignore_index=True)

        sinr_run_dl = data.loc [data ['mode'] == 'DL'] ['sinr'].replace ([np.inf, -np.inf], np.nan).dropna (how="all")
        sinr_dl_data = sinr_dl_data.append (sinr_run_dl, ignore_index=True)
                
        bler_ul_data = bler_ul_data.append (data.loc [data ['mode'] == 'UL'] ['tbler'])
        bler_dl_data = bler_dl_data.append (data.loc [data ['mode'] == 'DL'] ['tbler'])
    
    return (sinr_ul_data, sinr_dl_data, bler_ul_data, bler_dl_data)
    
def get_delay_data (params):
    delay_ul_data = pd.Series ()
    delay_dl_data = pd.Series ()
    
    for r in campaign.db.get_results (params):
        available_files = campaign.db.get_result_files (r)
        
        data_ul = pd.read_csv (available_files ['UlPdcpStats.txt'], delimiter = " ", index_col=False, usecols = [0, 1, 4, 5, 6], names = ['mode', 'time', 'drbid', 'size', 'delay'], engine='python', header=0)
        
        delay_run_ul = data_ul.loc [(data_ul ['mode'] == 'Rx') & (data_ul ['drbid'] >= 3)] ['delay'].replace ([np.inf, -np.inf], np.nan).dropna (how="all")
        delay_ul_data = delay_ul_data.append (delay_run_ul, ignore_index=True)
        
        data_dl = pd.read_csv (available_files ['DlPdcpStats.txt'], delimiter = " ", index_col=False, usecols = [0, 1, 4, 5, 6], names = ['mode', 'time', 'drbid', 'size', 'delay'], engine='python', header=0)
        
        delay_run_dl = data_dl.loc [(data_dl ['mode'] == 'Rx') & (data_dl ['drbid'] >= 3)] ['delay'].replace ([np.inf, -np.inf], np.nan).dropna (how="all")
        delay_dl_data = delay_dl_data.append (delay_run_dl, ignore_index=True)
        
        return (delay_ul_data, delay_dl_data)


ipiList = [150, 1500]
harqList = [False, True]
rlcAmList = [False, True]

for rlcAm in rlcAmList:
    for ipi in ipiList:
        for harq in harqList:
            
            title = 'rlcAm=' + str(rlcAm) + '_interPacketInterval=' + str (ipi) + "_harq=" + str (harq)
            
            fig_sinr_ul = plt.figure (1)
            fig_sinr_ul.suptitle('SINR UL ' + title)
            fig_sinr_dl = plt.figure (2)
            fig_sinr_dl.suptitle('SINR DL ' + title)
            fig_bler_ul = plt.figure (3)
            fig_bler_ul.suptitle('BLER UL ' + title)
            fig_bler_dl = plt.figure (4)
            fig_bler_dl.suptitle('BLER DL ' + title)
            fig_delay_ul = plt.figure (5)
            fig_delay_ul.suptitle('DELAY UL ' + title)
            fig_delay_dl = plt.figure (6)
            fig_delay_dl.suptitle('DELAY DL ' + title)

            
            for bfmod in ['ns3::MmWaveDftBeamforming', 'ns3::MmWaveFFTCodebookBeamforming']:    
                
                params = {
                'RngRun' : list (range (nruns)),
                'numEnb' : 1,
                'numUe' : 7,
                'simTime' : 1.2,
                'interPacketInterval' : ipi,
                'harq' : harq,
                'rlcAm' : rlcAm,
                'fixedTti' : False,
                'sched' : 'ns3::MmWaveFlexTtiMacScheduler',
                'bfmod' : bfmod,
                'nLayers' : 1,
                'useTCP' : False
                }
                
                (sinr_ul_data, sinr_dl_data, bler_ul_data, bler_dl_data) = get_sinr_data (params)
                (delay_ul_data, delay_dl_data) = get_delay_data (params)
                
                plt.figure (1)
                n, bins, patches = plt.hist(sinr_ul_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 1 layer')
                plt.figure (2)
                n, bins, patches = plt.hist(sinr_dl_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 1 layer')
                                            
                plt.figure (3)
                n, bins, patches = plt.hist(bler_ul_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 1 layer')
                plt.figure (4)
                n, bins, patches = plt.hist(bler_dl_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 1 layer')
                
                plt.figure (5)
                n, bins, patches = plt.hist(delay_ul_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 1 layer')
                
                plt.figure (6)
                n, bins, patches = plt.hist(delay_dl_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 1 layer')
                                            
            for bfmod in ['ns3::MmWaveDftBeamforming', 'ns3::MmWaveFFTCodebookBeamforming', 'ns3::MmWaveMMSEBeamforming', 'ns3::MmWaveMMSESpectrumBeamforming']:    
                
                params = {
                'RngRun' : list (range (nruns)),
                'numEnb' : 1,
                'numUe' : 7,
                'simTime' : 1.2,
                'interPacketInterval' : ipi,
                'harq' : harq,
                'rlcAm' : rlcAm,
                'fixedTti' : False,
                'sched' : 'ns3::MmWavePaddedHbfMacScheduler',
                'bfmod' : bfmod,
                'nLayers' : 4,
                'useTCP' : False
                }
                
                (sinr_ul_data, sinr_dl_data, bler_ul_data, bler_dl_data) = get_sinr_data (params)
                (delay_ul_data, delay_dl_data) = get_delay_data (params)
                
                plt.figure (1)
                n, bins, patches = plt.hist(sinr_ul_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 4 layer')
                plt.figure (2)
                n, bins, patches = plt.hist(sinr_dl_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 4 layer')

                plt.figure (3)
                n, bins, patches = plt.hist(bler_ul_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 4 layer')
                plt.figure (4)
                n, bins, patches = plt.hist(bler_dl_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 4 layer')
                plt.figure (5)
                n, bins, patches = plt.hist(delay_ul_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 4 layer')
                plt.figure (6)
                n, bins, patches = plt.hist(delay_dl_data, n_bins, density=True, histtype='step',
                                            cumulative=True, label=bfmod+' 4 layer')
                                            
            fig_sinr_ul.legend (loc='center right')
            fig_sinr_ul.savefig ('cdf_sinr_ul_'+title+'.png', bbox_inches='tight')
            plt.close (fig_sinr_ul)
            
            fig_sinr_dl.legend (loc='center right')
            fig_sinr_dl.savefig ('cdf_sinr_dl_'+title+'.png', bbox_inches='tight')
            plt.close (fig_sinr_dl)
            
            fig_bler_ul.legend (loc='center right')
            fig_bler_ul.savefig ('cdf_bler_ul_'+title+'.png', bbox_inches='tight')
            plt.close (fig_bler_ul)
            
            fig_bler_dl.legend (loc='center right')
            fig_bler_dl.savefig ('cdf_bler_dl_'+title+'.png', bbox_inches='tight')
            plt.close (fig_bler_dl)
            
            fig_delay_ul.legend (loc='center right')
            fig_delay_ul.savefig ('cdf_delay_ul_'+title+'.png', bbox_inches='tight')
            plt.close (fig_delay_ul)
            
            fig_delay_dl.legend (loc='center right')
            fig_delay_dl.savefig ('cdf_delay_dl_'+title+'.png', bbox_inches='tight')
            plt.close (fig_delay_dl)
                
                
