#include <iostream>
#include <vector>
#include <tuple>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include "locale"
#include "gmpfse.h"
#include "Benchmark.h"
using namespace std;
using namespace forwardsec;
using namespace relicxx;
bytes testVector = {{0x3a, 0x5d, 0x7a, 0x42, 0x44, 0xd3, 0xd8, 0xaf, 0xf5, 0xf3, 0xf1, 0x87, 0x81, 0x82, 0xb2,
						  0x53, 0x57, 0x30, 0x59, 0x75, 0x8d, 0xe6, 0x18, 0x17, 0x14, 0xdf, 0xa5, 0xa4, 0x0b,0x43,0xAD,0xBC}};
std::vector<string>makeTags(unsigned int n){
	std::vector<string> tags(n);
	for(unsigned int i=0;i<n;i++){
		tags[i] = "tag"+std::to_string(i);
	}
	return tags;
}
Benchmark benchKeygen(const unsigned int iterations, const unsigned int d =31,
		const unsigned int n = 1){
	Benchmark b;
    for(unsigned int i=0;i < iterations;i++){
		Pfse test(d,n);
		b.start();
		test.keygen();
		b.stop();
		b.computeTimeInMilliseconds();
	}
	return b;
}
Benchmark benchEnc(const unsigned int iterations, const unsigned int d =31,
		const unsigned int n = 1){
	Pfse test(d,n);
    test.keygen();
    Benchmark benchE;
    for(unsigned int i=0;i < iterations;i++){
        benchE.start();
        test.encrypt(test.pk,testVector,1,makeTags(n));
        benchE.stop();
        benchE.computeTimeInMilliseconds();
    }
	   return benchE;
}
Benchmark benchDec(const unsigned int iterations, const unsigned int d =31,
		const unsigned int n = 1){
	Pfse test(d,n);
    test.keygen();
    Benchmark benchD;
    PseCipherText ct = test.encrypt(test.pk,testVector,1,makeTags(n));;
    for(unsigned int i=0;i < iterations;i++){
        benchD.start();
        test.decrypt(ct);
        benchD.stop();
        benchD.computeTimeInMilliseconds();
    }
	   return benchD;
}
Benchmark benchPuncFirst(const unsigned int iterations, const unsigned int d =31,
		const unsigned int n = 1){

    Benchmark benchP;
    for(unsigned int i=0;i < iterations;i++){
    	Pfse test(d,n);
        test.keygen();
        benchP.start();
        test.puncture("punc"+std::to_string(i));
        benchP.stop();
        benchP.computeTimeInMilliseconds();
    }
	   return benchP;
}

Benchmark benchPunc(const unsigned int iterations, const unsigned int d =31,
		const unsigned int n = 1){

    Benchmark benchP;
    for(unsigned int i=0;i < iterations;i++){
    	Pfse test(d,n);
        test.keygen();
        test.puncture("punc");
        benchP.start();
        test.puncture("punc"+std::to_string(i));
        benchP.stop();
        benchP.computeTimeInMilliseconds();
    }
	   return benchP;
}

