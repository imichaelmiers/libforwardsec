from subprocess import PIPE, Popen
from sys import argv
from operator import itemgetter
import random , math, itertools
from numpy  import array,save,vstack
from collections import namedtuple



def main(argv):
	rates = [1]#[.01,.1,1,10,100,1000]
	windows = [10] # [1,60,60*60,60*60*12]
	results = []
	iterations = int(argv[3])
	duration = int(argv[2])
	path = argv[1]
	savename = argv[4]
	print "iterations: %d "%iterations
	print "duration: %d"%duration 
	print "path: %s"%path

	for window,rate in itertools.product(windows,rates):
		print "w: %s r:%s"%(window,rate)


		rs =sim(path = path, window = window,
		 	avg = rate, iterations = iterations, msgs = duration )
		results.append(rs)

	np = array(results)
	save(savename,np)
def parseLine(line):
	a,b,c = itemgetter(2,4,6)(line.split())
	return [float(a),float(b),float(c)]
def parseTimer(lines):
	result = []
	for l in lines[1:]:
			result += parseLine(l)
	return result
def pprint(data):
	for i,v in enumerate(['wall','user','system','cputime']):
		print "\t\t %s average %f stdev %f total %f "%(v,data[0+i*3],data[1+i*3],data[2+i*3])

def sim(path,window,avg,iterations,msgs,depth=31,numtags=1):
	rate = 1.0/avg;
	p = Popen(path, stdin=PIPE, stdout = PIPE, bufsize=1)
	args = "%d %s %d %d \n"%(window,30,numtags,iterations)
	print args
	p.stdin.write(args)
	#print p.stdout.readline()
	t=1;
	for i in xrange(msgs):
		t += random.expovariate(rate);
		if(t - math.floor(t)) > 1:
			p.stdin.write("0\n") 

		arg = int(math.floor(t))
		p.stdin.write("%d\n"%arg)
	p.stdin.close()
	p.wait()
	lines = p.stdout.readlines()
	results = []
	results += parseTimer(lines[0:5])
	results += parseTimer(lines[5:10])
	results += parseTimer(lines[10:15])
	results.append(float(lines[15].split()[1]))
	for l in lines:
		print l
	print "Dertime"
	pprint(results)
	print "DecTime"
	pprint(results[3:])
	print "PunTime"
	pprint(results[6:])
	print "Maxsize \t%f"%(results[-1])
	return results
if __name__ == "__main__":
	main(argv)
