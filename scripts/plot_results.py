import pandas as pd
import numpy as np
import numpy.ma as ma
import matplotlib.pyplot as plt
import math
import sem
import tikzplotlib

def get_std_err (x):
    std = np.nanstd (x) / math.sqrt(x.size - np.isnan(x).sum ())
    return std

def compute_prr (rx, tx):
    prr_runs = np.divide (rx, tx)
    mx_prr_runs = ma.masked_invalid (prr_runs, copy=False)
    return (mx_prr_runs.mean (), get_std_err (prr_runs))


def plot_bf_comparison (csv_path, figure_folder):
    
    results_df = pd.read_csv (csv_path)
    # plot the results

    # bf comparison
    # 
    # bf_comparison_sigle_layer = {
    # 'RngRun' : list (range (nruns)),
    # 'numEnb' : 1,
    # 'numUe' : 7,
    # 'simTime' : 1.2,
    # 'interPacketInterval' : [150, 1500],
    # 'harq' : [False, True],
    # 'rlcAm' : True,
    # 'fixedTti' : False,
    # 'sched' : 'ns3::MmWaveFlexTtiMacScheduler',
    # 'bfmod' : ['ns3::MmWaveDftBeamforming', 'ns3::MmWaveFFTCodebookBeamforming'],
    # 'nLayers' : 1,
    # 'useTCP' : False
    # }

    for rlcAm in [True, False]:
        for interPacketInterval in [150, 1500]:
            for harq in [False, True]:
                title = 'rlcAm=' + str (rlcAm) + '_interPacketInterval=' + str (interPacketInterval) + "_harq=" + str (harq)
                
                fig_sinr_ul, ax_sinr_ul = plt.subplots(1, 2)
                fig_sinr_ul.suptitle('SINR UL ' + title)
                fig_sinr_dl, ax_sinr_dl = plt.subplots(1, 2)
                fig_sinr_dl.suptitle('SINR DL ' + title)
                fig_bler_ul, ax_bler_ul = plt.subplots(1, 2)
                fig_bler_ul.suptitle('BLER UL ' + title)
                fig_bler_dl, ax_bler_dl = plt.subplots(1, 2)
                fig_bler_dl.suptitle('BLER DL ' + title)
                fig_delay_ul, ax_delay_ul = plt.subplots(1, 2)
                fig_delay_ul.suptitle('DELAY UL ' + title)
                fig_delay_dl, ax_delay_dl = plt.subplots(1, 2)
                fig_delay_dl.suptitle('DELAY DL ' + title)
                fig_prr_ul, ax_prr_ul = plt.subplots(1, 2)
                fig_prr_ul.suptitle('PRR UL ' + title)
                fig_prr_dl, ax_prr_dl = plt.subplots(1, 2)
                fig_prr_dl.suptitle('PRR DL ' + title)
                
                plt.setp(ax_sinr_ul, ylim=[0, 50])
                plt.setp(ax_sinr_dl, ylim=[0, 50])
                plt.setp(ax_bler_ul, ylim=[0, 0.5])
                plt.setp(ax_bler_dl, ylim=[0, 0.5])
                # plt.setp(ax_delay_ul, ylim=[0, 5])
                # plt.setp(ax_delay_dl, ylim=[0, 5])
                plt.setp(ax_prr_ul, ylim=[0, 1.1])
                plt.setp(ax_prr_dl, ylim=[0, 1.1])
                
                
                # single layer
                x = list (results_df ['bfmod'].unique ())
                y_sinr_ul = np.full ((len (x)), np.nan)
                y_sinr_dl = np.full ((len (x)), np.nan)
                y_bler_ul = np.full ((len (x)), np.nan)
                y_bler_dl = np.full ((len (x)), np.nan)
                y_delay_ul = np.full ((len (x)), np.nan)
                y_delay_dl = np.full ((len (x)), np.nan)
                y_prr_ul = np.full ((len (x)), np.nan)
                y_prr_dl = np.full ((len (x)), np.nan)
                
                y_err_sinr_ul = np.full ((len (x)), np.nan)
                y_err_sinr_dl = np.full ((len (x)), np.nan)
                y_err_bler_ul = np.full ((len (x)), np.nan)
                y_err_bler_dl = np.full ((len (x)), np.nan)
                y_err_delay_ul = np.full ((len (x)), np.nan)
                y_err_delay_dl = np.full ((len (x)), np.nan)
                y_err_prr_ul = np.full ((len (x)), np.nan)
                y_err_prr_dl = np.full ((len (x)), np.nan)
                
                for bfmod in ['ns3::MmWaveDftBeamforming', 'ns3::MmWaveFFTCodebookBeamforming']:
                    data = results_df.loc [(results_df ['numEnb'] == 1) & 
                                          (results_df ['numUe'] == 7) &
                                          (results_df ['simTime'] == 1.2) &
                                          (results_df ['interPacketInterval'] == interPacketInterval) &
                                          (results_df ['harq'] == harq) &
                                          (results_df ['rlcAm'] == rlcAm) &
                                          (results_df ['fixedTti'] == False) &
                                          (results_df ['sched'] == 'ns3::MmWaveFlexTtiMacScheduler') &
                                          (results_df ['bfmod'] == bfmod) &
                                          (results_df ['nLayers'] == 1) &
                                          (results_df ['useTCP'] == False)]
                            
                    y_sinr_ul [x.index (bfmod)] = data ['avgSinrUl'].mean ()
                    y_sinr_dl [x.index (bfmod)] = data ['avgSinrDl'].mean ()
                    y_bler_ul [x.index (bfmod)] = data ['avgBlerUl'].mean ()
                    y_bler_dl [x.index (bfmod)] = data ['avgBlerDl'].mean ()
                    y_delay_ul [x.index (bfmod)] = data ['ulPdcpDelay'].mean ()
                    y_delay_dl [x.index (bfmod)] = data ['dlPdcpDelay'].mean ()
                            
                    y_err_sinr_ul [x.index (bfmod)] = get_std_err (data ['avgSinrUl'])
                    y_err_sinr_dl [x.index (bfmod)] = get_std_err (data ['avgSinrDl'])
                    y_err_bler_ul [x.index (bfmod)] = get_std_err (data ['avgBlerUl'])
                    y_err_bler_dl [x.index (bfmod)] = get_std_err (data ['avgBlerDl'])
                    y_err_delay_ul [x.index (bfmod)] = get_std_err (data ['ulPdcpDelay'])
                    y_err_delay_dl [x.index (bfmod)] = get_std_err (data ['dlPdcpDelay'])
                    
                    (y_prr_ul [x.index (bfmod)], y_err_prr_ul [x.index (bfmod)]) = compute_prr (data ['ulRxPdcpData'].values, data['ulTxPdcpData'].values)
                    (y_prr_dl [x.index (bfmod)], y_err_prr_dl [x.index (bfmod)]) = compute_prr (data ['dlRxPdcpData'].values, data['dlTxPdcpData'].values)
                
                ax_sinr_ul [0].bar (range (len (x)), y_sinr_ul, tick_label=x, yerr=y_err_sinr_ul)
                ax_sinr_dl [0].bar (range (len (x)), y_sinr_dl, tick_label=x, yerr=y_err_sinr_dl)
                ax_bler_ul [0].bar (range (len (x)), y_bler_ul, tick_label=x, yerr=y_err_bler_ul)
                ax_bler_dl [0].bar (range (len (x)), y_bler_dl, tick_label=x, yerr=y_err_bler_dl)
                ax_delay_ul [0].bar (range (len (x)), y_delay_ul/1e6, tick_label=x, yerr=y_err_delay_ul/1e6)
                ax_delay_dl [0].bar (range (len (x)), y_delay_dl/1e6, tick_label=x, yerr=y_err_delay_dl/1e6)
                ax_prr_ul [0].bar (range (len (x)), y_prr_ul, tick_label=x, yerr=y_err_prr_ul)
                ax_prr_dl [0].bar (range (len (x)), y_prr_dl, tick_label=x, yerr=y_err_prr_dl)
                
                ax_sinr_ul [0].tick_params(axis='x', labelrotation=90)
                ax_sinr_dl [0].tick_params(axis='x', labelrotation=90)
                ax_bler_ul [0].tick_params(axis='x', labelrotation=90)
                ax_bler_dl [0].tick_params(axis='x', labelrotation=90)
                ax_delay_ul [0].tick_params(axis='x', labelrotation=90)
                ax_delay_dl [0].tick_params(axis='x', labelrotation=90)
                ax_prr_ul [0].tick_params(axis='x', labelrotation=90)
                ax_prr_dl [0].tick_params(axis='x', labelrotation=90)
                
                ax_sinr_ul [0].set (ylabel='SINR [dB]')
                ax_sinr_dl [0].set (ylabel='SINR [dB]')
                ax_bler_ul [0].set (ylabel='BLER')
                ax_bler_dl [0].set (ylabel='BLER')
                ax_delay_ul [0].set (ylabel='delay [ms]')
                ax_delay_dl [0].set (ylabel='delay [ms]')
                ax_prr_ul [0].set (ylabel='PRR')
                ax_prr_dl [0].set (ylabel='PRR')
                
                # multi layer
                x = list (results_df ['bfmod'].unique ())
                y_sinr_ul = np.full ((len (x)), np.nan)
                y_sinr_dl = np.full ((len (x)), np.nan)
                y_bler_ul = np.full ((len (x)), np.nan)
                y_bler_dl = np.full ((len (x)), np.nan)
                y_delay_ul = np.full ((len (x)), np.nan)
                y_delay_dl = np.full ((len (x)), np.nan)
                y_prr_ul = np.full ((len (x)), np.nan)
                y_prr_dl = np.full ((len (x)), np.nan)
                
                y_err_sinr_ul = np.full ((len (x)), np.nan)
                y_err_sinr_dl = np.full ((len (x)), np.nan)
                y_err_bler_ul = np.full ((len (x)), np.nan)
                y_err_bler_dl = np.full ((len (x)), np.nan)
                y_err_delay_ul = np.full ((len (x)), np.nan)
                y_err_delay_dl = np.full ((len (x)), np.nan)
                y_err_prr_ul = np.full ((len (x)), np.nan)
                y_err_prr_dl = np.full ((len (x)), np.nan)
                
                for bfmod in ['ns3::MmWaveDftBeamforming', 'ns3::MmWaveFFTCodebookBeamforming', 'ns3::MmWaveMMSEBeamforming', 'ns3::MmWaveMMSESpectrumBeamforming']:
                    data = results_df.loc [(results_df ['numEnb'] == 1) & 
                                          (results_df ['numUe'] == 7) &
                                          (results_df ['simTime'] == 1.2) &
                                          (results_df ['interPacketInterval'] == interPacketInterval) &
                                          (results_df ['harq'] == harq) &
                                          (results_df ['rlcAm'] == rlcAm) &
                                          (results_df ['fixedTti'] == False) &
                                          (results_df ['sched'] == 'ns3::MmWavePaddedHbfMacScheduler') &
                                          (results_df ['bfmod'] == bfmod) &
                                          (results_df ['nLayers'] == 4) &
                                          (results_df ['useTCP'] == False)]
                                          
                    y_sinr_ul [x.index (bfmod)] = data ['avgSinrUl'].mean ()
                    y_sinr_dl [x.index (bfmod)] = data ['avgSinrDl'].mean ()
                    y_bler_ul [x.index (bfmod)] = data ['avgBlerUl'].mean ()
                    y_bler_dl [x.index (bfmod)] = data ['avgBlerDl'].mean ()
                    y_delay_ul [x.index (bfmod)] = data ['ulPdcpDelay'].mean ()
                    y_delay_dl [x.index (bfmod)] = data ['dlPdcpDelay'].mean ()
                    
                    y_err_sinr_ul [x.index (bfmod)] = get_std_err (data ['avgSinrUl'])
                    y_err_sinr_dl [x.index (bfmod)] = get_std_err (data ['avgSinrDl'])
                    y_err_bler_ul [x.index (bfmod)] = get_std_err (data ['avgBlerUl'])
                    y_err_bler_dl [x.index (bfmod)] = get_std_err (data ['avgBlerDl'])
                    y_err_delay_ul [x.index (bfmod)] = get_std_err (data ['ulPdcpDelay'])
                    y_err_delay_dl [x.index (bfmod)] = get_std_err (data ['dlPdcpDelay'])
                    
                    (y_prr_ul [x.index (bfmod)], y_err_prr_ul [x.index (bfmod)]) = compute_prr (data ['ulRxPdcpData'].values, data['ulTxPdcpData'].values)
                    (y_prr_dl [x.index (bfmod)], y_err_prr_dl [x.index (bfmod)]) = compute_prr (data ['dlRxPdcpData'].values, data['dlTxPdcpData'].values)
                    
                ax_sinr_ul [1].bar (range (len (x)), y_sinr_ul, tick_label=x, yerr=y_err_sinr_ul)
                ax_sinr_dl [1].bar (range (len (x)), y_sinr_dl, tick_label=x, yerr=y_err_sinr_dl)
                ax_bler_ul [1].bar (range (len (x)), y_bler_ul, tick_label=x, yerr=y_err_bler_ul)
                ax_bler_dl [1].bar (range (len (x)), y_bler_dl, tick_label=x, yerr=y_err_bler_dl)
                ax_delay_ul [1].bar (range (len (x)), y_delay_ul/1e6, tick_label=x, yerr=y_err_delay_ul/1e6)
                ax_delay_dl [1].bar (range (len (x)), y_delay_dl/1e6, tick_label=x, yerr=y_err_delay_dl/1e6)
                ax_prr_ul [1].bar (range (len (x)), y_prr_ul, tick_label=x, yerr=y_err_prr_ul)
                ax_prr_dl [1].bar (range (len (x)), y_prr_dl, tick_label=x, yerr=y_err_prr_dl)
                
                ax_sinr_ul [1].tick_params(axis='x', labelrotation=90)
                ax_sinr_dl [1].tick_params(axis='x', labelrotation=90)
                ax_bler_ul [1].tick_params(axis='x', labelrotation=90)
                ax_bler_dl [1].tick_params(axis='x', labelrotation=90)
                ax_delay_ul [1].tick_params(axis='x', labelrotation=90)
                ax_delay_dl [1].tick_params(axis='x', labelrotation=90)
                ax_prr_ul [1].tick_params(axis='x', labelrotation=90)
                ax_prr_dl [1].tick_params(axis='x', labelrotation=90)
                
                fig_sinr_ul.savefig (figure_folder + 'sinr_ul_'+title+'.png', bbox_inches='tight')
                fig_sinr_dl.savefig (figure_folder + 'sinr_dl_'+title+'.png', bbox_inches='tight')
                fig_bler_ul.savefig (figure_folder + 'bler_ul_'+title+'.png', bbox_inches='tight')
                fig_bler_dl.savefig (figure_folder + 'bler_dl_'+title+'.png', bbox_inches='tight')
                fig_delay_ul.savefig (figure_folder + 'delay_ul_'+title+'.png', bbox_inches='tight')
                fig_delay_dl.savefig (figure_folder + 'delay_dl_'+title+'.png', bbox_inches='tight')
                fig_prr_ul.savefig (figure_folder + 'prr_ul_'+title+'.png', bbox_inches='tight')
                fig_prr_dl.savefig (figure_folder + 'prr_dl_'+title+'.png', bbox_inches='tight')
                

