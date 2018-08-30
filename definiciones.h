#ifndef DEFINICIONES_H_INCLUDED
#define DEFINICIONES_H_INCLUDED

    /////////////////////////////////////////////////////////////////////////////
    //                            DEFINICIONES DEL MOTOR
    /////////////////////////////////////////////////////////////////////////////

#define	PIC           1
#define HUMANO        0

#define INVALIDO      31000
#define MATE					29999
#define R             2
#define MAXPROF       30

#define Abs(a)                    (((a) > 0) ? (a) : -(a))
#define Max(a,b)                  (((a) > (b)) ? (a) : (b))
#define Min(a,b)                  (((a) < (b)) ? (a) : (b))
#define Fila(x)                   ((x)>>3)						//entrando con x desde 0 a 63 devuelve en x el numero de fila q le corresponde
#define Columna(x)				        ((x)&7)							//igual q el anterior devuelve el numero de columna q le corresponde
#define DistanciaFilas(a,b)       Abs(Fila(a) - Fila(b))
#define DistanciaColumnas(a,b)    Abs(Columna(a) - Columna(b))
#define Distancia(a,b)            Max(DistanciaFilas(a,b), DistanciaColumnas(a,b))
#define Flip(x)                   ((x)^1)
#define SetMask(a)                (set_mask[a])
#define ClearMask(a)              (clear_mask[a])
#define SetMask90(a)              (set_mask90[a])
#define ClearMask90(a)            (clear_mask90[a])
#define SetMaskA1(a)              (set_maskA1[a])
#define ClearMaskA1(a)            (clear_maskA1[a])
#define SetMaskA8(a)              (set_maskA8[a])
#define ClearMaskA8(a)            (clear_maskA8[a])
#define POPCNT(a)                 __builtin_popcountll(a)
#define ctz(a)                    (__builtin_ctzll(a))				//macro para implementar la instruccion clz directamente
#define clz(a)                    (__builtin_clzll(a))

//////////////////////////////////////////////////////////////////////////////
//                          DEFINICIONES OPCIONALES                         //
//////////////////////////////////////////////////////////////////////////////

#define USAR_RELOJ

#define USAR_NULO                   //comentar para desactivar caracteristicas o descomentar para implementarlas
#define PODAR_INUTILIDAD
//#define PODAR_INUTILIDAD_INVERSA
#define USAR_TT
//#define USAR_MULTICUT

typedef unsigned long long BITBOARD;	//llamo BITBOARD a los long long para q se haga mas facil de entender
typedef unsigned char BYTE;             //llamo BYTE a los char para q se haga mas facil de entender


#endif // DEFINICIONES_H_INCLUDED
