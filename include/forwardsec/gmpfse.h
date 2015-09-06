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
class GMPfsePrivateKey;
class GMPfsePublicKey: public BBGHibePublicKey,  public GmppkePublicKey{
	friend bool operator==(const GMPfsePublicKey& x, const GMPfsePublicKey& y){
		return (BBGHibePublicKey)x == (BBGHibePublicKey)y && (GmppkePublicKey)x == (GmppkePublicKey)y;
	}
	friend bool operator!=(const GMPfsePublicKey& x, const GMPfsePublicKey& y){
		return !(x==y);
	}
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(::cereal::base_class<BBGHibePublicKey>(this),
				::cereal::base_class<GmppkePublicKey>(this));
	}
	friend class ::cereal::access;
};

class GMPfseIntervalKey{
public:
	 bool punctured() const{
		return ppkeSK.punctured();
	}
	friend bool operator==(const GMPfseIntervalKey& x, const GMPfseIntervalKey& y){
		return x.hibeSK == y.hibeSK && x.ppkeSK == y.ppkeSK;
	}
	friend bool operator!=(const GMPfseIntervalKey& x, const GMPfseIntervalKey& y){
		return !(x==y);
	}
protected:
	BBGHibePrivateKey hibeSK;
	GmppkePrivateKey ppkeSK;
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(hibeSK,ppkeSK);
	}
	friend class ::cereal::access;
	friend class GMPfse;
	friend class GMPfsePrivateKey;
};

class GMPfseCiphertext{
public:
	friend class ::cereal::access;
	friend bool operator==(const GMPfseCiphertext& l,const GMPfseCiphertext& r){
		return l.ct0 == r.ct0 && l.hibeCT == r.hibeCT && l.ppkeCT == r.ppkeCT
				&& l.interval == r.interval && l.xorct == r.xorct;
	}
	friend bool operator!=(const GMPfseCiphertext& l,const GMPfseCiphertext& r){
		return !(l==r);
	}
	unsigned int interval;
protected:
	relicxx::GT ct0;
	BBGHibePartialCiphertext hibeCT;
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

class GMPfsePrivateKey{
public:
	GMPfsePrivateKey(){};
	GMPfsePrivateKey(const GmppkePrivateKey & unpuncturedKey,unsigned int depth);
	GMPfseIntervalKey getKey(unsigned int i) const;
	void updateKey(unsigned int i, const GMPfseIntervalKey & p);
	void addkey(unsigned int i, const BBGHibePrivateKey & h);
	void erase(unsigned int i);
	bool hasKey(const unsigned int i) const;
	bool needsChildKeys(const unsigned int i) const;
	void neuter(const unsigned int i);
	friend bool operator==(const GMPfsePrivateKey& l,const GMPfsePrivateKey& r){
		return l.puncturedKeys == r.puncturedKeys && l.unpucturedHIBEKeys == r.unpucturedHIBEKeys &&
				l.unpucturedPPKEKey == r.unpucturedPPKEKey;
	}
	friend bool operator!=(const GMPfsePrivateKey& l,const GMPfsePrivateKey& r){
		return !(l==r);
	}
	unsigned int nextParentInterval=0;
private:
	unsigned int depth;
	std::map<unsigned int,GMPfseIntervalKey> puncturedKeys;
	std::map<unsigned int,BBGHibePrivateKey> unpucturedHIBEKeys;
	GmppkePrivateKey unpucturedPPKEKey;
	template <class Archive>
	  void serialize( Archive & ar )
	{
		ar(nextParentInterval,depth,puncturedKeys,unpucturedHIBEKeys,unpucturedPPKEKey);
	}
	friend class ::cereal::access;
};

/**
Implements Puncturable forward secure encryption. We have one puncturable key per time interval.
*/
class GMPfse
{
public:
	/** Constructor
	* @param treeDepth  The depth of the  of the tree. We support 2^treeDepth intervals. 
	* @param numtags  The number of tags each ciphertext has. If you are just using this scheme
	* 					for forward security, use the default of one
	*/
	GMPfse(unsigned int treeDepth,unsigned int numtags = 1);
	 /**Generates the public and private keys which are stored in the passed in parameters
	  * @param pk the public key
	  * @param sk the private key
	  */
	void keygen(GMPfsePublicKey & pk, GMPfsePrivateKey & sk) const;

