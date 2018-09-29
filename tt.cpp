//#include <iostream>
#include <windows.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <setjmp.h>
//#include <string.h>
//#include "definiciones.h"
#include "global.h"
#include "tt.h"
#include "bitboard.h"





//estos son para el generador de numeros aleatorios/////////////////////////////
// The array for the state vector                                             //
static unsigned long long mt[NN];                                             //
// mti==NN+1 means mt[NN] is not initialized                                  //
static int mti=NN+1;                                                          //
////////////////////////////////////////////////////////////////////////////////

//aca comienza todo lo relacionado con la tabla de transposicion (tt)
struct sZob Zobrist;
/*
struct sZob                             //le meto la s adelante para saber que este es el tipo structura
{
    BITBOARD escaques[12][64];          //todas las combinaciones de piezas y casillas
    BITBOARD bando;                     //un numero para el bando q le toca mover
    BITBOARD enroques[16];              //16 numeros para los derechos de enrroques
    BITBOARD alpaso[9];                 //no confundir con alpaso del motor ya q soy registros distintos y no entran en conflicto
}Zobrist;                               //y este es el nombre de esta estructura del tipo sZob
*/
struct stt *tt;
/*
struct stt
{
    BITBOARD confirma;      //tiene la hash para saber si corresponde
    short int val;          //valor obtenido de la posicion
    short int mejorjugada;  //la mejor jugada q se encontro para esta posicion
    BYTE prof;              //profundidad a la que se analizo el nodo
    BYTE bandera;           //bandera q indica tipo de nodo (nodo alfa, beta o pv)
    BYTE edad;
} * tt;
*/
/*
enum banderas_tt
{
    TT_EXACTO,
    TT_ALFA,
    TT_BETA
};
*/
//fin de la tt
//comienzo de la hojastt y la evaltt
struct shojastt *hojastt;
/*
struct shojastt
{
    BITBOARD confirma;
    int val;
    int alfa;
    int beta;
    BITBOARD inutil;
} * hojastt;
*/
struct sevaltt *evaltt;
/*
struct sevaltt
{
    BITBOARD confirma;
    int val;
} * evaltt;
struct sperfttt
{
    BITBOARD confirma;
    BITBOARD nodos;
    BYTE prof;
    int relleno;
    int relleno2;
} * perfttt;
*/
struct sperfttt *perfttt;

int perfttt_size;
BITBOARD perfttt_hits;

BITBOARD mejortt;


BITBOARD hash_pos;                      //la llave zobrist con el valor hash de la posicion
BITBOARD hash_pos2;                     //esta es para guardarlo y recuperarlo al salir interrumpido por tiempo de Analiza()
BITBOARD tt_hits;                       //contador de hits de la tabla
int tt_size,mejor_jugada,tt_mejor;
BYTE tt_hit;
///////////////////////////////////////Variables de las HOJAS tt////////////////////////////////////////////////
int hojastt_size,evaltt_size;               //size de la tabla de nodos hojas y la de evaluaciones
BITBOARD hojastt_hits,evaltt_hits;          //contador de hits de la tabla de hojas y de evaluaciones


void init_genrand64(unsigned long long seed)    // inicializa mt[NN] con una semilla
{
    mt[0] = seed;
    for (mti=1; mti<NN; mti++)
        mt[mti] =  (6364136223846793005ULL * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
void InicializaAleatorio()
{
    unsigned long long init_key[4] = {0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
    unsigned long long key_length = 4;
    unsigned long long i, j, k;
    init_genrand64(19650218ULL);
    i=1; j=0;
    k = (NN>key_length ? NN : key_length);
    for (; k; k--){
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 3935559000370003845ULL))
          + init_key[j] + j; /* non linear */
        i++; j++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=NN-1; k; k--)
    {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 2862933555777941757ULL))
          - i; /* non linear */
        i++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
    }

    mt[0] = 1ULL << 63; /* MSB is 1; assuring non-zero initial array */
}

unsigned long long GeneraAleatorio(void) /* generates a random number on [0, 2^64-1]-interval */
{
    int i;
    unsigned long long x;
    static unsigned long long mag01[2]={0ULL, MATRIX_A};

    if (mti >= NN) { /* generate NN words at one time */

        /* if init_genrand64() has not been called, */
        /* a default initial seed is used     */
        if (mti == NN+1)
            init_genrand64(5489ULL);

        for (i=0;i<NN-MM;i++) {
            x = (mt[i]&UM)|(mt[i+1]&LM);
            mt[i] = mt[i+MM] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        }
        for (;i<NN-1;i++) {
            x = (mt[i]&UM)|(mt[i+1]&LM);
            mt[i] = mt[i+(MM-NN)] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        }
        x = (mt[NN-1]&UM)|(mt[0]&LM);
        mt[NN-1] = mt[MM-1] ^ (x>>1) ^ mag01[(int)(x&1ULL)];

        mti = 0;
    }

    x = mt[mti++];

    x ^= (x >> 29) & 0x5555555555555555ULL;
    x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
    x ^= (x << 37) & 0xFFF7EEE000000000ULL;
    x ^= (x >> 43);

    return x;
}

