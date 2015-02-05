from scipy.stats import poisson
from scipy import special
import matplotlib.pyplot as plt
import itertools
mili_p_s = 1000
s_p_m = 60
m_p_h = 60
h_p_d = 24
d_p_y  = 365
spy = s_p_m*m_p_h*h_p_d*d_p_y

rates = [1,.1,.01,.001]
intervals = [.001,.1]
for r,i in itertools.product(rates,intervals):
	print "rate = %s messages per second , interval = %s seconds"%(r,i)
	lost = (r*i - poisson.pmf(1,r*i))*spy/i
	expected = r*i*spy/i
	print "\t expected poission %s"%lost
	lost_bb = special.binom(spy*r,2) *1.0/ (spy/i)
	print "\t balls and bins expected %s"%lost_bb