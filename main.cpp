#include <iostream>
#include <string>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <math.h>

#include "definiciones.h"
#include "global.h"
#include "bitboard.h"
#include "tt.h"
#include "movgen.h"
#include "search.h"

using namespace std;

    /////////////////////////////////////////////////////////////////////////////
    //                            FUNCIONES LOCALES
    /////////////////////////////////////////////////////////////////////////////

void Uci();
void IniciaVar();
void InicializaRegistros();		            //inicia los registros a su estado por defecto
void ReiniciaRegistros();		            //antes de cada nueva partida hay q reiniciar varios registros
void LeeFEN(const std::string);				//prepara todos los registros necesarios para q comience la partida (nueva o con introduccion de FEN)
void EsperaJugada(char[4]);					//retorna cuando el user jugo algo y obtiene la jugada del user traducida al formato q entiende el programa
void InicializaMasks();						//inicializa las mascaras de columnas, filas set y clear
void IniciaBitboards();						//inicializa las bitboards ataques_peon,ataques_caballo,alpaso,etc
void IniciaRPr();                           //genera la tabla q contiene los valores para todas las posiciones del final RP vs r
void QuePieza();							//pone en el reg pieza el valor correspondiente a la pieza seleccioneda por el user

int Perft(BYTE,BYTE);						//para chequear el generador de movimientos

void Jugar(BYTE,BYTE,BYTE);					//esta es la q HACE la jugada (no durante el analisis sino de posta)

///////////////////Variables globales/////////////////////////////////

unsigned int prof_max, prof_max2;
BYTE turno,turno_c,bien,semibien,inicio,fin,pieza;
BYTE jugadas[MAXPROF][600],Qcapturas[40][80],Qcorona[40];
BYTE comio[MAXPROF],alpaso[MAXPROF],derechos_enroque[MAXPROF],vp[MAXPROF][3];
BYTE legales[MAXPROF],jugadas_reversibles,nulo,Qcomio[40];
BYTE salir,unica;
//BYTE ext;
char fen[85];                               //como maximo el string FEN puede tener 85 caracteres
BYTE mcut;                                  //indica si se esta haciendo o no una busqueda multi cut

char a,b,c,d,e;								//para cuando el user corona ver q fue
int ponderacion[200],Qponderacion[40];		//para el ordenamiento de jugadas con el enfoque MVV/LVA
unsigned int puntero,ultimajugada[MAXPROF],Qpuntero,ultimacaptura[40],total_jugadas;
unsigned int pv[MAXPROF][MAXPROF];
int valoracion,val,estimo,total;
int killer[MAXPROF][2];
unsigned long long divide[600]; //para la funcion divide dentro del perft() q permite detectar errores en el generador de movs
unsigned long long nodos,Qnodos,nodosnulos,cortesnulo,llamadasevaluar,cortes_inut_inversa;
unsigned long long cantventanas,falloventana,eficiencia_1,eficiencia_2,eficiencia;
float cant_analisis,proftotal,profmedia;					//igual q estas q muestran el nivel de juego promedio de la partida

unsigned long long freq,t_inicio,t_fin;    //para el manejo del tiempo
double timerFrequency, t_transcurrido;

BITBOARD peones_b,peones_n,caballos_b,caballos_n,alfiles_b,alfiles_n,torres_b,torres_n,damas_b,damas_n,rey_b,rey_n;
BITBOARD piezas_b,piezas_n;					//es la OR de todos los bitboards de tipos de pizas blancas o negras respectivamente
BITBOARD tablero;							//es la posicion de todas las piezas en el tablero (piezas_b OR piezas_n)
BITBOARD tablero90,tableroA1,tableroA8;     //son las bitboards rotadas q guardan las casillas ocupadas
BITBOARD tab90,tabA1,tabA8;                 //guardan los tableros rotados para recuperarlos luego
BITBOARD ataques_peones[2][64];				//hay q hacer la diferencia entre peones blancos y negros
BITBOARD mov_peones[2][64];					//los peones no atacan lo mismo q mueven por eso hay q tener este ademas de ataques
BITBOARD ataques_caballos[64];
BITBOARD ataques_alfiles[64];				//los ataques de dama se hacen combinando alfil y torre
BITBOARD ataques_torres[64];
BITBOARD ataques_rey[64];
BITBOARD atack_alfil,atack_torre;			//para generar mas rapido y en forma dinamica los movimientos deslizantes
BITBOARD clear_mask[65]; 					//permite poner a 0 el bit q se quiera
BITBOARD set_mask[65];						//permite poner a 1 el bit q se quiera
BITBOARD clear_mask90[64];                  //para las bitboards rotadas
BITBOARD set_mask90[64];
BITBOARD clear_maskA1[64];
BITBOARD set_maskA1[64] =					//soy un manco y no pude escribir una funcion para representar esta serie
{                                           //por eso lo inicializo asi y no en InicializaMasks()
    0x10000000,0x200000,0x8000,0x400,0x40,0x8,0x2,0x1,
    0x1000000000,0x20000000,0x400000,0x10000,0x800,0x80,0x10,0x4,
    0x80000000000,0x2000000000,0x40000000,0x800000,0x20000,0x1000,0x100,0x20,
    0x2000000000000,0x100000000000,0x4000000000,0x80000000,0x1000000,0x40000,0x2000,0x200,
    0x40000000000000,0x4000000000000,0x200000000000,0x8000000000,0x100000000,0x2000000,0x80000,0x4000,
    0x400000000000000,0x80000000000000,0x8000000000000,0x400000000000,0x10000000000,0x200000000,0x4000000,0x100000,
    0x2000000000000000,0x800000000000000,0x100000000000000,0x10000000000000,0x800000000000,0x20000000000,0x400000000,0x8000000,
    0x8000000000000000,0x4000000000000000,0x1000000000000000,0x200000000000000,0x20000000000000,0x1000000000000,0x40000000000,0x800000000
};
BITBOARD clear_maskA8[64];
BITBOARD set_maskA8[64] =
{
    0x1,0x4,0x20,0x200,0x4000,0x100000,0x8000000,0x800000000,
    0x2,0x10,0x100,0x2000,0x80000,0x4000000,0x400000000,0x40000000000,
    0x8,0x80,0x1000,0x40000,0x2000000,0x200000000,0x20000000000,0x1000000000000,
    0x40,0x800,0x20000,0x1000000,0x100000000,0x10000000000,0x800000000000,0x20000000000000,
    0x400,0x10000,0x800000,0x80000000,0x8000000000,0x400000000000,0x10000000000000,0x200000000000000,
    0x8000,0x400000,0x40000000,0x4000000000,0x200000000000,0x8000000000000,0x100000000000000,0x1000000000000000,
    0x200000,0x20000000,0x2000000000,0x100000000000,0x4000000000000,0x80000000000000,0x800000000000000,0x4000000000000000,
    0x10000000,0x1000000000,0x80000000000,0x2000000000000,0x40000000000000,0x400000000000000,0x2000000000000000,0x8000000000000000
};
BITBOARD fila_mask[8];
BITBOARD columna_mask[8];
BITBOARD mask_efgh;
BITBOARD mask_fgh;
BITBOARD mask_abc;
BITBOARD mask_abcd;
BITBOARD mask_ah;
BITBOARD casillas_b;
BITBOARD casillas_n;
BITBOARD mas1dir[65];                       //direccion una casilla a la derecha
BITBOARD mas7dir[65];                       //diagonal hacia arriba izquierda
BITBOARD mas8dir[65];                       //vertical hacia arriba
BITBOARD mas9dir[65];                       //diagonal hacia arriba derecha
BITBOARD menos1dir[65];                     //hacia la izquierda
BITBOARD menos7dir[65];                     //diagonal abajo derecha
BITBOARD menos8dir[65];                     //vertical hacia abajo
BITBOARD menos9dir[65];                     //diagonal abajo izquierda
BITBOARD peones_conectados[64];
BITBOARD peon_aislado[64];
BITBOARD peon_pasado[2][64];
BITBOARD dospasos[2][8];
BITBOARD regladelcuadrado[2][2][64];
BITBOARD OO[2];
BITBOARD OOO[2];
BITBOARD ataques_filas[64][256],ataques_columnas[64][256],ataques_A1H8[64][256],ataques_A8H1[64][256];