void InicializaZobrist()
{
    int i,j;

    for (i=0;i<64;i++)
    {
        for (j=0;j<12;j++)
        {
            Zobrist.escaques[j][i] = GeneraAleatorio();
        }
    }
    Zobrist.bando = GeneraAleatorio();
    for (i=0;i<16;i++)
    {
        Zobrist.enroques[i] = GeneraAleatorio();
    }
    for (i=0;i<9;i++)
    {
        Zobrist.alpaso[i] = GeneraAleatorio();
    }
}

void InicializaHash()
{
    BITBOARD aux;

    hash_pos = 0;           //inicio a 0

    inicio = MSB(rey_b);    //primero comienzo con las piezas y las casillas
    hash_pos ^= Zobrist.escaques[Rey_b][inicio];    //agrego la info del rey blanco q siempre esta
    aux = peones_b;
    while((inicio = MSB(aux)) != 64)
    {
        hash_pos ^= Zobrist.escaques[Peon_b][inicio];
        aux ^= SetMask(inicio);                         //borro este peon para q encuentre otro si lo hay
    };
    aux = caballos_b;
    while((inicio = MSB(aux)) != 64)
    {
        hash_pos ^= Zobrist.escaques[Caballo_b][inicio];
        aux ^= SetMask(inicio);
    };
    aux = alfiles_b;
    while((inicio = MSB(aux)) != 64)
    {
        hash_pos ^= Zobrist.escaques[Alfil_b][inicio];
        aux ^= SetMask(inicio);
    };
    aux = torres_b;
    while((inicio = MSB(aux)) != 64)
    {
        hash_pos ^= Zobrist.escaques[Torre_b][inicio];
        aux ^= SetMask(inicio);
    };
    aux = damas_b;
    while((inicio = MSB(aux)) != 64)
    {
        hash_pos ^= Zobrist.escaques[Dama_b][inicio];
        aux ^= SetMask(inicio);
    };

    inicio = MSB(rey_n);    //ahora = para las negras
    hash_pos ^= Zobrist.escaques[Rey_n][inicio];
    aux = peones_n;
    while((inicio = MSB(aux)) != 64)
    {
        hash_pos ^= Zobrist.escaques[Peon_n][inicio];
        aux ^= SetMask(inicio);
    };
    aux = caballos_n;
    while((inicio = MSB(aux)) != 64)
    {
        hash_pos ^= Zobrist.escaques[Caballo_n][inicio];
        aux ^= SetMask(inicio);
    };
    aux = alfiles_n;
    while((inicio = MSB(aux)) != 64)
    {
        hash_pos ^= Zobrist.escaques[Alfil_n][inicio];
        aux ^= SetMask(inicio);
    };
    aux = torres_n;
    while((inicio = MSB(aux)) != 64)
    {
        hash_pos ^= Zobrist.escaques[Torre_n][inicio];
        aux ^= SetMask(inicio);
    };
    aux = damas_n;
    while((inicio = MSB(aux)) != 64)
    {
        hash_pos ^= Zobrist.escaques[Dama_n][inicio];
        aux ^= SetMask(inicio);
    };

    if (turno_c == negras)          //si en la posicion de inicializacion le toca al negro
    {
        hash_pos ^= Zobrist.bando;  //agrego la data de la Zobrist de bando (si le toca al blanco se distingue por su ausencia)
    }

    hash_pos ^= Zobrist.enroques[derechos_enroque[0]];

    hash_pos ^= Zobrist.alpaso[alpaso[0]];  //esto Xorea la posicion hash con la correspondiente llave alpaso de la columna
}

int Setsize_tt(int size)
{
    free(tt);

    if (size & (size - 1))
    {
        size--;
        for (int i=1; i<32; i=i*2)
        size |= size >> i;
        size++;
        size>>=1;
    }
    if (size < 16)
    {
        tt_size = 0;
        return 0;
    }
    tt_size = (size / sizeof(stt)) -1;
    tt = (stt *) malloc(size);

    return 0;
}

void Save_tt(unsigned char prof,int val,char bandera,int mejor)
{
//    if (!tt_size) return;

    stt * phashe = &tt[hash_pos & tt_size];

    if ( (phashe->confirma == hash_pos) && (phashe->prof > prof) ) return;  //no queremos guardar el resultado del analisis
                                                                            //si es la misma posicion y tiene menos prof
    phashe->confirma = hash_pos;        //guardamos todos los datos que conforman una entrada de la tabla
    phashe->val = val;
    phashe->bandera = bandera;
    phashe->prof = prof;
    phashe->mejorjugada = mejor;
//    phashe->edad = ply_count;
}

