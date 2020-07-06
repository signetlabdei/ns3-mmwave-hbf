import pandas as pd
import numpy as np
import numpy.ma as ma
import matplotlib.pyplot as plt
import math

def get_std_err (x):
    std = np.nanstd (x) / math.sqrt(x.size - np.isnan(x).sum ())
    return std

def compute_prr (rx, tx):
    prr_runs = np.divide (rx, tx)
    mx_prr_runs = ma.masked_invalid (prr_runs, copy=False)
    return (mx_prr_runs.mean (), get_std_err (prr_runs))

results_folder = 'campaign-2/'
figure_folder = results_folder + 'figures/'
results_df = pd.read_csv (results_folder + 'parsed_results.csv')

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
                