unsigned int historia[2][64][64];   	//para implementar ordenamiento por historia de la jugada [turno][inicio][fin]
BYTE RPr[2][64][64][64];            	//turno a quien le toca, casilla del peon, del rey blanco y del negro (contiene todas las pos)
                                        //del final RP vs r con valores 0 si es tablas 1 si es victoria blanca

//estas son para guardar y recuperar el estado de la partida por si hay q salir prematuramente de negamax/////////////////////////////////////////////////////////
BYTE turno1,comio1,jugadas_reversibles1;																														//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//estas son para ir guardando las posiciones anteriores para detectar tablas por 3 repeticiones///////////////////////////////////////////////////////////////////
BITBOARD piezas_anteriores[MAXPROF][12];																																//
BYTE turnos_anteriores[MAXPROF],enroques_anteriores[MAXPROF],alpaso_anteriores[MAXPROF];																						//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//                                  MAIN								   //
/////////////////////////////////////////////////////////////////////////////

int main(void)
{
    QueryPerformanceCounter((LARGE_INTEGER *)&t_inicio);
    Uci();
/*
	while (1)
	{
		do						//aca va a estar todo el programa y se va a volver cada vez que se complete un movimiento satisfactorio del humano o del pic
		{
			if (turno == 0)//HUMANO)	//entra si le toca jugar al jugador humano
			{
				do					//punto de regreso al detectarse un error dado por la funcion legal() en la jugada enviada por el usuario
				{
//					EsperaJugada();		//retorna cuando el user jugo algo y obtiene la jugada del user traducida al formato q entiende el programa
					bien = Legal();		//se fija si la jugada es totalmente correcta en terminos legales (FALSE si es ilegal y TRUE si es legal)
				}while(bien == 0);		//mientras legal retorne falso (osea movimiento ilegal) o no caiga la aguja no salgo
			}//fin del turno del humano
			else							//le toca el turno al PIC
			{
				Analiza();			    	//busca la "mejor" jugada q se pueda (aca esta toda la magia y los errores jeje)
                wcout << a << b << c << d << "   " << valoracion << endl;    //escribe la jugada del pic y la valoracion
                if (salir)
                    wcout << "profundidad " << prof_max - 1 << endl;
                else
                    wcout << "profundidad " << prof_max << endl;
			}//fin del turno del pic
			//a partir de aca la jugada se hace si o si
			Jugar(inicio,fin,bien);						//actualiza los regs necesarios para q la jugada se realice (muy similar a HacerJugada pero no igual)
            Anotar();                                   //para que registre la partida asi la puedo sacar como archivo de texto
			resultado = versitermino();					//si la partida termino por algun motivo (el q sea) devuelve TRUE y sino FALSE y se sigue jugando
		}while (resultado == 0);						//mientras no se de una condicion q termine la partida se sigue jugando
		//aca la partida termino
		QueryPerformanceCounter((LARGE_INTEGER *)&t_fin);
		t_transcurrido = ((t_fin - t_inicio) * timerFrequency);
		wcout << t_transcurrido << endl;
		ExportarPartida();                              //que saque la partida con el formato del fritz
	};//end while de todo el programa (del q no se sale nunca)
*/
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////FIN DEL MAIN/////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void Init()
{
    IniciaVar();
    InicializaAleatorio();      //deja lista la funcion de generacion de numeros aleatorios necesarias para
    InicializaZobrist();        //llenar las claves Zobrist
    Setsize_tt(1048576);        //creo que es 1 MB
    Setsize_hojastt(1048576);
    Setsize_evaltt(1048576);
//    Setsize_perfttt(1048576);
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);   //toma la performance de la compu para tener de base en el reloj
    timerFrequency = (1.0/freq);                        //saca la frecuencia de la cpu como base para el reloj

	InicializaMasks();			//inicializa las mascaras usadas para trabajar con bits a lo largo de todo el programa
	IniciaBitboards();
	IniciaRPr();
	InicializaRegistros();		//en cada reinicio del programa hay q iniciar varias variables para q todo ande bien
}

void IniciaVar()
{
    nodos = 0;
    Qnodos = 0;
    nodosnulos = 0;
    cortesnulo = 0;
    llamadasevaluar = 0;
    cortes_inut_inversa = 0;

    mask_efgh = 0xf0f0f0f0f0f0f0f0;
    mask_fgh = 0xe0e0e0e0e0e0e0e0;
    mask_abc = 0x0707070707070707;
    mask_abcd = 0x0f0f0f0f0f0f0f0f;
    mask_ah = 0x8181818181818181;
    casillas_b = 0x55aa55aa55aa55aa;
    casillas_n = 0xaa55aa55aa55aa55;

    OO[0] = 0x6000000000000000;            //extern BITBOARD OO[2] = {0x6000000000000000, 0x0000000000000060};
    OO[1] = 0x0000000000000060;
    OOO[0] = 0x0E00000000000000;           //extern BITBOARD OOO[2] = {0x0E00000000000000, 0x000000000000000E};
    OOO[1] = 0x000000000000000E;

    mejortt = 0;
}

void InicializaRegistros()						//inicializa todos los registros a su valor por defecto
{
	int i,j;

	//los reg de las 3 repeticiones
	for (i=0;i<MAXPROF;i++)
	{
		for (j=0;j<12;j++)
		{
			piezas_anteriores[i][j] = 0;		//borro todo el historial de posiciones de las piezas
		}
		turnos_anteriores[i] = 2;				//esto es borrar para turno ya q solo puede valer 0 o 1 nunca va a coincidir
		enroques_anteriores[i] = 20;			//derechos_enroques solo llega hasta 0x0f o 15 en decimal
		alpaso_anteriores[i] = 9;				//alpaso solo va de 0 a 8 (8 vale cuando no existe peon al paso) por lo q 9 nunca va a coincidir
	}
}

