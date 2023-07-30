import sys
import argparse

def parse_pfc(pfc_line):
    values = pfc_line.split()
    time = int(values[0])
    dev = int(values[1])
    node_type = int(values[2])
    iface = int(values[3])
    is_pause = int(values[4])
    return time, dev, node_type, iface, is_pause


if __name__=="__main__":
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-t', '--time', dest='time', action='store', type=float, default=0.1, help="length of the simulation in seconds")
    parser.add_argument('-p', '--prefix', dest='prefix', action='store', default='pfc', help="the pfc file name prefix")
    parser.add_argument('-n', dest='dev_num', action='store', type=int, default=640, help="number of devices in topology")
    parser.add_argument('cc', action='store', nargs='+', help="the suffix for each congestion control protocol pfc file to examine")
    args = parser.parse_args()

    for cc in args.cc:
        file = "../simulation/mix/%s.txt.%s"%(args.prefix, cc)
        pauses = {}
        total_pause_time = 0
        with open(file, 'r') as f:
            lines = f.readlines()[:-1]
            lines = [l.strip() for l in lines]
            for l in lines:
                time, dev, node_type, iface, is_pause = parse_pfc(l)
                key = (dev, node_type, iface)
                if key in pauses:
                    pause_start_time = pauses[key]
                    total_pause_time += (time - pause_start_time)
                    del pauses[key]
                else:
                    pauses[key] = time

        pause_percentage = float(total_pause_time) / (args.dev_num * args.time * 1000000000)
            
        print "{} {} {}".format(cc, total_pause_time, pause_percentage)
