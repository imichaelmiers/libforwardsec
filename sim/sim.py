from subprocess import PIPE, Popen
from sys import argv
from operator import itemgetter
import random , math, itertools
from numpy  import array,save,vstack
from collections import namedtuple
dertab = {0.0: 75.8102, 1.0: 73.2636, 2.0: 70.8082, 3.0: 68.196, 4.0: 65.5956, 5.0: 63.1057, 6.0: 60.4677, 7.0: 58.0124, 8.0: 55.4729, 9.0: 52.8553, 10.0: 50.3524, 11.0: 47.7824, 12.0: 45.2485, 13.0: 42.6671, 14.0: 40.1075, 15.0: 37.5513, 16.0: 34.9894, 17.0: 32.4707, 18.0: 29.8858, 19.0: 27.4, 20.0: 24.8432, 21.0: 22.2391, 22.0: 19.6562, 23.0: 17.1255, 24.0: 14.581, 25.0: 12.0261, 26.0: 9.47223, 27.0: 6.91889, 28.0: 4.36226, 29: 2.0, 30: 1.0, 31:.5}

def simPunc(keys,path):
	keys[path]+=1 
	return 10.69

def simDec(numPunctures):
	r= 21.36+6.21911*numPunctures
	return r 
def simDer(path):
	return dertab[len(path)]

def treeSize(k):
    return (2**(k+1))-1

def indexToPath(index,l):
    path = []
    nodesSoFar=0
    for level in xrange(l):
        subtree_height = treeSize(l-level-1)
        if nodesSoFar == index:
            return path;
        elif index <= (subtree_height + nodesSoFar):
            path.append(0) 
            nodesSoFar +=1 
        else:
            path.append(1)
            nodesSoFar +=subtree_height+1
    if(nodesSoFar < index):
        raise ValueError("index greater than tree depth")
    return path


def pathToIndex(path,l):
    index = 0;
    for level in xrange(len(path)):
        if path[level] == 0:
            index +=1
        elif path[level] == 1:
           #If we go right,

          #Preorder normally goes left, visiting all the nodes
          # in the left sub-tree before going right.
          #Add the number of nodes in the left sub-tree.

                  left_subtree_level = level + 1
                  left_subtree_height = l - left_subtree_level
                  left_subtree_size = treeSize(left_subtree_height)

                  index += left_subtree_size

                  #Add one for the node we just visited.
                  index += 1
    return index;

def simDerKeys(keys,path):
	derTime = 0
	ancestor = path
	while ancestor not in keys:
		ancestor=ancestor[:-1]
		if(len(ancestor)==0):
			raise Exception("Cannot derive key")
		

	while len(ancestor) < len(path):
		ancestor += path[len(ancestor)]
		derTime += simDer(ancestor)
		keys[ancestor] = 0
		sibling= ""
		if(ancestor[-1] == '0'):
			sibling = ancestor[:-1] +'1'
		else:
			sibling = ancestor[:-1] +'0'
		derTime += simDer(sibling)
		keys[sibling] = 0

	return derTime


def simacc(path,window,avg,iterations,msgs,depth=31,numtags=1):
	rate = 1.0/avg;
	#print p.stdout.readline()
	keys={'0':0,'1':1}
	t=1;
	DecTime = 0
	Dertime = 0
	PunTime = 0
	MaxDer = 0
	MaxDec = 0
	ctr =0
	for i in xrange(msgs):
		ctr+=1
		t += random.expovariate(rate);
		if(t - math.floor(t)) > 1:
			overlap = ctr - window
			for i in xrange(overlap):
				if i in keys:
					del d[i]

		interval = int(math.floor(t))
		path = "".join(str(x) for x in indexToPath(interval,depth))

		d = simDerKeys(keys,path)
		dd = simDec(keys[path])
		PunTime += simPunc(keys,path)

		MaxDer = max(MaxDer,d)
		MaxDec = max(MaxDec,dd)

		Dertime +=d
		DecTime +=dd


	return [DecTime,Dertime,PunTime,MaxDer,MaxDec]

def main(argv):
	rates = [.01,.1,1,10,100,1000]
	windows =   [1,60,60*60,60*60*12]
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


		rs =simacc(path = path, window = window,
		 	avg = rate, iterations = iterations, msgs = duration )
		results.append(rs)

	np = array(results)
	print np
	#save(savename,np)
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
	latency=0
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
	results = [window,avg,depth,latency]
	results += parseTimer(lines[0:5])
	results += parseTimer(lines[5:10])
	results += parseTimer(lines[10:15])
	results.append(float(lines[15].split()[1]))

	# for l in lines:
	# 	print l
	# print "Dertime"
	# pprint(results)
	# print "DecTime"
	# pprint(results[3:])
	# print "PunTime"
	# pprint(results[6:])
	# print "Maxsize \t%f"%(results[-1])
	return results
if __name__ == "__main__":
	main(argv)
