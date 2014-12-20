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
	ZR LagrangeBasisCoefficients(uint j,const ZR &x , const vector<ZR> & polynomial_xcordinates) const;


	template <class type> type LagrangeInterpInExponent( const ZR &x, const vector<ZR> & polynomial_xcordinates,
			const vector<type> & exp_polynomial_ycordinates, const uint degree) const;
	ZR LagrangeInterp(const ZR &x , const vector<ZR> & polynomial_xcordinates,
			const vector<ZR> & polynomial_ycordinates, uint degree) const;


	G1 vG1(const std::vector<G1> & gqofxG1, const ZR & x) const;
	G2 vG2(const std::vector<G2> & gqofxG2, const ZR & x) const;

	void keygen(const BbhHIBEPublicKey & pkhibe, ZR & gamma,GmppkePublicKey & pk, GmppkePrivateKey & sk) const;
	void skgen(const GmppkePublicKey &pk,const ZR & alpha, GmppkePrivateKeyShare & skentry0) const;
	void puncture(const GmppkePublicKey & pk, GmppkePrivateKey & sk, const ZR & tag) const;
	void encrypt(const GmppkePublicKey & pk,const GT & M, const ZR & s,  const std::vector<ZR> & tags, GmmppkeCT & ct) const;
	void decrypt(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const GmmppkeCT & ct, GT & M) const;
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
std::vector<ZR>  indexToPath(uint index,uint l);
uint pathToIndex(std::vector<ZR> & path, uint l);
class Bbghibe
{
public:
	PairingGroup group;
	Bbghibe(){};
	~Bbghibe() {};

	void setup(int l, BbhHIBEPublicKey & pk, G2 & msk) const;

	void keygen(const BbhHIBEPublicKey & pk,const G2 & msk,const  std::vector<ZR> & id, BbghPrivatekey & sk) const;
    void keygen(const BbhHIBEPublicKey & pk,const  BbghPrivatekey & sk, const std::vector<ZR> & id,BbghPrivatekey & skout) const;

	void encrypt(const BbhHIBEPublicKey & pk, const GT & M ,const ZR &s, const  std::vector<ZR>  & id, BbghCT & ct) const;
	void encrypt(const BbhHIBEPublicKey & pk, const GT & M, const std::vector<ZR>  & id, BbghCT & ct) const;

	void decrypt(const BbghPrivatekey & sk, const BbghCT & ct, GT & m) const; // decrypt for PFSE
	GT decrypt(const BbghPrivatekey & sk,const BbghCT & ct) const; // actual decrypt

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
friend bool operator==(const PseCipherText& l,const PseCipherText& r){
		return l.ct0 == r.ct0 && l.hibeCT == r.hibeCT && l.ppkeCT == r.ppkeCT
				&& l.interval == r.interval && l.xorct == r.xorct;
	}
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
	 bool punctured() const{
		return ppkeSK.shares.size() > 1;
	}
};

class PfseKeyStore{
public:
	map<uint,PfsePuncturedPrivateKey> puncturedKeys;
	map<uint,BbghPrivatekey> unpucturedHIBEKeys;
	GmppkePrivateKey unpucturedPPKEKey;
	PfsePuncturedPrivateKey getKey(unsigned int i) const;
	void updateKey(unsigned int i, const PfsePuncturedPrivateKey & p);
	void addkey(unsigned int i, const BbghPrivatekey & h);
	void erase(unsigned int i);
	bool hasKey(const unsigned int i) const;
	bool needsChildKeys(const unsigned int i,const unsigned int d) const;
};

class Pfse
{
public:
	pfsepubkey pk;
	PfseKeyStore privatekeys;


	Pfse(uint d);
	/**Generates the public and private key. These are stored in object
	 *
	 */
	void keygen();

	/** Encryptes a message. Messages are limmited to 256 bits. (e.g. and AES key).
	 *
	 * @param pk the public key of the recipient
	 * @param aes_key the message
	 * @param interval the time interval the message is in
	 * @param tags the tags for the message
	 * @return the ciphertext
	 */
	PseCipherText encrypt (const pfsepubkey & pk, const AESKey aes_key, const uint interval, const vector<string> tags) const;

	/**Decrypt   a message using the private key stored in the object.
	 *
	 * @param ct the ciphertext
	 * @return the decrypted message.
	 */
	AESKey decrypt( const PseCipherText &ct) const;
	
	/**Derives the keys needed to decrypt the next interval.
	 *
	 */
	void prepareNextInterval();
	/** Erases the key for a given interval
	 *
	 * @param interval the interval to erase.
	 */
	void eraseKey(unsigned int interval);
    /** Punctures the key for the given time period.
     *
     * @param interval the time period
     * @param str the tag to puncture on
     */
	void puncture(uint interval, string str);
	/**Punctures the key for the current interval.
	 *
	 * @param str the tag to puncture on.
	 */
	void puncture( string str);


private:
	PairingGroup group;
	Bbghibe hibe;
	Gmppke ppke;
	uint depth;
	void bindKey(PfsePuncturedPrivateKey & k);
	PseCipherText encryptFO( const pfsepubkey & pk, const AESKey & bitmsg,
			              const uint interval, const vector<ZR>  & tags) const;
	PseCipherText encryptFO( const pfsepubkey & pk, const AESKey & bitmsg,
			const GT & x, const uint interval, const vector<ZR>  & tags) const;

	PseCipherText encrypt( const pfsepubkey & pk, const GT & M,              const uint interval, const vector<ZR>  & tags) const;
	PseCipherText encrypt( const pfsepubkey & pk, const GT & M,const ZR & s, const uint interval, const vector<ZR>  & tags) const;


	AESKey decryptFO(const PfsePuncturedPrivateKey &sk, const PseCipherText &ct) const;
	GT decryptGT(const PfsePuncturedPrivateKey & sk, const PseCipherText &ct) const;
	uint nextParentInterval;
};



#endif

