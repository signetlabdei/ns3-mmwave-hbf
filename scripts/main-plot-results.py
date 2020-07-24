import plot_results as pr

campaign_dir = 'campaign-2/'
csv_path = campaign_dir + 'parsed_results.csv'
appTime = 1.2 - 0.4 # simTime - startTime

# pr.plot_bf_comparison (csv_path=csv_path, figure_folder=campaign_dir + 'figures/bf-comparison/')
# pr.plot_bf_cdfs (campaign_dir=campaign_dir, nruns=20, n_bins=100, figure_folder=campaign_dir + 'figures/bf-comparison/')
# pr.plot_sched_comparison_tcp (csv_path=csv_path, figure_folder=campaign_dir + 'figures/sched-comparison/tcp/')
# pr.plot_sched_comparison_udp (csv_path=csv_path, figure_folder=campaign_dir + 'figures/sched-comparison/udp/')
# pr.plot_sched_cdfs_tcp (campaign_dir=campaign_dir, nruns=20, n_bins=100, figure_folder=campaign_dir + 'figures/sched-comparison/tcp/')
# pr.plot_sched_cdfs_udp (campaign_dir=campaign_dir, nruns=20, n_bins=100, figure_folder=campaign_dir + 'figures/sched-comparison/udp/')
# pr.plot_sched_scatter_udp (csv_path=csv_path, figure_folder=campaign_dir + 'figures/sched-comparison/udp/', appTime=appTime)
pr.plot_sched_scatter_tcp (csv_path=csv_path, figure_folder=campaign_dir + 'figures/sched-comparison/udp/', appTime=appTime)