void ReiniciaRegistros()
{
	int i,j,k;

	total_jugadas = 0;							//van 0 jugadas
	jugadas_reversibles = 0;
    turno = PIC;
    for (i=0; i<2; i++)     //por ultimo reinicio los registros de la historia de la partida
    {
        for (j=0; j<64; j++)
        {
            for (k=0; k<64; k++)
            {
                historia[i][j][k] = 0;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////MOTOR DE AJEDREZ//////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InicializaMasks()		//inicializa las mascaras de columnas, filas set y clear
{
	int i;
/*
 mascaras para setear/borrar un bit en una casilla especifica
*/
	for (i = 0; i < 64; i++)
	{
		ClearMask(i) = ~((BITBOARD) 1 << i);
		SetMask(i) = (BITBOARD) 1 << i;
		SetMask90(i) = (BITBOARD) 1 << (((i%8)*8)+(7-i/8));
		ClearMask90(i) = ~ SetMask90(i);
		ClearMaskA1(i) = ~ SetMaskA1(i);
		ClearMaskA8(i) = ~ SetMaskA8(i);
	}
	ClearMask(MALA_CASILLA) = 0;	//para q siempre sea 0 la casilla 64 de cualquier bitboard
	SetMask(MALA_CASILLA) = 0;
/*
mascaras de filas y columnas
*/
	fila_mask[0] = (BITBOARD) 255;
	for (i = 1; i < 8; i++)
	fila_mask[i] = fila_mask[i - 1] << 8;
	columna_mask[columnaA] = (BITBOARD) 1;
	for (i = 1; i < 8; i++)
	columna_mask[columnaA] = columna_mask[columnaA] | columna_mask[columnaA] << 8;
	for (i = 1; i < 8; i++)
	columna_mask[i] = columna_mask[i - 1] << 1;
}

void IniciaBitboards()	//inicializa las bitboards ataques_peon,ataques_caballo,etc (todas las que son fijas durante todo el programa)
{
	int i, j, fcol, ffila, tcol, tfila, coronacion;
	int sq, lastsq;
	static const int caballosq[8] = { -17, -15, -10, -6, 6, 10, 15, 17 };
	static const int alfilsq[4] = { -9, -7, 7, 9 };
	static const int torresq[4] = { -8, -1, 1, 8 };

	for (i=0;i<64;i++)		//primero los limpio a todos
	{
		ataques_peones[blancas][i] = 0;
		ataques_peones[negras][i] = 0;
		mov_peones[blancas][i] = 0;
		mov_peones[negras][i] = 0;
		ataques_caballos[i] = 0;
		ataques_alfiles[i] = 0;
		ataques_torres[i] = 0;
		ataques_rey[i] = 0;
		for (j=0;j<256;j++)
        {
            ataques_filas[i][j] = 0;
            ataques_columnas[i][j] = 0;
            ataques_A1H8[i][j] = 0;
            ataques_A8H1[i][j] = 0;
        }
	}
/*
inicializa bitboard ataques de peon
*/
	for (i = 0; i < 64; i++)
	{
		if (i < 56)
			for (j = 2; j < 4; j++)
			{
				sq = i + alfilsq[j];
				if ((Abs(Columna(sq) - Columna(i)) == 1) && (Abs(Fila(sq) - Fila(i)) == 1) && (sq < 64) && (sq > -1))
					ataques_peones[blancas][i] = ataques_peones[blancas][i] | (BITBOARD) 1 << sq;
			}
		if (i > 7)
			for (j = 0; j < 2; j++)
			{
				sq = i + alfilsq[j];
				if ((Abs(Columna(sq) - Columna(i)) == 1) && (Abs(Fila(sq) - Fila(i)) == 1) && (sq < 64) && (sq > -1))
					ataques_peones[negras][i] = ataques_peones[negras][i] | (BITBOARD) 1 << sq;
			}
	}
/*
inicializa bitboard de movimientos de peon (agrega movimiento hacia adelante de uno y 2 pasos q no son ataques)
*/
	for (i = 0; i < 64; i++)
	{
		mov_peones[blancas][i] = ataques_peones[blancas][i];		//esto pone las capturas en diagonal tal como en ataques_peones
		mov_peones[negras][i] = ataques_peones[negras][i];
	}
	for (i = 8; i < 56; i++)
	{
		if (i < 16)
		{
			mov_peones[blancas][i] |= SetMask(i+8);		//si estan en la fila 2 pueden de a dos pasos y de a uno
			mov_peones[blancas][i] |= SetMask(i+16);
		}
		else
		{
			mov_peones[blancas][i] |= SetMask(i+8);		//si estan en cualquier otra solo pueden de un paso
		}
	}
	for (i = 8; i < 56; i++)
	{
		if (i > 47)
		{
			mov_peones[negras][i] |= SetMask(i-8);		//si estan en la fila 7 pueden de a dos pasos y de a uno
			mov_peones[negras][i] |= SetMask(i-16);
		}
		else
		{
			mov_peones[negras][i] |= SetMask(i-8);		//si estan en cualquier otra solo pueden de un paso
		}
	}
/*
inicializa bitboard ataques de caballo
*/
	for (i = 0; i < 64; i++)
	{
		fcol = Columna(i);
		ffila = Fila(i);
		for (j = 0; j < 8; j++)
		{
			sq = i + caballosq[j];
			if ((sq < 0) || (sq > 63))		//si se sale del tablero
			continue;						//sigo con la proxima
			tcol = Columna(sq);
			tfila = Fila(sq);
			if ((Abs(fcol - tcol) > 2) || (Abs(ffila - tfila) > 2))			//si las casillas distan mas de 2 columnas o mas de 2 filas no es mov de caballo
			continue;														//entonces q siga con la otra
			ataques_caballos[i] = ataques_caballos[i] | (BITBOARD) 1 << sq;	//si esta atacada la casilla entonces se pone un 1 en el bit sq de ataques_caballos[i]
		}
	}
	mas1dir[64] = 0;	//para q las casillas malas sean siempre 0 y no produzcan errores
	mas7dir[64] = 0;
	mas8dir[64] = 0;
	mas9dir[64] = 0;
	menos1dir[64] = 0;
	menos7dir[64] = 0;
	menos8dir[64] = 0;
	menos9dir[64] = 0;
/*
inicializa bitboard de ataques por diagonales (alfiles y damas)
*/
	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 4; j++)
		{
			sq = i;
			lastsq = sq;
			sq = sq + alfilsq[j];
			while ((Abs(Columna(sq) - Columna(lastsq)) == 1) && (Abs(Fila(sq) - Fila(lastsq)) == 1) && (sq < 64) && (sq > -1))
			{
				if (alfilsq[j] == 7)
					mas7dir[i] = mas7dir[i] | (BITBOARD) 1 << sq;
				else if (alfilsq[j] == 9)
					mas9dir[i] = mas9dir[i] | (BITBOARD) 1 << sq;
				else if (alfilsq[j] == -7)
					menos7dir[i] = menos7dir[i] | (BITBOARD) 1 << sq;
				else
					menos9dir[i] = menos9dir[i] | (BITBOARD) 1 << sq;
				lastsq = sq;
				sq = sq + alfilsq[j];
			}
		}
	}
/*
inicializa bitboard de ataques por columnas y filas (torres y damas)
*/
	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 4; j++)
		{
			sq = i;
			lastsq = sq;
			sq = sq + torresq[j];
			while ((((Abs(Columna(sq) - Columna(lastsq)) == 1) && (Abs(Fila(sq) - Fila(lastsq)) == 0)) || ((Abs(Columna(sq) - Columna(lastsq)) == 0) && (Abs(Fila(sq) - Fila(lastsq)) == 1))) && (sq < 64) && (sq > -1))
			{
				if (torresq[j] == 1)
					mas1dir[i] = mas1dir[i] | (BITBOARD) 1 << sq;
				else if (torresq[j] == 8)
					mas8dir[i] = mas8dir[i] | (BITBOARD) 1 << sq;
				else if (torresq[j] == -1)
					menos1dir[i] = menos1dir[i] | (BITBOARD) 1 << sq;
				else
					menos8dir[i] = menos8dir[i] | (BITBOARD) 1 << sq;
				lastsq = sq;
				sq = sq + torresq[j];
			}
		}
	}
/*
inicializa bitboard ataques de alfil
*/
	for (i = 0; i < 64; i++)
	{
		ataques_alfiles[i] = mas9dir[i] | menos9dir[i] | mas7dir[i] | menos7dir[i];
	}
/*
inicializa bitboard ataques de torre
*/
	for (i = 0; i < 64; i++)
	{
		ataques_torres[i] = mas1dir[i] | menos1dir[i] | mas8dir[i] | menos8dir[i];
	}
/*
inicializa bitboard ataques de rey
*/
	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 64; j++)
		{
			if (Distancia(i,j) == 1)
			ataques_rey[i] = ataques_rey[i] | SetMask(j);
		}
	}
/*
inicia la mascara para detectar si un peon esta protegido por otro o no
tambien permite detectar peones "colgantes" con un poquito de trabajo
*/
	for (i = 8; i < 56; i++)
	{
		if (Columna(i) > 0 && Columna(i) < 7)
			peones_conectados[i] = SetMask(i - 1) | SetMask(i + 1) | SetMask(i - 9) | SetMask(i - 7) | SetMask(i + 7) | SetMask(i + 9);	//las 6 casillas adyacentes a la considerada
		if (Columna(i) == columnaA)
			peones_conectados[i] = SetMask(i + 1) | SetMask(i - 7) | SetMask(i + 9);	//como esta en la columna A solo las 3 casillas de la derecha pueden ser tenidas en cuenta
		if (Columna(i) == columnaH)
			peones_conectados[i] = SetMask(i - 1) | SetMask(i - 9) | SetMask(i + 7);
	}
/*
mascara para detectar peones aislados, tienen 1's
en las columnas adyacentes a la q estoy considerando
*/
	for (i = 0; i < 64; i++)
	{
		if (Columna(i) > 0 && Columna(i) < 7)												//si es una columna interior
			peon_aislado[i] = columna_mask[Columna(i) - 1] | columna_mask[Columna(i) + 1];	//pone a 1 las columnas vecinas (aca son dos xq no esta en el borde del tablero)
		if (Columna(i) == columnaA)															//si es la columna A el peon esta aislado si no tiene un peon en la columna B
			peon_aislado[i] = columna_mask[Columna(i) + 1];									//esto pone a 1 las casillas B1,B2,B3, etc
		if (Columna(i) == columnaH)
			peon_aislado[i] = columna_mask[Columna(i) - 1];									//pone a uno toda la columna G
	}