int Probar_tt(BYTE prof,int alfa,int beta)
{
//    if (!tt_size) return INVALIDO;          //si no estamos usando la tt entonces salimos sin hacer nada

    stt * phashe = &tt[hash_pos & tt_size]; //buscamos en la tt la entrada q tiene la info de esta posicion y la vinculamos al
                                            //puntero a estructura "phashe" que creamos
    if (phashe->confirma == hash_pos)       //nos fijamos si la posicion actual (hash_pos) coincide con la guardada en la tabla
    {
        if (phashe->prof >= prof)           //ahora miramos si podemos obtener el valor de la posicion
        {                                   //(la profundidad guardada debe ser mayor o igual a la actual)
            if (phashe->bandera == TT_EXACTO)
            {
                val = phashe->val;
                if (val > 29000)
//                    val += ply_count - phashe->edad;
                if (val < -29000)
//                    val -= ply_count - phashe->edad;
                return val;
            }
            if ((phashe->bandera == TT_ALFA) && (phashe->val <= alfa))
                return alfa;
            if ((phashe->bandera == TT_BETA) && (phashe->val >= beta))
                return beta;
        }                                       //si fallaron todos los limites para terminar la evaluacion entonces
        tt_mejor = phashe->mejorjugada;         //aca al menos obtenemos una buena jugada que sera util para ordenar la busqueda
        tt_hit = 1;                             //aviso q hubo un hit para q ordenar() sepa q hay una jugada de tt disponible
    }
    return INVALIDO;     //esto significaria (hasta donde entiendo) que no se produjo un table hit
}

int Setsize_hojastt(int size)            //dimensiona la hojastt
{
    free(hojastt);

    if (size & (size - 1))
    {
        size--;
        for (int i=1; i<32; i=i*2)
        size |= size >> i;
        size++;
        size>>=1;
    }
    if (size < 16)
    {
        hojastt_size = 0;
        return 0;
    }

    hojastt_size = (size / sizeof(shojastt)) -1;
    hojastt = (shojastt *) malloc(size);
    return 0;
}

int Probar_hojastt(int alfa, int beta)      //verifica si se produce un hit en hojastt lo que corta todo Quies()
{
//    if (!hojastt_size) return INVALIDO;

    shojastt * phashe = &hojastt[hash_pos & hojastt_size];
    if ((phashe->confirma == hash_pos) && (phashe->alfa == alfa) && (phashe->beta == beta))
        return phashe->val;

    return INVALIDO;
}

void Save_hojastt(int alfa,int beta, int val)       //guarda la hoja con la valoracion calculada y los limites
{
//    if (!hojastt_size) return;

    shojastt * phashe = &hojastt[hash_pos & hojastt_size];

    phashe->confirma = hash_pos;
    phashe->val = val;
    phashe->alfa = alfa;
    phashe->beta = beta;
}

int Setsize_evaltt(int size)            //dimensiona la evaltt
{
    free(evaltt);

    if (size & (size - 1))
    {
        size--;
        for (int i=1; i<32; i=i*2)
        size |= size >> i;
        size++;
        size>>=1;
    }
    if (size < 16)
    {
        evaltt_size = 0;
        return 0;
    }

    evaltt_size = (size / sizeof(sevaltt)) -1;
    evaltt = (sevaltt *) malloc(size);
    return 0;
}

int Probar_evaltt()      //verifica si se produce un hit en evaltt lo que evita gastar tiempo en Evaluar()
{
//    if (!evaltt_size) return INVALIDO;

    sevaltt * phashe = &evaltt[hash_pos & evaltt_size];
    if (phashe->confirma == hash_pos)
        return phashe->val;

    return INVALIDO;
}

void Save_evaltt(int val)       //guarda la posicion con la valoracion calculada
{
//    if (!evaltt_size) return;

    sevaltt * phashe = &evaltt[hash_pos & evaltt_size];

    phashe->confirma = hash_pos;
    phashe->val = val;
}

int Setsize_perfttt(int size)            //dimensiona la perfttt
{
    free(perfttt);

    if (size & (size - 1))
    {
        size--;
        for (int i=1; i<32; i=i*2)
        size |= size >> i;
        size++;
        size>>=1;
    }
    if (size < 16)
    {
        perfttt_size = 0;
        return 0;
    }

    perfttt_size = (size / sizeof(sperfttt)) -1;
    perfttt = (sperfttt *) malloc(size);
    return 0;
}

BITBOARD Probar_perfttt(BYTE prof)      //verifica si se produce un hit en perfttt
{
//    if (!perfttt_size) return INVALIDO;

    sperfttt * phashe = &perfttt[hash_pos & perfttt_size];
    if (phashe->confirma == hash_pos && phashe->prof == prof)
        return phashe->nodos;

    return 0;
}

void Save_perfttt(BITBOARD nodos,BYTE prof)       //guarda la cant de nodos hoja q derivan de la posicion a una prof dada
{
//    if (!perfttt_size) return;

    sperfttt * phashe = &perfttt[hash_pos & perfttt_size];

    phashe->confirma = hash_pos;
    phashe->nodos = nodos;
    phashe->prof = prof;
}
