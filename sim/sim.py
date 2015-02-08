from subprocess import PIPE, Popen
from sys import argv
from operator import itemgetter
import   math, itertools
from random import expovariate
from numpy  import array,save,vstack,random,arange
from collections import namedtuple
import click
mili_p_s = 1000
s_p_m = 60
m_p_h = 60
h_p_d = 24
d_p_y  = 365

milipy = mili_p_s*s_p_m*m_p_h*h_p_d*d_p_y
spy = s_p_m*m_p_h*h_p_d*d_p_y
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
			raise Exception("Cannot derive key %s"%len(path))
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
	mili_p_s = 1000

	intervalsizes = [.001,.01,.1,1,10,100,1000,10000]
	depths = [int(math.ceil(math.log(spy/i,2))) for i in intervalsizes]
	print depths
	results = []
	for i,d in zip(intervalsizes,depths):
		rs =sim_acc_fast(path = '', window = 1,
		 	avg = msgs_per_second, interval_length = i, timeduration = 100000,depth = d)
		results.append(rs)
	np = array(results)
	return np

def sim_acc_fast(path,window,avg,interval_length,timeduration,depth=31,numtags=1,iterations=1):
	latency = 0
	interval = 0
	#print p.stdout.readline()
	elapsed_time = 0
	intervals = []
	print "window %s seconds"%window
	print "avg %s msgs per second "%avg
	print "interval length: %s seconds"%interval_length
	print "duration %s seconds "%timeduration
	print "tree dpeth %s"%depth
	print "iterations %s"%iterations
	while elapsed_time < timeduration:
		elapsed_time += expovariate(avg)
		interval = int(math.floor(elapsed_time)/interval_length)+1 
		#print "avg: %s , pois : %s"%(avg,arrived_msgs)
		intervals.append(interval)

	#print ' '.join(str(x) for x in intervals)
	print "total number of messages = %s"%len(intervals)
	keys={'0':0,'1':0}
	ctr=0
	prev =0
	intetvalctr = 0
	msg_ctr = 0
	DecTime = 0
	Dertime = 0
	PunTime = 0
	MaxDer = 0
	MaxDec = 0

	with click.progressbar(intervals,
                       label='%d msgs a second for %d seconds with %d size intervals'%(avg,timeduration,interval_length)
                       ) as bar:
		for i in bar:
			if i != prev:
				prev = i
				path = "".join(str(x) for x in indexToPath(i,depth))
				d= simDerKeys(keys,path,depth)
				Dertime += d
				intetvalctr+=1
			msg_ctr+=1
			dd = simDec(keys[path])
			PunTime += simPunc(keys,path)

			MaxDer = max(MaxDer,d)
			MaxDec = max(MaxDec,dd)

			DecTime +=dd
			msg_ctr+=1
	ms = 1000# max(1,interval) #  int_length_in_ms  #interval_length * numintervals
	return [window/ms,interval_length,depth,latency/ms,DecTime/ms,Dertime/ms,PunTime/ms,MaxDer/ms,MaxDec/ms]

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
def sim(path,window,avg,interval_length,timeduration,depth=31,numtags=1,iterations=1):
	latency = 0
	interval = 0
	#print p.stdout.readline()
	elapsed_time = 0
	intervals = []
	# print "window %s seconds"%window
	# print "avg %s msgs per second "%avg
	# print "interval length: %s seconds"%interval_length
	# print "duration %s seconds "%timeduration
	# print "tree dpeth %s"%depth
	# print "iterations %s"%iterations
	while elapsed_time < timeduration:
		elapsed_time += expovariate(avg)
		interval = int(math.floor(elapsed_time)/interval_length)+1 
		#print "avg: %s , pois : %s"%(avg,arrived_msgs)
		intervals.append(interval)

	args = "%d %s %d %d \n"%(window,depth,numtags,iterations)
	#print ' '.join(str(x) for x in intervals)
	#print "total number of messages = %s"%len(intervals)
	p = Popen(path, stdin=PIPE, stdout = PIPE, bufsize=1)
	p.stdin.write(args)
	ctr=0
	with click.progressbar(intervals,
                       label='%f msgs a second for %d seconds with %f size intervals'%(avg,timeduration,interval_length)
                       ) as bar:
		for i in bar:
			p.stdin.write("%d\n"%i)
			ctr +=1
			if ctr%10 ==0:
				p.stdout.readline()
	p.stdin.close()
	p.wait()
	results = []
	lines = p.stdout.readlines()
	while 'DeriveTime' not in lines[0]:
		lines = lines[1:]
	results = [window,avg,depth,interval_length,timeduration,latency]
	results += parseTimer(lines[0:5])
	results += parseTimer(lines[5:10])
	results += parseTimer(lines[10:15])
	results.append(float(lines[15].split()[1])/1024)
	#print lines[15]
	return results
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

def main(argv):
	intervalsizes = [.001,.01,.1,1,10,100,1000]
	depths = [min(30,int(math.ceil(math.log(spy/i,2)))) for i in intervalsizes]
	path = argv[1]
	name = argv[2]
	print "path: %s"%path
	msgs_per_second =  1
	rates = [.01,.001,.0001,1]
	window = 1000.0
	results=[]
	with click.progressbar(itertools.product(rates,zip(intervalsizes,depths))) as foo:
		for (r,(i,d)) in foo:
			rs =sim(path = path, window = window,
			 	avg = r, interval_length = i, timeduration =2*window ,depth = d)
			results.append(rs)
			np = array(results)
			save(name,np)

	#np = array(results)

	print np
#'DeriveTime\n',
 # '\t\t wall average 580.628 stdev 540.212 total 16838.2\n',
 # '\t\t user average 578.966 stdev 539.546 total 16790\n',
 # '\t\t system average 0.344828 stdev 1.85695 total 10\n',
 # '\t\tcputime average 579.31 stdev 539.689 total 16800\n',
 # 'DecTime\n',
 # '\t\t wall average 32.3056 stdev 0.138637 total 613.807\n',
 # '\t\t user average 32.6316 stdev 4.52414 total 620\n',
 # '\t\t system average 0 stdev 0 total 0\n',
 # '\t\tcputime average 32.6316 stdev 4.52414 total 620\n',
 # 'PunTime\n',
 # '\t\t wall average 38.6533 stdev 0.250229 total 734.413\n',
 # '\t\t user average 39.4737 stdev 2.29416 total 750\n',
 # '\t\t system average 0 stdev 0 total 0\n',
 # '\t\tcputime average 39.4737 stdev 2.29416 total 750\n',
 # 'MaxSize\t641560\n']
def parseLine(line):
	a,b,c = itemgetter(2,4,6)(line.split())
	return [float(a)/1000,float(b)/1000,float(c)/1000]
def parseTimer(lines):
	result = []
	for l in lines[1:]:
			result += parseLine(l)
	return result
def pprint(data):
	for i,v in enumerate(['wall','user','system','cputime']):
		print "\t\t %s average %f stdev %f total %f "%(v,data[0+i*3],data[1+i*3],data[2+i*3])

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
	# depth = 64
	# for i in xrange(depth):
	# 	path = "".join(str(x) for x in indexToPath(i,depth))
	# 	print "path len %s"%len(path)
	# 	print "extrapolated %s"%simDer(path,depth)
	# 	#print "empircal %s"%dertab[i]
	# 	print '\n'
	main(argv)
