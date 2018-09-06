#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include "definiciones.h"

void Init();
void LeeFEN();
void ReiniciaRegistros();
void EsperaJugada(char[4]);
BYTE Legal();								//se fija si la jugada es totalmente correcta en terminos legales (FALSE si es ilegal y TRUE si es legal)
void Jugar(BYTE,BYTE,BYTE);					//esta es la q HACE la jugada (no durante el analisis sino de posta)
void Analiza();								//busca la "mejor" jugada q se pueda (aca esta toda la magia y los errores jeje)

typedef enum							//la casilla A1 corresponde al indice 0, H1 es 7, A8 es 56 y H8 es 63 y A1 corresponde al LSB de los 64 y H8 es el MSB
{
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8,
  MALA_CASILLA							//MALA CASILLA se produce en varias situaciones en las q inicio o fin valen 64
}casillas;

typedef enum
{
	fila1, fila2, fila3, fila4, fila5, fila6, fila7, fila8
}filas;
typedef enum
{
	columnaA, columnaB, columnaC, columnaD, columnaE, columnaF, columnaG, columnaH
}columnas;
typedef enum
{
	VACIA = 0, PEON = 1, CABALLO = 2, ALFIL = 3, TORRE = 4, DAMA = 5, REY = 6
}piezas;
typedef enum
{
	blancas = 1, negras = 0
}color;
typedef enum
{
	ilegal = 0, normal = 1, corona = 2, enroque = 3, ap = 4, doble = 5, movrey = 6, movtorre = 7
}movidas;
typedef enum
{
	TABLAS = 1, BLANCASGANAN = 2, NEGRASGANAN = 3
}resultados;

typedef enum
{
    Peon_b = 0, Caballo_b = 1, Alfil_b = 2, Torre_b = 3, Dama_b = 4, Rey_b = 5,
    Peon_n = 6, Caballo_n = 7, Alfil_n = 8, Torre_n = 9, Dama_n = 10, Rey_n = 11,
}trebejos;

extern int segunda;
extern BYTE nueva,resultado;
extern BYTE turno,turno_c,colorpic,bien,semibien,inicio,fin,pieza;
extern BYTE prof_max,jugadas[MAXPROF][600],Qcapturas[40][80],Qcorona[40];
extern BYTE comio[MAXPROF],alpaso[MAXPROF],derechos_enroque[MAXPROF],vp[MAXPROF][3];
extern BYTE legales[MAXPROF],jugadas_reversibles,nulo,Qcomio[40];
extern BYTE salir,unica;
//BYTE ext;
extern char fen[85];                               //como maximo el string FEN puede tener 85 caracteres
extern char planilla[7][400];                      //permite anotar una partida de hasta 400 jugadas
extern unsigned int ply_count;
extern BYTE mcut;                                  //indica si se esta haciendo o no una busqueda multi cut

extern char a,b,c,d,e;								//para cuando el user corona ver q fue
extern int ponderacion[200],Qponderacion[40];		//para el ordenamiento de jugadas con el enfoque MVV/LVA
extern unsigned int puntero,ultimajugada[MAXPROF],Qpuntero,ultimacaptura[40],total_jugadas;
extern unsigned int pv[MAXPROF][MAXPROF];
extern int valoracion,val,estimo,total;
extern int killer[MAXPROF][2];
extern unsigned long long divide[600]; //para la funcion divide dentro del perft() q permite detectar errores en el generador de movs
extern unsigned long long saliointerrumpido,salionormal;	//estas son para ver estadisticas
extern unsigned long long nodos,Qnodos,nodosnulos,cortesnulo,llamadasevaluar,cortes_inut_inversa;
extern unsigned long long cantventanas,falloventana,eficiencia_1,eficiencia_2,eficiencia,qpasaaca;
extern float cant_analisis,proftotal,profmedia;					//igual q estas q muestran el nivel de juego promedio de la partida

extern unsigned long long freq,t_inicio,t_fin;    //para el manejo del tiempo
extern double timerFrequency, t_transcurrido;

