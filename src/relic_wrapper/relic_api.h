#ifndef RELIC_API_H
#define RELIC_API_H
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <math.h>
#include <bitset> 
#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>

#include <array>
#include <type_traits> // for static assert
#include <cstring> // for memcpy
#include <algorithm> // for std::fill


// define classes
#ifdef __cplusplus
	// gmp.h uses __cplusplus to decide if it's right to include c++ headers.
	// At last on osx  causes error: conflicting types for 'operator<<'.
	// undefinning __cplusplus "FIXES" this.
	//#include <gmpxx.h>
		#define ___cplusplus __cplusplus
		#undef __cplusplus
	extern "C" {
#endif
   #include <relic/relic.h>
   #include <relic/relic_conf.h>

#ifdef ___cplusplus
}
	#define __cplusplus ___cplusplus
	#undef  ___cplusplus
#endif

extern "C" {
	   #include "common.h"
}
// this exists to deal with a const issue with relic. we can either copy (guaranteed  to be safe) or just
// cast away the const under the assumption that the underlying methods are ocnst (which they are supposed to be already)
#ifdef RELICXX_UNCONST
#define RELICXX_G1unconst(x,y) G1 & y = const_cast<G1&>(x)
#define RELICXX_G2unconst(x,y) G2 & y = const_cast<G2&>(x)
#define RELICXX_GTunconst(x,y) GT & y = const_cast<GT&>(x)
#define RELICXX_ZRunconst(x,y) ZR & y = const_cast<ZR&>(x)
#else
#define RELICXX_G1unconst(x,y) G1 y(x)
#define RELICXX_G2unconst(x,y) G2 y(x)
#define RELICXX_GTunconst(x,y) GT y(x)

#define RELICXX_ZRunconst(x,y) ZR y(x)

#endif

// ensures that if we try to enable OpenMP support , it's enabled in relic or fails .
//  MULTI may not be defined , so we hav two asserts. One if it isn't, one it it is.
#ifdef RELICXX_USE_OPENMP
#ifndef MULTI
static_assert(0, "Error. Relicxx is compiled to use OPENMP. But Relic is not configured to use any threading.");
#else
static_assert(MULTI == OPENMP, "Error. Relicxx compiled to use OPENMP. But Relic is not.");
#endif
#endif

#define convert_str(a)  a /* nothing */

namespace relicxx{
typedef  std::vector<uint8_t> bytes;
void ro_error(void);

const static std::string HASH_FUNCTION_STRINGS       = "0";
const static std::string HASH_FUNCTION_BYTES_TO_Zr_CRH = "1";
const static std::string HASH_FUNCTION_BYTES_TO_G1_ROM = "2";
const static std::string HASH_FUNCTION_BYTES_TO_G2_ROM = "3";

class RelicDividByZero : public std::logic_error
{
public:
	RelicDividByZero(std::string const& error)
        : std::logic_error(error)
    {}
};

void error_if_relic_not_init();
class ZR
{

public:
	bn_t z;
	bn_t order;
	bool isInit;
	ZR() 	 {error_if_relic_not_init(); bn_inits(z); bn_inits(order); g1_get_ord(order); isInit = true;bn_set_dig(z,1); }

	ZR(int);
	ZR(char*);
	ZR(const bn_t y) {error_if_relic_not_init(); bn_inits(z); bn_inits(order); g1_get_ord(order); isInit = true; bn_copy(z, y); }
	ZR(const ZR& w) { error_if_relic_not_init();bn_inits(z); bn_inits(order); bn_copy(z, w.z); bn_copy(order, w.order); isInit = true; }

#ifdef RELICXX_MOVEZR
	ZR(ZR&& other){
		*this=std::move(other);
	}

