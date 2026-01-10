#define bittab __fsmu8

#include <stdint.h>
#include <features.h>

extern hidden const uint32_t bittab[];

#define OOB(c,b) (((((b)>>3)-0x10)|(((b)>>3)+((int32_t)(c)>>26))) & ~7)

#define R(a,b) ((uint32_t)((a==0x80 ? 0x40u-b : 0u-a) << 23))
#define FAILSTATE R(0x80,0x80)

#define SA 0xc2u
#define SB 0xf4u

#define CODEUNIT(c) (0xdfff & (signed char)(c))
#define IS_CODEUNIT(c) ((unsigned)(c)-0xdf80 < 0x80)

#include "locale_impl.h"
