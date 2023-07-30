import sys
import argparse

def get_pctl(a, p):
	i = int(len(a) * p)
	return a[i]

if __name__=="__main__":
	parser = argparse.ArgumentParser(description='')
	parser.add_argument('-p', dest='prefix', action='store', default='qlen', help="Specify the prefix of the fct file. Usually like qlen_<topology>_<trace>")
	parser.add_argument('-d', dest='dev_num', action='store', type=int, default=640, help="number of devices in topology")
	args = parser.parse_args()

	# Please list all the cc (together with parameters) that you want to compare.
	CCs = [
        'd',
        'dp',
        'dpn',
        'dn',
        't',
        'tp',
        'tpn',
        'tn',
        'h',
        'hp',
        'hpn',
	]

        res = []
	for cc in CCs:
		file = "../simulation/mix/%s.txt.%s"%(args.prefix, cc)
                with open(file, 'r') as f:
                    contents = f.readlines()
                    contents = [map(lambda y : int(y), x.strip().split(' ')[2:]) for x in contents[-args.dev_num:]]
                    max_queue = max([len(c) for c in contents])
                    distribution = [0]*max_queue
                    for c in contents:
                        for i in range(len(c)):
                            distribution[i] = distribution[i] + c[i]
                    total = float(sum(distribution))
                    distribution = [float(x)/total for x in distribution]
                    for i in range(len(distribution) - 1, -1, -1):
                        for j in range(i+1, len(distribution)):
                            distribution[j] = distribution[j] + distribution[i]
                    res.append(distribution)
        for i in range(max([len(r) for r in res])):
            sys.stdout.write(str(i) + ",")
            for r in res:
                if i < len(r):
                    sys.stdout.write(str(r[i]) + ",")
                else:
                    sys.stdout.write("1.0,")
            sys.stdout.write('\n')
