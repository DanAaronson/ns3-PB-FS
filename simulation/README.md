## Quick Start

### Build
`./waf configure`

Please note if gcc version > 5, compilation will fail due to some ns3 code style.  If this what you encounter, please use:

`CC='gcc-5' CXX='g++-5' ./waf configure`

### Experiment config
Please see `mix/config.txt` for example. 

`mix/config_doc.txt` is a explanation of the example (texts in {..} are explanations).

`mix/topology.txt` is the topology used for our PB-FS evaluation.

### Run
The direct command to run is:
`./waf --run 'scratch/third mix/config.txt'`

## Files added/edited for PB-FS and IRN
The major ones are listed here. There could be some files not listed here that are not important or not related to core logic. In the corresponding README of the [HPCC simulator](https://github.com/alibaba-edu/High-Precision-Congestion-Control) they list the files they have changed.

`point-to-point/model/nack-header.h`: the IRN NACK header

`point-to-point/model/pbt-header.cc`: the PBT header used by PB-FS

`point-to-point/model/qbb-net-device.cc/h`: the net-device RDMA

`point-to-point/model/rdma-queue-pair.cc/h`: queue pair

`point-to-point/model/rdma-hw.cc/h`: the core logic of congestion control

`point-to-point/model/switch-node.cc/h`: the node class for switch

`network/utils/custom-header.cc/h`: a customized header class for speeding up header parsing