	ZR&  operator=(ZR && rhs){
		if(this !=&rhs && rhs.isInit){
			isInit = rhs.isInit;
			rhs.isInit = false;
			if(isInit){
				bn_free(z); bn_free(order);
			}
#if ALLOC == AUTO
			z[0] = rhs.z[0];
			order[0] = rhs.order[0];
			std::memset((&rhs.z[0]),sizeof(rhs.z[0]),0);
#else
			z=rhs.z;
			order=rhs.order;
			rhs.z=nullptr;
			rhs.order=nulltpr;
#endif
		}
		return * this;
	}
#endif
	~ZR() {
		if(isInit){
			bn_free(z); bn_free(order);
		}
	}
	ZR& operator=(const ZR& w)
	{
		if (isInit == true) { bn_copy(z, w.z); bn_copy(order, w.order); }
		else ro_error();
		return *this;
	}
		bool ismember() const;
		ZR inverse() const;
		std::vector<uint8_t> getBytes() const;
		friend class cereal::access;
		template <class Archive>
		void save( Archive & ar) const
		{
			auto data = getBytes();
			ar(data);
		}
		template <class Archive>
		void load( Archive & ar)
		{
			std::vector<uint8_t>data(BN_BYTES);
			ar(data);
			bn_read_bin(z,&data[0],BN_BYTES);
		}
//		template <>
//		void save<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & ar) const{
//			std::cout <<"binarysave" << std::endl;
//			auto data = getBytes();
//			ar.saveBinaryValue(&data[0],data.size());
//		}
//		template <>
//		void load<cereal::JSONInputArchive>(cereal::JSONInputArchive & ar)
//		{
//			std::vector<uint8_t>data(BN_BYTES);
//			ar.loadBinaryValue(&data[0],data.size());
//			bn_read_bin(z,&data[0],BN_BYTES);
//		}
	friend ZR hashToZR(const bytes &);
	friend ZR power(const ZR&, int);
	friend ZR power(const ZR&, const ZR&);
	friend ZR operator-(const ZR&);
	friend ZR operator-(const ZR&, const ZR&);
	friend ZR operator+(const ZR&, const ZR&);
	friend ZR operator*(const ZR&, const ZR&);
	friend ZR operator/(const ZR&, const ZR&);
    friend ZR operator&(const ZR&, const ZR&);  // bitwise-AND
//    friend ZR operator|(const ZR&, const ZR&);  // bitwise-OR
//    friend ZR operator^(const ZR&, const ZR&);  // bitwise-XOR
   friend ZR operator<<(const ZR&, int);
   friend ZR operator>>(const ZR&, int);

   friend std::ostream& operator<<(std::ostream&, const ZR&);
	friend bool operator==(const ZR& x, const ZR& y)
      {if(bn_cmp(x.z, y.z) == CMP_EQ) return true; else return false; }
	friend bool operator!=(const ZR& x, const ZR& y)
      {if (bn_cmp(x.z, y.z) != CMP_EQ) return true; else return false; }
	friend bool operator>(const ZR& x, const ZR& y)
      {if (bn_cmp(x.z, y.z) == CMP_GT) return true; else return false; }
	friend bool operator<(const ZR& x, const ZR& y)
      {if (bn_cmp(x.z, y.z) == CMP_LT) return true; else return false; }
};

class G1
{
public:
	g1_t g;
	bool isInit;
    G1()   {error_if_relic_not_init(); g1_inits(g); isInit = true; g1_set_infty(g); }
	G1(const G1& w) { g1_inits(g); g1_copy(g, w.g); isInit = true; }

	~G1()  {
		if(isInit) {
			g1_free(g);
		}
	}
#ifdef RELICXX_MOVEG1
	G1(G1 && other){
		*this=std::move(other);
	}
	G1&  operator=(G1 && rhs){
		if(this !=&rhs && rhs.isInit){
			isInit =rhs.isInit;
			rhs.isInit = false;
			if(isInit){
				g1_free(g);
			}
#if ALLOC == AUTO
			g[0] = rhs.g[0];
			std::memset((&rhs.g[0]),sizeof(rhs.g[0]),0);
#else
			g=rhs.g;
			rhs.g=nullptr;
#endif
		}
		return * this;
	}

#endif
	G1& operator=(const G1& w)
	{
		if (isInit == true) g1_copy(g, w.g);
		else ro_error();
		return *this;
	}

	bool ismember(const bn_t) const;
	std::vector<uint8_t> getBytes(bool compress = 0) const;
    template<class Archive>
    void save(Archive & ar) const
    {
    	auto data = getBytes(POINT_COMPRESS);
		ar(data);
    }
    template<class Archive>
    void load(Archive & ar){
    	std::vector<uint8_t>data;
		ar(data);
    	g1_read_bin(g,&data[0],data.size());
    }
    friend class cereal::access;


