/*
 * BBGHibe.h
 *
 *  Created on: Dec 21, 2014
 *      Author: imiers
 */

#ifndef SRC_BBGHIBE_H_
#define SRC_BBGHIBE_H_
#include <cereal/types/base_class.hpp>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>

#include "relic_wrapper/relic_api.h"

#include "forwardsec.h"
namespace forwardsec{

class GMPfse;
class Bbghibe;
class BbghPrivatekey{
public:

	friend bool operator==(const BbghPrivatekey& x, const BbghPrivatekey& y){
		return  (x.a0 == y.a0 && x.a1 == y.a1 && x.b == y.b &&
				x.bG2 == y.bG2);
	}
	friend bool operator!=(const BbghPrivatekey& x, const BbghPrivatekey& y){
		return !(x==y);
	}
protected:
	relicxx::G2 a0;
	relicxx::G2 a1;
	std::vector<relicxx::G1> b;
	std::vector<relicxx::G2> bG2;
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(a0,a1,b,bG2);
	}
	friend class ::cereal::access;
	friend class GMPfse;
	friend class Bbghibe;
};


class BbhHIBEPublicKey:  public virtual  baseKey{
public:
	friend bool operator==(const BbhHIBEPublicKey& x, const BbhHIBEPublicKey& y){
		return  ((baseKey)x == (baseKey)y &&
				x.l == y.l && x.hibeg1 == y.hibeg1 && x.g3G1 == y.g3G1 &&
				x.g3G2 == y.g3G2 && x.hG1 == y.hG1 && x.hG2 == y.hG2);
	}
	friend bool operator!=(const BbhHIBEPublicKey& x, const BbhHIBEPublicKey& y){
		return !(x==y);
	}

protected:
	unsigned int l;
	relicxx::G2 hibeg1;
	relicxx::G1 g3G1;
	relicxx::G2 g3G2;
	std::vector<relicxx::G1> hG1;
	std::vector<relicxx::G2> hG2;
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(::cereal::virtual_base_class<baseKey>(this),
				l,hibeg1,g3G1,g3G2,hG1,hG2);
	}
	friend class ::cereal::access;
	friend class GMPfse;
	friend class Bbghibe;
};

class PartialBbghCT{
public:
	PartialBbghCT(){};

	friend bool operator==(const PartialBbghCT& x,const PartialBbghCT& y){
		return x.B == y.B && x.C == y.C;
	}
	friend bool operator!=(const PartialBbghCT& x,const PartialBbghCT& y){
		return !(x==y);
	}
protected:
	relicxx::G1 B;
	relicxx::G1 C;
	template <class Archive>
	void serialize( Archive & ar ){
		ar(B,C);
	}
	friend class ::cereal::access;
	friend class GMPfse;
	friend class Bbghibe;
};
class BbghCT: public PartialBbghCT{
public:
	BbghCT(){};
	BbghCT(const  PartialBbghCT & c) : PartialBbghCT(c){}

	friend bool operator==(const BbghCT& x,const BbghCT& y){
		return x.A == y.A  && (PartialBbghCT) x == (PartialBbghCT) y;
	}
	friend bool operator!=(const BbghCT& x,const BbghCT& y){
		return !(x==y);
	}
protected:
	relicxx::GT A;
	template <class Archive>
	void serialize( Archive & ar ){
		ar(::cereal::base_class<PartialBbghCT>(this),A);
	}
	friend class ::cereal::access;
	friend class GMPfse;
	friend class Bbghibe;
};

class Bbghibe
{
public:
	relicxx::PairingGroup group;
	Bbghibe(){};
	~Bbghibe() {};

	void setup(const unsigned int & l, BbhHIBEPublicKey & pk, relicxx::G2 & msk) const;

	void keygen(const BbhHIBEPublicKey & pk,const relicxx::G2 & msk,const  std::vector<relicxx::ZR> & id, BbghPrivatekey & sk) const;
    void keygen(const BbhHIBEPublicKey & pk,const  BbghPrivatekey & sk, const std::vector<relicxx::ZR> & id,BbghPrivatekey & skout) const;

    PartialBbghCT blind(const BbhHIBEPublicKey & pk,const relicxx::ZR &s, const  std::vector<relicxx::ZR>  & id) const;
    BbghCT encrypt(const BbhHIBEPublicKey & pk, const relicxx::GT & M, const std::vector<relicxx::ZR>  & id ) const;

    relicxx::GT recoverBlind(const BbghPrivatekey & sk, const PartialBbghCT & ct) const; // decrypt for GMPfse
    relicxx::GT decrypt(const BbghPrivatekey & sk,const BbghCT & ct) const; // actual decrypt

};
}
// cereal can't find the serialization function if we don't do this.
// this has to be outside of the namespace..
namespace cereal
{
 template <class Archive>
 struct specialize<Archive, forwardsec::BbhHIBEPublicKey, cereal::specialization::member_serialize> {};
 // cereal no longer has any ambiguity when serializing MyDerived
}
#endif /* SRC_BBGHIBE_H_ */
