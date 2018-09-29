#include "bitboard.h"

BYTE LSB(BITBOARD x)
{
	if (x == 0)
		return 64;
	return ctz(x);                              		//retorna el indice (0..63) del LSB q es 1 (64 si bb == 0)
}

BYTE MSB(BITBOARD x)
{
    if (x == 0)
        return 64;
    return (63 - clz(x));
}

BITBOARD FlipVertical(BITBOARD x)
{
    return  ( (x << 56)                        ) |
            ( (x << 40) & (0x00ff000000000000) ) |
            ( (x << 24) & (0x0000ff0000000000) ) |
            ( (x <<  8) & (0x000000ff00000000) ) |
            ( (x >>  8) & (0x00000000ff000000) ) |
            ( (x >> 24) & (0x0000000000ff0000) ) |
            ( (x >> 40) & (0x000000000000ff00) ) |
            ( (x >> 56) );
}

BITBOARD FlipDiagA8H1(BITBOARD x)
{
   BITBOARD t;
   const BITBOARD k1 = 0xaa00aa00aa00aa00;
   const BITBOARD k2 = 0xcccc0000cccc0000;
   const BITBOARD k4 = 0xf0f0f0f00f0f0f0f;

   t  =       x ^ (x << 36) ;
   x ^= k4 & (t ^ (x >> 36));
   t  = k2 & (x ^ (x << 18));
   x ^=       t ^ (t >> 18) ;
   t  = k1 & (x ^ (x <<  9));
   x ^=       t ^ (t >>  9) ;
   return x;
}

BITBOARD Rotar90clockwise (BITBOARD x)
{
   return FlipDiagA8H1(FlipVertical(x));
}

BITBOARD Rotar90antiClockwise (BITBOARD x)
{
   return FlipVertical(FlipDiagA8H1(x));
}