	friend G1 hashToG1(const bytes &);
	friend G1 power(const G1&, const ZR&);
	friend G1 operator-(const G1&);
	friend G1 operator-(const G1&, const G1&);
	friend G1 operator+(const G1&, const G1&);
   friend std::ostream& operator<<(std::ostream&, const G1&);
	friend bool operator==(const G1& x, const G1& y)
      {return g1_cmp(x.g, y.g) == CMP_EQ; }
	friend bool operator!=(const G1& x, const G1& y)
      {return g1_cmp(x.g, y.g) != CMP_EQ; }
};

class G2
{
public:
	g2_t g;
	bool isInit;
    G2()   {error_if_relic_not_init(); g2_inits(g); isInit = true; g2_set_infty(g); }
	G2(const G2& w) { g2_inits(g); g2_copy(g, const_cast<G2&>(w).g); isInit = true; }

	~G2()  {
		if(isInit){
			g2_free(g);
		}
	}
#ifdef RELICXX_MOVEG2
	G2(G2 && other){
		*this=std::move(other);
	}
	G2&  operator=(G2 && rhs){
		if(this !=&rhs && rhs.isInit){
			isInit = rhs.isInit;
			rhs.isInit = false;
			if(isInit){
				g2_free(g);
			}
#if ALLOC == AUTO
			g[0] = rhs.g[0];
			std::memset((&rhs.g[0]),sizeof(rhs.g[0]),0);
#else
			g=rhs.g;
			rhs.g=nulltpr;
#endif
		}
		return * this;
	}
#endif

	G2& operator=(const G2& w)
	{
		if (isInit == true) g2_copy(g,  const_cast<G2&>(w).g);
		else ro_error();
		return *this;
	}
	bool ismember(bn_t);
	std::vector<uint8_t> getBytes( bool compress = 0) const;

    template<class Archive>
    void save(Archive & ar) const
    {
    	auto data = getBytes(POINT_COMPRESS);
		ar(data);
    }
    template<class Archive>
    void load(Archive & ar){
    	std::vector<uint8_t>data;
		ar(data);
    	g2_read_bin(g,&data[0],data.size());
    }
    friend class cereal::access;

	friend G2 hashToG2(const bytes &);
	friend G2 power(const G2&, const ZR&);
	friend G2 operator-(const G2&);
	friend G2 operator-(const G2&, const G2&);
	friend G2 operator+(const G2&, const G2&);
	friend std::ostream& operator<<(std::ostream& s, const G2&);
	friend bool operator==(const G2& x, const G2& y)
      {return g2_cmp( const_cast<G2&>(x).g,  const_cast<G2&>(y).g) == CMP_EQ;}
	friend bool operator!=(const G2& x, const G2& y)
      {return g2_cmp( const_cast<G2&>(x).g,  const_cast<G2&>(y).g) != CMP_EQ;}
};

class GT
{
public:
	gt_t g;
	bool isInit;
    GT()   { error_if_relic_not_init();gt_inits(g); isInit = true; gt_set_unity(g); }
    GT(const GT& x) { error_if_relic_not_init();gt_inits(g); isInit = true; gt_copy(g, const_cast<GT&>(x).g); }
    ~GT()  {
    	if(isInit) {
    		gt_free(g);
    	}
    }

	GT& operator=(const GT& x)
	{
		if (isInit == true) gt_copy(g, const_cast<GT&>(x).g);
		else ro_error();
		return *this;
	}
#ifdef RELICXX_MOVEGT
	GT&  operator=(GT && rhs){
		if(this !=&rhs && rhs.isInit){
			isInit = rhs.isInit;
			rhs.isInit = false;
			if(isInit){
				gt_free(g);
			}
#if ALLOC == AUTO
			std::memcpy(*g,*(rhs.g),sizeof(g));
			std::memset(*(rhs.g),sizeof(rhs.g),0);
#else
			g=rhs.g;
			rhs.g=nullptr;
#endif
		}
		return * this;
	}
#endif

	bool ismember(bn_t);
	std::vector<uint8_t> getBytes( bool compress = 0) const;

