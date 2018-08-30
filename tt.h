#ifndef TT_H_INCLUDED
#define TT_H_INCLUDED

#include "definiciones.h"

//definiciones para el generador de numeros aleatorios de 64 bits/////
#define NN 312                                                      //
#define MM 156                                                      //
#define MATRIX_A 0xB5026F5AA96619E9ULL                              //
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */     //
#define LM 0x7FFFFFFFULL /* Least significant 31 bits */            //
//////////////////////////////////////////////////////////////////////

//declaracion de funciones del generador de numeros aleatorios////
void init_genrand64(unsigned long long);                        //
void InicializaAleatorio(void);                                 //
unsigned long long GeneraAleatorio(void);                       //
//////////////////////////////////////////////////////////////////

void InicializaZobrist();           //inicia con numeros aleatorios las llaves Zobrist usadas en la tabla de transposicion
void InicializaHash();              //dada una posicion en el tablero obtiene la clave hash que le corresponde
int Setsize_tt(int);
void Save_tt(unsigned char,int,char,int);
int Probar_tt(BYTE,int,int);

int Setsize_hojastt(int);           //dimensiona la hojastt
int Probar_hojastt(int,int);        //verifica si se produce un hit en hojastt lo que corta todo Quies()
void Save_hojastt(int,int,int);     //guarda en la tt la posicion y valoracion de la hoja actual

int Setsize_evaltt(int);            //selecciona el size de la tt de evaluacion
int Probar_evaltt();                //verifica si se produce un hit en evaltt lo que evita gastar tiempo en Evaluar()
void Save_evaltt(int);              //guarda en evaltt el valor q devuelve Evaluar() para la posicion actual

int Setsize_perfttt(int);           //selecciona el size de la tt de Perft()
BITBOARD Probar_perfttt(BYTE);      //verifica si se produce un hit en perfttt (de ser asi devuelve el nro de nodos hojas)
void Save_perfttt(BITBOARD,BYTE);   //guarda en perfttt la cantidad de nodos hoja q se desprenden del nodo actual

//////////////////////////////////////////////////////////////
////////////////////////VARIABLES/////////////////////////////
//////////////////////////////////////////////////////////////


//aca comienza todo lo relacionado con la tabla de transposicion (tt)
extern struct sZob                             //le meto la s adelante para saber que este es el tipo structura
{
    BITBOARD escaques[12][64];          //todas las combinaciones de piezas y casillas
    BITBOARD bando;                     //un numero para el bando q le toca mover
    BITBOARD enroques[16];              //16 numeros para los derechos de enrroques
    BITBOARD alpaso[9];                 //no confundir con alpaso del motor ya q soy registros distintos y no entran en conflicto
}Zobrist;                               //y este es el nombre de esta estructura del tipo sZob



extern struct stt
{
    BITBOARD confirma;      //tiene la hash para saber si corresponde
    short int val;          //valor obtenido de la posicion
    short int mejorjugada;  //la mejor jugada q se encontro para esta posicion
    BYTE prof;              //profundidad a la que se analizo el nodo
    BYTE bandera;           //bandera q indica tipo de nodo (nodo alfa, beta o pv)
    BYTE edad;
} * tt;

enum banderas_tt
{
    TT_EXACTO,
    TT_ALFA,
    TT_BETA
};

//fin de la tt
//comienzo de la hojastt y la evaltt

extern struct shojastt
{
    BITBOARD confirma;
    int val;
    int alfa;
    int beta;
    BITBOARD inutil;
} * hojastt;

extern struct sevaltt
{
    BITBOARD confirma;
    int val;
} * evaltt;

extern struct sperfttt
{
    BITBOARD confirma;
    BITBOARD nodos;
    BYTE prof;
    int relleno;
    int relleno2;
} * perfttt;

extern int perfttt_size;
extern BITBOARD perfttt_hits;

extern BITBOARD mejortt;


extern BITBOARD hash_pos;                      //la llave zobrist con el valor hash de la posicion
extern BITBOARD hash_pos2;                     //esta es para guardarlo y recuperarlo al salir interrumpido por tiempo de Analiza()
extern BITBOARD tt_hits;                       //contador de hits de la tabla
extern int tt_size,mejor_jugada,tt_mejor;
extern BYTE tt_hit;
///////////////////////////////////////Variables de las HOJAS tt////////////////////////////////////////////////
extern int hojastt_size,evaltt_size;               //size de la tabla de nodos hojas y la de evaluaciones
extern BITBOARD hojastt_hits,evaltt_hits;          //contador de hits de la tabla de hojas y de evaluaciones


#endif // TT_H_INCLUDED
