#ifndef PFSE_H
#define PFSE_H

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <cereal/types/base_class.hpp>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/array.hpp>

#include "forwardsec.h"
#include "GMPpke.h"
#include "BBGHibe.h"

namespace forwardsec{
class Pfse;
class PfseKeyStore;
class pfsepubkey: public BbhHIBEPublicKey,  public GmppkePublicKey{
	friend bool operator==(const pfsepubkey& x, const pfsepubkey& y){
		return (BbhHIBEPublicKey)x == (BbhHIBEPublicKey)y && (GmppkePublicKey)x == (GmppkePublicKey)y;
	}
	friend bool operator!=(const pfsepubkey& x, const pfsepubkey& y){
		return !(x==y);
	}
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(::cereal::base_class<BbhHIBEPublicKey>(this),
				::cereal::base_class<GmppkePublicKey>(this));
	}
	friend class ::cereal::access;
};

class PfsePuncturedPrivateKey{
public:
	 bool punctured() const{
		return ppkeSK.punctured();
	}
	friend bool operator==(const PfsePuncturedPrivateKey& x, const PfsePuncturedPrivateKey& y){
		return x.hibeSK == y.hibeSK && x.ppkeSK == y.ppkeSK;
	}
	friend bool operator!=(const PfsePuncturedPrivateKey& x, const PfsePuncturedPrivateKey& y){
		return !(x==y);
	}
protected:
	BbghPrivatekey hibeSK;
	GmppkePrivateKey ppkeSK;
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(hibeSK,ppkeSK);
	}
	friend class ::cereal::access;
	friend class Pfse;
	friend class PfseKeyStore;
};

class PseCipherText{
public:
	friend class ::cereal::access;
	friend bool operator==(const PseCipherText& l,const PseCipherText& r){
		return l.ct0 == r.ct0 && l.hibeCT == r.hibeCT && l.ppkeCT == r.ppkeCT
				&& l.interval == r.interval && l.xorct == r.xorct;
	}
	friend bool operator!=(const PseCipherText& l,const PseCipherText& r){
		return !(l==r);
	}
protected:
	GT ct0;
	PartialBbghCT hibeCT;
	PartialGmmppkeCT ppkeCT;
	unsigned int interval;
	bytes xorct;
	template <class Archive>
	void serialize( Archive & ar )
	{
		ar(ct0,hibeCT,ppkeCT,interval,xorct);
	}
	friend class Pfse;
};
//template <class Archive>
//void serialize( Archive & ar, PseCipherText & b ){
//		//ar(b.ct0,b.hibeCT,b.ppkeCT,b.interval,b.xorct);
//}

class PfseKeyStore{
public:
	PfseKeyStore(){};
	PfseKeyStore(const GmppkePrivateKey & unpuncturedKey);
	PfsePuncturedPrivateKey getKey(unsigned int i) const;
	void updateKey(unsigned int i, const PfsePuncturedPrivateKey & p);
	void addkey(unsigned int i, const BbghPrivatekey & h);
	void erase(unsigned int i);
	bool hasKey(const unsigned int i) const;
	bool needsChildKeys(const unsigned int i,const unsigned int d) const;

	friend bool operator==(const PfseKeyStore& l,const PfseKeyStore& r){
		return l.puncturedKeys == r.puncturedKeys && l.unpucturedHIBEKeys == r.unpucturedHIBEKeys &&
				l.unpucturedPPKEKey == r.unpucturedPPKEKey;
	}
	friend bool operator!=(const PfseKeyStore& l,const PfseKeyStore& r){
		return !(l==r);
	}
private:
	std::map<unsigned int,PfsePuncturedPrivateKey> puncturedKeys;
	std::map<unsigned int,BbghPrivatekey> unpucturedHIBEKeys;
	GmppkePrivateKey unpucturedPPKEKey;
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(puncturedKeys,unpucturedHIBEKeys,unpucturedPPKEKey);
	}
	friend class ::cereal::access;
};

class Pfse
{
public:
	pfsepubkey pk;
	PfseKeyStore privatekeys;


	Pfse(unsigned int d);
	/**Generates the public and private key. These are stored in  the object.
	 *
	 */
	void keygen();

	/** Encrypts a message. Messages are limited to 256 bits. (e.g. an AES key).
	 *
	 * @param pk the public key of the recipient
	 * @param msg the message
	 * @param interval the time interval the message is in
	 * @param tags the tags for the message
	 * @return the ciphertext
	 */
	PseCipherText encrypt(const pfsepubkey & pk, const bytes msg, const unsigned int interval, const std::vector<std::string> tags) const;

	/**Decrypt a message using the private key stored in the object.
	 *
	 * @param ct the ciphertext
	 * @return the decrypted message.
	 */
	bytes decrypt( const PseCipherText &ct) const;
	
	/**Derives the keys needed to decrypt the next interval.
	 *
	 */
	void prepareNextInterval();
	/** Erases the key for a given interval.
	 *
	 * @param interval the interval to erase.
	 */
	void eraseKey(unsigned int interval);
    /** Punctures the key for the given time period.
     *
     * @param interval the time period
     * @param str the tag to puncture on
     */
	void puncture(unsigned int interval, std::string str);
	/**Punctures the key for the current interval.
	 *
	 * @param str the tag to puncture on.
	 */
	void puncture( std::string str);


private:
	PairingGroup group;
	Bbghibe hibe;
	Gmppke ppke;
	unsigned int depth;
	void bindKey(PfsePuncturedPrivateKey & k);
	PseCipherText encryptFO( const pfsepubkey & pk, const bytes & bitmsg,
			              const unsigned int interval, const std::vector<std::string>  & tags) const;
	PseCipherText encryptFO( const pfsepubkey & pk, const bytes & bitmsg,
			const GT & x, const unsigned int interval, const std::vector<std::string>  & tags) const;

	PseCipherText encrypt( const pfsepubkey & pk, const GT & M,              const unsigned int interval, const std::vector<std::string>  & tags) const;
	PseCipherText encrypt( const pfsepubkey & pk, const GT & M,const ZR & s, const unsigned int interval, const std::vector<std::string>  & tags) const;


	bytes decryptFO(const PfsePuncturedPrivateKey &sk, const PseCipherText &ct) const;
	GT decryptGT(const PfsePuncturedPrivateKey & sk, const PseCipherText &ct) const;
	unsigned int nextParentInterval;
};
}
namespace cereal
{
 template <class Archive>
 struct specialize<Archive, forwardsec::pfsepubkey, cereal::specialization::member_serialize> {};
 // cereal no longer has any ambiguity when serializing MyDerived
}
#endif

