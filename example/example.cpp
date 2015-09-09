#include <assert.h>     /* assert */
#include <forwardsec/gmpfse.h> 
#include <cereal/archives/binary.hpp>


int main(){
	// The scheme takes two parameters: the number of intervals to have in the key 
	// and the number number of tags a ciphertext has.
	// We explicitly set the number of tags to 1 here for clarity, but this is the default.
	// We only need one tag for forward security. Tag's are what we puncture on
	// if the key has been punctured on any tag in a message, the message
	// can't be decrypted. 

	// These parameters need to be the same for both the object constructed for encryption.
	forwardsec::GMPfse forwardSecureEncryption(31,1);
	forwardsec::GMPfsePublicKey pk; //  the public key 
	forwardsec::GMPfsePrivateKey sk; //  the private key

	forwardSecureEncryption.keygen(pk,sk); // Generate the keys and stores them in pk and sk

	// a message. This is just a byte vector. For real messages, it should be a random AES key
 	forwardsec::bytes msg = {{0x3a, 0x5d, 0x7a, 0x42, 0x44, 0xd3, 0xd8, 0xaf, 0xf5, 0xf3, 0xf1, 0x87, 0x81, 0x82, 0xb2,
						  0x53, 0x57, 0x30, 0x59, 0x75, 0x8d, 0xe6, 0x18, 0x17, 0x14, 0xdf, 0xa5, 0xa4, 0x0b,0x43,0xAD,0xBC}};
	
	// If you are using this library to get forward security, tags should be unique for the recipient
	// A random  GUID or a counter + the senders address works. 
	std::string tag = "ThisShouldBeUniquePerMessage"; 

	// Encrypt a message. Pk is the key of the intended recipient, msg is  the message 
	// the third argument is the time interval. Intervals start at 1.

	// The API supports instantiating the scheme with multiple tags. Because we constructed 
	// the instance with one tag, we can only use one. So we use {{tags}} to make 
	// a vector with one element in it
	forwardsec::GMPfseCiphertext ct = forwardSecureEncryption.encrypt(pk,msg,1,{{tag}});

	// decryption is simple. 
	forwardsec::bytes decrypted_msg = forwardSecureEncryption.decrypt(pk,sk,ct);
	assert(decrypted_msg==msg);

	// now that we decrypted the message we need to remove are ability to do so
	/// to ensure forward security of that message.

	// To do so we need the tag for the message. Again, because we only configured 
	// this instance to have one tag, we only need that one tag 
	std::string msg_tag = ct.getTags().at(0);
 
	// Now we puncture. We specify the interval and the tag
	forwardSecureEncryption.puncture(pk,sk,ct.interval,msg_tag);

	// When this interval is over we can derive the next interval 
	forwardSecureEncryption.prepareNextInterval(pk,sk);
	// if at some point you need to derive some interval in the future
	// e.g. your intervals are one second long , but you for 30 seconds got no traffic,
	// then use 

	forwardSecureEncryption.deriveKeyFor(pk,sk,31);
	// note, to puncture this interval , you will then need to actually prepare it's child keys too
	// if you don't do so, you cannot decrypt. To do so, use:
	forwardSecureEncryption.prepareIntervalAfter(pk,sk,31);

	// finally at some point you should actually delete interval keys.
	// If you don't any messages that weren't delivered (perhaps because an attacker intercepted them)
	// are still exposed.

	sk.erase(1); // deletes the keys for interval one. 

	// finally. Here is how you serialize stuff. This works for pk,sk and ct.
	std::stringstream ss; // this can also be a file or any compatible stream

	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(ct);
	}
	// and deserialize 
	forwardsec::GMPfseCiphertext ctnew;
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(ctnew);
	}
}