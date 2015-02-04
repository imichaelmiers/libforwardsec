from scipy import special
import matplotlib.pyplot as plt
mili_p_s = 10
s_p_m = 60
m_p_h = 60
h_p_d = 24
d_p_y  = 365

bins = 10 *s_p_m * m_p_h * h_p_d *d_p_y
rates = [1,5,10,15,20,25,30,35,40,45,50,55,60]
collisions_10= [(special.binom(mph * h_p_d*d_p_y,2) *1.0/ bins) for mph in rates]
bins = 100 *s_p_m * m_p_h * h_p_d *d_p_y
collisions_100= [(special.binom(mph * h_p_d*d_p_y,2) *1.0/ bins) for mph in rates]
bins = 1000 *s_p_m * m_p_h * h_p_d *d_p_y
collisions_1000= [(special.binom(mph * h_p_d*d_p_y,2) *1.0/ bins) for mph in rates]

bins = 1 *s_p_m * m_p_h * h_p_d *d_p_y
collisions_1= [(special.binom(mph * h_p_d*d_p_y,2) *1.0/ bins) for mph in rates]

#plt.plot(rates,collisions_1,'ko',label='1  second')
plt.plot(rates,collisions_10,'bo',label='1/10 of a second')
plt.plot(rates,collisions_100,'ro',label='1/100 of a second')
plt.plot(rates,collisions_1000,'yo',label='1/1000 of a second')
plt.gca().legend(loc='upper left',shadow=True)

plt.show()
for i in xrange(len(rates)):
	print "%s : %s"%(rates[i],collisions[i])