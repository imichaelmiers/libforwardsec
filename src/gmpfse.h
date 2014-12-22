#ifndef PFSE_H
#define PFSE_H

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "forwardsec.h"
#include "GMPpke.h"
#include "BBGHibe.h"



class pfsepubkey: public BbhHIBEPublicKey,  public GmppkePublicKey{
};

class PfsePuncturedPrivateKey{
public:
	BbghPrivatekey hibeSK;
	GmppkePrivateKey ppkeSK;
	 bool punctured() const{
		return ppkeSK.shares.size() > 1;
	}
};

class PseCipherText{
public:
	PairingGroup group;
	GT ct0;
	PartialBbghCT hibeCT;
	PartialGmmppkeCT ppkeCT;
	unsigned int interval;
	bitset256 xorct;
friend bool operator==(const PseCipherText& l,const PseCipherText& r){
		return l.ct0 == r.ct0 && l.hibeCT == r.hibeCT && l.ppkeCT == r.ppkeCT
				&& l.interval == r.interval && l.xorct == r.xorct;
	}
};

class PfseKeyStore{
public:
	std::map<unsigned int,PfsePuncturedPrivateKey> puncturedKeys;
	std::map<unsigned int,BbghPrivatekey> unpucturedHIBEKeys;
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
	PseCipherText encrypt(const pfsepubkey & pk, const bitset256 msg, const unsigned int interval, const std::vector<std::string> tags) const;

	/**Decrypt a message using the private key stored in the object.
	 *
	 * @param ct the ciphertext
	 * @return the decrypted message.
	 */
	bitset256 decrypt( const PseCipherText &ct) const;
	
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
	PseCipherText encryptFO( const pfsepubkey & pk, const bitset256 & bitmsg,
			              const unsigned int interval, const std::vector<ZR>  & tags) const;
	PseCipherText encryptFO( const pfsepubkey & pk, const bitset256 & bitmsg,
			const GT & x, const unsigned int interval, const std::vector<ZR>  & tags) const;

	PseCipherText encrypt( const pfsepubkey & pk, const GT & M,              const unsigned int interval, const std::vector<ZR>  & tags) const;
	PseCipherText encrypt( const pfsepubkey & pk, const GT & M,const ZR & s, const unsigned int interval, const std::vector<ZR>  & tags) const;


	bitset256 decryptFO(const PfsePuncturedPrivateKey &sk, const PseCipherText &ct) const;
	GT decryptGT(const PfsePuncturedPrivateKey & sk, const PseCipherText &ct) const;
	unsigned int nextParentInterval;
};
#endif