/*
mascara para detectar peones pasados. Tiene 1's en la
columna del peon y en las adyacentes, pero solo en las
casillas que estan delante del peon
*/
	for (i = 0; i < 64; i++)
	{
		if (Columna(i) == columnaA)
		{
			peon_pasado[blancas][i] = mas8dir[i] | mas8dir[i + 1];
			peon_pasado[negras][i] = menos8dir[i] | menos8dir[i + 1];
		}
		if (Columna(i) == columnaH)
		{
			peon_pasado[blancas][i] = mas8dir[i - 1] | mas8dir[i];
			peon_pasado[negras][i] = menos8dir[i - 1] | menos8dir[i];
		}
		if (Columna(i) > 0 && Columna(i) < 7)
		{
			peon_pasado[blancas][i] = mas8dir[i - 1] | mas8dir[i] | mas8dir[i + 1];
			peon_pasado[negras][i] = menos8dir[i - 1] | menos8dir[i] | menos8dir[i + 1];
		}
	}
/*
mascara para detectar errores en movimientos
de peones de a dos pasos
*/
	for (i = 0; i < 8; i++)
	{
		dospasos[blancas][i] = SetMask(16+i) | SetMask(24+i);
		dospasos[negras][i] = SetMask(40+i) | SetMask(32+i);
	}
/*
mascaras para detectar peones q no pueden ser
alcanzados por el rey adversario
(util en finales casi exclusivamente)
[blancas][blancas] indica q el rey blanco persigue al peon negro, y es el turno del blanco para jugar (al momento de analizar la hoja)
[blancas][negras] es el rey blanco persiguiendo al peon negro siendo el turno del negro para jugar
[negras][blancas] rey negro persiguiendo peon blanco tocandole el turno al blanco y por ultimo
[negras][negras] rey negro persiguiendo peon blanco tocandole el turno al negro
*/
	for (i=8;i<56;i++)
	{
		coronacion = 56 + Columna(i);		//coronacion es la casilla en la q el peon blanco en la casilla "i" coronaria al avanzar todo recto
		if (Fila(i) == fila2)				//caso especial xq aca el peon blanco puede (y asi se considera) avanzar doble
		{
			for (j=0;j<64;j++)				//j recorre todo el tablero para cada BITBOARD seteando las casillas q corresponde setear
			{
				if (Distancia(i,coronacion) > Distancia(j,coronacion))
					regladelcuadrado[negras][blancas][i] |= SetMask(j);
				if (Distancia(i,coronacion) > (Distancia(j,coronacion)-1))	//como aca les toca jugar a las negras se puede permitir una casilla mas de distancia
					regladelcuadrado[negras][negras][i] |= SetMask(j);
			}
		}
		else
		{
			for (j=0;j<64;j++)
			{
				if (Distancia(i,coronacion) >= Distancia(j,coronacion))
					regladelcuadrado[negras][blancas][i] |= SetMask(j);
				if (Distancia(i,coronacion) >= (Distancia(j,coronacion)-1))
					regladelcuadrado[negras][negras][i] |= SetMask(j);
			}
		}
	}
	for (i=8;i<56;i++)						//similar para el rey blanco intentando alcanzar peones negros
	{
		coronacion = Columna(i);			//coronacion es la casilla en la q el peon negro en la casilla "i" coronaria al avanzar todo recto
		if (Fila(i) == fila7)
		{
			for (j=0;j<64;j++)
			{
				if (Distancia(i,coronacion) > Distancia(j,coronacion))
					regladelcuadrado[blancas][negras][i] |= SetMask(j);
				if (Distancia(i,coronacion) > (Distancia(j,coronacion)-1))
					regladelcuadrado[blancas][blancas][i] |= SetMask(j);
			}
		}
		else
		{
			for (j=0;j<64;j++)
			{
				if (Distancia(i,coronacion) >= Distancia(j,coronacion))
					regladelcuadrado[blancas][negras][i] |= SetMask(j);
				if (Distancia(i,coronacion) >= (Distancia(j,coronacion)-1))
					regladelcuadrado[blancas][blancas][i] |= SetMask(j);
			}
		}
	}
	//mascaras para generar movimientos de piezas deslizantes en la forma de bitboards rotadas
	BITBOARD bloqueos,k,aux;
	BYTE casilla_bloqueada;

	for (i=0;i<64;i++)
    {
        if (i<8)    //genero las mascaras para las casillas A1,B1,....,H1
        {
            for (k=0;k<256;k++)     //para cada casilla genero los 256 posibles estados de la fila
            {
                bloqueos = mas1dir[i] & k;
                casilla_bloqueada = LSB(bloqueos);
                ataques_filas[i][k] = mas1dir[i] ^ mas1dir[casilla_bloqueada];  //precomputo los ataques por la fila
                bloqueos = menos1dir[i] & k;
                casilla_bloqueada = MSB(bloqueos);
                ataques_filas[i][k] |= menos1dir[i] ^ menos1dir[casilla_bloqueada]; //precomputo los ataques por la fila
            }
        }
        else    //para el resto de las casillas simplemente necesito rotar los ataques_filas ya calculados hasta que
        {       //alcance la fila en la que le corresponde estar
            for (k=0;k<256;k++)
            {
                ataques_filas[i][k] = ataques_filas[i%8][k] << ((i/8)*8);
            }
        }
    }
    for (i=0;i<64;i++)  //ahora para los ataques_columnas la misma idea solo q hay q rotar 90� y las casillas que se "copian"
    {                   //son A1,A2,....,A8
        for (k=0;k<256;k++)
        {
            if (i%8 == 0)   //A1,A2,....,A8
            {
                aux = Rotar90clockwise(k);
                bloqueos = mas8dir[i] & aux;
                casilla_bloqueada = LSB(bloqueos);
                ataques_columnas[i][k] = mas8dir[i] ^ mas8dir[casilla_bloqueada];
                bloqueos = menos8dir[i] & aux;
                casilla_bloqueada = MSB(bloqueos);
                ataques_columnas[i][k] |= menos8dir[i] ^ menos8dir[casilla_bloqueada];
            }
            else
            {
                ataques_columnas[i][k] = ataques_columnas[i & 0xf8][k] << (i%8);
            }
        }
    }
    for (i=0;i<64;i++)
    {
        for (k=0;k<256;k++)
        {

        }
    }
}

