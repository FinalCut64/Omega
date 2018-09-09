#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED

#include "definiciones.h"

BYTE MSB(BITBOARD);							//devuelve la posicion del bit mas significativo q es uno dentro de la BITBOARD
BYTE LSB(BITBOARD);							//devuelve la posicion del bit menos significativo q es uno dentro de la BITBOARD
BITBOARD FlipVertical(BITBOARD);            //hace que la fila 1 pase a la 8, la 2 a la 7, etc.
BITBOARD FlipDiagA8H1(BITBOARD);            //voltea la bitboard a lo largo del eje de la diagonal A8-H1
BITBOARD Rotar90clockwise (BITBOARD);       //rota la bitboard 90 grados en sentido horario
BITBOARD Rotar90antiClockwise (BITBOARD);   //rota la bitboard 90 grados en sentido antihorario

#endif // BITBOARD_H_INCLUDED
