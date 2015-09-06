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

#include "relic_api.h"

#include "forwardsec.h"
namespace forwardsec{

class GMPfse;
class BBGHibe;
class BBGHibePrivateKey{
public:

	friend bool operator==(const BBGHibePrivateKey& x, const BBGHibePrivateKey& y){
		return  (x.a0 == y.a0 && x.a1 == y.a1 && x.b == y.b &&
				x.bG2 == y.bG2);
	}
	friend bool operator!=(const BBGHibePrivateKey& x, const BBGHibePrivateKey& y){
		return !(x==y);
	}
	void neuter();
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
	friend class BBGHibe;
};


class BBGHibePublicKey:  public virtual  baseKey{
public:
	friend bool operator==(const BBGHibePublicKey& x, const BBGHibePublicKey& y){
		return  ((baseKey)x == (baseKey)y &&
				x.l == y.l && x.hibeg1 == y.hibeg1 && x.g3G1 == y.g3G1 &&
				x.g3G2 == y.g3G2 && x.hG1 == y.hG1 && x.hG2 == y.hG2);
	}
	friend bool operator!=(const BBGHibePublicKey& x, const BBGHibePublicKey& y){
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
	friend class BBGHibe;
};

class BBGHibePartialCiphertext{
public:
	BBGHibePartialCiphertext(){};

	friend bool operator==(const BBGHibePartialCiphertext& x,const BBGHibePartialCiphertext& y){
		return x.B == y.B && x.C == y.C;
	}
	friend bool operator!=(const BBGHibePartialCiphertext& x,const BBGHibePartialCiphertext& y){
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
	friend class BBGHibe;
};
class BBGHibeCiphertext: public BBGHibePartialCiphertext{
public:
	BBGHibeCiphertext(){};
	BBGHibeCiphertext(const  BBGHibePartialCiphertext & c) : BBGHibePartialCiphertext(c){}

	friend bool operator==(const BBGHibeCiphertext& x,const BBGHibeCiphertext& y){
		return x.A == y.A  && (BBGHibePartialCiphertext) x == (BBGHibePartialCiphertext) y;
	}
	friend bool operator!=(const BBGHibeCiphertext& x,const BBGHibeCiphertext& y){
		return !(x==y);
	}
protected:
	relicxx::GT A;
	template <class Archive>
	void serialize( Archive & ar ){
		ar(::cereal::base_class<BBGHibePartialCiphertext>(this),A);
	}
	friend class ::cereal::access;
	friend class GMPfse;
	friend class BBGHibe;
};

class BBGHibe
{
public:
	relicxx::PairingGroup group;
	BBGHibe(){};
	~BBGHibe() {};

	void setup(const unsigned int & l, BBGHibePublicKey & pk, relicxx::G2 & msk) const;

	void keygen(const BBGHibePublicKey & pk,const relicxx::G2 & msk,const  std::vector<relicxx::ZR> & id, BBGHibePrivateKey & sk) const;
    void keygen(const BBGHibePublicKey & pk,const  BBGHibePrivateKey & sk, const std::vector<relicxx::ZR> & id,BBGHibePrivateKey & skout) const;

    BBGHibePartialCiphertext blind(const BBGHibePublicKey & pk,const relicxx::ZR &s, const  std::vector<relicxx::ZR>  & id) const;
    BBGHibeCiphertext encrypt(const BBGHibePublicKey & pk, const relicxx::GT & M, const std::vector<relicxx::ZR>  & id ) const;

    relicxx::GT recoverBlind(const BBGHibePrivateKey & sk, const BBGHibePartialCiphertext & ct) const; // decrypt for GMPfse
    relicxx::GT decrypt(const BBGHibePrivateKey & sk,const BBGHibeCiphertext & ct) const; // actual decrypt

};
}
// cereal can't find the serialization function if we don't do this.
// this has to be outside of the namespace..
namespace cereal
{
 template <class Archive>
 struct specialize<Archive, forwardsec::BBGHibePublicKey, cereal::specialization::member_serialize> {};
 // cereal no longer has any ambiguity when serializing MyDerived
}
#endif /* SRC_BBGHIBE_H_ */
