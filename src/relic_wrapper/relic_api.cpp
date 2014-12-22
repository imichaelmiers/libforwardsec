#include <assert.h>
#include <stdexcept>
#include "relic_api.h"
using namespace std;
void ro_error(void)
{
	throw  std::invalid_argument("writing to read only object");
}

void invertZR(ZR & c, const ZR & a, const bn_t order)
{
	ZR a1 =a;
	bn_t s;
	bn_inits(s);
	// compute c = (1 / a) mod n
	bn_gcd_ext(s, c.z, NULL, a1.z, order);
	if(bn_sign(c.z) == BN_NEG) bn_add(c.z, c.z, order);
	bn_free(s);
}

// Begin ZR-specific classes
ZR::ZR(int x)
{
	bn_inits(z);
	bn_inits(order);
	g1_get_ord(order);
	isInit = true;
	if(x < 0) {
		bn_set_dig(z, x * -1); // set positive value
		bn_neg(z, z);			// set bn to negative
	}
	else {
		bn_set_dig(z, x);
	}
}

ZR::ZR(char *str)
{
	bn_inits(z);
	bn_inits(order);
	g1_get_ord(order);
	isInit = true;
	bn_read_str(z, (const char *) str, strlen(str), DECIMAL);
	// bn_mod(z, z, order);
}
const ZR ZR::inverse() const{
	ZR i;
	invertZR(i,z,i.order);
	return i;
}

ZR operator+(const ZR& x, const ZR& y)
{
	ZR zr;
	bn_add(zr.z, x.z, y.z);
	bn_mod(zr.z, zr.z, zr.order);
	return zr;
}

ZR operator-(const ZR& x, const ZR& y)
{
	ZR zr;

	bn_sub(zr.z, x.z, y.z);
	if(bn_sign(zr.z) == BN_NEG) bn_add(zr.z, zr.z, zr.order);
	else {
		bn_mod(zr.z, zr.z, zr.order);
	}
	return zr;
}

ZR operator-(const ZR& x)
{
	ZR zr;
	bn_neg(zr.z, x.z);
	if(bn_sign(zr.z) == BN_NEG) {
		bn_add(zr.z, zr.z, zr.order);
		return zr;
	}else{
		return zr;
	}
}


ZR operator*(const ZR& x, const ZR& y)
{
	ZR zr;
	bn_mul(zr.z, x.z, y.z);
	if(bn_sign(zr.z) == BN_NEG) bn_add(zr.z, zr.z, zr.order);
	else {
		bn_mod(zr.z, zr.z, zr.order);
	}

	return zr;
}
int bn_is_one(bn_t a)
{
	if(a->used == 0) return 0; // false
	else if((a->used == 1) && (a->dp[0] == 1)) return 1; // true
	else return 0; // false
}

ZR operator/(const ZR& x, const ZR& y)
{
	if(bn_is_zero(y.z)) {
		throw RelicDividByZero("divide by zero");
	}
	ZR i;
	invertZR(i,y,i.order);
	return x*i;
}

ZR power(const ZR& x, int r)
{
	ZR zr;
	bn_mxp(zr.z, x.z, ZR(r).z, zr.order);
	return zr;
}


ZR power(const ZR& x, const ZR& r)
{
	ZR zr;
	bn_mxp(zr.z, x.z, r.z, zr.order);
	return zr;
}

ZR hashToZR(string str)
{
	ZR zr;
	unsigned int digest_len = SHA_LEN;
	unsigned char digest[digest_len + 1];
	memset(digest, 0, digest_len);
	string str2 = string(HASH_FUNCTION_STR_TO_Zr_CRH) + str;
	SHA_FUNC(digest, (unsigned char *) str2.c_str(), (int) str2.size());

	bn_read_bin(zr.z, digest, digest_len);
	if(bn_cmp(zr.z, zr.order) == CMP_GT) bn_mod(zr.z, zr.z, zr.order);
	return zr;
}

bool ZR::ismember(void) const
{
	bool result;
	if((bn_cmp(z, order) < CMP_EQ) && (bn_sign(z) == BN_POS))
		result = true;
	else
		result = false;
	return result;
}

