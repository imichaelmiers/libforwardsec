/*
 * GMPpke.h
 *
 *  Created on: Dec 21, 2014
 *      Author: imiers
 */

#ifndef GMPPKE_H_
#define GMPPKE_H_
#include "forwardsec.h"


class GmppkePublicKey: public  virtual  baseKey{
public:
	G2 ppkeg1;
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

class PartialGmmppkeCT{
public:
	PairingGroup group;
	G1 ct2;
	std::vector<G1> ct3;
	std::vector<ZR> tags;
	friend bool operator==(const PartialGmmppkeCT& x,const PartialGmmppkeCT& y){
		return x.ct2 == y.ct2 && x.ct3 == y.ct3 && x.tags == y.tags;
	}
};
class GmmppkeCT: public PartialGmmppkeCT{
public:
	GmmppkeCT(const  PartialGmmppkeCT & c) : PartialGmmppkeCT(c){}
	GT ct1;
	friend bool operator==(const GmmppkeCT& x,const GmmppkeCT& y){
		return x.ct1 == y.ct1 && x.ct2 == y.ct2 && x.ct3 == y.ct3 && x.tags == y.tags;
	}
};
class Gmppke
{
public:

	Gmppke(){};
	~Gmppke() {};

	PairingGroup group;
	G1 vG1(const std::vector<G1> & gqofxG1, const ZR & x) const;
	G2 vG2(const std::vector<G2> & gqofxG2, const ZR & x) const;

	void keygen(GmppkePublicKey & pk, GmppkePrivateKey & sk,const unsigned int & d = 1) const;
	void keygenPartial(const ZR & gamma,GmppkePublicKey & pk, GmppkePrivateKey & sk,const unsigned int & d=1) const;
	GmppkePrivateKeyShare skgen(const GmppkePublicKey &pk,const ZR & alpha ) const;
	void puncture(const GmppkePublicKey & pk, GmppkePrivateKey & sk, const ZR & tag) const;
	GmmppkeCT encrypt(const GmppkePublicKey & pk,const GT & M,const std::vector<ZR> & tags) const;
	PartialGmmppkeCT blind(const GmppkePublicKey & pk, const ZR & s,  const std::vector<ZR> & tags) const;

	GT recoverBlind(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const PartialGmmppkeCT & ct ) const;
	GT decrypt(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const GmmppkeCT & ct ) const;
	//For testing purposes only
	GT decrypt_unchecked(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const GmmppkeCT & ct ) const;

};
/** Checks if you can decrypt a pfse ciphertext
 *
 * @param sk
 * @param ct
 * @return
 */
bool canDecrypt(const GmppkePrivateKey & sk,const PartialGmmppkeCT & ct);


#endif /* GMPPKE_H_ */
