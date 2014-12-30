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
class GMPfse;
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
	friend class GMPfse;
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
	unsigned int interval;
protected:
	relicxx::GT ct0;
	PartialBbghCT hibeCT;
	PartialGmmppkeCT ppkeCT;
	bytes xorct;
	template <class Archive>
	void serialize( Archive & ar )
	{
		ar(ct0,hibeCT,ppkeCT,interval,xorct);
	}
	friend class GMPfse;
};
//template <class Archive>
//void serialize( Archive & ar, PseCipherText & b ){
//		//ar(b.ct0,b.hibeCT,b.ppkeCT,b.interval,b.xorct);
//}

class PfseKeyStore{
public:
	PfseKeyStore(){};
	PfseKeyStore(const GmppkePrivateKey & unpuncturedKey,unsigned int depth);
	PfsePuncturedPrivateKey getKey(unsigned int i) const;
	void updateKey(unsigned int i, const PfsePuncturedPrivateKey & p);
	void addkey(unsigned int i, const BbghPrivatekey & h);
	void erase(unsigned int i);
	bool hasKey(const unsigned int i) const;
	bool needsChildKeys(const unsigned int i) const;

	friend bool operator==(const PfseKeyStore& l,const PfseKeyStore& r){
		return l.puncturedKeys == r.puncturedKeys && l.unpucturedHIBEKeys == r.unpucturedHIBEKeys &&
				l.unpucturedPPKEKey == r.unpucturedPPKEKey;
	}
	friend bool operator!=(const PfseKeyStore& l,const PfseKeyStore& r){
		return !(l==r);
	}
	unsigned int nextParentInterval=0;
private:
	unsigned int depth;
	std::map<unsigned int,PfsePuncturedPrivateKey> puncturedKeys;
	std::map<unsigned int,BbghPrivatekey> unpucturedHIBEKeys;
	GmppkePrivateKey unpucturedPPKEKey;
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(nextParentInterval,depth,puncturedKeys,unpucturedHIBEKeys,unpucturedPPKEKey);
	}
	friend class ::cereal::access;
};

class GMPfse
{
public:
	GMPfse(unsigned int d,unsigned int numtags = 1);
	 /**Generates the public and private key. These are stored in  the object.
	  *
	  *
	  * @param pk the public key
	  * @param sk the private key
	  */
	void keygen(pfsepubkey & pk, PfseKeyStore & sk) const;

	/** Encrypts a message. Messages are limited to 32 bytes. (e.g. an AES key).
	 *
	 * @param pk the public key of the recipient
	 * @param msg the message
	 * @param interval the time interval the message is in
	 * @param tags the tags for the message
	 * @return the ciphertext
	 */
	PseCipherText encrypt(const pfsepubkey & pk, const bytes msg, const unsigned int interval, const std::vector<std::string> tags) const;

	/**Decrypt a message using the provided public and private keys.
	 *
	 * @param pk the public key
	 * @param sk the private key
	 * @param ct the ciphertext
	 * @return the message
	 */
	bytes decrypt(const pfsepubkey & pk, const PfseKeyStore &sk, const PseCipherText &ct) const;
	
	/**Derives keys from the current interval and updates the provided
	 * secret key to store them.
	 *
	 * @param pk the public key
	 * @param sk the private key.
	 */
	void prepareNextInterval(const pfsepubkey & pk, PfseKeyStore &sk) const;

	/** Derives  keys from the specified interval  and updates the provided
	 * secret key to store them. This or prepareNextInterval must be run
	 * to allow interval i to be punctured.
	 *
	 *
	 * @param pk the public key
	 * @param sk the private key
	 * @param i the interval to derive the keys from.
	 */
	void prepareIntervalAfter(const pfsepubkey & pk, PfseKeyStore &sk,const unsigned int &i) const;

	/** Derives the key for the given interval and updates sk to store it
	 *
	 * @param pk the public key
	 * @param sk the private ky
	 * @param i the interval
	 * @param storeIntermediateKeys (defaults to true) whether the intermediate keys derived along the way are stored
	 */
	void deriveKeyFor(const pfsepubkey & pk, PfseKeyStore &sk,const unsigned int &i, const bool & storeIntermediateKeys = true)const;

	/**Puncture the key in the specified interval
	 *
	 * @param pk the public key
	 * @param sk the private ky
	 * @param i the interval
	 * @param str the string to puncture the key with.
	 */
	void puncture(const pfsepubkey & pk, PfseKeyStore &sk,unsigned int interval, std::string str) const;
	/**Puncture the key for the current interval
	 *
	 * @param pk the public key
	 * @param sk the private ky
	 * @param str the string to puncture the key with.
	 */
	void puncture(const pfsepubkey & pk, PfseKeyStore &sk, std::string str) const;


private:
	relicxx::PairingGroup group;
	Bbghibe hibe;
	Gmppke ppke;
	unsigned int depth;
	unsigned int numtags;

	void bindKey(const pfsepubkey & pk,PfsePuncturedPrivateKey & k) const;

	PseCipherText encryptFO(const pfsepubkey & pk, const bytes & bitmsg,
			              const unsigned int interval, const std::vector<std::string>  & tags) const;
	PseCipherText encryptFO( const pfsepubkey & pk, const bytes & bitmsg,
			const relicxx::GT & x, const unsigned int interval, const std::vector<std::string>  & tags) const;
	PseCipherText encryptGT( const pfsepubkey & pk, const relicxx::GT & M,const relicxx::ZR & s,
			const unsigned int interval, const std::vector<std::string>  & tags) const;


	bytes decryptFO(const pfsepubkey & pk, const PfsePuncturedPrivateKey &ski, const PseCipherText &ct) const;
	relicxx::GT decryptGT(const pfsepubkey & pk, const PfsePuncturedPrivateKey & ski, const PseCipherText &ct) const;
};
}
namespace cereal
{
 template <class Archive>
 struct specialize<Archive, forwardsec::pfsepubkey, cereal::specialization::member_serialize> {};
}
#endif