ostream& operator<<(ostream& s, const ZR& zr)
{
	int length = (compute_length(ZR_t) * 4);
	char data[length + 1];
	memset(data, 0, length);
	bn_write_str(data, length, zr.z, DECIMAL);
	string s1(data, length);
	s << s1;
	memset(data, 0, length);
	return s;
}

ZR operator<<(const ZR& a, int b)
{
	// left shift
	ZR zr;
	bn_lsh(zr.z, a.z, b);
	return zr;
}

ZR operator>>(const ZR& a, int b)
{
	// right shift
	ZR zr;
	bn_rsh(zr.z, a.z, b);
	return zr;
}

ZR operator&(const ZR& a, const ZR& b)
{
	int i, d = (a.z->used > b.z->used) ? b.z->used : a.z->used;
	bn_t c;
	bn_inits(c);

	for(i = 0; i < d; i++)
		c->dp[i] = (a.z->dp[i] & b.z->dp[i]);

	c->used = d;
	ZR zr(c);
	bn_free(c);
	return zr;
}

//ZR operator|(const ZR& a, const ZR& b)
//{
//	int i, d = (a.z->used > b.z->used) ? b.z->used : a.z->used;
//	bn_t c;
//	bn_inits(c);
//
//	for(i = 0; i < d; i++)
//		c->dp[i] = a.z->dp[i] | b.z->dp[i];
//
//	c->used = d;
//	ZR zr(c);
//	bn_free(c);
//	return zr;
//}

//ZR operator^(const ZR& a, const ZR& b)
//{
//	int i, d = (a.z->used > b.z->used) ? a.z->used : b.z->used;
//	bn_t c;
//	bn_inits(c);
//
//	for(i = 0; i < d; i++)
//		c->dp[i] = a.z->dp[i] ^ b.z->dp[i];
//
//	c->used = d;
//	ZR zr(c);
//	bn_free(c);
//	return zr;
//}


// End ZR-specific classes

// Begin G1-specific classes

G1 operator+(const G1& x, const G1& y)
{
	G1 z;
	g1_add(z.g, x.g, y.g);
	g1_norm(z.g,z.g);

	return z;
}

G1 operator-(const G1& x, const G1& y)
{
	G1 z ;
	g1_sub(z.g,x.g, y.g);
	g1_norm(z.g,z.g);

	return z;
}

G1 operator-(const G1& x)
{
	G1 z;
	g1_neg(z.g, x.g);
	return z;
}

G1 power(const G1& g, const ZR& zr)
{
	G1 g1;
	g1_mul(g1.g, g.g, zr.z);
	return g1;
}

G1 hashToG1(string str)
{
	G1 g1;
	int digest_len = SHA_LEN;
	unsigned char digest[digest_len + 1];
	memset(digest, 0, digest_len);
	string str2 = string(HASH_FUNCTION_Zr_TO_G1_ROM) + str;
	SHA_FUNC(digest, (unsigned char *) str2.c_str(), (int) str2.size());

	g1_map(g1.g, digest, digest_len);
	return g1;
}

bool G1::ismember(const bn_t order) const
{
	bool result;
	g1_t r;
	g1_inits(r);

	g1_mul(r, g, order);
	if(g1_is_infty(r) == 1)
		result = true;
	else
		result = false;
	g1_free(r);
	return result;
}

ostream& operator<<(ostream& s, const G1& g1)
{
	unsigned int l  = g1_size_bin(g1.g,POINT_COMPRESS);
	std::vector<uint8_t>data(l);
	g1_write_bin(&data[0], data.size(), g1.g,POINT_COMPRESS);
	s << "0x";
	for(auto i : data){
	std::cout << std::hex << data[i];
	}
	std::cout << std::endl;
	return s;
}

// End G1-specific classes

// Begin G2-specific classes

G2 operator+(const G2& x, const G2& y)
{
	G2 z, x1=x,y1=y ;
	g2_add(z.g, x1.g, y1.g);
	g2_norm(z.g,z.g);
	return z;
}

G2 operator-(const G2& x, const G2& y)
{
	G2 z,x1=x,y1=y;
	g2_sub(z.g,x1.g, y1.g);
	g2_norm(z.g,z.g);
	return z;
}

