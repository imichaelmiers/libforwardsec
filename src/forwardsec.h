/*
 * forwardsec.h
 *
 *  Created on: Dec 21, 2014
 *      Author: imiers
 */

#ifndef SRC_FORWARDSEC_H_
#define SRC_FORWARDSEC_H_
#include <cereal/types/vector.hpp>
#include "relic_wrapper/relic_api.h"


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
};



#endif /* SRC_FORWARDSEC_H_ */
