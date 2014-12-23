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
#include <array>

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
#define LFORWARDSEC_UNCONST
#ifdef LFORWARDSEC_UNCONST
#define lfrowdsec_G1unconst(x,y) G1 & y = const_cast<G1&>(x)
#define lfrowdsec_G2unconst(x,y) G2 & y = const_cast<G2&>(x)
#define lfrowdsec_GTunconst(x,y) GT & y = const_cast<GT&>(x)
#define lfrowdsec_ZRunconst(x,y) ZR & y = const_cast<ZR&>(x)
#else
#define lfrowdsec_G1unconst(x,y) G1 y(x)
#define lfrowdsec_G2unconst(x,y) G2 y(x)
#define lfrowdsec_GTunconst(x,y) GT y(x)

#define lfrowdsec_ZRunconst(x,y) ZR y(x)

#endif

#define convert_str(a)  a /* nothing */
void ro_error(void);


typedef  std::array<uint8_t,256> bitset256;


class RelicDividByZero : public std::logic_error
{
public:
	RelicDividByZero(std::string const& error)
        : std::logic_error(error)
    {}
};
class ZR
{

public:
	bn_t z;
	bn_t order;
	bool isInit;
	ZR() 	 { bn_inits(z); bn_inits(order); g1_get_ord(order); isInit = true;bn_set_dig(z,1); }
	ZR(int);
	ZR(char*);
	ZR(const bn_t y) { bn_inits(z); bn_inits(order); g1_get_ord(order); isInit = true; bn_copy(z, y); }
	ZR(const ZR& w) { bn_inits(z); bn_inits(order); bn_copy(z, w.z); bn_copy(order, w.order); isInit = true; }
//	ZR&&  operator=(ZR && rhs){
//		if(this !=&rhs){
//			if(isInit){
//				bn_free(z); bn_free(order);
//			}
//			rhs.isInit = false;
//			z[0] = rhs.z[0];
//			order[0] = rhs.order[0];
//		}
//		return * this;
//	}
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
		const ZR inverse() const;

		friend class cereal::access;
		template <class Archive>
		void save( Archive & ar) const
		{
			std::vector<uint8_t>data(BN_BYTES);
			bn_write_bin(&data[0], BN_BYTES, z);
			ar(data);
		}
		template <class Archive>
		void load( Archive & ar)
		{
			std::vector<uint8_t>data(BN_BYTES);
			ar(data);
			bn_read_bin(z,&data[0],BN_BYTES);
		}
	friend ZR hashToZR(std::string);
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
    G1()   { g1_inits(g); isInit = true; g1_set_infty(g); }
	G1(const G1& w) { g1_inits(g); g1_copy(g, w.g); isInit = true; }
	~G1()  {
		if(isInit) {
			g1_free(g);
		}
	}

	G1& operator=(const G1& w)
	{
		if (isInit == true) g1_copy(g, w.g);
		else ro_error();
		return *this;
	}
		bool ismember(const bn_t) const;
    template<class Archive>
    void save(Archive & ar) const
    {
    	unsigned int l  = g1_size_bin(g,POINT_COMPRESS);
    	std::vector<uint8_t>data(l);
		g1_write_bin(&data[0], data.size(), g,POINT_COMPRESS);
		ar(data);
    }
    template<class Archive>
    void load(Archive & ar){
    	std::vector<uint8_t>data;
		ar(data);
    	g1_read_bin(g,&data[0],data.size());
    }
    friend class cereal::access;


	friend G1 hashToG1(std::string);
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
    G2()   { g2_inits(g); isInit = true; g2_set_infty(g); }
	G2(const G2& w) { g2_inits(g); g2_copy(g, const_cast<G2&>(w).g); isInit = true; }
	~G2()  {
		if(isInit){
			g2_free(g);
		}
	}

	G2& operator=(const G2& w)
	{
		if (isInit == true) g2_copy(g,  const_cast<G2&>(w).g);
		else ro_error();
		return *this;
	}
	bool ismember(bn_t);

    template<class Archive>
    void save(Archive & ar) const
    {

		G2 gg(*this);
    	unsigned int l  = g2_size_bin(gg.g,POINT_COMPRESS);

    	std::vector<uint8_t>data(l);
		g2_write_bin(&data[0], l,gg.g,POINT_COMPRESS);
		ar(data);
    }
    template<class Archive>
    void load(Archive & ar){
    	std::vector<uint8_t>data;
		ar(data);
    	g2_read_bin(g,&data[0],data.size());
    }
    friend class cereal::access;

	friend G2 hashToG2(std::string);
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
    GT()   { gt_inits(g); isInit = true; gt_set_unity(g); }
    GT(const GT& x) { gt_inits(g); isInit = true; gt_copy(g, const_cast<GT&>(x).g); }
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
	bool ismember(bn_t);
    template<class Archive>
    void save(Archive & ar) const
    {
		GT gg(*this);
    	unsigned int l  = gt_size_bin(gg.g,POINT_COMPRESS);

    	std::vector<uint8_t>data(l);
		gt_write_bin(&data[0], l, gg.g,POINT_COMPRESS);
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

class PairingGroup
{
public:
	PairingGroup();
	PairingGroup(int);
	// PairingGroup(int ptype,bool init,bn_t o){
	// 	pairingType = ptype;
	// 	isInit = init;
	// 	bn_copy(o,grp_order);
	// }

	~PairingGroup();
	void setCurve(int sec_level);


	ZR randomZR() const;
	G1 randomG1() const;
	G2 randomG2() const;
	GT randomGT() const;
//	bool ismember(CharmMetaListZR&);
//	bool ismember(CharmMetaListG1&);
//	bool ismember(CharmMetaListG2&);
//	bool ismember(CharmMetaListGT&);
//	bool ismember(CharmListStr&);
//	bool ismember(CharmListZR&);
//	bool ismember(CharmListG1&);
//	bool ismember(CharmListG2&);
//	bool ismember(CharmListGT&);
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

	// hash -- not done
	ZR hashListToZR(std::string) const;
	G1 hashListToG1(std::string) const;
	G2 hashListToG2(std::string) const;

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
	int pairingType; // defined by above #defines SYMMETRIC or ASYMMETRIC (for now)
	bool isInit;
	bn_t grp_order;
};

//template<> 
//ZR PairingGroup::random12<ZR>(){return this->random(ZR_t);}
bitset256 intToBits(const ZR & id);

#endif