G2 operator-(const G2& x)
{
	G2 z,x1=x;
	g2_neg(z.g, x1.g);
	return z;
}

G2 power(const G2& g, const ZR& zr)
{
	G2 g2,g1=g;
	ZR zr1=zr;
	g2_mul(g2.g,g1.g, zr1.z);
	return g2;
}

G2 hashToG2(string str)
{
	G2 g2;
	int digest_len = SHA_LEN;
	unsigned char digest[digest_len + 1];
	memset(digest, 0, digest_len);
	string str2 = string(HASH_FUNCTION_Zr_TO_G2_ROM) + str;
	SHA_FUNC(digest, (unsigned char *) str2.c_str(), (int) str2.size());

	g2_map(g2.g, digest, digest_len);
	return g2;
}

bool G2::ismember(bn_t order)
{
	bool result;
	g2_t r;
	g2_inits(r);
	g2_mul(r, g, order);
	if(g2_is_infty(r) == 1)
		result = true;
	else
		result = false;
	g2_free(r);
	return result;
}

ostream& operator<<(ostream& s, const G2& g2)
{
	lfrowdsec_G2unconst(g2);
	unsigned int l  = g2_size_bin(gg.g,POINT_COMPRESS);
	std::vector<uint8_t>data(l);
	g2_write_bin(&data[0], data.size(), gg.g,POINT_COMPRESS);
	s << "0x";
	for(auto i : data){
	std::cout << std::hex << data[i];
	}
	std::cout << std::endl;
	return s;
}

// End G2-specific classes

// Begin GT-specific classes

GT operator*(const GT& x, const GT& y)
{
	GT z, x1 = x, y1 = y;
	gt_mul(z.g, x1.g, y1.g);
	return z;
}

GT operator/(const GT& x, const GT& y)
{
	GT z, x1=x, y1 = y;
	// z = x * y^-1
	gt_t t;
	gt_inits(t);
	gt_inv(t, y1.g);
	gt_mul(z.g, x1.g, t);
	gt_free(t);
	return z;
}

GT power(const GT& g, const ZR& zr)
{
	GT gt,gg=g;
	ZR zr1 = zr;
	if(zr == ZR(-1)) { // find efficient way for comparing bn_t to ints
		// compute inverse
		return -g;
	}
	else {
		gt_exp(gt.g, gg.g, zr1.z);
	}
	return gt;
}

GT operator-(const GT& g)
{
	GT gt,gg=g;
	gt_inv(gt.g, gg.g);
	return gt;
}

GT pairing(const G1& g1, const G2& g2)
{
	GT gt;
	G1 g11=g1;
	G2 g22=g2;
	/* compute optimal ate pairing */
	pp_map_oatep_k12(gt.g, g11.g, g22.g);
	//pp_map_k12(gt.g, g11.g, g22.g);
	return gt;
}

//GT pairing(const G1& g10, const G1& g11)
//{
//	GT gt;
//	/* compute optimal ate pairing */
////	pp_map_oatep(gt.g, g1.g, g2.g);
//	return gt;
//}

bool GT::ismember(bn_t order)
{
	bool result;
	gt_t r;
	gt_inits(r);
	gt_exp(r,g, order);
	if(gt_is_unity(r) == 1)
		result = true;
	else
		result = false;
	gt_free(r);
	return result;
}

