import random
from argparse import ArgumentParser


def parse_flow(flow_line):
    values = flow_line.split()
    src = int(values[0])
    dst = int(values[1])
    size = int(values[4])
    time = float(values[5])
    return src, dst, size, time

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-n", "--nhost", dest = "nhost", type = int, help = "the number of hosts", default = 320)
    parser.add_argument("-bw", "--bandwidth", dest = "bw", type = int, help = "the host bandwidth in Gbps", default = 25)
    parser.add_argument("-inum", "--incast-num", dest = "inum", type = int, help = "the number of hosts creating incast traffic", default = 100)
    parser.add_argument("-s", "--size", dest = "size", type = int, help = "the size of each incast flow in Bytes", default = 100 * 1024)
    parser.add_argument("-t", "--time", dest = "time", type = float, help = "the incast start time", default = 2.05)
    parser.add_argument("input", help = "the flow file to add incast to")
    parser.add_argument("-o", "--output", dest = "output", help = "the output file", default = "tmp_traffic_with_incast.txt")
    args = parser.parse_args()

    nhost = args.nhost
    bw = args.bw
    inum = args.inum
    size = args.size
    time = args.time

    hosts = set(range(nhost))

    receiver = random.choice(range(nhost))
    hosts.remove(receiver)

    senders = random.sample(sorted(hosts), inum)
    incast_transmission_time = size * 8.0 / bw / (10**9)

    with open(args.input, "r") as f, open(args.output, "w") as out:
        flows = f.readlines()
        flows = [flow.rstrip() for flow in flows]
        flow_num = int(flows[0])
        flows = flows[1:]

        flow_idx = 0
        flow_count = 0
        output_flows = []
        while True:
            flow = flows[flow_idx]
            _, _, _, flow_time = parse_flow(flow)
            if flow_time > time - incast_transmission_time:
                break
            
            output_flows.append(flow)
            flow_idx += 1
            flow_count += 1

        while True:
            flow = flows[flow_idx]
            flow_sender, _, _, flow_time = parse_flow(flow)
            if flow_time >= time:
                break

            if flow_sender not in senders:
                output_flows.append(flow)
                flow_count += 1
            else:
                print "Removed: {}".format(flow)
            flow_idx += 1

        for sender in senders:
            output_flows.append("{} {} 3 200 {} {}".format(sender, receiver, size, time))

        flow_count += inum

        while True:
            flow = flows[flow_idx]
            flow_sender, _, _, flow_time = parse_flow(flow)
            if flow_time > time + incast_transmission_time:
                break

            if flow_sender not in senders:
                output_flows.append(flow)
                flow_count += 1
            else:
                print "Removed: {}".format(flow)
            flow_idx += 1

        while flow_idx < flow_num:
            flow = flows[flow_idx]
            output_flows.append(flow)
            flow_idx += 1
            flow_count += 1

        out.write("{}\n".format(flow_count))
        for flow in output_flows:
            out.write("{}\n".format(flow))	