extern BITBOARD peones_b,peones_n,caballos_b,caballos_n,alfiles_b,alfiles_n,torres_b,torres_n,damas_b,damas_n,rey_b,rey_n;
extern BITBOARD piezas_b,piezas_n;					//es la OR de todos los bitboards de tipos de pizas blancas o negras respectivamente
extern BITBOARD tablero;							//es la posicion de todas las piezas en el tablero (piezas_b OR piezas_n)
extern BITBOARD tablero90,tableroA1,tableroA8;     //son las bitboards rotadas q guardan las casillas ocupadas
extern BITBOARD tab90,tabA1,tabA8;                 //guardan los tableros rotados para recuperarlos luego
extern BITBOARD ataques_peones[2][64];				//hay q hacer la diferencia entre peones blancos y negros
extern BITBOARD mov_peones[2][64];					//los peones no atacan lo mismo q mueven por eso hay q tener este ademas de ataques
extern BITBOARD ataques_caballos[64];
extern BITBOARD ataques_alfiles[64];				//los ataques de dama se hacen combinando alfil y torre
extern BITBOARD ataques_torres[64];
extern BITBOARD ataques_rey[64];
extern BITBOARD atack_alfil,atack_torre;			//para generar mas rapido y en forma dinamica los movimientos deslizantes
extern BITBOARD clear_mask[65]; 					//permite poner a 0 el bit q se quiera
extern BITBOARD set_mask[65];						//permite poner a 1 el bit q se quiera
extern BITBOARD clear_mask90[64];                   //para las bitboards rotadas
extern BITBOARD set_mask90[64];
extern BITBOARD clear_maskA1[64];
extern BITBOARD set_maskA1[64];                     //se inicializa con una lista
extern BITBOARD clear_maskA8[64];
extern BITBOARD set_maskA8[64];                     //se inicializa con una lista
extern BITBOARD fila_mask[8];
extern BITBOARD columna_mask[8];
extern BITBOARD mask_efgh;
extern BITBOARD mask_fgh;
extern BITBOARD mask_abc;
extern BITBOARD mask_abcd;
extern BITBOARD mask_ah;
extern BITBOARD casillas_b;
extern BITBOARD casillas_n;
extern BITBOARD mas1dir[65];                       //direccion una casilla a la derecha
extern BITBOARD mas7dir[65];                       //diagonal hacia arriba izquierda
extern BITBOARD mas8dir[65];                       //vertical hacia arriba
extern BITBOARD mas9dir[65];                       //diagonal hacia arriba derecha
extern BITBOARD menos1dir[65];                     //hacia la izquierda
extern BITBOARD menos7dir[65];                     //diagonal abajo derecha
extern BITBOARD menos8dir[65];                     //vertical hacia abajo
extern BITBOARD menos9dir[65];                     //diagonal abajo izquierda
extern BITBOARD peones_conectados[64];
extern BITBOARD peon_aislado[64];
extern BITBOARD peon_pasado[2][64];
extern BITBOARD dospasos[2][8];
extern BITBOARD regladelcuadrado[2][2][64];
extern BITBOARD OO[2];
extern BITBOARD OOO[2];
extern BITBOARD ataques_filas[64][256],ataques_columnas[64][256],ataques_A1H8[64][256],ataques_A8H1[64][256];

extern unsigned int historia[2][64][64];   //para implementar ordenamiento por historia de la jugada [turno][inicio][fin]
extern BYTE RPr[2][64][64][64];            //turno a quien le toca, casilla del peon, del rey blanco y del negro (contiene todas las pos)
                                           //del final RP vs r con valores 0 si es tablas 1 si es victoria blanca

