# PB-FS simulation
This is an extension of the [ns-3 HPCC simulator](https://github.com/alibaba-edu/High-Precision-Congestion-Control). It implements PB-FS, IRN, and TCP Slow Start. The extension allows simulating RDMA in both a lossless and lossy environment.

## NS-3 simulation
The ns-3 simulation is under `simulation/`. Refer to the README.md under it for more details.

## Traffic generator
The traffic generator is under `traffic_gen/`. Refer to the README.md under it for more details.

## Analysis
Various analysis scripts are under `analysis/`. The fct analysis script has been updated for analysing PB-FS. We have also added new scripts for both queue length and PFC analysis. Refer to the README.md under it for more details.
