/*
 * forwardsec.h
 *
 *  Created on: Dec 21, 2014
 *      Author: imiers
 */

#ifndef SRC_FORWARDSEC_H_
#define SRC_FORWARDSEC_H_
#include "relic_wrapper/relic_api.h"
#include <cereal/archives/binary.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/access.hpp>


class BadCiphertext : public std::invalid_argument
{
public:
    BadCiphertext(std::string const& error)
        : std::invalid_argument(error)
    {}
};

class PuncturedCiphertext : public BadCiphertext
{
public:
	PuncturedCiphertext(std::string const& error)
        : BadCiphertext(error)
    {}
};



class baseKey{
public:
	PairingGroup group;
	G1 gG1;
	G2 gG2;
	G1 g2G1;
	G2 g2G2;
	friend bool operator==(const baseKey& x, const baseKey& y){
		return (x.gG1 == y.gG1 && x.gG2 == y.gG2 && x.g2G1 == y.g2G1
				&& x.g2G2 == y.g2G2);
	}
	friend bool operator!=(const baseKey& x, const baseKey& y){
		return !(x==y);
	}
};

// (from cereal documentation )Note the non-member serialize - trying to call serialize
// from a derived class wouldn't work
template <class Archive>
void serialize( Archive & ar, baseKey & b )
{ ar( b.gG1,b.gG2,b.g2G1,b.g2G2 ); }


#endif /* SRC_FORWARDSEC_H_ */