	/** Encrypts a message. Messages are limited to 32 bytes. (e.g. an AES key).
	 *
	 * @param pk the public key of the recipient
	 * @param msg the message
	 * @param interval the time interval the message is in
	 * @param tags the tags for the message
	 * @return the ciphertext
	 */
	GMPfseCiphertext encrypt(const GMPfsePublicKey & pk, const forwardsec::bytes msg, const unsigned int interval, const std::vector<std::string> tags) const;

	/**Decrypt a message using the provided public and private keys.
	 *
	 * @param pk the public key
	 * @param sk the private key
	 * @param ct the ciphertext
	 * @return the message
	 */
	forwardsec::bytes decrypt(const GMPfsePublicKey & pk, const GMPfsePrivateKey &sk, const GMPfseCiphertext &ct) const;
	
	/**Derives keys from the current interval and updates the provided
	 * secret key to store them. This or prepareNextInterval must be run
	 * to allow the current interval  to be punctured. 
	 *
	 * @param pk the public key
	 * @param sk the private key.
	 */
	void prepareNextInterval(const GMPfsePublicKey & pk, GMPfsePrivateKey &sk) const;

	/** Derives  keys from the specified interval  and updates the provided
	 * secret key to store them. This or prepareNextInterval must be run
	 * to allow interval i to be punctured. 
	 *
	 *
	 * @param pk the public key
	 * @param sk the private key
	 * @param i the interval to derive the keys from.
	 */
	void prepareIntervalAfter(const GMPfsePublicKey & pk, GMPfsePrivateKey &sk,const unsigned int &i) const;

	/** Derives the key for the given interval and updates sk to store it
	 *  To puncture the newly derived interval key, you must run prepareIntervalAfter first.
	 *
	 * @param pk the public key
	 * @param sk the private key
	 * @param i the interval to derive
	 * @param storeIntermediateKeys (defaults to true) whether the intermediate keys derived along the way are stored
	 * @param neuter whether to make keys undelegatable from as we go. Defaults to true
	 */
	void deriveKeyFor(const GMPfsePublicKey & pk, GMPfsePrivateKey &sk,const unsigned int &i, const bool & storeIntermediateKeys = true, 
		const bool &neuter = true)const;

	/**Puncture the key in the specified interval
	 *
	 * @param pk the public key
	 * @param sk the private ky
	 * @param i the interval
	 * @param str the string to puncture the key with.
	 */
	void puncture(const GMPfsePublicKey & pk, GMPfsePrivateKey &sk,unsigned int interval, std::string str) const;
	/**Puncture the key for the current interval
	 *
	 * @param pk the public key
	 * @param sk the private ky
	 * @param str the string to puncture the key with.
	 */
	void puncture(const GMPfsePublicKey & pk, GMPfsePrivateKey &sk, std::string str) const;
private:
	relicxx::PairingGroup group;
	relicxx::relicResourceHandle handle;
	BBGHibe hibe;
	Gmppke ppke;
	unsigned int depth;
	unsigned int numtags;

	void bindKey(const GMPfsePublicKey & pk,GMPfseIntervalKey & k) const;

	GMPfseCiphertext encryptFO(const GMPfsePublicKey & pk, const forwardsec::bytes & bitmsg,
			              const unsigned int interval, const std::vector<std::string>  & tags) const;
	GMPfseCiphertext encryptFO( const GMPfsePublicKey & pk, const forwardsec::bytes & bitmsg,
			const relicxx::GT & x, const unsigned int interval, const std::vector<std::string>  & tags) const;
	GMPfseCiphertext encryptGT( const GMPfsePublicKey & pk, const relicxx::GT & M,const relicxx::ZR & s,
			const unsigned int interval, const std::vector<std::string>  & tags) const;


	forwardsec::bytes decryptFO(const GMPfsePublicKey & pk, const GMPfseIntervalKey &ski, const GMPfseCiphertext &ct) const;
	relicxx::GT decryptGT(const GMPfsePublicKey & pk, const GMPfseIntervalKey & ski, const GMPfseCiphertext &ct) const;
};
}
namespace cereal
{
 template <class Archive>
 struct specialize<Archive, forwardsec::GMPfsePublicKey, cereal::specialization::member_serialize> {};
}
#endif

