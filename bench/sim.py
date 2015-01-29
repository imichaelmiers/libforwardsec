from subprocess import PIPE, Popen
from sys import argv
from operator import itemgetter
import random , math
from numpy  import array,save,vstack
def main(argv):
	sims = [
	(10*60,60)
	]
	results = []
	iterations = int(argv[3])
	duration = int(argv[2])
	path = argv[1]
	print "iterations: %d "%iterations
	print "duration: %d"%duration 
	print "path: %s"%path

	for window,rate in sims:
		print "w: %s r:%s"%(window,rate)


		rs =sim(path = path, window = window,
		 	avg = rate, iterations = iterations, msgs = duration )
		results.append(rs)

	np = array(results)

def sim(path,window,avg,iterations,msgs,depth=31,numtags=1):
	rate = 1.0/avg;
	p = Popen(path, stdin=PIPE, stdout = PIPE, bufsize=1)
	args = "%s %s %s %s \n"%(window,30,numtags,iterations)
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
	DeriveTime = p.stdout.readline()
	DecTime =  p.stdout.readline()
	PunTime = p.stdout.readline()
	MaxSize = p.stdout.readline();
	dert,derdev = itemgetter(2,4)(DeriveTime.split())
	dect,decdev = itemgetter(2,4)(DecTime.split())
	punt,pundev = itemgetter(2,4)(PunTime.split())
	maxs = float(MaxSize.split()[1]);
	print DeriveTime
	print DecTime
	print PunTime
	print MaxSize
	return [window,avg,float(dert),float(derdev),float(dect),float(maxs),float(decdev),float(punt),float(pundev)]
if __name__ == "__main__":
	main(argv)