ostream& operator<<(ostream& s, const GT& gt)
{
	// designed for BN curves at the moment
	// int length = (compute_length(GT_t) * 2)+2;
	// char data[length + 1];
	// char data2[length + 1];
	// memset(data, 0, length+1);
	// memset(data2, 0, length+1);
	// gt_write_str( (uint8_t *) data, length,const_cast<GT&>(gt).g,DECIMAL);

	// int len2 = FP_STR;
	// int dist_x01 = len2, dist_x10 = len2 * 2, dist_x11 = len2 * 3,
	// 	dist_x20 = len2 * 4, dist_x21 = len2 * 5, dist_y00 = len2 * 6,
	// 	dist_y01 = len2 * 7, dist_y10 = len2 * 8, dist_y11 = len2 * 9,
	// 	dist_y20 = len2 * 10, dist_y21 = len2 * 11;
	//  snprintf(data2, length, "[%s, %s, %s, %s, %s, %s],\n[%s, %s, %s, %s, %s, %s]",
	//  			  data, &(data[dist_x01]), &(data[dist_x10]), &(data[dist_x11]),
	// 			  &(data[dist_x20]), &(data[dist_x21]),
	// 			  &(data[dist_y00]), &(data[dist_y01]), &(data[dist_y10]), &(data[dist_y11]),
	// 			  &(data[dist_y20]), &(data[dist_y21]));

	// string s1(data2, length);
	// s << s1;
	// memset(data, 0, length);
	// memset(data2, 0, length);
	s << "BROKEN";
	return s;
}


PairingGroup::PairingGroup()
{
	this->setCurve(256);
	isInit = true ; // user needs to call setCurve after construction
}

PairingGroup::PairingGroup(int sec_level)
{
	this->setCurve(sec_level);
}

void PairingGroup::setCurve(int sec_level)
{
//	cout << "Initializing PairingGroup: RELIC" << endl;
	int err_code = core_init();
//	cout <<"core_init_done" << endl;
	if(err_code != STS_OK) isInit = false;
//	conf_print();
	pc_param_set_any(); // see if we can open this up?
	isInit = true;
	bn_inits(grp_order);
	g1_get_ord(grp_order);
}

PairingGroup::~PairingGroup()
{
	if(isInit) {
//		core_clean();
//		bn_free(grp_order);
	}
}

void PairingGroup::init(ZR & r, char *value) const
{
	r = ZR(value);
}

ZR PairingGroup::init(ZR_type t, int value) const
{
	ZR zr(value);
	return zr;
}

void PairingGroup::init(ZR & r, int value) const
{
	r = ZR(value); //should copy this
	return;
}

ZR PairingGroup::init(ZR_type t) const
{
	ZR zr;
	return zr;
}

G1 PairingGroup::init(G1_type t) const
{
	G1 g1;
	return g1;
}

void PairingGroup::init(G1 & t, int value) const
{
	G1 g1;
	if(value == 1) t = g1; // set to the identity element
	return;
}

G1 PairingGroup::init(G1_type t, int value) const
{
	G1 g1; // = new G1();
	return g1;
}

//#ifdef ASYMMETRIC
G2 PairingGroup::init(G2_type t) const
{
	G2 g2; // = new G2();
	return g2;
}

// G2 PairingGroup::init(G2_type t, int value)
// {
// 	G2 g2; // = new G2();
// 	return g2;
// }

// void PairingGroup::init(G2 & t, int value)
// {
// 	G2 g2;
// 	t = g2;
// 	return;
// }
//#endif

GT PairingGroup::init(GT_type t) const
{
	GT g;
	return g;
}

GT PairingGroup::init(GT_type t, int value) const
{
	GT g;
	return g;
}

// void PairingGroup::init(GT & t, int value)
// {
// 	GT g;
// 	t = g;
// 	return;
// }

ZR PairingGroup::random(ZR_type t) const
{
	ZR zr,tt;
	bn_rand(tt.z, BN_POS, bn_bits(grp_order));
	bn_mod(zr.z,  tt.z, grp_order);
	return zr;
}

G1 PairingGroup::random(G1_type t) const
{
	G1  g1;
	g1_rand(g1.g);
	return g1;
}

G2 PairingGroup::random(G2_type t) const
{
	G2  g2;
	g2_rand(g2.g);
	return g2;
}

GT PairingGroup::random(GT_type t) const
{
	GT gts;
	gt_rand(gts.g);
    return gts;
}

ZR PairingGroup::neg(const ZR & r) const
{

    return -r;
}

ZR PairingGroup::inv(const ZR &  r) const
{
     return r.inverse();
}
G1 PairingGroup::inv(const G1 & g) const
{
	return -g;
}
G2 PairingGroup::inv(const G2 & g) const
{
	return -g;
}
GT PairingGroup::inv(const GT & g) const
{
	return -g;
}