    template<class Archive>
    void save(Archive & ar) const
    {
    	auto data = getBytes(POINT_COMPRESS);
		ar(data);
    }
    template<class Archive>
    void load(Archive & ar){
    	std::vector<uint8_t>data;
		ar(data);
    	gt_read_bin(g,&data[0],data.size());
    }
    friend class cereal::access;

	friend GT pairing(const G1&, const G1&);
	friend GT pairing(const G1&, const G2&);
	friend GT power(const GT&, const ZR&);
	friend GT operator-(const GT&);
	friend GT operator/(const GT&, const GT&);
	friend GT operator*(const GT&, const GT&);
	friend std::ostream& operator<<(std::ostream& s, const GT&);
	friend bool operator==(const GT& x, const GT& y)
      { return gt_cmp(const_cast<GT&>(x).g, const_cast<GT&>(y).g) == CMP_EQ;}
	friend bool operator!=(const GT& x, const GT& y)
      { return  gt_cmp(const_cast<GT&>(x).g, const_cast<GT&>(y).g) != CMP_EQ;}
};

class relicResourceHandle{
public:
	/**
	 * Tries to initialize relic.  If allowAlreadyInitilazed, will
	 * simply become a no op if someone has already initialized the
	 * code.
	 * @param allowAlreadyInitilazed
	 */
	relicResourceHandle(const bool & allowAlreadyInitilazed = true);
	~relicResourceHandle();

	// you cannot meaningfully copy this resource
	relicResourceHandle(const relicResourceHandle & t) = delete;
	bool isInitalized();
private:
	bool isInit;
};


class PairingGroup
{
public:
	PairingGroup();
	// PairingGroup(int ptype,bool init,bn_t o){
	// 	pairingType = ptype;
	// 	isInit = init;
	// 	bn_copy(o,grp_order);
	// }

	~PairingGroup();


	ZR randomZR() const;
	G1 randomG1() const;
	G2 randomG2() const;
	GT randomGT() const;

	bool ismember(ZR&);
	bool ismember(G1&);
	bool ismember(GT&);
	bool ismember(G2&);


	G2 random(G2_type) const;
	G2 mul(const G2 &, const G2 &) const;
	G2 div(const G2 &, const G2 &) const;
	G2 exp(const G2 &, const ZR &) const;
	G2 exp(const G2 &, const int &) const;
	GT pair(const G1 &, const G2 &) const;
	GT pair(const G2 &, const G1 &) const;
	ZR order() const; // returns the order of the group

	ZR hashListToZR(const std::string & str) const;
	ZR hashListToZR(const bytes &) const;
	G1 hashListToG1(const std::string & str) const;
	G1 hashListToG1(const bytes &) const;
	G2 hashListToG2(const bytes &) const;

	GT pair(const G1 &, const G1 &) const;
	int mul(const int &, const int &) const;
	ZR mul(const ZR &,  const ZR &) const;
	G1 mul(const G1 &, const G1 &)const ;
	GT mul(const GT &, const GT &)const ;
	int div(const int &, const int &)const ;
	ZR div(const int &, const ZR &) const ;
	ZR div(const ZR &, const ZR &)const ;
	G1 div(const G1 &, const G1 &)const ;
	GT div(const GT &, const GT &)const ;

	ZR exp(const ZR &, const int &)const ;
	ZR exp(const ZR &, const ZR &) const;
	G1 exp(const G1 &, const ZR &) const;
	G1 exp(const G1 &, const int &) const;
	GT exp(const GT &, const ZR &) const;
	GT exp(const GT &, const int &)const ;

	ZR add(const ZR &, const ZR &) const;
	int add(const int &, const  int &) const;

	int sub(const int &, const int &)const ;
	ZR sub(const ZR &, const ZR &) const;
	ZR neg(const ZR &) const;
	ZR inv(const ZR &) const;
	G1 inv(const G1 &) const;
	G2 inv(const G2 &)const;
	GT inv(const GT &) const;
	std::string aes_key(const GT & g);

private:
	bool isInit;
	bn_t grp_order;
};

//template<> 
//ZR PairingGroup::random12<ZR>(){return this->random(ZR_t);}
//byte256 intToBits(const ZR & id);
}
#endif