void IniciaRPr()
{
    int t,p,rb,rn;
    BITBOARD aux;

    for (t=0;t<2;t++)   //turno q le toca mover (0 negras 1 blancas)
    {
    for (p=0;p<64;p++)  //posicion del peon
    {
    for (rb=0;rb<64;rb++)  //posicion del rey blanco
    {
    for (rn=0;rn<64;rn++)  //posicion del rey negro
    {
        RPr[t][p][rb][rn] = 5;  //los inicializo a todos a un valor imposible para detectar posibles fallas en la clasificacion
        rey_b = SetMask(rb);    //primero llenamos con lo q corresponde a las bitboards
        rey_n = SetMask(rn);
        peones_b = SetMask(p);
        //ahora hay q escribir el codigo para caracterizar todas las posibles posiciones de RP vs r
        //primero detectamos las posiciones q son ilegales a las q les asignamos el valor 0
        if (p < 8 || p > 55)            //si hay peon en 1era o en 8ava
        {
            RPr[t][p][rb][rn] = 0;      //es posicion ilegal
            continue;
        }
        if (EnJaque(negras) && t == 1)  //si el negro esta en jaque y le toca al blanco
        {
            RPr[t][p][rb][rn] = 0;      //es posicion ilegal
            continue;
        }
        if (ataques_rey[rb] & rey_n)    //si los reyes se estan atacando mutuamente (estan pegados)
        {
            RPr[t][p][rb][rn] = 0;      //ILEGAL
            continue;
        }
        if (p == rb || p == rn || rb == rn)     //si coinciden los indices significaria q hay dos piezas en la misma casilla
        {
            RPr[t][p][rb][rn] = 0;      //ILEGAL
            continue;
        }
        //eso descarta todas las posiciones ilegales (no importan mucho ya que el motor no permite ilegales de todas maneras)
        //ahora hay q separar las posiciones tablas de las ganadas
        if (!(rey_n & regladelcuadrado[negras][t][p]))	//si el rey no entra en el cuadrado
        {
            RPr[t][p][rb][rn] = 1;      //FINAL GANADO (0 = tablas o ilegal, 1 = ganado)
            continue;
        }
        if (peones_b & mask_ah)         //entra si el peon es peon torre
        {
            if (rey_n & peon_pasado[blancas][p])    //si el rey negro esta en una casilla q evita el paso del peon blanco
            {                                       //entonces es tablas facil
                RPr[t][p][rb][rn] = 0;
                continue;
            }
            if (t == negras && !(ataques_rey[rn] & regladelcuadrado[negras][blancas][p] & ~ataques_rey[rb]))
            {                               //Se cumple cuando le toca al negro y esta dentro de la regla del cuadrado, pero no
                RPr[t][p][rb][rn] = 1;      //tiene movimientos hacia el cuadrado tocandole a las blancas xq el rey blanco lo
                continue;                   //esta cortando. Entonces ganan blancas
            }
            if (DistanciaColumnas(rn,p) == 2 && Fila(rn) > Fila(p))
            {
                RPr[t][p][rb][rn] = 0;
                continue;
            }
            BYTE sq;
            aux = peon_pasado[blancas][p];  //cargo las casillas q interesan para este caso en aux
            while(1)                        //en este loop busco la menor distancia de rey_n a las casillas q producen tablas
            {
                sq = MSB(aux);
                if (sq == 64)               //si se terminaron las casillas salgo
                    break;
                if (Distancia(rn,sq) < Distancia(rb,sq))
                {
                    RPr[t][p][rb][rn] = 0;
                    break;
                }
                if (Distancia(rn,sq) <= Distancia(rb,sq) && t == negras)
                {
                    RPr[t][p][rb][rn] = 0;
                    break;
                }
                aux ^= SetMask(sq);         //borro el bit correspondiente a la casilla ya considerada
            }
            if (t == blancas)
            {
                RPr[t][p][rb][rn] = 0;
                break;
            }
//            if (ataques_rey[rn] & )
//            {
//
//            }

        }//end de peon torre
        else            //aca esta el codigo para todos los otros finales de peon q no estan en las columnas torre
        {
            if (t == negras && ataques_rey[rn] & peones_b & ~ataques_rey[rb])   //rey_n dispone de una captura inmediata del peon
            {
                RPr[t][p][rb][rn] = 0;
                    break;
            }

        }
//                    return subtotal + 900;
    }
    }
    }
    }
    BITBOARD ceros=0,unos=0,sinclasificar=0,incompleto=0;
    for (t=0;t<2;t++)   //turno q le toca mover (0 negras 1 blancas)
    {
    for (p=0;p<64;p++)  //posicion del peon
    {
    for (rb=0;rb<64;rb++)  //posicion del rey blanco
    {
    for (rn=0;rn<64;rn++)  //posicion del rey negro
    {
        switch (RPr[t][p][rb][rn])
        {
            case 0:
            {
                ceros++;
            }break;
            case 1:
            {
                unos++;
            }break;
            default:
            {
                sinclasificar++;
                if (SetMask(p) & mask_ah)
                    incompleto++;
            }break;
        }
    }
    }
    }
    }
}

void EsperaJugada(char algeb[4])		//interpreta las jugadas enviadas por la GUI
{
	char a1,b1,c1,d1;
	while (1)
	{
        a = algeb[0];
        b = algeb[1];
        c = algeb[2];
        d = algeb[3];
		if (a!='a'&&a!='b'&&a!='c'&&a!='d'&&a!='e'&&a!='f'&&a!='g'&&a!='h')
			continue;
		if (b!='1'&&b!='2'&&b!='3'&&b!='4'&&b!='5'&&b!='6'&&b!='7'&&b!='8')
			continue;
		if (c!='a'&&c!='b'&&c!='c'&&c!='d'&&c!='e'&&c!='f'&&c!='g'&&c!='h')
			continue;
		if (d!='1'&&d!='2'&&d!='3'&&d!='4'&&d!='5'&&d!='6'&&d!='7'&&d!='8')
			continue;
		//si esta algebraicamente bien estructurada la jugada entonces:
		a1 = a - 0x61;	//los paso a todos a numeros de 0 a 7 (pero conservo los originales en ASCII)
		b1 = b - 0x31;
		c1 = c - 0x61;
		d1 = d - 0x31;
		inicio = (b1 * 8) + a1;		//inicio y fin van de 0 a 63 q es lo q uso en las bitboards y se calculan como 8 * numcolumna + numfila
		fin = (d1 * 8) + c1;
		if (turno_c == blancas)		//turno_c es el color del turno al q le toca
		{
			if ((piezas_b & SetMask(inicio)) == 0)	//si no corresponde a una pieza del user hay error
				continue;
		}
		else
		{
			if ((piezas_n & SetMask(inicio)) == 0)	//si no corresponde a una pieza del user hay error
				continue;
		}
		//si se comprueba q se indico una pieza propia del user entonces se mira cual fue
		if ((peones_b | peones_n) & SetMask(inicio))
		{
			pieza = PEON;
		}
		else
		{
			if ((caballos_b | caballos_n) & SetMask(inicio))
			{
				pieza = CABALLO;
			}
			else
			{
				if ((alfiles_b | alfiles_n) & SetMask(inicio))
				{
					pieza = ALFIL;
				}
				else
				{
					if ((torres_b | torres_n) & SetMask(inicio))
					{
						pieza = TORRE;
					}
					else
					{
						if ((damas_b | damas_n) & SetMask(inicio))
						{
							pieza = DAMA;
						}
						else
						{
							pieza = REY;
						}
					}
				}
			}
		}
		return;		//si sale ya tengo mis valores inicio, fin y pieza en el formato conveniente para que legal() se encargue
	};
}

int Perft(BYTE prof, BYTE pmax)		//perft para cuando cambio algo saber si esta generando bien todos los movimientos
{
	BYTE z;

	if (prof == pmax)
	{
		nodos++;
		return 0;
	}
	GenerarTodas(prof);
	for (z = 0 ; z < ultimajugada[prof] ; z = z + 3)
	{
		HacerJugada(jugadas[prof][z],jugadas[prof][z+1],jugadas[prof][z+2],prof);
		if (!EnJaque(turno_c))	//si el rey del bando q jugo no esta en jaque es totalmente legal
		{
			valoracion = -Perft(prof + 1, pmax);
		}
		DeshacerJugada(jugadas[prof][z],jugadas[prof][z+1],jugadas[prof][z+2],prof);
		if (prof == 0)       //si terminamos de analizar una jugada de la posicion raiz
        {
            divide[z/3] = nodos;    //anotamos la cantidad de nodos de esa jugada (funcion divide)
            nodos = 0;              //y reiniciamos para la proxima
        }
	}
	if (prof == 0)   //si estamos terminando es tiempo de recuperar la cantidad total de nodos
    {
        nodos = 0;
        for (z=0;z<ultimajugada[0];z=z+3)
        {
            nodos += divide[z/3];       //sumamos todos los resultados parciales de cada jugada de la posicion raiz
        }
        std::cout << nodos << " Posiciones encontradas" << std::endl;
    }
	return 0;
}



