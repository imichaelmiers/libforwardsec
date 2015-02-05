from subprocess import PIPE, Popen
from sys import argv
from operator import itemgetter
import   math, itertools
from random import expovariate
from numpy  import array,save,vstack,random,arange
from collections import namedtuple
dertab = {0.0: 75.8102, 1.0: 73.2636, 2.0: 70.8082, 3.0: 68.196, 4.0: 65.5956, 5.0: 63.1057, 6.0: 60.4677, 7.0: 58.0124, 8.0: 55.4729, 9.0: 52.8553, 10.0: 50.3524, 11.0: 47.7824, 12.0: 45.2485, 13.0: 42.6671, 14.0: 40.1075, 15.0: 37.5513, 16.0: 34.9894, 17.0: 32.4707, 18.0: 29.8858, 19.0: 27.4, 20.0: 24.8432, 21.0: 22.2391, 22.0: 19.6562, 23.0: 17.1255, 24.0: 14.581, 25.0: 12.0261, 26.0: 9.47223, 27.0: 6.91889, 28.0: 4.36226, 29: 2.0, 30: 1.0, 31:.5}

def simPunc(keys,path):
	keys[path]= keys[path]+1
	return 10.69

def simDec(numPunctures):
	r= 21.36+6.21911*numPunctures
	return r 
def simDer(path,depth):
	x =len(path) - (depth-31)
	#	print "calculatd x %s"%x

#	if( x < 0):
#		raise Exception("PathLess Than zero")
	return 75.682-2.422*x 
	#return dertab[len(path)]

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
        if path[level] == '0':
            index +=1
        elif path[level] == '1':
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
def findlastAncestor(keys,path):
	ancestor = path
	while ancestor not in keys:
		ancestor=ancestor[:-1]
		if(len(ancestor)==0):
			raise Exception("Cannot derive key")
	return ancestor
def simFastDir(keys,path):
	ancestor = findlastAncestor(keys,path)
	derTime += simDer(path)
	sibling= ""
	derTime += simDer(sibling)
	keys[sibling] = 0
	if(path[-1] == '0'):
		sibling = path[:-1] +'1'
	else:
		sibling = path[:-1] +'0'
	assert(ancestor not in keys or keys[ancestor] == 0)
	keys[ancestor]+=1
	while len(ancestor) < len(path):
		ancestor += path[len(ancestor)]
		keys[ancestor] = 1

	return derTime
def simDerKeys(keys,path,depth):
	if path in keys:
		return 0

	derTime = 0
	ancestor = findlastAncestor(keys,path)
	while len(ancestor) < len(path):
		ancestor += path[len(ancestor)]
		derTime += simDer(ancestor,depth)
		keys[ancestor] = 0
		sibling= ""
		if(ancestor[-1] == '0'):
			sibling = ancestor[:-1] +'1'
		else:
			sibling = ancestor[:-1] +'0'
		derTime += simDer(sibling,depth)
		keys[sibling] = 0
	return derTime


def sim_intervals(msgs_per_second):
	intervalsizes = [.001,.01,.1,1,10]
	depths  = [35,32,29,26,23]
	results = []
	for i,d in zip(intervalsizes,depths):
		rs =simacc(path = '', window = 1,
		 	avg = msgs_per_second*i, interval_length = i, numintervals = 1000/i,depth = d)
		results.append(rs)
	np = array(results)
	return np
def simacc(path,window,avg,interval_length,numintervals,depth=31,numtags=1):
	latency = 0
	print avg
	keys={'0':0,'1':1}
	t=1;
	DecTime = 0
	Dertime = 0
	PunTime = 0
	MaxDer = 0
	MaxDec = 0
	intetvalctr =0
	interval = 0
	intervals = random.poisson(avg,numintervals)
	msg_ctr=0
	for arrived_msgs in intervals:
		interval +=1
		#print "avg: %s , pois : %s"%(avg,arrived_msgs)
		if (arrived_msgs ==0):
			continue
		path = "".join(str(x) for x in indexToPath(interval,depth))
		d = simDerKeys(keys,path,depth)
		Dertime +=d
		for foo in xrange(arrived_msgs):
			intetvalctr+=1
			dd = simDec(keys[path])
			PunTime += simPunc(keys,path)

			MaxDer = max(MaxDer,d)
			MaxDec = max(MaxDec,dd)

			DecTime +=dd
			msg_ctr+=1

	#print max(keys.iteritems(), key=itemgetter(1))
	#print "maxdec %s"%MaxDec
	int_length_in_ms =  (interval_length*1000) 
	ms = 1000# max(1,interval) #  int_length_in_ms  #int_length_in_ms * numintervals
	print "int length %s dertime  %s result %s "%(int_length_in_ms,Dertime, Dertime/int_length_in_ms)
	return [window/ms,int_length_in_ms/ms,depth,latency/ms,DecTime/ms,Dertime/ms,PunTime/ms,MaxDer/ms,MaxDec/ms]
