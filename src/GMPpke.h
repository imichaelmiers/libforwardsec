/*
 * GMPpke.h
 *
 *  Created on: Dec 21, 2014
 *      Author: imiers
 */

#ifndef GMPPKE_H_
#define GMPPKE_H_
#include <cereal/types/base_class.hpp>
#include <cereal/access.hpp>
#include "forwardsec.h"
#include <cereal/types/vector.hpp>


class GmppkePublicKey: public  virtual  baseKey{
public:
	G2 ppkeg1;
	unsigned int d;
	std::vector<G1> gqofxG1;
	std::vector<G2> gqofxG2;
	friend bool operator==(const GmppkePublicKey& x, const GmppkePublicKey& y){
		return  ((baseKey)x == (baseKey)y &&
				x.ppkeg1 == y.ppkeg1 && x.d == y.d && x.gqofxG1 == y.gqofxG1 &&
				x.gqofxG2 == y.gqofxG2);
	}
	friend bool operator!=(const GmppkePublicKey& x, const GmppkePublicKey& y){
		return !(x==y);
	}
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(cereal::virtual_base_class<baseKey>(this),
				ppkeg1,d,gqofxG1,gqofxG2);
	}
	friend class cereal::access;
};
// cereal can't find the function if we don't do this.
namespace cereal
{
 template <class Archive>
 struct specialize<Archive, GmppkePublicKey, cereal::specialization::member_serialize> {};
 // cereal no longer has any ambiguity when serializing MyDerived
}

 class GmppkePrivateKeyShare{
public:
	PairingGroup group;
	G2 sk1;
	G2 sk2;
	G2 sk3;
	ZR sk4;
	friend bool operator==(const GmppkePrivateKeyShare& x, const GmppkePrivateKeyShare& y){
		return  (x.sk1 == y.sk1 && x.sk2 == y.sk2 && x.sk3 == y.sk3 &&
				x.sk4 == y.sk4);
	}
	friend bool operator!=(const GmppkePrivateKeyShare& x, const GmppkePrivateKeyShare& y){
		return !(x==y);
	}
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(sk1,sk2,sk3,sk4);
	}
	friend class cereal::access;
};
 class GmppkePrivateKey{
public:
	std::vector<GmppkePrivateKeyShare> shares;
	friend bool operator==(const GmppkePrivateKey & l, const GmppkePrivateKey & r){
		return l.shares == r.shares;
	}
	friend bool operator!=(const GmppkePrivateKey & l, const GmppkePrivateKey & r){
		return !(l.shares == r.shares);
	}
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(shares);
	}
	friend class cereal::access;
 };

class PartialGmmppkeCT{
public:
	 PartialGmmppkeCT(){};
	PairingGroup group;
	G1 ct2;
	std::vector<G1> ct3;
	std::vector<ZR> tags;
	friend bool operator==(const PartialGmmppkeCT& x,const PartialGmmppkeCT& y){
		return x.ct2 == y.ct2 && x.ct3 == y.ct3 && x.tags == y.tags;
	}
	friend bool operator!=(const PartialGmmppkeCT& x, const PartialGmmppkeCT& y){
		return !(x==y);
	}
	template <class Archive>
	void serialize( Archive & ar ){
		ar(ct2,ct3,tags);
	}
	friend class cereal::access;
};

class GmmppkeCT: public PartialGmmppkeCT{
public:
	GmmppkeCT(){};
	GmmppkeCT(const  PartialGmmppkeCT & c) : PartialGmmppkeCT(c){}
	GT ct1;
	friend bool operator==(const GmmppkeCT& x,const GmmppkeCT& y){
		return x.ct1 == y.ct1 && (PartialGmmppkeCT) x == (PartialGmmppkeCT) y;
	}
	friend bool operator!=(const GmmppkeCT& x, const GmmppkeCT& y){
		return !(x==y);
	}
	template <class Archive>
	void serialize( Archive & ar ){
		ar(cereal::base_class<PartialGmmppkeCT>(this),ct1);
	}
	friend class cereal::access;
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
