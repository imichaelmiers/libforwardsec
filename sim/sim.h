#include <boost/timer/timer.hpp>
#include <math.h>       /* sqrt */
#include <stdexcept>
using  boost::timer::cpu_timer;
using  boost::timer::cpu_times;
using  boost::timer::nanosecond_type;
using namespace std;
nanosecond_type const milisecond =1000000LL;
// used from http://www.johndcook.com/blog/standard_deviation/
class RunningStat
    {
    public:
        RunningStat() : m_n(0) {}

        void Clear()
        {
            m_n = 0;
        }

        void Push(double x)
        {
            m_n++;
            total +=x;
            // See Knuth TAOCP vol 2, 3rd edition, page 232
            if (m_n == 1)
            {
                m_oldM = m_newM = x;
                m_oldS = 0.0;
            }
            else
            {
                m_newM = m_oldM + (x - m_oldM)/m_n;
                m_newS = m_oldS + (x - m_oldM)*(x - m_newM);
    
                // set up for next iteration
                m_oldM = m_newM; 
                m_oldS = m_newS;
            }
        }

        int NumDataValues() const
        {
            return m_n;
        }

        double Mean() const
        {
            return (m_n > 0) ? m_newM : 0.0;
        }

        double Variance() const
        {
            return ( (m_n > 1) ? m_newS/(m_n - 1) : 0.0 );
        }

        double StandardDeviation() const
        {
            return sqrt( Variance() );
        }
    friend ostream& operator<<(ostream& os, const RunningStat& b){
		os << "average " << b.Mean()/milisecond << " stdev " << b.StandardDeviation()/milisecond << " total " << b.total/milisecond;
        return os;
    }
    private:
        int m_n;
        double m_oldM, m_newM, m_oldS, m_newS;
        double total = 0;
    };

class CPUTimeAndAvg
{
public:
	CPUTimeAndAvg(){}
	void start(){
		if(!stoped){
			throw invalid_argument("Cannot start timer. Already running ");
		}
        stoped = false;
		cpu_timer timer;
		t=timer;
	}
	void stop(){
		if(stoped){
			throw invalid_argument("Cannot stop timer. Already stopped ");
		}
		t.stop();
        stoped = true;
	}
	void reg(){
		if(!stoped){
			throw invalid_argument("Cannot avg timer. Not stopped ");

		}
		cpu_times e = t.elapsed();
		wall.Push(e.wall);
		user.Push(e.user);
	    system.Push(e.system);
	    cputime.Push(e.system+e.user);
	}
	void resume(){
		t.resume();
	}
	bool stoped = true;
	cpu_timer t;
	RunningStat wall;
	RunningStat user;
	RunningStat system;
	RunningStat cputime;
    friend ostream& operator<<(ostream& os, const CPUTimeAndAvg& b){
    	os << "\t wall " << b.wall << endl <<"\t\t user "<< b.user << endl << "\t\t system " << b.system << endl << "\t\tcputime " << b.cputime;
        return os;
    }

};