# def run_sim_acc(duration):
# 	rates = arange(.1,10,.1)
# 	windows =  [1]# [1,60,60*60,60*60*12]
# 	results = []
# 	iterations = 0
# 	for window,rate in itertools.product(windows,rates):
# 		rs =simacc(path = '', window = 1,
# 		 	avg = rate, iterations = iterations, msgs = duration )
# 		results.append(rs)

# 	np = array(results)
# 	return np

# def main(argv):
# 	rates = [.0001] #[ .01,.1,1,10,100,1000]
# 	windows =  [1]# [1,60,60*60,60*60*12]
# 	results = []
# 	iterations = int(argv[3])
# 	duration = int(argv[2])
# 	path = argv[1]
# 	savename = argv[4]
# 	print "iterations: %d "%iterations
# 	print "duration: %d"%duration 
# 	print "path: %s"%path

# 	for window,rate in itertools.product(windows,rates):
# 		print "w: %s r:%s"%(window,rate)


# 		rs =sim_new(path = path, window = window,
# 		 	avg = rate, iterations = iterations, msgs = duration )
# 		results.append(rs)

# 	np = array(results)
# 	print np
# 	#save(savename,np)
# def parseLine(line):
# 	a,b,c = itemgetter(2,4,6)(line.split())
# 	return [float(a),float(b),float(c)]
# def parseTimer(lines):
# 	result = []
# 	for l in lines[1:]:
# 			result += parseLine(l)
# 	return result
# def pprint(data):
# 	for i,v in enumerate(['wall','user','system','cputime']):
# 		print "\t\t %s average %f stdev %f total %f "%(v,data[0+i*3],data[1+i*3],data[2+i*3])

# def sim(path,window,avg,iterations,msgs,depth=31,numtags=1):
# 	latency=0
# 	rate = 1.0/avg;
# 	p = Popen(path, stdin=PIPE, stdout = PIPE, bufsize=1)
# 	args = "%d %s %d %d \n"%(window,30,numtags,iterations)
# 	print args
# 	p.stdin.write(args)
# 	#print p.stdout.readline()
# 	t=1;
# 	for i in xrange(msgs):
# 		t += rrr.expovariate(rate);
# 		if(t - math.floor(t)) > 1:
# 			p.stdin.write("0\n") 
# 		print t 
# 		arg = int(math.floor(t))
# 		p.stdin.write("%d\n"%arg)
# 	p.stdin.close()
# 	p.wait()
# 	lines = p.stdout.readlines()
# 	results = [window,avg,depth,latency]
# 	results += parseTimer(lines[0:5])
# 	results += parseTimer(lines[5:10])
# 	results += parseTimer(lines[10:15])
# 	results.append(float(lines[15].split()[1]))

# 	# for l in lines:
# 	# 	print l
# 	# print "Dertime"
# 	# pprint(results)
# 	# print "DecTime"
# 	# pprint(results[3:])
# 	# print "PunTime"
# 	# pprint(results[6:])
# 	# print "Maxsize \t%f"%(results[-1])
# 	return results
# def sim_new(window,avg,msgs,depth=31,numtags=1):
# 	latency=0
# 	rate = 1.0/avg
# 	#print p.stdout.readline()
# 	t=1;
# 	for i in xrange(msgs):
# 		t += expovariate(rate);
# 		#if(t - math.floor(t)) > 1:
# 		#	p.stdin.write("0\n") 
# 		print t 
# 	# 	arg = int(math.floor(t))
# 	# 	p.stdin.write("%d\n"%arg)
# 	# p.stdin.close()
# 	# p.wait()
# 	# lines = p.stdout.readlines()
# 	# results = [window,avg,depth,latency]
# 	# results += parseTimer(lines[0:5])
# 	# results += parseTimer(lines[5:10])
# 	# results += parseTimer(lines[10:15])
# 	# results.append(float(lines[15].split()[1]))

if __name__ == "__main__":
	# keys={'0':0,'1':1}
	# print simDerKeys(keys,'1'*31)
	# s = 0
	# for i in dertab:
	# 	s+= 2*dertab[i]
	# print s
	depth = 64
	for i in xrange(depth):
		path = "".join(str(x) for x in indexToPath(i,depth))
		print "path len %s"%len(path)
		print "extrapolated %s"%simDer(path,depth)
		#print "empircal %s"%dertab[i]
		print '\n'
	#main(argv)