bool PairingGroup::ismember(ZR & zr)
{
	return zr.ismember();
}

bool PairingGroup::ismember(G1 & g)
{
	return g.ismember(grp_order);
}

bool PairingGroup::ismember(G2 & g)
{
	return g.ismember(grp_order); // add code to check
}


G2 PairingGroup::mul(const G2 & g, const G2 & h) const
{
	return g + h;
}

G2 PairingGroup::div(const G2 & g, const G2 & h) const
{
	return g + -h;
}

G2 PairingGroup::exp(const G2 & g, const ZR & r) const
{
	// g ^ r == g * r OR scalar multiplication
	return power(g, r);
}

G2 PairingGroup::exp(const G2 & g, const int & r) const
{
	// g ^ r == g * r OR scalar multiplication
	return power(g, ZR(r));
}

GT PairingGroup::pair(const G1 & g, const G2 & h) const
{
	return pairing(g, h);
}

GT PairingGroup::pair(const G2 & h, const G1 & g) const
{
	return pairing(g, h);
}


// GT PairingGroup::pair(G1 g, G1 h)
// {
// 	return pairing(g, h);
// }

bool PairingGroup::ismember(GT & g)
{
	return g.ismember(grp_order); // add code to check
}

ZR PairingGroup::order() const
{
	return ZR(grp_order);
}

int PairingGroup::add(const int&  g, const int & h) const
{
	return g + h;
}

ZR PairingGroup::add(const ZR & g, const ZR & h) const
{
	return g + h;
}

int PairingGroup::sub(const int& g, const int & h) const
{
	return g - h;
}

ZR PairingGroup::sub(const ZR & g, const ZR & h) const
{
	return g - h;
}

int PairingGroup::mul(const int & g, const int & h) const
{
	return g * h;
}


ZR PairingGroup::mul(const ZR & g, const ZR & h) const
{
	return g * h;
}

// mul for G1 & GT
G1 PairingGroup::mul(const G1 & g, const G1 & h) const
{
	return g + h;
}

GT PairingGroup::mul(const GT & g, const GT & h) const
{
	return g * h;
}

ZR PairingGroup::div(const int & g, const ZR & h) const
{
	return ZR(g) / h;
}

ZR PairingGroup::div(const ZR & g, const ZR& h) const
{
	return g / h;
}

// div for G1 & GT
G1 PairingGroup::div(const G1 & g, const G1&  h) const
{
	return g + -h;
}

GT PairingGroup::div(const GT & g, const GT&  h) const
{
	return g / h;
}

int PairingGroup::div(const int & g, const int & h) const
{
	return g / h;
}

ZR PairingGroup::exp(const ZR & x, const int & y) const
{
	return power(x, y);
}

ZR PairingGroup::exp(const ZR & x, const ZR & y) const
{
	return power(x, y);
}

//// exp for G1 & GT
G1 PairingGroup::exp(const G1 & g, const ZR & r) const
{
	// g ^ r == g * r OR scalar multiplication
 	return power(g, r);
}

G1 PairingGroup::exp(const G1 & g , const int & r) const
{
	// g ^ r == g * r OR scalar multiplication
 	return power(g, ZR(r));
}

GT PairingGroup::exp(const GT & g, const ZR & r) const
{
	// g ^ r == g * r OR scalar multiplication
	return power(g, r);
}

GT PairingGroup::exp(const GT & g, const int & r) const
{
	// g ^ r == g * r OR scalar multiplication
	return power(g, ZR(r));
}

ZR PairingGroup::hashListToZR(string str) const
{
	ZR r = hashToZR(str);
	return r;
}

G1 PairingGroup::hashListToG1(string str) const
{
	G1 l = hashToG1(str);
	return l;
}

G2 PairingGroup::hashListToG2(string str) const
{
	G2 l = hashToG2(str);
	return l;
}

bitset256 intToBits(const ZR &id){
	bitset256 zrlist;
	int l=256;
    int intval;
    int j = l-1;

    for(int i = 0; i < l; i++) {
    	intval = bn_get_bit(id.z,i);
    	/* store in reverse */
    	zrlist[j-i] = intval;
    }
    return zrlist;
}