//estas son para guardar y recuperar el estado de la partida por si hay q salir prematuramente de negamax/////////////////////////////////////////////////////////
extern BYTE turno1,comio1,jugadas_reversibles1;																														//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//estas son para ir guardando las posiciones anteriores para detectar tablas por 3 repeticiones///////////////////////////////////////////////////////////////////
extern BITBOARD piezas_anteriores[MAXPROF][12];																																//
extern BYTE turnos_anteriores[MAXPROF],enroques_anteriores[MAXPROF],alpaso_anteriores[MAXPROF];																						//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int mate[64] =						//para facilitar los mates en general
{
  200, 180, 160, 140, 140, 160, 180, 200,
  180, 160, 140, 120, 120, 140, 160, 180,
  160, 140, 120, 100, 100, 120, 140, 160,
  140, 120, 100, 100, 100, 100, 120, 140,
  140, 120, 100, 100, 100, 100, 120, 140,
  160, 140, 120, 100, 100, 120, 140, 160,
  180, 160, 140, 120, 120, 140, 160, 180,
  200, 180, 160, 140, 140, 160, 180, 200
};
const int rey_final[64] =					//para la posicion del rey en los finales
{
  -20, -9, -8, -7, -7, -8, -9, -20,
   -9, -8, -7, -6, -6, -7, -8, -9,
   -8, -7, -6, -5, -5, -6, -7, -8,
   -7, -6, -5, -4, -4, -5, -6, -7,
   -7, -6, -5, -4, -4, -5, -6, -7,
   -8, -7, -6, -5, -5, -6, -7, -8,
   -9, -8, -7, -6, -6, -7, -8, -9,
  -20, -9, -8, -7, -7, -8, -9, -20
};
const int mate_ac_n[64] =					//para facilitar el mate de alfil de casillas negras y caballo
{
  99, 90, 80, 70, 60, 50, 40, 30,
  90, 80, 70, 60, 50, 40, 30, 40,
  80, 70, 60, 50, 40, 30, 40, 50,
  70, 60, 50, 40, 30, 40, 50, 60,
  60, 50, 40, 30, 40, 50, 60, 70,
  50, 40, 30, 40, 50, 60, 70, 80,
  40, 30, 40, 50, 60, 70, 80, 90,
  30, 40, 50, 60, 70, 80, 90, 99
};
const int mate_ac_b[64] =					//para facilitar el mate de alfil de casillas blancas y caballo
{
  30, 40, 50, 60, 70, 80, 90, 99,
  40, 30, 40, 50, 60, 70, 80, 90,
  50, 40, 30, 40, 50, 60, 70, 80,
  60, 50, 40, 30, 40, 50, 60, 70,
  70, 60, 50, 40, 30, 40, 50, 60,
  80, 70, 60, 50, 40, 30, 40, 50,
  90, 80, 70, 60, 50, 40, 30, 40,
  99, 90, 80, 70, 60, 50, 40, 30
};
const int caballo_outpost[2][64] =
{
  { 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 4, 4, 4, 4, 1, 0,
    0, 2, 6, 8, 8, 6, 2, 0,
    0, 1, 4, 4, 4, 4, 1, 0,   // [negro][64]
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0 },

  { 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 4, 4, 4, 4, 1, 0,
    0, 2, 6, 8, 8, 6, 2, 0,   // [blanco][64]
    0, 1, 4, 4, 4, 4, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0 }
};
const int alfil_outpost[2][64] =
{
  { 0, 0, 0, 0, 0, 0, 0, 0,
   -1, 0, 0, 0, 0, 0, 0,-1,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 3, 3, 3, 3, 1, 0,
    0, 3, 5, 5, 5, 5, 3, 0,   // [negro][64]
    0, 1, 2, 2, 2, 2, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0 },

  { 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 2, 2, 2, 1, 0,
    0, 3, 5, 5, 5, 5, 3, 0,
    0, 1, 3, 3, 3, 3, 1, 0,   // [blanco][64]
    0, 0, 1, 1, 1, 1, 0, 0,
   -1, 0, 0, 0, 0, 0, 0,-1,
    0, 0, 0, 0, 0, 0, 0, 0 }
};
const int peon_pos[2][64] =
{
    {966, 966, 966, 966, 966, 966, 966, 966,
      66,  66,  66,  66,  66,  66,  66,  66,
      10,  10,  10,  30,  30,  10,  10,  10,
       6,   6,   6,  16,  16,   6,   6,   6,
       3,   4,   5,  13,  13,   5,   4,   3,   /* [negras][64] */
       1,   2,   3,  10,  10,   3,   2,   1,
       0,   1,   2, -12, -12,   2,   1,   0,
       0,   0,   0,   0,   0,   0,   0,   0 },

    {  0,   0,   0,   0,   0,   0,   0,   0,
       0,   1,   2, -12, -12,   2,   1,   0,
       1,   2,   3,  10,  10,   3,   2,   1,
       3,   4,   5,  13,  13,   5,   4,   3,
       6,   6,   6,  16,  16,   6,   6,   6,   /* [blancas][64] */
      10,  10,  10,  30,  30,  10,  10,  10,
      66,  66,  66,  66,  66,  66,  66,  66,
     966, 966, 966, 966, 966, 966, 966, 966 }
};
const int caballo_pos[2][64] =
{
   {-29, -19, -19,  -9,  -9, -19, -19, -29,
      1,  12,  18,  22,  22,  18,  12,   1,
      1,  14,  23,  27,  27,  23,  14,   1,
      1,  14,  23,  28,  28,  23,  14,   1,
      1,  12,  21,  24,  24,  21,  12,   1,  /* [negras][64] */
      1,   2,  19,  17,  17,  19,   2,   1,
      1,   2,   2,   2,   2,   2,   2,   1,
    -19, -19, -19, -19, -19, -19, -19, -19 },

   {-19, -19, -19, -19, -19, -19, -19, -19,
      1,   2,   2,   2,   2,   2,   2,   1,
      1,   2,  19,  17,  17,  19,   2,   1,
      1,  12,  21,  24,  24,  21,  12,   1,
      1,  14,  23,  28,  28,  23,  14,   1,  /* [blancas][64] */
      1,  14,  23,  27,  27,  23,  14,   1,
      1,  12,  18,  22,  22,  18,  12,   1,
    -29, -19, -19,  -9,  -9, -19, -19, -29 }
};
const int alfil_pos[2][64] =
{
   {  0,   0,   2,   4,   4,   2,   0,   0,
      0,   8,   6,   8,   8,   6,   8,   0,
      2,   6,  12,  10,  10,  12,   6,   2,
      4,   8,  10,  16,  16,  10,   8,   4,
      4,   8,  10,  16,  16,  10,   8,   4,   /* [negras][64] */
      2,   6,  12,  10,  10,  12,   6,   2,
      0,   8,   6,   8,   8,   6,   8,   0,
    -10, -10,  -8,  -6,  -6,  -8, -10, -10 },

   {-10, -10,  -8,  -6,  -6,  -8, -10, -10,
      0,   8,   6,   8,   8,   6,   8,   0,
      2,   6,  12,  10,  10,  12,   6,   2,
      4,   8,  10,  16,  16,  10,   8,   4,
      4,   8,  10,  16,  16,  10,   8,   4,   /* [blancas][64] */
      2,   6,  12,  10,  10,  12,   6,   2,
      0,   8,   6,   8,   8,   6,   8,   0,
      0,   0,   2,   4,   4,   2,   0,   0 }
};
const int torre_pos[2][64] =
{
  	{ 0,   0,   0,   0,   0,   0,   0,   0,
  	 10,  10,  10,  10,  10,  10,  10,  10,
 	 -5,   0,   0,   0,   0,   0,   0,  -5,
 	 -5,   0,   0,   0,   0,   0,   0,  -5,		//negras
 	 -5,   0,   0,   0,   0,   0,   0,  -5,
 	 -5,   0,   0,   0,   0,   0,   0,  -5,
 	 -5,   0,   0,   0,   0,   0,   0,  -5,
 	  0,   0,   0,   5,   5,   0,   0,   0 },

  	{ 0,   0,   0,   5,   5,   0,   0,   0,
  	 -5,   0,   0,   0,   0,   0,   0,  -5,
 	 -5,   0,   0,   0,   0,   0,   0,  -5,
 	 -5,   0,   0,   0,   0,   0,   0,  -5,		//blancas
 	 -5,   0,   0,   0,   0,   0,   0,  -5,
 	 -5,   0,   0,   0,   0,   0,   0,  -5,
 	 10,  10,  10,  10,  10,  10,  10,  10,
 	  0,   0,   0,   0,   0,   0,   0,   0 },

};
const int dama_pos[64] =
{
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   4,   4,   4,   4,   0,   0,
      0,   4,   4,   6,   6,   4,   4,   0,
      0,   4,   6,   8,   8,   6,   4,   0,
      0,   4,   6,   8,   8,   6,   4,   0,   /* blancas y negras[64] (es simetrico) */
      0,   4,   4,   6,   6,   4,   4,   0,
      0,   0,   4,   4,   4,   4,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0
};
const int rey_pos[2][64] =
{
   {-30, -40, -40, -50, -50, -40, -40, -30,
	-30, -40, -40, -50, -50, -40, -40, -30,
	-30, -40, -40, -50, -50, -40, -40, -30,
	-30, -40, -40, -50, -50, -40, -40, -30,		//negras
	-20, -30, -30, -40, -40, -30, -30, -20,
	-10, -20, -20, -20, -20, -20, -20, -10,
	 20,  20,   0,   0,   0,   0,  20,  20,
	 20,  30,  10,   0,   0,  10,  30,  20 },

    {20,  30,  10,   0,   0,  10,  30,  20,
	 20,  20,   0,   0,   0,   0,  20,  20,
	-10, -20, -20, -20, -20, -20, -20, -10,
	-20, -30, -30, -40, -40, -30, -30, -20,		//blancas
	-30, -40, -40, -50, -50, -40, -40, -30,
	-30, -40, -40, -50, -50, -40, -40, -30,
	-30, -40, -40, -50, -50, -40, -40, -30,
	-30, -40, -40, -50, -50, -40, -40, -30,	}
};

#endif // GLOBAL_H_INCLUDED