void Jugar(BYTE inicio,BYTE fin,BYTE bien)									//hace la jugada verdaderamente (no dentro de un analisis)
{
    //primero elimino de la llave hash los datos de la posicion vieja
    hash_pos ^= Zobrist.enroques[derechos_enroque[0]];  					//saco los viejos derechos de la llave

    hash_pos ^= Zobrist.alpaso[alpaso[0]];              					//saco de la llave el viejo valor de alpaso

	alpaso[0] = 8;															//limpio el registro (solo se setea si avanza un peon de a 2) ya q columnas validas van de 0 a 7
	tablero &= ClearMask(inicio);											//borro la pieza de la casilla de salida del tablero
	tablero |= SetMask(fin);												//y la pongo en la de destino
	tablero90 &= ClearMask90(inicio);
	tablero90 |= SetMask90(fin);
	tableroA1 &= ClearMaskA1(inicio);
	tableroA1 |= SetMaskA1(fin);
	tableroA8 &= ClearMaskA8(inicio);
	tableroA8 |= SetMaskA8(fin);
	jugadas_reversibles++;													//util para saber si la partida termino por regla de 50 jugadas
	total_jugadas++;														//lleva la cuenta del n� de semi-jugadas realizadas hasta ahora
	if (turno_c == blancas)
	{
		piezas_b &= ClearMask(inicio);										//esto es comun a todos los case de bien
		piezas_b |= SetMask(fin);
		switch (bien)
		{
			case normal:
			{
				if (peones_b & SetMask(inicio) || piezas_n & SetMask(fin))	//si movio un peon o fue una captura
					jugadas_reversibles = 0;								//es irreversible
				ActualizaBlancas(inicio,fin);								//realizan las acciones normales de un movimiento
				CapturaBlancas(fin,0);										//que se fije si fue una captura y remueva la pieza correspondiente
			}break;
			case corona:
			{
				peones_b &= ClearMask(inicio);								//borro el peon de la casilla de inicio
				hash_pos ^= Zobrist.escaques[Peon_b][inicio];
				if (turno == HUMANO)										//si jugo el user le doy la posibilidad de elegir q coronar
				{
					e = 'D';
					//coronar();											//llama a la funcion q se encarga de la coronacion
					if (e == 'D')											//al volver esta cargado en "c" q se corono
					{
						damas_b |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Dama_b][fin];
					}
					if (e == 'T')
                    {
						torres_b |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Torre_b][fin];
                    }
					if (e == 'A')
                    {
						alfiles_b |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Alfil_b][fin];
                    }
					if (e == 'C')
                    {
						caballos_b |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Caballo_b][fin];
                    }
				}
				else														//si es jugada del PIC corona siempre dama (en elfuturo se puede mejorar)
				{
					damas_b |= SetMask(fin);								//y pongo una DAMA en la casilla de fin (solo considera esto en principio)
                    hash_pos ^= Zobrist.escaques[Dama_b][fin];
				}
				CapturaBlancas(fin,0);
				jugadas_reversibles = 0;									//una coronacion cambia todo asi q reinicio el contador
			}break;
			case enroque:
			{
				derechos_enroque[0] &= 0xFC;								//borro bit 1 y 2 impidiendo asi cualquier otro enroque blanco en el futuro
				rey_b &= ClearMask(inicio);									//muevo el rey
				rey_b |= SetMask(fin);
				hash_pos ^= Zobrist.escaques[Rey_b][inicio];
				hash_pos ^= Zobrist.escaques[Rey_b][fin];
				if (fin == G1)												//si fue 0-0
				{
					piezas_b &= ClearMask(H1);								//saco la pieza de H1
					piezas_b |= SetMask(F1);								//y la pongo en F1
					torres_b &= ClearMask(H1);								//lo mismo con la bitboard de las torres
					torres_b |= SetMask(F1);
					tablero &= ClearMask(H1);								//y de igual forma con el tablero
					tablero |= SetMask(F1);
					tablero90 &= ClearMask90(H1);
					tablero90 |= SetMask90(F1);
					tableroA1 &= ClearMaskA1(H1);
					tableroA1 |= SetMaskA1(F1);
					tableroA8 &= ClearMaskA8(H1);
					tableroA8 |= SetMaskA8(F1);
					hash_pos ^= Zobrist.escaques[Torre_b][H1];
					hash_pos ^= Zobrist.escaques[Torre_b][F1];
				}
				else														//0-0-0
				{
					piezas_b &= ClearMask(A1);								//saco la pieza de A1
					piezas_b |= SetMask(D1);								//y la pongo en D1
					torres_b &= ClearMask(A1);								//lo mismo con la bitboard de las torres
					torres_b |= SetMask(D1);
					tablero &= ClearMask(A1);								//y de igual forma con el tablero
					tablero |= SetMask(D1);
					tablero90 &= ClearMask90(A1);
					tablero90 |= SetMask90(D1);
					tableroA1 &= ClearMaskA1(A1);
					tableroA1 |= SetMaskA1(D1);
					tableroA8 &= ClearMaskA8(A1);
					tableroA8 |= SetMaskA8(D1);
					hash_pos ^= Zobrist.escaques[Torre_b][A1];
					hash_pos ^= Zobrist.escaques[Torre_b][D1];
				}
				comio[0] = 0;												//no puede haber sido captura si es un enroque
				jugadas_reversibles = 0;
			}break;
			case ap:
			{
				peones_b &= ClearMask(inicio);								//borro el peon de la casilla de inicio
				peones_b |= SetMask(fin);									//y lo pongo en la de fin
				piezas_n &= ClearMask(fin-8);								//borro la pieza y
				peones_n &= ClearMask(fin-8);								//el peon negro comido al paso
				tablero &= ClearMask(fin-8);								//tambien lo saco del tablero
				tablero90 &= ClearMask90(fin-8);
				tableroA1 &= ClearMaskA1(fin-8);
				tableroA8 &= ClearMaskA8(fin-8);
				comio[0] = PEON;											//fue al paso asi q es una captura de peon
				jugadas_reversibles = 0;
				hash_pos ^= Zobrist.escaques[Peon_b][inicio];
				hash_pos ^= Zobrist.escaques[Peon_b][fin];
				hash_pos ^= Zobrist.escaques[Peon_n][fin-8];
			}break;
			case doble:
			{
				peones_b &= ClearMask(inicio);								//borro el peon de la casilla de inicio
				peones_b |= SetMask(fin);									//y lo pongo en la de fin
				comio[0] = 0;												//no puede haber sido captura si es un doble mov de peon
				alpaso[0] = Columna(inicio);								//aviso q es posible capturar al paso en esa columna lo proxima jugada
				jugadas_reversibles = 0;
				hash_pos ^= Zobrist.escaques[Peon_b][inicio];
				hash_pos ^= Zobrist.escaques[Peon_b][fin];
			}break;
			case movrey:
			{
				if ((derechos_enroque[0] & 3)||(piezas_n & SetMask(fin)))	//si todavia tenia derecho a algun enroque o la jugada es una captura entonces es una jugada
					jugadas_reversibles = 0;								//irreversible
				ActualizaBlancas(inicio,fin);
				CapturaBlancas(fin,0);
				derechos_enroque[0] &= 0xFC;								//le quito los derechos de enroque a las blancas de aqui en adelante (no interesa si los tenia o no)
			}break;
			case movtorre:
			{
				if (inicio == H1)											//si se movio la torre de H1
				{
					if((derechos_enroque[0] & 1)||(piezas_n & SetMask(fin)))//si todavia tenia derecho a enrocar corto o es una captura entonces es una jugada
						jugadas_reversibles = 0;							//irreversible
					derechos_enroque[0] &= 0xFE;							//le quito los derechos de enroque corto a las blancas de aqui en adelante
				}
				if (inicio == A1)											//si se movio la torre de H1
				{
					if((derechos_enroque[0] & 2)||(piezas_n & SetMask(fin)))//si todavia tenia derecho a enrocar largo o es una captura entonces es una jugada
						jugadas_reversibles = 0;							//irreversible
					derechos_enroque[0] &= 0xFD;							//le quito los derechos de enroque largo a las blancas de aqui en adelante
				}
				ActualizaBlancas(inicio,fin);								//todo el resto normal
				CapturaBlancas(fin,0);
			}break;
		}
	}//end turno blancas
	else	//si le toca a las negras
	{
		piezas_n &= ClearMask(inicio);
		piezas_n |= SetMask(fin);
		switch (bien)
		{
			case normal:
			{
				if (peones_n & SetMask(inicio) || piezas_b & SetMask(fin))
					jugadas_reversibles = 0;
				ActualizaNegras(inicio,fin);
				CapturaNegras(fin,0);
			}break;
			case corona:
			{
				peones_n &= ClearMask(inicio);
				if (turno == HUMANO)
				{
					e = 'D';
					//coronar();
					if (e == 'D')
                    {
                        damas_n |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Dama_n][fin];
                    }
					if (e == 'T')
                    {
                        torres_n |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Torre_n][fin];
                    }
					if (e == 'A')
                    {
                        alfiles_n |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Alfil_n][fin];
                    }
					if (e == 'C')
                    {
                        caballos_n |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Caballo_n][fin];
                    }
				}
				else
				{
					damas_n |= SetMask(fin);
					hash_pos ^= Zobrist.escaques[Dama_n][fin];
				}
				CapturaNegras(fin,0);
				jugadas_reversibles = 0;
			}break;
			case enroque:
			{
				derechos_enroque[0] &= 0xF3;
				rey_n &= ClearMask(inicio);
				rey_n |= SetMask(fin);
				hash_pos ^= Zobrist.escaques[Rey_n][inicio];
				hash_pos ^= Zobrist.escaques[Rey_n][fin];
				if (fin == G8)
				{
					piezas_n &= ClearMask(H8);
					piezas_n |= SetMask(F8);
					torres_n &= ClearMask(H8);
					torres_n |= SetMask(F8);
					tablero &= ClearMask(H8);
					tablero |= SetMask(F8);
					tablero90 &= ClearMask90(H8);
					tablero90 |= SetMask90(F8);
					tableroA1 &= ClearMaskA1(H8);
					tableroA1 |= SetMaskA1(F8);
					tableroA8 &= ClearMaskA8(H8);
					tableroA8 |= SetMaskA8(F8);
					hash_pos ^= Zobrist.escaques[Torre_b][H8];
					hash_pos ^= Zobrist.escaques[Torre_b][F8];
				}
				else
				{
					piezas_n &= ClearMask(A8);
					piezas_n |= SetMask(D8);
					torres_n &= ClearMask(A8);
					torres_n |= SetMask(D8);
					tablero &= ClearMask(A8);
					tablero |= SetMask(D8);
					tablero90 &= ClearMask90(A8);
					tablero90 |= SetMask90(D8);
					tableroA1 &= ClearMaskA1(A8);
					tableroA1 |= SetMaskA1(D8);
					tableroA8 &= ClearMaskA8(A8);
					tableroA8 |= SetMaskA8(D8);
					hash_pos ^= Zobrist.escaques[Torre_n][A8];
					hash_pos ^= Zobrist.escaques[Torre_n][D8];
				}
				comio[0] = 0;
				jugadas_reversibles = 0;
			}break;
			case ap:
			{
				peones_n &= ClearMask(inicio);
				peones_n |= SetMask(fin);
				piezas_b &= ClearMask(fin+8);
				peones_b &= ClearMask(fin+8);
				tablero &= ClearMask(fin+8);
				tablero90 &= ClearMask90(fin+8);
				tableroA1 &= ClearMaskA1(fin+8);
				tableroA8 &= ClearMaskA8(fin+8);
				comio[0] = PEON;
				jugadas_reversibles = 0;
				hash_pos ^= Zobrist.escaques[Peon_n][inicio];
				hash_pos ^= Zobrist.escaques[Peon_n][fin];
				hash_pos ^= Zobrist.escaques[Peon_b][fin+8];
			}break;
			case doble:
			{
				peones_n &= ClearMask(inicio);
				peones_n |= SetMask(fin);
				comio[0] = 0;
				alpaso[0] = Columna(inicio);
				jugadas_reversibles = 0;
				hash_pos ^= Zobrist.escaques[Peon_n][inicio];
				hash_pos ^= Zobrist.escaques[Peon_n][fin];
			}break;
			case movrey:
			{
				if ((derechos_enroque[0] & 0x0C)||(piezas_b & SetMask(fin)))
					jugadas_reversibles = 0;
				ActualizaNegras(inicio,fin);
				CapturaNegras(fin,0);
				derechos_enroque[0] &= 0xF3;									//le quito los derechos de enroque a las negras de aqui en adelante
			}break;
			case movtorre:
			{
				if (inicio == H8)
				{
					if((derechos_enroque[0] & 4)||(piezas_b & SetMask(fin)))
						jugadas_reversibles = 0;
					derechos_enroque[0] &= 0xFB;
				}
				if (inicio == A8)
				{
					if((derechos_enroque[0] & 8)||(piezas_b & SetMask(fin)))
						jugadas_reversibles = 0;
					derechos_enroque[0] &= 0xF7;
				}
				ActualizaNegras(inicio,fin);
				CapturaNegras(fin,0);
			}break;
		}
	}//end turno negras
	//ahora actualizo la llave hash con datos de la nueva posicion
    hash_pos ^= Zobrist.enroques[derechos_enroque[0]];  						//pongo los nuevos derechos en la llave (pueden ser los mismos)
    hash_pos ^= Zobrist.alpaso[alpaso[0]];              						//pongo el nuevo alpaso en la llave
	hash_pos ^= Zobrist.bando;                          						//el bando siempre se togglea

	turno ^= 1;
	turno_c ^= 1;
}

