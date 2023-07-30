import subprocess
import argparse

def get_pctl(a, p):
	i = int(len(a) * p)
	return a[i]

if __name__=="__main__":
	parser = argparse.ArgumentParser(description='')
	parser.add_argument('-p', dest='prefix', action='store', default='fct_fat', help="Specify the prefix of the fct file. Usually like fct_<topology>_<trace>")
	parser.add_argument('-s', dest='step', action='store', default='5')
	parser.add_argument('-t', dest='type', action='store', type=int, default=0, help="0: normal, 1: incast, 2: all")
	parser.add_argument('-T', dest='time_limit', action='store', type=int, default=3000000000, help="only consider flows that finish before T")
	parser.add_argument('-b', dest='bw', action='store', type=int, default=25, help="bandwidth of edge link (Gbps)")
	parser.add_argument('--slowdown', default=False, action='store_true')
	parser.add_argument('--no-slowdown', dest='slowdown', action='store_false')
	parser.add_argument('-pl', dest='payload_size', action='store', type=int, default=1000)
	args = parser.parse_args()
        
	type = args.type
	time_limit = args.time_limit

	# Please list all the cc (together with parameters) that you want to compare.
	# For example, here we list two CC: 1. HPCC-PINT with utgt=95,AI=50Mbps,pint_log_base=1.05,pint_prob=1; 2. HPCC with utgt=95,ai=50Mbps.
	# For the exact naming, please check ../simulation/mix/fct_*.txt output by the simulation.
	CCs = [
        'dcqcn',
        'dcqcn_pbt',
        'dcqcn_pbt_nopfc',
        'dcqcn_nopfc',
        'timely',
        'timely_pbt',
        'timely_pbt_nopfc',
        'timely_nopfc',
        'hpcc',
        'hpcc_pbt',
        'hpcc_pbt_nopfc',
	]

        sizes = [1000, 2000, 4000, 6000, 10000, 14000, 18000, 22000, 30000, 40000, 60000, 80000, 100000, 325000, 550000, 675000, 1000000, 3145728]

        res = [[i] for i in sizes] 
	for cc in CCs:
		#file = "%s_%s.txt"%(args.prefix, cc)
		file = "../simulation/mix/%s_%s.txt"%(args.prefix, cc)
		if type == 0:
			cmd = "cat %s"%(file)+" | awk '{if ($4==100 && $6+$7<"+"%d"%time_limit+") "+("{slow=$7/$8;print slow<1?1:slow, $5}" if args.slowdown else "{fct=$7;print fct, $5}")+"}' | sort -n -k 2"
			# print cmd
			output = subprocess.check_output(cmd, shell=True)
		elif type == 1:
			cmd = "cat %s"%(file)+" | awk '{if ($4==200 && $6+$7<"+"%d"%time_limit+") "+("{slow=$7/$8;print slow<1?1:slow, $5}" if args.slowdown else "{fct=$7;print fct, $5}")+"}' | sort -n -k 2"
			#print cmd
			output = subprocess.check_output(cmd, shell=True)
		else:
			cmd = "cat %s"%(file)+" | awk '{$6+$7<"+"%d"%time_limit+") "+("{slow=$7/$8;print slow<1?1:slow, $5}" if args.slowdown else "{fct=$7;print fct, $5}")+"}' | sort -n -k 2"
			#print cmd
			output = subprocess.check_output(cmd, shell=True)

		# up to here, `output` should be a string of multiple lines, each line is: fct, size
		a = output.split('\n')[:-2]
                a = map(lambda x: [float(x.split()[0]), int(x.split()[1])], a)
                n = len(a)
                l = 0
		for i in range(len(sizes)): 
			if i == len(sizes) - 1:
                            r = len(a)
                        else:
                            r = next(x for x, val in enumerate(a) if val[1] > sizes[i])
                        d = a[l:r]
			fct=sorted(map(lambda x: x[0], d))
                        res[i].append(float(len(d))/float(n))
			#res[i/step].append(sum(fct) / len(fct)) # avg fct
			res[i].append(get_pctl(fct, 0.5)) # mid fct
			res[i].append(get_pctl(fct, 0.95)) # 95-pct fct
			res[i].append(get_pctl(fct, 0.99)) # 99-pct fct
                        l = r

	for item in res:
		line = "%d"%(item[0])
		i = 1
		for cc in CCs:
			line += ",%.6f,%.3f,%.3f,%.3f"%(item[i], item[i+1], item[i+2], item[i+3])
			i += 4
		print line