def get_sinr_data (campaign, params):
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
    
def get_delay_data (campaign, params):
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

def compute_ecdf (samples):
    # create x axis
    nbins = 100
    x = np.linspace (start=min (samples), stop=max (samples), num=nbins)

    # compute ecdf
    y = np.zeros (len (x))

    for i in range (0, len (x)):
        tmp = samples [samples <= x [i]]
        y [i] = len (tmp) / len (samples)
    return (x,y)


def plot_bf_cdfs (campaign_dir, nruns, n_bins, figure_folder):
    
    campaign = sem.CampaignManager.load(campaign_dir, runner_type = "ParallelRunner", check_repo = False)
    
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
                    
                    (sinr_ul_data, sinr_dl_data, bler_ul_data, bler_dl_data) = get_sinr_data (campaign, params)
                    (delay_ul_data, delay_dl_data) = get_delay_data (campaign, params)
                    
                    plt.figure (fig_sinr_ul.number)
                    (x, y) = compute_ecdf (sinr_ul_data)
                    plt.plot (x, y, label=bfmod+' 1 layer')
                    
                    plt.figure (fig_sinr_dl.number)
                    (x, y) = compute_ecdf (sinr_dl_data)
                    plt.plot (x, y, label=bfmod+' 1 layer')
                                                
                    plt.figure (fig_bler_ul.number)
                    (x, y) = compute_ecdf (bler_ul_data)
                    plt.plot (x, y, label=bfmod+' 1 layer')
                    
                    plt.figure (fig_bler_dl.number)
                    (x, y) = compute_ecdf (bler_dl_data)
                    plt.plot (x, y, label=bfmod+' 1 layer')
                    
                    plt.figure (fig_delay_ul.number)
                    (x, y) = compute_ecdf (delay_ul_data)
                    plt.plot (x, y, label=bfmod+' 1 layer')
                    
                    plt.figure (fig_delay_dl.number)
                    (x, y) = compute_ecdf (delay_dl_data)
                    plt.plot (x, y, label=bfmod+' 1 layer')
                                                
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
                    
                    (sinr_ul_data, sinr_dl_data, bler_ul_data, bler_dl_data) = get_sinr_data (campaign, params)
                    (delay_ul_data, delay_dl_data) = get_delay_data (campaign, params)
                    
                    plt.figure (fig_sinr_ul.number)
                    (x, y) = compute_ecdf (sinr_ul_data)
                    plt.plot (x, y, label=bfmod+' 4 layer')
                    
                    plt.figure (fig_sinr_dl.number)
                    (x, y) = compute_ecdf (sinr_dl_data)
                    plt.plot (x, y, label=bfmod+' 4 layer')
                    
                    plt.figure (fig_bler_ul.number)
                    (x, y) = compute_ecdf (bler_ul_data)
                    plt.plot (x, y, label=bfmod+' 4 layer')
                    
                    plt.figure (fig_bler_dl.number)
                    (x, y) = compute_ecdf (bler_dl_data)
                    plt.plot (x, y, label=bfmod+' 4 layer')
                    
                    plt.figure (fig_delay_ul.number)
                    (x, y) = compute_ecdf (delay_ul_data)
                    plt.plot (x, y, label=bfmod+' 4 layer')
                    
                    plt.figure (fig_delay_dl.number)
                    (x, y) = compute_ecdf (delay_dl_data)
                    plt.plot (x, y, label=bfmod+' 4 layer')
                                                
                fig_sinr_ul.legend (loc='center right')
                fig_sinr_ul.savefig (figure_folder + 'cdf_sinr_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_sinr_ul.number)
                tikzplotlib.save (figure_folder + 'cdf_sinr_ul_'+title+'.tex')
                plt.close (fig_sinr_ul)
                
                fig_sinr_dl.legend (loc='center right')
                fig_sinr_dl.savefig (figure_folder + 'cdf_sinr_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_sinr_dl.number)
                tikzplotlib.save (figure_folder + 'cdf_sinr_dl_'+title+'.tex')
                plt.close (fig_sinr_dl)
                
                fig_bler_ul.legend (loc='center right')
                fig_bler_ul.savefig (figure_folder + 'cdf_bler_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_bler_ul.number)
                tikzplotlib.save (figure_folder + 'cdf_bler_ul_'+title+'.tex')
                plt.close (fig_bler_ul)
                
                fig_bler_dl.legend (loc='center right')
                fig_bler_dl.savefig (figure_folder + 'cdf_bler_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_bler_dl.number)
                tikzplotlib.save (figure_folder + 'cdf_bler_dl_'+title+'.tex')
                plt.close (fig_bler_dl)
                
                fig_delay_ul.legend (loc='center right')
                fig_delay_ul.savefig (figure_folder + 'cdf_delay_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_delay_ul.number)
                tikzplotlib.save (figure_folder + 'cdf_delay_ul_'+title+'.tex')
                plt.close (fig_delay_ul)
                
                fig_delay_dl.legend (loc='center right')
                fig_delay_dl.savefig (figure_folder + 'cdf_delay_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_delay_dl.number)
                tikzplotlib.save (figure_folder + 'cdf_delay_dl_'+title+'.tex')
                plt.close (fig_delay_dl)
                
                
                
def plot_sched_comparison_tcp (csv_path, figure_folder):
    
    results_df = pd.read_csv (csv_path)
    # plot the results

    # sched comparison
    # 
    # tcp_sched_comparison_sigle_layer = {
    # 'RngRun' : list (range (nruns)),
    # 'numEnb' : 1,
    # 'numUe' : 7,
    # 'simTime' : 1.2,
    # 'interPacketInterval' : 0, # not used in tcp app
    # 'harq' : [False, True],
    # 'rlcAm' : [True, False],
    # 'fixedTti' : False,
    # 'sched' : ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler'],
    # 'bfmod' : 'ns3::MmWaveDftBeamforming',
    # 'nLayers' : 1,
    # 'useTCP' : True
    # }

    for rlcAm in [True, False]:
            for harq in [False, True]:
                title = 'rlcAm=' + str (rlcAm) + "_harq=" + str (harq)
                
                fig_sinr_ul, ax_sinr_ul = plt.subplots(1, 2)
                fig_sinr_ul.suptitle('SINR UL ' + title)
                fig_sinr_dl, ax_sinr_dl = plt.subplots(1, 2)
                fig_sinr_dl.suptitle('SINR DL ' + title)
                fig_bler_ul, ax_bler_ul = plt.subplots(1, 2)
                fig_bler_ul.suptitle('BLER UL ' + title)
                fig_bler_dl, ax_bler_dl = plt.subplots(1, 2)
                fig_bler_dl.suptitle('BLER DL ' + title)
                fig_delay_ul, ax_delay_ul = plt.subplots(1, 2)
                fig_delay_ul.suptitle('DELAY UL ' + title)
                fig_delay_dl, ax_delay_dl = plt.subplots(1, 2)
                fig_delay_dl.suptitle('DELAY DL ' + title)
                fig_dataRx_ul, ax_dataRx_ul = plt.subplots(1, 2)
                fig_dataRx_ul.suptitle('DATA RX UL ' + title)
                fig_dataRx_dl, ax_dataRx_dl = plt.subplots(1, 2)
                fig_dataRx_dl.suptitle('DATA RX DL ' + title)
                
                plt.setp(ax_sinr_ul, ylim=[0, 50])
                plt.setp(ax_sinr_dl, ylim=[0, 50])
                plt.setp(ax_bler_ul, ylim=[0, 0.5])
                plt.setp(ax_bler_dl, ylim=[0, 0.5])
                # plt.setp(ax_delay_ul, ylim=[0, 5])
                # plt.setp(ax_delay_dl, ylim=[0, 5])
                # plt.setp(ax_dataRx_ul, ylim=[0, 1.1])
                # plt.setp(ax_dataRx_dl, ylim=[0, 1.1])
                
                
                # single layer
                x = list (results_df ['sched'].unique ())
                y_sinr_ul = np.full ((len (x)), np.nan)
                y_sinr_dl = np.full ((len (x)), np.nan)
                y_bler_ul = np.full ((len (x)), np.nan)
                y_bler_dl = np.full ((len (x)), np.nan)
                y_delay_ul = np.full ((len (x)), np.nan)
                y_delay_dl = np.full ((len (x)), np.nan)
                y_dataRx_ul = np.full ((len (x)), np.nan)
                y_dataRx_dl = np.full ((len (x)), np.nan)
                
                y_err_sinr_ul = np.full ((len (x)), np.nan)
                y_err_sinr_dl = np.full ((len (x)), np.nan)
                y_err_bler_ul = np.full ((len (x)), np.nan)
                y_err_bler_dl = np.full ((len (x)), np.nan)
                y_err_delay_ul = np.full ((len (x)), np.nan)
                y_err_delay_dl = np.full ((len (x)), np.nan)
                y_err_dataRx_ul = np.full ((len (x)), np.nan)
                y_err_dataRx_dl = np.full ((len (x)), np.nan)
                
                for sched in ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:
                    data = results_df.loc [(results_df ['numEnb'] == 1) & 
                                          (results_df ['numUe'] == 7) &
                                          (results_df ['simTime'] == 1.2) &
                                          (results_df ['interPacketInterval'] == 0) &
                                          (results_df ['harq'] == harq) &
                                          (results_df ['rlcAm'] == rlcAm) &
                                          (results_df ['fixedTti'] == False) &
                                          (results_df ['sched'] == sched) &
                                          (results_df ['bfmod'] == 'ns3::MmWaveDftBeamforming') &
                                          (results_df ['nLayers'] == 1) &
                                          (results_df ['useTCP'] == True)]
                            
                    y_sinr_ul [x.index (sched)] = data ['avgSinrUl'].mean ()
                    y_sinr_dl [x.index (sched)] = data ['avgSinrDl'].mean ()
                    y_bler_ul [x.index (sched)] = data ['avgBlerUl'].mean ()
                    y_bler_dl [x.index (sched)] = data ['avgBlerDl'].mean ()
                    y_delay_ul [x.index (sched)] = data ['ulPdcpDelay'].mean ()
                    y_delay_dl [x.index (sched)] = data ['dlPdcpDelay'].mean ()
                            
                    y_err_sinr_ul [x.index (sched)] = get_std_err (data ['avgSinrUl'])
                    y_err_sinr_dl [x.index (sched)] = get_std_err (data ['avgSinrDl'])
                    y_err_bler_ul [x.index (sched)] = get_std_err (data ['avgBlerUl'])
                    y_err_bler_dl [x.index (sched)] = get_std_err (data ['avgBlerDl'])
                    y_err_delay_ul [x.index (sched)] = get_std_err (data ['ulPdcpDelay'])
                    y_err_delay_dl [x.index (sched)] = get_std_err (data ['dlPdcpDelay'])
                    
                    y_dataRx_ul [x.index (sched)] = data ['ulRxPdcpData'].mean ()
                    y_err_dataRx_ul [x.index (sched)] = get_std_err (data ['ulRxPdcpData'])
                    y_dataRx_dl [x.index (sched)] = data ['dlRxPdcpData'].mean ()
                    y_err_dataRx_dl [x.index (sched)] = get_std_err (data ['dlRxPdcpData'])
                
                ax_sinr_ul [0].bar (range (len (x)), y_sinr_ul, tick_label=x, yerr=y_err_sinr_ul)
                ax_sinr_dl [0].bar (range (len (x)), y_sinr_dl, tick_label=x, yerr=y_err_sinr_dl)
                ax_bler_ul [0].bar (range (len (x)), y_bler_ul, tick_label=x, yerr=y_err_bler_ul)
                ax_bler_dl [0].bar (range (len (x)), y_bler_dl, tick_label=x, yerr=y_err_bler_dl)
                ax_delay_ul [0].bar (range (len (x)), y_delay_ul/1e6, tick_label=x, yerr=y_err_delay_ul/1e6)
                ax_delay_dl [0].bar (range (len (x)), y_delay_dl/1e6, tick_label=x, yerr=y_err_delay_dl/1e6)
                ax_dataRx_ul [0].bar (range (len (x)), y_dataRx_ul, tick_label=x, yerr=y_err_dataRx_ul)
                ax_dataRx_dl [0].bar (range (len (x)), y_dataRx_dl, tick_label=x, yerr=y_err_dataRx_dl)
                
                ax_sinr_ul [0].tick_params(axis='x', labelrotation=90)
                ax_sinr_dl [0].tick_params(axis='x', labelrotation=90)
                ax_bler_ul [0].tick_params(axis='x', labelrotation=90)
                ax_bler_dl [0].tick_params(axis='x', labelrotation=90)
                ax_delay_ul [0].tick_params(axis='x', labelrotation=90)
                ax_delay_dl [0].tick_params(axis='x', labelrotation=90)
                ax_dataRx_ul [0].tick_params(axis='x', labelrotation=90)
                ax_dataRx_dl [0].tick_params(axis='x', labelrotation=90)
                
                ax_sinr_ul [0].set (ylabel='SINR [dB]')
                ax_sinr_dl [0].set (ylabel='SINR [dB]')
                ax_bler_ul [0].set (ylabel='BLER')
                ax_bler_dl [0].set (ylabel='BLER')
                ax_delay_ul [0].set (ylabel='delay [ms]')
                ax_delay_dl [0].set (ylabel='delay [ms]')
                ax_dataRx_ul [0].set (ylabel='PRR')
                ax_dataRx_dl [0].set (ylabel='PRR')
                
                # multi layer
                x = list (results_df ['sched'].unique ())
                y_sinr_ul = np.full ((len (x)), np.nan)
                y_sinr_dl = np.full ((len (x)), np.nan)
                y_bler_ul = np.full ((len (x)), np.nan)
                y_bler_dl = np.full ((len (x)), np.nan)
                y_delay_ul = np.full ((len (x)), np.nan)
                y_delay_dl = np.full ((len (x)), np.nan)
                y_dataRx_ul = np.full ((len (x)), np.nan)
                y_dataRx_dl = np.full ((len (x)), np.nan)
                
                y_err_sinr_ul = np.full ((len (x)), np.nan)
                y_err_sinr_dl = np.full ((len (x)), np.nan)
                y_err_bler_ul = np.full ((len (x)), np.nan)
                y_err_bler_dl = np.full ((len (x)), np.nan)
                y_err_delay_ul = np.full ((len (x)), np.nan)
                y_err_delay_dl = np.full ((len (x)), np.nan)
                y_err_dataRx_ul = np.full ((len (x)), np.nan)
                y_err_dataRx_dl = np.full ((len (x)), np.nan)
                
                for sched in ['ns3::MmWaveAsyncHbfMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:
                    data = results_df.loc [(results_df ['numEnb'] == 1) & 
                                          (results_df ['numUe'] == 7) &
                                          (results_df ['simTime'] == 1.2) &
                                          (results_df ['interPacketInterval'] == 0) &
                                          (results_df ['harq'] == harq) &
                                          (results_df ['rlcAm'] == rlcAm) &
                                          (results_df ['fixedTti'] == False) &
                                          (results_df ['sched'] == sched) &
                                          (results_df ['bfmod'] == 'ns3::MmWaveMMSESpectrumBeamforming') &
                                          (results_df ['nLayers'] == 4) &
                                          (results_df ['useTCP'] == True)]
                                          
                    y_sinr_ul [x.index (sched)] = data ['avgSinrUl'].mean ()
                    y_sinr_dl [x.index (sched)] = data ['avgSinrDl'].mean ()
                    y_bler_ul [x.index (sched)] = data ['avgBlerUl'].mean ()
                    y_bler_dl [x.index (sched)] = data ['avgBlerDl'].mean ()
                    y_delay_ul [x.index (sched)] = data ['ulPdcpDelay'].mean ()
                    y_delay_dl [x.index (sched)] = data ['dlPdcpDelay'].mean ()
                    
                    y_err_sinr_ul [x.index (sched)] = get_std_err (data ['avgSinrUl'])
                    y_err_sinr_dl [x.index (sched)] = get_std_err (data ['avgSinrDl'])
                    y_err_bler_ul [x.index (sched)] = get_std_err (data ['avgBlerUl'])
                    y_err_bler_dl [x.index (sched)] = get_std_err (data ['avgBlerDl'])
                    y_err_delay_ul [x.index (sched)] = get_std_err (data ['ulPdcpDelay'])
                    y_err_delay_dl [x.index (sched)] = get_std_err (data ['dlPdcpDelay'])
                    
                    y_dataRx_ul [x.index (sched)] = data ['ulRxPdcpData'].mean ()
                    y_err_dataRx_ul [x.index (sched)] = get_std_err (data ['ulRxPdcpData'])
                    y_dataRx_dl [x.index (sched)] = data ['dlRxPdcpData'].mean ()
                    y_err_dataRx_dl [x.index (sched)] = get_std_err (data ['dlRxPdcpData'])
                                        
                ax_sinr_ul [1].bar (range (len (x)), y_sinr_ul, tick_label=x, yerr=y_err_sinr_ul)
                ax_sinr_dl [1].bar (range (len (x)), y_sinr_dl, tick_label=x, yerr=y_err_sinr_dl)
                ax_bler_ul [1].bar (range (len (x)), y_bler_ul, tick_label=x, yerr=y_err_bler_ul)
                ax_bler_dl [1].bar (range (len (x)), y_bler_dl, tick_label=x, yerr=y_err_bler_dl)
                ax_delay_ul [1].bar (range (len (x)), y_delay_ul/1e6, tick_label=x, yerr=y_err_delay_ul/1e6)
                ax_delay_dl [1].bar (range (len (x)), y_delay_dl/1e6, tick_label=x, yerr=y_err_delay_dl/1e6)
                ax_dataRx_ul [1].bar (range (len (x)), y_dataRx_ul, tick_label=x, yerr=y_err_dataRx_ul)
                ax_dataRx_dl [1].bar (range (len (x)), y_dataRx_dl, tick_label=x, yerr=y_err_dataRx_dl)
                
                ax_sinr_ul [1].tick_params(axis='x', labelrotation=90)
                ax_sinr_dl [1].tick_params(axis='x', labelrotation=90)
                ax_bler_ul [1].tick_params(axis='x', labelrotation=90)
                ax_bler_dl [1].tick_params(axis='x', labelrotation=90)
                ax_delay_ul [1].tick_params(axis='x', labelrotation=90)
                ax_delay_dl [1].tick_params(axis='x', labelrotation=90)
                ax_dataRx_ul [1].tick_params(axis='x', labelrotation=90)
                ax_dataRx_dl [1].tick_params(axis='x', labelrotation=90)
                
                fig_sinr_ul.savefig (figure_folder + 'sinr_ul_'+title+'.png', bbox_inches='tight')
                fig_sinr_dl.savefig (figure_folder + 'sinr_dl_'+title+'.png', bbox_inches='tight')
                fig_bler_ul.savefig (figure_folder + 'bler_ul_'+title+'.png', bbox_inches='tight')
                fig_bler_dl.savefig (figure_folder + 'bler_dl_'+title+'.png', bbox_inches='tight')
                fig_delay_ul.savefig (figure_folder + 'delay_ul_'+title+'.png', bbox_inches='tight')
                fig_delay_dl.savefig (figure_folder + 'delay_dl_'+title+'.png', bbox_inches='tight')
                fig_dataRx_ul.savefig (figure_folder + 'dataRx_ul_'+title+'.png', bbox_inches='tight')
                fig_dataRx_dl.savefig (figure_folder + 'dataRx_dl_'+title+'.png', bbox_inches='tight')
                
def plot_sched_comparison_udp (csv_path, figure_folder):
    
    results_df = pd.read_csv (csv_path)
    # plot the results

    # sched comparison
    # udp_sched_comparison_sigle_layer = {
    # 'RngRun' : list (range (nruns)),
    # 'numEnb' : 1,
    # 'numUe' : 7,
    # 'simTime' : 1.2,
    # 'interPacketInterval' : [150, 1500],
    # 'harq' : [False, True],
    # 'rlcAm' : [True, False],
    # 'fixedTti' : False,
    # 'sched' : ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler'],
    # 'bfmod' : 'ns3::MmWaveDftBeamforming',
    # 'nLayers' : 1,
    # 'useTCP' : False
    # }

    for rlcAm in [True, False]:
        for interPacketInterval in [150, 1500]:
            for harq in [False, True]:
                title = 'rlcAm=' + str (rlcAm) + "_interPacketInterval=" + str (interPacketInterval) + "_harq=" + str (harq)
                
                fig_sinr_ul, ax_sinr_ul = plt.subplots(1, 2)
                fig_sinr_ul.suptitle('SINR UL ' + title)
                fig_sinr_dl, ax_sinr_dl = plt.subplots(1, 2)
                fig_sinr_dl.suptitle('SINR DL ' + title)
                fig_bler_ul, ax_bler_ul = plt.subplots(1, 2)
                fig_bler_ul.suptitle('BLER UL ' + title)
                fig_bler_dl, ax_bler_dl = plt.subplots(1, 2)
                fig_bler_dl.suptitle('BLER DL ' + title)
                fig_delay_ul, ax_delay_ul = plt.subplots(1, 2)
                fig_delay_ul.suptitle('DELAY UL ' + title)
                fig_delay_dl, ax_delay_dl = plt.subplots(1, 2)
                fig_delay_dl.suptitle('DELAY DL ' + title)
                fig_dataRx_ul, ax_dataRx_ul = plt.subplots(1, 2)
                fig_dataRx_ul.suptitle('DATA RX UL ' + title)
                fig_dataRx_dl, ax_dataRx_dl = plt.subplots(1, 2)
                fig_dataRx_dl.suptitle('DATA RX DL ' + title)
                
                plt.setp(ax_sinr_ul, ylim=[0, 50])
                plt.setp(ax_sinr_dl, ylim=[0, 50])
                plt.setp(ax_bler_ul, ylim=[0, 0.5])
                plt.setp(ax_bler_dl, ylim=[0, 0.5])
                # plt.setp(ax_delay_ul, ylim=[0, 5])
                # plt.setp(ax_delay_dl, ylim=[0, 5])
                # plt.setp(ax_dataRx_ul, ylim=[0, 1.1])
                # plt.setp(ax_dataRx_dl, ylim=[0, 1.1])
                
                
                # single layer
                x = list (results_df ['sched'].unique ())
                y_sinr_ul = np.full ((len (x)), np.nan)
                y_sinr_dl = np.full ((len (x)), np.nan)
                y_bler_ul = np.full ((len (x)), np.nan)
                y_bler_dl = np.full ((len (x)), np.nan)
                y_delay_ul = np.full ((len (x)), np.nan)
                y_delay_dl = np.full ((len (x)), np.nan)
                y_dataRx_ul = np.full ((len (x)), np.nan)
                y_dataRx_dl = np.full ((len (x)), np.nan)
                
                y_err_sinr_ul = np.full ((len (x)), np.nan)
                y_err_sinr_dl = np.full ((len (x)), np.nan)
                y_err_bler_ul = np.full ((len (x)), np.nan)
                y_err_bler_dl = np.full ((len (x)), np.nan)
                y_err_delay_ul = np.full ((len (x)), np.nan)
                y_err_delay_dl = np.full ((len (x)), np.nan)
                y_err_dataRx_ul = np.full ((len (x)), np.nan)
                y_err_dataRx_dl = np.full ((len (x)), np.nan)
                
                for sched in ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:
                    data = results_df.loc [(results_df ['numEnb'] == 1) & 
                                          (results_df ['numUe'] == 7) &
                                          (results_df ['simTime'] == 1.2) &
                                          (results_df ['interPacketInterval'] == interPacketInterval) &
                                          (results_df ['harq'] == harq) &
                                          (results_df ['rlcAm'] == rlcAm) &
                                          (results_df ['fixedTti'] == False) &
                                          (results_df ['sched'] == sched) &
                                          (results_df ['bfmod'] == 'ns3::MmWaveDftBeamforming') &
                                          (results_df ['nLayers'] == 1) &
                                          (results_df ['useTCP'] == False)]
                            
                    y_sinr_ul [x.index (sched)] = data ['avgSinrUl'].mean ()
                    y_sinr_dl [x.index (sched)] = data ['avgSinrDl'].mean ()
                    y_bler_ul [x.index (sched)] = data ['avgBlerUl'].mean ()
                    y_bler_dl [x.index (sched)] = data ['avgBlerDl'].mean ()
                    y_delay_ul [x.index (sched)] = data ['ulPdcpDelay'].mean ()
                    y_delay_dl [x.index (sched)] = data ['dlPdcpDelay'].mean ()
                            
                    y_err_sinr_ul [x.index (sched)] = get_std_err (data ['avgSinrUl'])
                    y_err_sinr_dl [x.index (sched)] = get_std_err (data ['avgSinrDl'])
                    y_err_bler_ul [x.index (sched)] = get_std_err (data ['avgBlerUl'])
                    y_err_bler_dl [x.index (sched)] = get_std_err (data ['avgBlerDl'])
                    y_err_delay_ul [x.index (sched)] = get_std_err (data ['ulPdcpDelay'])
                    y_err_delay_dl [x.index (sched)] = get_std_err (data ['dlPdcpDelay'])
                    
                    y_dataRx_ul [x.index (sched)] = data ['ulRxPdcpData'].mean ()
                    y_err_dataRx_ul [x.index (sched)] = get_std_err (data ['ulRxPdcpData'])
                    y_dataRx_dl [x.index (sched)] = data ['dlRxPdcpData'].mean ()
                    y_err_dataRx_dl [x.index (sched)] = get_std_err (data ['dlRxPdcpData'])
                
                ax_sinr_ul [0].bar (range (len (x)), y_sinr_ul, tick_label=x, yerr=y_err_sinr_ul)
                ax_sinr_dl [0].bar (range (len (x)), y_sinr_dl, tick_label=x, yerr=y_err_sinr_dl)
                ax_bler_ul [0].bar (range (len (x)), y_bler_ul, tick_label=x, yerr=y_err_bler_ul)
                ax_bler_dl [0].bar (range (len (x)), y_bler_dl, tick_label=x, yerr=y_err_bler_dl)
                ax_delay_ul [0].bar (range (len (x)), y_delay_ul/1e6, tick_label=x, yerr=y_err_delay_ul/1e6)
                ax_delay_dl [0].bar (range (len (x)), y_delay_dl/1e6, tick_label=x, yerr=y_err_delay_dl/1e6)
                ax_dataRx_ul [0].bar (range (len (x)), y_dataRx_ul, tick_label=x, yerr=y_err_dataRx_ul)
                ax_dataRx_dl [0].bar (range (len (x)), y_dataRx_dl, tick_label=x, yerr=y_err_dataRx_dl)
                
                ax_sinr_ul [0].tick_params(axis='x', labelrotation=90)
                ax_sinr_dl [0].tick_params(axis='x', labelrotation=90)
                ax_bler_ul [0].tick_params(axis='x', labelrotation=90)
                ax_bler_dl [0].tick_params(axis='x', labelrotation=90)
                ax_delay_ul [0].tick_params(axis='x', labelrotation=90)
                ax_delay_dl [0].tick_params(axis='x', labelrotation=90)
                ax_dataRx_ul [0].tick_params(axis='x', labelrotation=90)
                ax_dataRx_dl [0].tick_params(axis='x', labelrotation=90)
                
                ax_sinr_ul [0].set (ylabel='SINR [dB]')
                ax_sinr_dl [0].set (ylabel='SINR [dB]')
                ax_bler_ul [0].set (ylabel='BLER')
                ax_bler_dl [0].set (ylabel='BLER')
                ax_delay_ul [0].set (ylabel='delay [ms]')
                ax_delay_dl [0].set (ylabel='delay [ms]')
                ax_dataRx_ul [0].set (ylabel='Rx data [bytes]')
                ax_dataRx_dl [0].set (ylabel='Rx data [bytes]')
                
                # multi layer
                x = list (results_df ['sched'].unique ())
                y_sinr_ul = np.full ((len (x)), np.nan)
                y_sinr_dl = np.full ((len (x)), np.nan)
                y_bler_ul = np.full ((len (x)), np.nan)
                y_bler_dl = np.full ((len (x)), np.nan)
                y_delay_ul = np.full ((len (x)), np.nan)
                y_delay_dl = np.full ((len (x)), np.nan)
                y_dataRx_ul = np.full ((len (x)), np.nan)
                y_dataRx_dl = np.full ((len (x)), np.nan)
                
                y_err_sinr_ul = np.full ((len (x)), np.nan)
                y_err_sinr_dl = np.full ((len (x)), np.nan)
                y_err_bler_ul = np.full ((len (x)), np.nan)
                y_err_bler_dl = np.full ((len (x)), np.nan)
                y_err_delay_ul = np.full ((len (x)), np.nan)
                y_err_delay_dl = np.full ((len (x)), np.nan)
                y_err_dataRx_ul = np.full ((len (x)), np.nan)
                y_err_dataRx_dl = np.full ((len (x)), np.nan)
                
                for sched in ['ns3::MmWaveAsyncHbfMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:
                    data = results_df.loc [(results_df ['numEnb'] == 1) & 
                                          (results_df ['numUe'] == 7) &
                                          (results_df ['simTime'] == 1.2) &
                                          (results_df ['interPacketInterval'] == interPacketInterval) &
                                          (results_df ['harq'] == harq) &
                                          (results_df ['rlcAm'] == rlcAm) &
                                          (results_df ['fixedTti'] == False) &
                                          (results_df ['sched'] == sched) &
                                          (results_df ['bfmod'] == 'ns3::MmWaveMMSESpectrumBeamforming') &
                                          (results_df ['nLayers'] == 4) &
                                          (results_df ['useTCP'] == False)]
                                          
                    y_sinr_ul [x.index (sched)] = data ['avgSinrUl'].mean ()
                    y_sinr_dl [x.index (sched)] = data ['avgSinrDl'].mean ()
                    y_bler_ul [x.index (sched)] = data ['avgBlerUl'].mean ()
                    y_bler_dl [x.index (sched)] = data ['avgBlerDl'].mean ()
                    y_delay_ul [x.index (sched)] = data ['ulPdcpDelay'].mean ()
                    y_delay_dl [x.index (sched)] = data ['dlPdcpDelay'].mean ()
                    
                    y_err_sinr_ul [x.index (sched)] = get_std_err (data ['avgSinrUl'])
                    y_err_sinr_dl [x.index (sched)] = get_std_err (data ['avgSinrDl'])
                    y_err_bler_ul [x.index (sched)] = get_std_err (data ['avgBlerUl'])
                    y_err_bler_dl [x.index (sched)] = get_std_err (data ['avgBlerDl'])
                    y_err_delay_ul [x.index (sched)] = get_std_err (data ['ulPdcpDelay'])
                    y_err_delay_dl [x.index (sched)] = get_std_err (data ['dlPdcpDelay'])
                    
                    y_dataRx_ul [x.index (sched)] = data ['ulRxPdcpData'].mean ()
                    y_err_dataRx_ul [x.index (sched)] = get_std_err (data ['ulRxPdcpData'])
                    y_dataRx_dl [x.index (sched)] = data ['dlRxPdcpData'].mean ()
                    y_err_dataRx_dl [x.index (sched)] = get_std_err (data ['dlRxPdcpData'])
                                        
                ax_sinr_ul [1].bar (range (len (x)), y_sinr_ul, tick_label=x, yerr=y_err_sinr_ul)
                ax_sinr_dl [1].bar (range (len (x)), y_sinr_dl, tick_label=x, yerr=y_err_sinr_dl)
                ax_bler_ul [1].bar (range (len (x)), y_bler_ul, tick_label=x, yerr=y_err_bler_ul)
                ax_bler_dl [1].bar (range (len (x)), y_bler_dl, tick_label=x, yerr=y_err_bler_dl)
                ax_delay_ul [1].bar (range (len (x)), y_delay_ul/1e6, tick_label=x, yerr=y_err_delay_ul/1e6)
                ax_delay_dl [1].bar (range (len (x)), y_delay_dl/1e6, tick_label=x, yerr=y_err_delay_dl/1e6)
                ax_dataRx_ul [1].bar (range (len (x)), y_dataRx_ul, tick_label=x, yerr=y_err_dataRx_ul)
                ax_dataRx_dl [1].bar (range (len (x)), y_dataRx_dl, tick_label=x, yerr=y_err_dataRx_dl)
                
                ax_sinr_ul [1].tick_params(axis='x', labelrotation=90)
                ax_sinr_dl [1].tick_params(axis='x', labelrotation=90)
                ax_bler_ul [1].tick_params(axis='x', labelrotation=90)
                ax_bler_dl [1].tick_params(axis='x', labelrotation=90)
                ax_delay_ul [1].tick_params(axis='x', labelrotation=90)
                ax_delay_dl [1].tick_params(axis='x', labelrotation=90)
                ax_dataRx_ul [1].tick_params(axis='x', labelrotation=90)
                ax_dataRx_dl [1].tick_params(axis='x', labelrotation=90)
                
                fig_sinr_ul.savefig (figure_folder + 'sinr_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_sinr_ul.number)
                tikzplotlib.save (figure_folder + 'sinr_ul_'+title+'.tex')
                
                fig_sinr_dl.savefig (figure_folder + 'sinr_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_sinr_dl.number)
                tikzplotlib.save (figure_folder + 'sinr_dl_'+title+'.tex')
                
                fig_bler_ul.savefig (figure_folder + 'bler_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_bler_ul.number)
                tikzplotlib.save (figure_folder + 'bler_ul_'+title+'.tex')
                
                fig_bler_dl.savefig (figure_folder + 'bler_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_bler_dl.number)
                tikzplotlib.save (figure_folder + 'bler_dl_'+title+'.tex')
                
                fig_delay_ul.savefig (figure_folder + 'delay_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_delay_ul.number)
                tikzplotlib.save (figure_folder + 'delay_ul_'+title+'.tex')
                
                fig_delay_dl.savefig (figure_folder + 'delay_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_delay_dl.number)
                tikzplotlib.save (figure_folder + 'delay_dl_'+title+'.tex')
                
                fig_dataRx_ul.savefig (figure_folder + 'dataRx_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_dataRx_ul.number)
                tikzplotlib.save (figure_folder + 'dataRx_ul_'+title+'.tex')
                
                fig_dataRx_dl.savefig (figure_folder + 'dataRx_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_dataRx_dl.number)
                tikzplotlib.save (figure_folder + 'dataRx_dl_'+title+'.tex')
                
def plot_sched_cdfs_tcp (campaign_dir, nruns, n_bins, figure_folder):
    
    campaign = sem.CampaignManager.load(campaign_dir, runner_type = "ParallelRunner", check_repo = False)
    
    harqList = [False, True]
    rlcAmList = [False, True]

    for rlcAm in rlcAmList:
            for harq in harqList:
                
                title = 'rlcAm=' + str(rlcAm) + "_harq=" + str (harq)
                
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
                
                for sched in ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:    
                    
                    params = {
                    'RngRun' : list (range (nruns)),
                    'numEnb' : 1,
                    'numUe' : 7,
                    'simTime' : 1.2,
                    'interPacketInterval' : 0,
                    'harq' : harq,
                    'rlcAm' : rlcAm,
                    'fixedTti' : False,
                    'sched' : sched,
                    'bfmod' : 'ns3::MmWaveDftBeamforming',
                    'nLayers' : 1,
                    'useTCP' : True
                    }
                    
                    (sinr_ul_data, sinr_dl_data, bler_ul_data, bler_dl_data) = get_sinr_data (campaign, params)
                    (delay_ul_data, delay_dl_data) = get_delay_data (campaign, params)
                    
                    plt.figure (fig_sinr_ul.number)
                    (x, y) = compute_ecdf (sinr_ul_data)
                    plt.plot (x, y, label=sched+' 1 layer')
                    
                    plt.figure (fig_sinr_dl.number)
                    (x, y) = compute_ecdf (sinr_dl_data)
                    plt.plot (x, y, label=sched+' 1 layer')
                                                
                    plt.figure (fig_bler_ul.number)
                    (x, y) = compute_ecdf (bler_ul_data)
                    plt.plot (x, y, label=sched+' 1 layer')
                    
                    plt.figure (fig_bler_dl.number)
                    (x, y) = compute_ecdf (bler_dl_data)
                    plt.plot (x, y, label=sched+' 1 layer')
                    
                    plt.figure (fig_delay_ul.number)
                    (x, y) = compute_ecdf (delay_ul_data)
                    plt.plot (x/1e6, y, label=sched+' 1 layer')
                    
                    plt.figure (fig_delay_dl.number)
                    (x, y) = compute_ecdf (delay_dl_data)
                    plt.plot (x/1e6, y, label=sched+' 1 layer')
                                                
                for sched in ['ns3::MmWaveAsyncHbfMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:    
                    
                    params = {
                    'RngRun' : list (range (nruns)),
                    'numEnb' : 1,
                    'numUe' : 7,
                    'simTime' : 1.2,
                    'interPacketInterval' : 0,
                    'harq' : harq,
                    'rlcAm' : rlcAm,
                    'fixedTti' : False,
                    'sched' : sched,
                    'bfmod' : 'ns3::MmWaveMMSESpectrumBeamforming',
                    'nLayers' : 4,
                    'useTCP' : True
                    }
                    
                    (sinr_ul_data, sinr_dl_data, bler_ul_data, bler_dl_data) = get_sinr_data (campaign, params)
                    (delay_ul_data, delay_dl_data) = get_delay_data (campaign, params)
                    
                    plt.figure (fig_sinr_ul.number)
                    (x, y) = compute_ecdf (sinr_ul_data)
                    plt.plot (x, y, label=sched+' 4 layer')
                    
                    plt.figure (fig_sinr_dl.number)
                    (x, y) = compute_ecdf (sinr_dl_data)
                    plt.plot (x, y, label=sched+' 4 layer')

                    plt.figure (fig_bler_ul.number)
                    (x, y) = compute_ecdf (bler_ul_data)
                    plt.plot (x, y, label=sched+' 4 layer')
                    
                    plt.figure (fig_bler_dl.number)
                    (x, y) = compute_ecdf (bler_dl_data)
                    plt.plot (x, y, label=sched+' 4 layer')
                    
                    plt.figure (fig_delay_ul.number)
                    (x, y) = compute_ecdf (delay_ul_data)
                    plt.plot (x/1e6, y, label=sched+' 4 layer')
                    
                    plt.figure (fig_delay_dl.number)
                    (x, y) = compute_ecdf (delay_dl_data)
                    plt.plot (x/1e6, y, label=sched+' 4 layer')
                                                
                fig_sinr_ul.legend (loc='center right')
                fig_sinr_ul.savefig (figure_folder + 'cdf_sinr_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_sinr_ul.number)
                tikzplotlib.save (figure_folder + 'cdf_sinr_ul_'+title+'.tex')
                plt.close (fig_sinr_ul)
                
                fig_sinr_dl.legend (loc='center right')
                fig_sinr_dl.savefig (figure_folder + 'cdf_sinr_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_sinr_dl.number)
                tikzplotlib.save (figure_folder + 'cdf_sinr_dl_'+title+'.tex')
                plt.close (fig_sinr_dl)
                
                fig_bler_ul.legend (loc='center right')
                fig_bler_ul.savefig (figure_folder + 'cdf_bler_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_bler_ul.number)
                tikzplotlib.save (figure_folder + 'cdf_bler_ul_'+title+'.tex')
                plt.close (fig_bler_ul)
                
                fig_bler_dl.legend (loc='center right')
                fig_bler_dl.savefig (figure_folder + 'cdf_bler_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_bler_dl.number)
                tikzplotlib.save (figure_folder + 'cdf_bler_dl_'+title+'.tex')
                plt.close (fig_bler_dl)
                
                fig_delay_ul.legend (loc='center right')
                fig_delay_ul.savefig (figure_folder + 'cdf_delay_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_delay_ul.number)
                tikzplotlib.save (figure_folder + 'cdf_delay_ul_'+title+'.tex')
                plt.close (fig_delay_ul)
                
                fig_delay_dl.legend (loc='center right')
                fig_delay_dl.savefig (figure_folder + 'cdf_delay_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_delay_dl.number)
                tikzplotlib.save (figure_folder + 'cdf_delay_dl_'+title+'.tex')
                plt.close (fig_delay_dl) 
                
def plot_sched_cdfs_udp (campaign_dir, nruns, n_bins, figure_folder):
    
    campaign = sem.CampaignManager.load(campaign_dir, runner_type = "ParallelRunner", check_repo = False)
    
    harqList = [False, True]
    ipiList = [150, 1500]
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
                
                for sched in ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:    
                    
                    params = {
                    'RngRun' : list (range (nruns)),
                    'numEnb' : 1,
                    'numUe' : 7,
                    'simTime' : 1.2,
                    'interPacketInterval' : ipi,
                    'harq' : harq,
                    'rlcAm' : rlcAm,
                    'fixedTti' : False,
                    'sched' : sched,
                    'bfmod' : 'ns3::MmWaveDftBeamforming',
                    'nLayers' : 1,
                    'useTCP' : False
                    }
                    
                    (sinr_ul_data, sinr_dl_data, bler_ul_data, bler_dl_data) = get_sinr_data (campaign, params)
                    (delay_ul_data, delay_dl_data) = get_delay_data (campaign, params)
                    
                    plt.figure (fig_sinr_ul.number)
                    (x, y) = compute_ecdf (sinr_ul_data)
                    plt.plot (x, y, label=sched+' 1 layer')
                    
                    plt.figure (fig_sinr_dl.number)
                    (x, y) = compute_ecdf (sinr_dl_data)
                    plt.plot (x, y, label=sched+' 1 layer')
                                                
                    plt.figure (fig_bler_ul.number)
                    (x, y) = compute_ecdf (bler_ul_data)
                    plt.plot (x, y, label=sched+' 1 layer')
                    
                    plt.figure (fig_bler_dl.number)
                    (x, y) = compute_ecdf (bler_dl_data)
                    plt.plot (x, y, label=sched+' 1 layer')
                    
                    plt.figure (fig_delay_ul.number)
                    (x, y) = compute_ecdf (delay_ul_data)
                    plt.plot (x/1e6, y, label=sched+' 1 layer')
                    
                    plt.figure (fig_delay_dl.number)
                    (x, y) = compute_ecdf (delay_dl_data)
                    plt.plot (x/1e6, y, label=sched+' 1 layer')
                                                
                for sched in ['ns3::MmWaveAsyncHbfMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:    
                    
                    params = {
                    'RngRun' : list (range (nruns)),
                    'numEnb' : 1,
                    'numUe' : 7,
                    'simTime' : 1.2,
                    'interPacketInterval' : ipi,
                    'harq' : harq,
                    'rlcAm' : rlcAm,
                    'fixedTti' : False,
                    'sched' : sched,
                    'bfmod' : 'ns3::MmWaveMMSESpectrumBeamforming',
                    'nLayers' : 4,
                    'useTCP' : False
                    }
                    
                    (sinr_ul_data, sinr_dl_data, bler_ul_data, bler_dl_data) = get_sinr_data (campaign, params)
                    (delay_ul_data, delay_dl_data) = get_delay_data (campaign, params)
                    
                    plt.figure (fig_sinr_ul.number)
                    (x, y) = compute_ecdf (sinr_ul_data)
                    plt.plot (x, y, label=sched+' 4 layer')
                    
                    plt.figure (fig_sinr_dl.number)
                    (x, y) = compute_ecdf (sinr_dl_data)
                    plt.plot (x, y, label=sched+' 4 layer')
                                                
                    plt.figure (fig_bler_ul.number)
                    (x, y) = compute_ecdf (bler_ul_data)
                    plt.plot (x, y, label=sched+' 4 layer')
                    
                    plt.figure (fig_bler_dl.number)
                    (x, y) = compute_ecdf (bler_dl_data)
                    plt.plot (x, y, label=sched+' 4 layer')
                    
                    plt.figure (fig_delay_ul.number)
                    (x, y) = compute_ecdf (delay_ul_data)
                    plt.plot (x/1e6, y, label=sched+' 4 layer')
                    
                    plt.figure (fig_delay_dl.number)
                    (x, y) = compute_ecdf (delay_dl_data)
                    plt.plot (x/1e6, y, label=sched+' 4 layer')
                                                
                fig_sinr_ul.legend (loc='center right')
                fig_sinr_ul.savefig (figure_folder + 'cdf_sinr_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_sinr_ul.number)
                tikzplotlib.save (figure_folder + 'cdf_sinr_ul_'+title+'.tex')
                plt.close (fig_sinr_ul)
                
                fig_sinr_dl.legend (loc='center right')
                fig_sinr_dl.savefig (figure_folder + 'cdf_sinr_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_sinr_dl.number)
                tikzplotlib.save (figure_folder + 'cdf_sinr_dl_'+title+'.tex')
                plt.close (fig_sinr_dl)
                
                fig_bler_ul.legend (loc='center right')
                fig_bler_ul.savefig (figure_folder + 'cdf_bler_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_bler_ul.number)
                tikzplotlib.save (figure_folder + 'cdf_bler_ul_'+title+'.tex')
                plt.close (fig_bler_ul)
                
                fig_bler_dl.legend (loc='center right')
                fig_bler_dl.savefig (figure_folder + 'cdf_bler_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_bler_dl.number)
                tikzplotlib.save (figure_folder + 'cdf_bler_dl_'+title+'.tex')
                plt.close (fig_bler_dl)
                
                fig_delay_ul.legend (loc='center right')
                fig_delay_ul.savefig (figure_folder + 'cdf_delay_ul_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_delay_ul.number)
                tikzplotlib.save (figure_folder + 'cdf_delay_ul_'+title+'.tex')
                plt.close (fig_delay_ul)
                
                fig_delay_dl.legend (loc='center right')
                fig_delay_dl.savefig (figure_folder + 'cdf_delay_dl_'+title+'.png', bbox_inches='tight')
                plt.figure (fig_delay_dl.number)
                tikzplotlib.save (figure_folder + 'cdf_delay_dl_'+title+'.tex')
                plt.close (fig_delay_dl)           

def plot_sched_scatter_udp (csv_path, figure_folder, appTime):
    
    results_df = pd.read_csv (csv_path)
    # plot the results

    # sched comparison
    # udp_sched_comparison_sigle_layer = {
    # 'RngRun' : list (range (nruns)),
    # 'numEnb' : 1,
    # 'numUe' : 7,
    # 'simTime' : 1.2,
    # 'interPacketInterval' : [150, 1500],
    # 'harq' : [False, True],
    # 'rlcAm' : [True, False],
    # 'fixedTti' : False,
    # 'sched' : ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler'],
    # 'bfmod' : 'ns3::MmWaveDftBeamforming',
    # 'nLayers' : 1,
    # 'useTCP' : False
    # }

    for interPacketInterval in [150, 1500]:
        
        title='IPI=' + str (interPacketInterval)

        fig_ul = plt.figure (1)
        fig_dl = plt.figure (2)
        
        for rlcAm in [True, False]:
            for harq in [False, True]:
                
                if (rlcAm ^ harq): 
                    continue
                
                for sched in ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:
                    data = results_df.loc [(results_df ['numEnb'] == 1) & 
                                          (results_df ['numUe'] == 7) &
                                          (results_df ['simTime'] == 1.2) &
                                          (results_df ['interPacketInterval'] == interPacketInterval) &
                                          (results_df ['harq'] == harq) &
                                          (results_df ['rlcAm'] == rlcAm) &
                                          (results_df ['fixedTti'] == False) &
                                          (results_df ['sched'] == sched) &
                                          (results_df ['bfmod'] == 'ns3::MmWaveDftBeamforming') &
                                          (results_df ['nLayers'] == 1) &
                                          (results_df ['useTCP'] == False)]
                    
                    y_delay_ul = data ['ulPdcpDelay'].mean ()
                    y_delay_dl = data ['dlPdcpDelay'].mean ()
                        
                    y_dataRx_ul = data ['ulRxPdcpData'].mean ()
                    y_dataRx_dl = data ['dlRxPdcpData'].mean ()
                    
                    y_thr_ul = y_dataRx_ul * 8 / appTime
                    y_thr_dl = y_dataRx_dl * 8 / appTime
                    
                    plt.figure (fig_ul.number)
                    plt.plot (y_delay_ul/1e6, y_thr_ul/1e6, marker=1, label = sched + ' 1 layer rlcAm=' + str (rlcAm) + ' harq=' + str (harq))
                    plt.figure (fig_dl.number)
                    plt.plot (y_delay_dl/1e6, y_thr_dl/1e6, marker=1, label = sched + ' 1 layer rlcAm=' + str (rlcAm) + ' harq=' + str (harq))
                
                
                for sched in ['ns3::MmWaveAsyncHbfMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:
                    data = results_df.loc [(results_df ['numEnb'] == 1) & 
                                          (results_df ['numUe'] == 7) &
                                          (results_df ['simTime'] == 1.2) &
                                          (results_df ['interPacketInterval'] == interPacketInterval) &
                                          (results_df ['harq'] == harq) &
                                          (results_df ['rlcAm'] == rlcAm) &
                                          (results_df ['fixedTti'] == False) &
                                          (results_df ['sched'] == sched) &
                                          (results_df ['bfmod'] == 'ns3::MmWaveMMSESpectrumBeamforming') &
                                          (results_df ['nLayers'] == 4) &
                                          (results_df ['useTCP'] == False)]

                    y_delay_ul = data ['ulPdcpDelay'].mean ()
                    y_delay_dl = data ['dlPdcpDelay'].mean ()
                        
                    y_dataRx_ul = data ['ulRxPdcpData'].mean ()
                    y_dataRx_dl = data ['dlRxPdcpData'].mean ()
                    
                    y_thr_ul = y_dataRx_ul * 8 / appTime
                    y_thr_dl = y_dataRx_dl * 8 / appTime
                    
                    
                    plt.figure (fig_ul.number)
                    plt.plot (y_delay_ul/1e6, y_thr_ul/1e6, marker=1, label = sched + ' 4 layers rlcAm=' + str (rlcAm) + ' harq=' + str (harq))
                    plt.figure (fig_dl.number)
                    plt.plot (y_delay_dl/1e6, y_thr_dl/1e6, marker=1, label = sched + ' 4 layers rlcAm=' + str (rlcAm) + ' harq=' + str (harq))
                
                
        plt.figure (fig_ul.number)
        plt.xlabel ('Delay [ms]')
        plt.ylabel ('Throughput [Mbps]')
        fig_ul.legend ()    
        fig_ul.savefig (figure_folder + 'scatter_ul_'+title+'.png', bbox_inches='tight')
        tikzplotlib.save (figure_folder + 'scatter_ul_'+title+'.tex')
        plt.close (fig_ul)
        
        plt.figure (fig_dl.number)
        plt.xlabel ('Delay [ms]')
        plt.ylabel ('Throughput [Mbps]')
        fig_dl.legend ()
        fig_dl.savefig (figure_folder + 'scatter_dl_'+title+'.png', bbox_inches='tight')
        tikzplotlib.save (figure_folder + 'scatter_dl_'+title+'.tex')
        plt.close (fig_dl)
                
def plot_sched_scatter_tcp (csv_path, figure_folder, appTime):
    
    results_df = pd.read_csv (csv_path)
    # plot the results

    # sched comparison
    # udp_sched_comparison_sigle_layer = {
    # 'RngRun' : list (range (nruns)),
    # 'numEnb' : 1,
    # 'numUe' : 7,
    # 'simTime' : 1.2,
    # 'interPacketInterval' : [150, 1500],
    # 'harq' : [False, True],
    # 'rlcAm' : [True, False],
    # 'fixedTti' : False,
    # 'sched' : ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler'],
    # 'bfmod' : 'ns3::MmWaveDftBeamforming',
    # 'nLayers' : 1,
    # 'useTCP' : False
    # }

    fig_ul = plt.figure (1)
    fig_dl = plt.figure (2)
    
    for rlcAm in [True, False]:
        for harq in [False, True]:
            
            if (rlcAm ^ harq): 
                continue
            
            for sched in ['ns3::MmWaveFlexTtiMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:
                data = results_df.loc [(results_df ['numEnb'] == 1) & 
                                      (results_df ['numUe'] == 7) &
                                      (results_df ['simTime'] == 1.2) &
                                      (results_df ['interPacketInterval'] == 0) &
                                      (results_df ['harq'] == harq) &
                                      (results_df ['rlcAm'] == rlcAm) &
                                      (results_df ['fixedTti'] == False) &
                                      (results_df ['sched'] == sched) &
                                      (results_df ['bfmod'] == 'ns3::MmWaveDftBeamforming') &
                                      (results_df ['nLayers'] == 1) &
                                      (results_df ['useTCP'] == True)]

                y_delay_ul = data ['ulPdcpDelay'].mean ()
                y_delay_dl = data ['dlPdcpDelay'].mean ()
                    
                y_dataRx_ul = data ['ulRxPdcpData'].mean ()
                y_dataRx_dl = data ['dlRxPdcpData'].mean ()
                
                y_thr_ul = y_dataRx_ul * 8 / appTime
                y_thr_dl = y_dataRx_dl * 8 / appTime
                
                plt.figure (fig_ul.number)
                plt.plot (y_delay_ul/1e6, y_thr_ul/1e6, marker=1, label = sched + ' 1 layer rlcAm=' + str (rlcAm) + ' harq=' + str (harq))
                plt.figure (fig_dl.number)
                plt.plot (y_delay_dl/1e6, y_thr_dl/1e6, marker=1, label = sched + ' 1 layer rlcAm=' + str (rlcAm) + ' harq=' + str (harq))
            
            
            for sched in ['ns3::MmWaveAsyncHbfMacScheduler', 'ns3::MmWavePaddedHbfMacScheduler']:
                data = results_df.loc [(results_df ['numEnb'] == 1) & 
                                      (results_df ['numUe'] == 7) &
                                      (results_df ['simTime'] == 1.2) &
                                      (results_df ['interPacketInterval'] == 0) &
                                      (results_df ['harq'] == harq) &
                                      (results_df ['rlcAm'] == rlcAm) &
                                      (results_df ['fixedTti'] == False) &
                                      (results_df ['sched'] == sched) &
                                      (results_df ['bfmod'] == 'ns3::MmWaveMMSESpectrumBeamforming') &
                                      (results_df ['nLayers'] == 4) &
                                      (results_df ['useTCP'] == True)]

                y_delay_ul = data ['ulPdcpDelay'].mean ()
                y_delay_dl = data ['dlPdcpDelay'].mean ()
                    
                y_dataRx_ul = data ['ulRxPdcpData'].mean ()
                y_dataRx_dl = data ['dlRxPdcpData'].mean ()
                
                y_thr_ul = y_dataRx_ul * 8 / appTime
                y_thr_dl = y_dataRx_dl * 8 / appTime
                
                plt.figure (fig_ul.number)
                plt.plot (y_delay_ul/1e6, y_thr_ul/1e6, marker=1, label = sched + ' 4 layers rlcAm=' + str (rlcAm) + ' harq=' + str (harq))
                plt.figure (fig_dl.number)
                plt.plot (y_delay_dl/1e6, y_thr_dl/1e6, marker=1, label = sched + ' 4 layers rlcAm=' + str (rlcAm) + ' harq=' + str (harq))
                
                
        plt.figure (fig_ul.number)
        plt.xlabel ('Delay [ms]')
        plt.ylabel ('Throughput [Mbps]')
        fig_ul.legend ()    
        fig_ul.savefig (figure_folder + 'tcp_scatter_ul.png', bbox_inches='tight')
        tikzplotlib.save (figure_folder + 'tcp_scatter_ul.tex')
        plt.close (fig_ul)
        
        plt.figure (fig_dl.number)
        plt.xlabel ('Delay [ms]')
        plt.ylabel ('Throughput [Mbps]')
        fig_dl.legend ()
        fig_dl.savefig (figure_folder + 'tcp_scatter_dl.png', bbox_inches='tight')
        tikzplotlib.save (figure_folder + 'tcp_scatter_dl.tex')
        plt.close (fig_dl)
