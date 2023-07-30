#!/bin/bash

nohup ./waf --run "scratch/third mix/config_dcqcn.txt" > res.txt.d &
sleep 1
nohup ./waf --run "scratch/third mix/config_dcqcn_pbt.txt" > res.txt.dp &
sleep 1
nohup ./waf --run "scratch/third mix/config_dcqcn_pbt_nopfc.txt" > res.txt.dpn &
sleep 1
nohup ./waf --run "scratch/third mix/config_dcqcn_nopfc.txt" > res.txt.dn &
sleep 1
nohup ./waf --run "scratch/third mix/config_timely.txt" > res.txt.t &
sleep 1
nohup ./waf --run "scratch/third mix/config_timely_pbt.txt" > res.txt.tp &
sleep 1
nohup ./waf --run "scratch/third mix/config_timely_pbt_nopfc.txt" > res.txt.tpn &
sleep 1
nohup ./waf --run "scratch/third mix/config_timely_nopfc.txt" > res.txt.tn &
sleep 1
nohup ./waf --run "scratch/third mix/config_hpcc.txt" > res.txt.h &
sleep 1
nohup ./waf --run "scratch/third mix/config_hpcc_pbt.txt" > res.txt.hp &
sleep 1
nohup ./waf --run "scratch/third mix/config_hpcc_pbt_nopfc.txt" > res.txt.hpn &
sleep 1
nohup ./waf --run "scratch/third mix/config_hpcc_nopfc.txt" > res.txt.hn &
sleep 1
