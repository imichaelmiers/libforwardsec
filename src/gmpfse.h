#ifndef PFSE_H
#define PFSE_H

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <bitset>
#include "relic_wrapper/relic_api.h"
using namespace std;
#ifndef AES_SECURITY
#define AES_SECURITY 256
#endif
class BadCiphertext : public std::invalid_argument
{
public:
    BadCiphertext(std::string const& error)
        : std::invalid_argument(error)
    {}
};

typedef unsigned int uint;

class GmppkePublicKey{
public:
	PairingGroup group;
	G1 gG1;
	G2 gG2;
	G2 g1;
	G1 g2G1;
	G2 g2G2;
	unsigned int d;
	std::vector<G1> gqofxG1;
	std::vector<G2> gqofxG2;
};
 class GmppkePrivateKeyShare{
public:
	PairingGroup group;
	G2 sk1;
	G2 sk2;
	G2 sk3;
	ZR sk4;
};
 class GmppkePrivateKey{
public:
	std::vector<GmppkePrivateKeyShare> shares;
//	friend bool operator==(const GmppkePrivateKey & l, const GmppkePrivateKey &r){
//		return l.shares == r.shares;
//	}
 };

class GmmppkeCT{
public:
	PairingGroup group;
	GT ct1;
	G1 ct2;
	std::vector<G1> ct3;
	std::vector<ZR> tags;
friend bool operator==(const GmmppkeCT& x,const GmmppkeCT& y){
	return x.ct1 == y.ct1 && x.ct2 == y.ct2 && x.ct3 == y.ct3 && x.tags == y.tags;
}

};
class BbhHIBEPublicKey;
class Gmppke
{
public:

	Gmppke(){};
	~Gmppke() {};
	PairingGroup group;
	ZR LagrangeBasisCoefficients(uint j,const ZR &x , const vector<ZR> & polynomial_xcordinates);


	template <class type> type LagrangeInterpInExponent( const ZR &x, const vector<ZR> & polynomial_xcordinates,
    const vector<type> & exp_polynomial_ycordinates, const uint degree);
	ZR LagrangeInterp(const ZR &x , const vector<ZR> & polynomial_xcordinates,
    const vector<ZR> & polynomial_ycordinates, uint degree);


	G1 vG1(const std::vector<G1> & gqofxG1, const ZR & x);
	G2 vG2(const std::vector<G2> & gqofxG2, const ZR & x);

	void keygen(const BbhHIBEPublicKey & pkhibe, ZR & gamma,GmppkePublicKey & pk, GmppkePrivateKey & sk);
	void skgen(const GmppkePublicKey &pk,const ZR & alpha, GmppkePrivateKeyShare & skentry0);
	void puncture(const GmppkePublicKey & pk, GmppkePrivateKey & sk, const ZR & tag);
	void encrypt(const GmppkePublicKey & pk,const GT & M, const ZR & s,  const std::vector<ZR> & tags, GmmppkeCT & ct);
	void decrypt(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const GmmppkeCT & ct, GT & M);
};




class BbghPrivatekey{
public:
	PairingGroup group;
	G2 a0;
	G2 a1;
	std::vector<G1> b;
	std::vector<G2> bG2;
};

class BbhHIBEPublicKey{
public:
int l;
PairingGroup group;
G1 gG1;
G2 gG2;
G2 g1;
G1 g2G1;
G2 g2G2;
G1 g3G1;
G2 g3G2;
std::vector<G1> hG1;
std::vector<G2> hG2;
};

class BbghCT{
public:
	PairingGroup group;
	GT A;
	G1 B;
	G1 C;
friend bool operator==(const BbghCT& x,const BbghCT& y){
	return x.A == y.A && x.B == y.B && x.C == y.C;
}
};
class Bbghibe
{
public:
	PairingGroup group;
	Bbghibe(){};
	~Bbghibe() {};
	std::vector<ZR>  indexToPath(uint index,uint l);
	uint pathToIndex(std::vector<ZR> & path, uint l);
	void setup(int l, BbhHIBEPublicKey & pk, G2 & msk);

	void keygen(const BbhHIBEPublicKey & pk,const G2 & msk,const  std::vector<ZR> & id, BbghPrivatekey & sk);
    void keygen(const BbhHIBEPublicKey & pk,const  BbghPrivatekey & sk, const std::vector<ZR> & id,BbghPrivatekey & skout);

	void encrypt(const BbhHIBEPublicKey & pk, const GT & M ,const ZR &s, const  std::vector<ZR>  & id, BbghCT & ct);
	void encrypt(const BbhHIBEPublicKey & pk, const GT & M, const std::vector<ZR>  & id, BbghCT & ct);

	void decrypt(const BbghPrivatekey & sk, BbghCT & ct, GT & m); // decrypt for PFSE
	GT decrypt(const BbghPrivatekey & sk, BbghCT & ct); // actual decrypt

};
typedef  bitset<256> AESKey;

class PseCipherText{
public:
	PairingGroup group;
	GT ct0;
	BbghCT hibeCT;
	GmmppkeCT ppkeCT;
	unsigned int interval;
	AESKey xorct;
};


typedef BbghPrivatekey HIBEkey;
typedef GmppkePrivateKey  PPKEKey;
class pfsepubkey{
public:
	BbhHIBEPublicKey hibe;
	GmppkePublicKey ppke;
};

class PfsePuncturedPrivateKey{
public:
	BbghPrivatekey hibeSK;
	GmppkePrivateKey ppkeSK;
	const bool punctured(){
		return ppkeSK.shares.size() > 1;
	}
};

class PfseKeyStore{
public:
	map<uint,PfsePuncturedPrivateKey> puncturedKeys;
	map<uint,BbghPrivatekey> unpucturedHIBEKeys;
	GmppkePrivateKey unpucturedPPKEKey;
	const PfsePuncturedPrivateKey getKey(unsigned int i);
	void updateKey(unsigned int i, const PfsePuncturedPrivateKey & p);
	void addkey(unsigned int i, const BbghPrivatekey & h);
	void erase(unsigned int i);
};

class Pfse
{
public:
	PairingGroup group;
	pfsepubkey pk;
	PfseKeyStore privatekeys;
	PPKEKey unpucturedKey;
	PPKEKey activeKey;

	Pfse(uint d);
	void keygen();
	PseCipherText encrypt(pfsepubkey & pk, AESKey aes_key,uint interval,vector<string> tags);
	AESKey decrypt(PseCipherText &ct);
	
	void prepareNextInterval();
	//void deleteInterval(uint interval);

	void puncture(uint interval, string str);
	void puncture( string str);

	PseCipherText encrypt(pfsepubkey & pk, GT & M,uint interval, vector<ZR>  & tags);
	GT decryptGT(PseCipherText &ct);	
private:
	Bbghibe hibe;
	Gmppke ppke;
	uint depth;
	void updateppkesk(GmppkePrivateKeyShare & skentry,GmppkePrivateKeyShare & skentryold);
	PseCipherText encryptFO(pfsepubkey & pk, AESKey & bitmsg,uint interval,vector<ZR>  & tags);
	PseCipherText encryptFO(pfsepubkey & pk,  AESKey & bitmsg,GT & x, uint interval,vector<ZR>  & tags);

	PseCipherText encrypt(pfsepubkey & pk, GT & M, ZR & s,uint interval, vector<ZR>  & tags);

	AESKey decryptFO(PseCipherText &ct);

	uint latestInterval;
};



#endif