Benchmark benchNextInterval(const unsigned int iterations, const unsigned int d =31,
		const unsigned int n = 1){

    Benchmark benchN;
    for(unsigned int i=0;i < iterations;i++){
    	Pfse test(d,n);
        test.keygen();
        benchN.start();
        test.prepareNextInterval();
        benchN.stop();
        benchN.computeTimeInMilliseconds();
    }
	   return benchN;

}
std::vector<std::tuple<unsigned int ,Benchmark>> benchDecPunctured(const unsigned int iterations,
		const unsigned int punctures, const unsigned int puncture_steps,
		const unsigned int d =31,
		const unsigned int n = 1){
	std::vector<std::tuple<unsigned int ,Benchmark>>  b;
	for(unsigned int p = 0; p < punctures;p+=puncture_steps){
		cout << ".";
		cout.flush();
        Benchmark benchDP;
    	Pfse test(d,n);
        test.keygen();
        PseCipherText ct = test.encrypt(test.pk,testVector,1,makeTags(n));;
        for(unsigned int i =0;i<p;i++){
        	if(i%10 == 0) cout << ".";
            test.puncture("punc"+std::to_string(i));
        }
        for(unsigned int i=0;i < iterations;i++){
        	if(i%10 == 0) cout << ".";
           benchDP.start();
		   test.decrypt(ct);
		   benchDP.stop();
		   benchDP.computeTimeInMilliseconds();
        }
        b.push_back(std::make_tuple (p,benchDP));
	}
	cout <<endl;
	return b;
}
template <class T>
void sizes(const unsigned int d =31,const unsigned int n = 1){
	Pfse test(d,n);
    test.keygen();
    PairingGroup group;
    {
    		ZR z = group.randomZR();
    		stringstream ss;
    		{
    			T oarchive(ss);
    			oarchive(z);
    		}
    		cout << "\tZR size:\t" << ss.tellp() <<" bytes " << endl;
	}
    {
    		G1 g = group.randomG1();
    		stringstream ss;
    		{
    			T oarchive(ss);
    			oarchive(g);
    		}
    		cout << "\tG1 size:\t" << ss.tellp() <<" bytes " << endl;
	}

    {
    		G2 g = group.randomG2();
    		stringstream ss;
    		{
    			T oarchive(ss);
    			oarchive(g);
    		}
    		cout << "\tG2 size:\t" << ss.tellp() <<" bytes " << endl;
	}
    {
    		GT g = group.randomGT();
    		stringstream ss;
    		{
    			T oarchive(ss);
    			oarchive(g);
    		}
    		cout << "\tGT size:\t" << ss.tellp() <<" bytes " << endl;
	}
	{
		stringstream ss;
		{
			T oarchive(ss);
			oarchive(test.pk);
		}
		cout << "\tPK size:\t" << ss.tellp() <<" bytes " << endl;
	}
	{
		stringstream ss;
		{
			T oarchive(ss);
			oarchive(test.privatekeys);
		}
		cout << "\tSK size:\t" << ss.tellp() <<" bytes " << endl;
	}

    PseCipherText ct = test.encrypt(test.pk,testVector,1,makeTags(n));;
	{
		stringstream ss;
		{
			T oarchive(ss);
			oarchive(ct);
		}
		cout << "\tCT size:\t" << ss.tellp() <<" bytes " << endl;
	}
}
int main()
{
	relicResourceHandle h;
	unsigned int i = 10;
	unsigned int d = 31;
	unsigned int n = 1;
    Benchmark K,E,D,PF,PS,N,DP;
    cout << "Benchmarking " << i << " iterations of Puncturable forward secure encryption with depth " <<
    		d << " and " << n << " tags" << endl;
    std::locale::global(std::locale(""));
    std::cout.imbue(std::locale());
    cout <<"Sizes(BinaryOutputArchive) POINT_COMPRESSION=" << POINT_COMPRESS << ":"<< endl;
    sizes<cereal::BinaryOutputArchive>();
    cout <<"Sizes(PortableBinaryOutputArchive) POINT_COMPRESSION=" << POINT_COMPRESS << ":"<< endl;
    sizes<cereal::PortableBinaryOutputArchive>();
    cout <<"Sizes(JSONOutputArchive) POINT_COMPRESSION=" << POINT_COMPRESS << ":"<< endl;
    sizes<cereal::JSONOutputArchive>();
    cout <<"Performance:" << endl;

    K = benchKeygen(i,d,n);
    cout << "\tkeygen:\t\t\t" << K << endl;

    E = benchEnc(i,d,n);
    cout << "\tEnc:\t\t\t" << E << endl;

    D = benchDec(i,d,n);
    cout << "\tDec(unpunctured):\t" << D << endl;

    PF = benchPuncFirst(i,d,n);
    cout << "\tInitial Puncture:\t" << PF << endl;
    PS = benchPunc(i,d,n);
    cout << "\tSubsequent Puncture:\t" << PS << endl;

    N = benchNextInterval(i,d,n);
    cout << "\tNextInterval:\t\t" << N << endl;
	cout << "\tDec(punctured):" << endl;
    auto marks = benchDecPunctured(i,100,20,d,n);
    for(auto m:marks){
    	cout <<"\t\t" << std::get<0>(m) <<"\t" << std::get<1>(m) << endl;
    }
}
