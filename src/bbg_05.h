#include <relic/relic.h>

#define HIBE_DEPTH 10 
#ifndef BBG_05_H
#define BBG_05_H
typedef struct{
  uint max_depth;
  g1_t g;
  g2_t gg;
  g2_t g1;
  g1_t g2;
  g1_t g3;
  g1_t h[HIBE_DEPTH];
} bbg_pk;

typedef struct{
	uint k; // current depth 	


} bbg_sk;

typedef struct{

} bbg_ct;
#endif