int EvaluarRapido()     //solo balance material para ver si es posible aplicar poda de inutilidad
{
    total = 900 * (POPCNT(damas_b) - POPCNT(damas_n));
	total += 500 * (POPCNT(torres_b) - POPCNT(torres_n));
	total += 310 * (POPCNT(alfiles_b) - POPCNT(alfiles_n));
	total += 300 * (POPCNT(caballos_b) - POPCNT(caballos_n));
	total += 100 * (POPCNT(peones_b) - POPCNT(peones_n));
	if ((alfiles_b & casillas_b) && (alfiles_b & casillas_n))
		total += 30;
	if ((alfiles_n & casillas_b) && (alfiles_n & casillas_n))
		total -= 30;

	if (turno_c == blancas)
		return total;
	return -total;
}

void LeeFEN(const std::string& auxfen)			    	//pide informacion del usuario acerca de la posicion inicial y deja todo listo para q comience la partida
{
	BYTE i,error,columna,espacio,indice;
	BITBOARD aux;

	peones_b = 0;		    							//inicializo a 0 las bitboards q tienen la posicion del tablero
	caballos_b = 0;
	alfiles_b = 0;
	torres_b = 0;
	damas_b = 0;
	rey_b = 0;

	peones_n = 0;
	caballos_n = 0;
	alfiles_n = 0;
	torres_n = 0;
	damas_n = 0;
	rey_n = 0;

	//ahora comienza lo propio de la FEN
	do
	{
		error = 0;										//empezamos sin errores
		espacio = 0;
		indice = 56;									//la notacion FEN arranca en la casilla a8 y va bajando por filas
		columna = 0;									//arranca en la columna 0
		derechos_enroque[0] = 0;
		for (i=0;i<85;i++)								//limpio primero por las dudas el vector q va a tener los datos de la posicion
		{
			fen[i] = 0;
		}
		for (i=0;i<auxfen.size();i++)
		{
			fen[i] = auxfen[i];
		}

		for (i=0;i<auxfen.size();i++)					//lee el vector q se recibio
		{
            if (error)
                break;
            switch (espacio)
			{
				case 0:									//ubica las piezas en el tablero
				{
					switch (fen[i])
					{
						case '1':						//solo hay q avanzar una casilla
						{
							indice++;
							columna++;
						}break;
						case '2':						//hay q avanzar 2 casillas, etc
						{
							indice += 2;
							columna += 2;
						}break;
						case '3':
						{
							indice += 3;
							columna += 3;
						}break;
						case '4':
						{
							indice += 4;
							columna += 4;
						}break;
						case '5':
						{
							indice += 5;
							columna += 5;
						}break;
						case '6':
						{
							indice += 6;
							columna += 6;
						}break;
						case '7':
						{
							indice += 7;
							columna += 7;
						}break;
						case '8':
						{
							indice += 8;
							columna += 8;
						}break;
						case 'p':							//peon negro
						{
							peones_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'n':							//caballo negro
						{
							caballos_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'b':							//alfil negro
						{
							alfiles_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'r':							//torre negra
						{
							torres_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'q':							//dama negra
						{
							damas_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'k':							//rey negro
						{
							rey_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'P':							//peon blanco
						{
							peones_b |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'N':
						{
							caballos_b |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'B':
						{
							alfiles_b |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'R':
						{
							torres_b |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'Q':
						{
							damas_b |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'K':
						{
							rey_b |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case '/':
						{
							if (columna != 8)
								error = 1;
							columna = 0;					//reinicio la columna
							indice -= 16;					//baja una fila
						}break;
						case ' ':
						{
							espacio++;						//indica q se pasa al siguiente campo de informacion
						}break;
						default:							//si llega cualquier otra cosa
						{
							error = 1;						//hay error en el string FEN
						}break;
					}										//end switch
				}break;										//end case del primer campo de info
				case 1:										//si ya se dio un espacio (cambia el significado de los caracteres)
				{
					switch (fen[i])
					{
						case 'w':
						{
							turno_c = blancas;				//empiezan las blancas
						}break;
						case 'b':
						{
							turno_c = negras;				//empiezan las negras
						}break;
						case ' ':
						{
							espacio++;						//indica q se pasa al siguiente campo de informacion
						}break;
						default:							//si llega cualquier otra cosa
						{
							error = 1;						//hay error en el string FEN
						}break;
					}										//end switch
				}break;										//end case de bando q mueve primero
				case 2:										//enroques posibles
				{
					switch (fen[i])
					{
						case '-':
						{
							derechos_enroque[0] = 0;		//no es posible ningun enroque
						}break;
						case 'K':
						{
							derechos_enroque[0] += 1;		//se permite el enroque corto de las blancas
							if (!(rey_b & SetMask(E1)) || !(torres_b & SetMask(H1)))		//si no esta el rey en E1 o la torre de H1 hay error
								error = 1;
						}break;
						case 'Q':
						{
							derechos_enroque[0] += 2;		//se permite el enroque largo de las blancas
							if (!(rey_b & SetMask(E1)) || !(torres_b & SetMask(A1)))
								error = 1;
						}break;
						case 'k':
						{
							derechos_enroque[0] += 4;		//se permite el enroque corto de las negras
							if (!(rey_n & SetMask(E8)) || !(torres_n & SetMask(H8)))
								error = 1;
						}break;
						case 'q':
						{
							derechos_enroque[0] += 8;		//se permite el enroque largo de las negras
							if (!(rey_n & SetMask(E8)) || !(torres_n & SetMask(A8)))
								error = 1;
						}break;
						case ' ':
						{
							espacio++;						//va al otro campo de info
						}break;
						default:							//si llega cualquier otra cosa
						{
							error = 1;						//hay error en el string FEN
						}break;
					}										//end switch
				}break;										//end case de enroques
				case 3:										//peon al paso
				{
					switch (fen[i])
					{
						case '-':
						{
							alpaso[0] = 8;					//no es posible ningun peon al paso (en la jugada anterior ningun peon movio de a 2 casillas)
						}break;
						case 'a':							//el peon de "a" (no importa si blanco o negro) movio de a dos casillas en la jugada anterior (podria comerse al paso si se diera el caso)
						{
							alpaso[0] = columnaA;
							if ((turno_c == blancas && !(peones_n & SetMask(A5))) || (turno_c == negras && !(peones_b & SetMask(A4))))	//si no esta el peon q deberia hay error
								error = 1;
						}break;
						case 'b':
						{
							alpaso[0] = columnaB;
							if ((turno_c == blancas && !(peones_n & SetMask(B5))) || (turno_c == negras && !(peones_b & SetMask(B4))))
								error = 1;
						}break;
						case 'c':
						{
							alpaso[0] = columnaC;
							if ((turno_c == blancas && !(peones_n & SetMask(C5))) || (turno_c == negras && !(peones_b & SetMask(C4))))
								error = 1;
						}break;
						case 'd':
						{
							alpaso[0] = columnaD;
							if ((turno_c == blancas && !(peones_n & SetMask(D5))) || (turno_c == negras && !(peones_b & SetMask(D4))))
								error = 1;
						}break;
						case 'e':
						{
							alpaso[0] = columnaE;
							if ((turno_c == blancas && !(peones_n & SetMask(E5))) || (turno_c == negras && !(peones_b & SetMask(E4))))
								error = 1;
						}break;
						case 'f':
						{
							alpaso[0] = columnaF;
							if ((turno_c == blancas && !(peones_n & SetMask(F5))) || (turno_c == negras && !(peones_b & SetMask(F4))))
								error = 1;
						}break;
						case 'g':
						{
							alpaso[0] = columnaG;
							if ((turno_c == blancas && !(peones_n & SetMask(G5))) || (turno_c == negras && !(peones_b & SetMask(G4))))
								error = 1;
						}break;
						case 'h':
						{
							alpaso[0] = columnaH;
							if ((turno_c == blancas && !(peones_n & SetMask(H5))) || (turno_c == negras && !(peones_b & SetMask(H4))))
								error = 1;
						}break;
						case ' ':
						{
							espacio++;
						}break;
						case '3':				//3 y 6 pueden venir en la notacion FEN para indicar si son las blancas o las negras
						{						//yo no necesito hacer nada xq lo manejo por columnas y funciona = pero tienen q estar los case para q
												//no tire error al recibir el numero y entrar en default
						}break;
						case '6':
						{

						}break;
						default:				//si llega cualquier otra cosa
						{
							error = 1;			//hay error en el string FEN
						}break;
					}							//end switch
				}break;							//end case 3 y ultimo (el del peon al paso)
				case 4:							//no hago nada xq termino la lectura (espero a q termine el for)
				{

				}break;
			}									//end switch de los campos a llenar
		}										//end for
		comio[0] = 0;							//antes de la posicion inicial no pudo haber una captura (no es muy necesario esto de todas formas)
		jugadas_reversibles = 0;
		piezas_b = peones_b | caballos_b | alfiles_b | torres_b | damas_b | rey_b;
		piezas_n = peones_n | caballos_n | alfiles_n | torres_n | damas_n | rey_n;
		tablero = piezas_b | piezas_n;
        tablero90 = Rotar90antiClockwise(tablero);      //inicializo la bitboard rotada para generar ataques por columnas
        aux = tablero;
        tableroA1 = 0;                                  //y las dos de diagonales
        tableroA8 = 0;
        while (aux)                                     //para cada bit '1' de tablero
        {
            inicio = MSB(aux);
            aux &= ClearMask(inicio);
            tableroA1 |= SetMaskA1(inicio);             //inicializo los tableros de diagonales
            tableroA8 |= SetMaskA8(inicio);
        };
		if (POPCNT(rey_b) != 1 || POPCNT(rey_n) != 1)	//si no hay un rey blanco y uno negro exactamente entonces hay error
			error = 1;
		if ((EnJaque(blancas) && turno_c == blancas) || (EnJaque(negras) && turno_c == negras))	//si esta en jaque un rey al q no le toca jugar hay error
			error = 1;
		if (POPCNT(tablero) > 32 || POPCNT(piezas_b) > 16 || POPCNT(piezas_n) > 16)
			error = 1;
		if (POPCNT(peones_b) > 8 || POPCNT(peones_n) > 8)
			error = 1;
		if (POPCNT(caballos_b) > 10 || POPCNT(caballos_n) > 10)
			error = 1;
		if (POPCNT(alfiles_b) > 10 || POPCNT(alfiles_n) > 10)
			error = 1;
		if (POPCNT(torres_b) > 10 || POPCNT(torres_n) > 10)
			error = 1;
		if (POPCNT(damas_b) > 9 || POPCNT(damas_n) > 9)
			error = 1;
		if (((peones_b | peones_n) & fila_mask[0]) || ((peones_b | peones_n) & fila_mask[7]))		//si se intento poner peones en la primer u octava fila hay error
			error = 1;
	}while(error == 1);					//mientras haya error no se sale (a menos q se escriba "atras")
	TresRepet();						//paso para q considere la posicion inicial como la primera repeticion e inicialize todo los reg necesarios
}
