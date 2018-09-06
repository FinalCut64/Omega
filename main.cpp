///// CAMBIOS RESPECTO A LA VERSION ANTERIOR /////
//
//- Mejore QGenerarCapturas() y EvaluarMJ() eliminaando la parte de if( == 64) q era totalmente ineficiente e inútil


#include <iostream>
#include <string>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#include "definiciones.h"
#include "global.h"
#include "bitboard.h"
#include "tt.h"



using namespace std;



#define USAR_RELOJ


    /////////////////////////////////////////////////////////////////////////////
    //                            FUNCIONES LOCALES
    /////////////////////////////////////////////////////////////////////////////

void Uci();
void IniciaVar();
void InicializaRegistros();		            //inicia los registros a su estado por defecto
void ReiniciaRegistros();		            //antes de cada nueva partida hay q reiniciar varios registros
void LeeFEN();								//prepara todos los registros necesarios para q comience la partida (nueva o con introduccion de FEN)
void EsperaJugada(char[4]);					//retorna cuando el user jugo algo y obtiene la jugada del user traducida al formato q entiende el programa
void InicializaMasks();						//inicializa las mascaras de columnas, filas set y clear
void IniciaBitboards();						//inicializa las bitboards ataques_peon,ataques_caballo,alpaso,etc
void IniciaRPr();                           //genera la tabla q contiene los valores para todas las posiciones del final RP vs r
void QuePieza();							//pone en el reg pieza el valor correspondiente a la pieza seleccioneda por el user
BYTE Legal();								//se fija si la jugada es totalmente correcta en terminos legales (FALSE si es ilegal y TRUE si es legal)
BYTE SemiLegal(BYTE);						//con esta funcion sola el programa puede jugar come todo jeje
BYTE ChequeaDiagonal();						//devuelve TRUE si la jugada puede realizarse por la diogonal y FLASE sino
BYTE ChequeaOrtogonal();					//lo mismo q ChequeaDiagonal pero con los movimientos ortogonales
void Analiza();								//busca la "mejor" jugada q se pueda (aca esta toda la magia y los errores jeje)
int  Raiz(int,int,BYTE);                    //Negamax() para la posicion raiz
int  NegaMax(int,int,BYTE,BYTE);			//funcion de busqueda Negamax con poda alfa-beta y demas tecnicas de busqueda
BITBOARD Perft(BYTE);						//para chequear el generador de movimientos
void GenerarTodas(BYTE);					//genera todas las jugadas semilegales en la posicion y profundidad actual
BITBOARD GeneraAlfil(BYTE);					//usada por GenerarTodas para los movimientos diagonales
BITBOARD GeneraTorre(BYTE);					//usada por GenerarTodas para los movimientos ortogonales
void ListaJugadas(BYTE,BYTE,BYTE,BYTE);		//llena la lista de jugadas semilegales en base a lo que le "manda" GenerarTodas()
void Ordenar(BYTE);
void BorrarVP();
void DescomprimirVP();
BYTE Nulo_ok(BYTE);                         //devuelve 1 si se puede aplicar poda de movimiento nulo en el nodo actual (0 sino)
BYTE LMR_ok(BYTE,unsigned int);             //devuelve 1 si se puede aplicar LMR al movimiento actual (0 sino)
void Jugar(BYTE,BYTE,BYTE);					//esta es la q HACE la jugada (no durante el analisis sino de posta)
void HacerJugada(BYTE,BYTE,BYTE,BYTE);		//actualiza los registros necesarios para q el analisis prospere (es decir efectua la jugada en la representacion del programa)
void ActualizaBlancas(BYTE,BYTE);
void CapturaBlancas(BYTE,BYTE);
void ActualizaNegras(BYTE,BYTE);
void CapturaNegras(BYTE,BYTE);
void DeshacerJugada(BYTE,BYTE,BYTE,BYTE);	//hace todo lo contrario a HacerJugada()
void DesactualizaBlancas(BYTE,BYTE);
void DescapturaBlancas(BYTE,BYTE);
void DesactualizaNegras(BYTE,BYTE);
void DescapturaNegras(BYTE,BYTE);
int  CapturasBlancas();
int  CapturasNegras();
BYTE EnJaque(BYTE);							//dice si el rey del bando q mueve queda en jaque o no
BYTE CasillaAtacada(BYTE,BYTE);
int  EvaluarRapido();                       //version muy reducida de evaluar() q da una idea de si conviene podar o no
int  Evaluar();								//funcion de evaluacion de la posicion
int  EvaluarFinal();
int  EvaluarMJ();
BYTE versitermino();						//si la partida termino por algun motivo (el q sea) devuelve TRUE y sino FALSE y se sigue jugando
BYTE Ahogado();
void Guardar();								//esta junto con recuperar permite manejar la posibilidad de salir de NegaMax sin concluir
void Recuperar();							//por lo q todos los registros quedarian en cualquier estado. Estas 2 funciones solucionan eso
BYTE Repite();								//avisa si la posicion q esta analizando el pic ya se produjo para q la evalue como tablas si es asi y no repita en posiciones ganadas
BYTE TresRepet();							//permite determinar si la partida debe acabar por regla de tres repeticiones
BYTE Libro();                               //el libro de aperturas propio del modulo
void Anotar();                              //registra la partida en curso a medida que se realizan las jugadas
void ExportarPartida();
void ManejarReloj();

void QuickSort(int a[],int,int, BYTE);

int  Quies(int,int,BYTE);
void QGenerarCapturas(BYTE);
void QListaCapturas(BYTE,BYTE,BYTE);
void QHacerCaptura(BYTE,BYTE,BYTE);
void QDeshacerCaptura(BYTE,BYTE,BYTE);
void QActualizaBlancas(BYTE,BYTE);
void QActualizaNegras(BYTE,BYTE);
void QDesactualizaBlancas(BYTE,BYTE);
void QDesactualizaNegras(BYTE,BYTE);
void QCapturaBlancas(BYTE,BYTE);
void QCapturaNegras(BYTE,BYTE);
void QDescapturaBlancas(BYTE,BYTE);
void QDescapturaNegras(BYTE,BYTE);
void QQuickSort(int a[],int,int, BYTE);





jmp_buf env;								//esta es para la funcion longjump q permite salir de negamax rapido

///////////////////Variables globales/////////////////////////////////

BYTE nueva,resultado;
BYTE turno,turno_c,colorpic,bien,semibien,inicio,fin,pieza;
BYTE prof_max,jugadas[MAXPROF][600],Qcapturas[40][80],Qcorona[40];
BYTE comio[MAXPROF],alpaso[MAXPROF],derechos_enroque[MAXPROF],vp[MAXPROF][3];
BYTE legales[MAXPROF],jugadas_reversibles,nulo,Qcomio[40];
BYTE salir,unica;
//BYTE ext;
char fen[85];                               //como maximo el string FEN puede tener 85 caracteres
char planilla[7][400];                      //permite anotar una partida de hasta 400 jugadas
unsigned int ply_count;
BYTE mcut;                                  //indica si se esta haciendo o no una busqueda multi cut

char a,b,c,d,e;								//para cuando el user corona ver q fue
int ponderacion[200],Qponderacion[40];		//para el ordenamiento de jugadas con el enfoque MVV/LVA
unsigned int puntero,ultimajugada[MAXPROF],Qpuntero,ultimacaptura[40],total_jugadas;
unsigned int pv[MAXPROF][MAXPROF];
int valoracion,val,estimo,total;
int killer[MAXPROF][2];
unsigned long long divide[600]; //para la funcion divide dentro del perft() q permite detectar errores en el generador de movs
unsigned long long saliointerrumpido,salionormal;	//estas son para ver estadisticas
unsigned long long nodos,Qnodos,nodosnulos,cortesnulo,llamadasevaluar,cortes_inut_inversa;
unsigned long long cantventanas,falloventana,eficiencia_1,eficiencia_2,eficiencia,qpasaaca;
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
BITBOARD set_maskA1[64] =                                  //soy un manco y no pude escribir una funcion para representar esta serie
{                                             //por eso lo inicializo asi y no en InicializaMasks()
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

unsigned int historia[2][64][64];   //para implementar ordenamiento por historia de la jugada [turno][inicio][fin]
BYTE RPr[2][64][64][64];            //turno a quien le toca, casilla del peon, del rey blanco y del negro (contiene todas las pos)
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

	while (1)
	{
/*
        LeeFEN();
        InicializaHash();       //una vez configurada la posición se establece la clave hash de la misma
		ReiniciaRegistros();	//al comenzar una nueva partida hay q reiniciar varios registros para q todo funcione bien (esto inicia la partida)
//        valoracion = Evaluar();
        QueryPerformanceCounter((LARGE_INTEGER *)&t_inicio);
*/
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
	char m = 0;
	int i,j,k;

	resultado = 0;								//la partida no ha terminado aun (recien empieza)
	total_jugadas = 0;							//van 0 jugadas
	jugadas_reversibles = 0;

//    printf("Seleccionar color: b,n ");
//    scanf(" %c",&m);
//    if (m == 'b')
//    if(condicion que indique el color al q le toca jugar)
        colorpic = negras;
//    else
//        colorpic = blancas;
/*
    if (colorpic == blancas)
    {
        if (turno_c == blancas)
            turno = PIC;
        else
            turno = HUMANO;
    }
    else
    {
        if (turno_c == negras)
            turno = PIC;
        else
            turno = HUMANO;
    }
*/
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

    nodos = 1;
}

void EsperaJugada(char algeb[4])		//retorna cuando el user jugo algo y obtiene la jugada del user traducida al formato q entiende el programa
{
//	char algeb[4],a1,b1,c1,d1;
	char a1,b1,c1,d1;
	while (1)
	{
//        printf("Introduce una jugada: ");
//        scanf("%s",algeb);
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

void Anotar()                               //anota la partida en el formato convencional del fritz
{
    int aclara = 0,i;

    if (piezas_b & SetMask(fin))            //si fue una jugada de las blancas (miro en fin xq ya pase por Jugar())
    {
        if (bien == corona)
        {
            planilla[0][ply_count] = a;
            if (Columna(inicio) != Columna(fin))    //si capturo
            {
                planilla[1][ply_count] = 'x';
                planilla[2][ply_count] = c;
                planilla[3][ply_count] = d;
                planilla[4][ply_count] = '=';
                planilla[5][ply_count] = 'Q';
            }
            else
            {
                planilla[1][ply_count] = d;
                planilla[2][ply_count] = '=';
                planilla[3][ply_count] = 'Q';
            }
        }
        else
        {
            if (peones_b & SetMask(fin))        //si fue mov de peon
            {
                planilla[0][ply_count] = a;
                if (Columna(inicio) != Columna(fin))    //si capturo
                {
                    planilla[1][ply_count] = 'x';
                    planilla[2][ply_count] = c;
                    planilla[3][ply_count] = d;
                }
                else
                {
                    planilla[1][ply_count] = d;
                    if (SetMask(fin) & fila_mask[fila8]) //si corono
                    {
                        planilla[2][ply_count] = '=';
                        planilla[3][ply_count] = 'Q';
                    }
                }
            }
            if (caballos_b & SetMask(fin))                                  //si fue mov de caballo
            {
                planilla[0][ply_count] = 'N';                               //anoto las iniciales en ingles
                if ((POPCNT(ataques_caballos[fin] & caballos_b)) != 0)      //si mas de dos caballos pueden jugar a fin aclaro cual
                {
                    aclara = 1;                                             //indica q la jugada es tipo Cbd2(aclara q pieza se mueve)
                    if ((POPCNT(caballos_b & columna_mask[Columna(inicio)]) != 0))  //si habia mas de dos en la misma columna
                    {
                        planilla[1][ply_count] = b;                         //especifico el numero de fila
                    }
                    else                                                    //sino doy la letra de la columna de inicio
                    {
                        planilla[1][ply_count] = a;
                    }
                    planilla[2][ply_count] = c;
                    planilla[3][ply_count] = d;
                }
                else
                {
                    planilla[1][ply_count] = c;
                    planilla[2][ply_count] = d;
                }
            }
            if (alfiles_b & SetMask(fin))        //similar para el resto de las piezas
            {
                planilla[0][ply_count] = 'B';
                if ((POPCNT(ataques_alfiles[fin] & alfiles_b)) != 0)
                {
                    aclara = 1;
                    if ((POPCNT(alfiles_b & columna_mask[Columna(inicio)]) != 0))
                    {
                        planilla[1][ply_count] = b;
                    }
                    else
                    {
                        planilla[1][ply_count] = a;
                    }
                    planilla[2][ply_count] = c;
                    planilla[3][ply_count] = d;
                }
                else
                {
                    planilla[1][ply_count] = c;
                    planilla[2][ply_count] = d;
                }
            }
            if (torres_b & SetMask(fin))
            {
                planilla[0][ply_count] = 'R';
                if ((POPCNT(ataques_torres[fin] & torres_b)) != 0)
                {
                    aclara = 1;
                    if ((POPCNT(torres_b & columna_mask[Columna(inicio)]) != 0))
                    {
                        planilla[1][ply_count] = b;
                    }
                    else
                    {
                        planilla[1][ply_count] = a;
                    }
                    planilla[2][ply_count] = c;
                    planilla[3][ply_count] = d;
                }
                else
                {
                    planilla[1][ply_count] = c;
                    planilla[2][ply_count] = d;
                }
            }
            if ((damas_b & SetMask(fin)))
            {
                planilla[0][ply_count] = 'Q';
                if (((POPCNT(ataques_alfiles[fin] & damas_b)) != 0) || (POPCNT(ataques_torres[fin] & damas_b)) != 0)
                {
                    aclara = 1;
                    if ((POPCNT(damas_b & columna_mask[Columna(inicio)]) != 0))
                    {
                        planilla[1][ply_count] = b;
                    }
                    else
                    {
                        planilla[1][ply_count] = a;
                    }
                    planilla[2][ply_count] = c;
                    planilla[3][ply_count] = d;
                }
                else
                {
                    planilla[1][ply_count] = c;
                    planilla[2][ply_count] = d;
                }

            }
            if (rey_b & SetMask(fin))
            {
                if (bien == enroque)
                {
                    if (fin == G1)
                    {
                        planilla[0][ply_count] = '0';
                        planilla[1][ply_count] = '-';
                        planilla[2][ply_count] = '0';
                    }
                    else
                    {
                        planilla[0][ply_count] = '0';
                        planilla[1][ply_count] = '-';
                        planilla[2][ply_count] = '0';
                        planilla[3][ply_count] = '-';
                        planilla[4][ply_count] = '0';
                    }
                }
                else
                {
                    planilla[0][ply_count] = 'K';
                    planilla[1][ply_count] = c;
                    planilla[2][ply_count] = d;
                }
            }
        }
        if (comio[0] && !(peones_b & SetMask(fin)) && (bien != corona))
        {                                                       //ahora miro si fue una captura de pieza (las de peones ya estan hechas)
            planilla[6][ply_count] = planilla[5][ply_count];    //muevo toda la jugada para dejar lugar al signo x de captura
            planilla[5][ply_count] = planilla[4][ply_count];
            planilla[4][ply_count] = planilla[3][ply_count];
            planilla[3][ply_count] = planilla[2][ply_count];
            if (aclara)                                         //si es una jugada con aclaracion (estilo Cbxd2) el x esta en
            {
                planilla[2][ply_count] = 'x';                   //la posicion 2 dentro del array
            }
            else                                                //sino en la uno
            {
                planilla[2][ply_count] = planilla[1][ply_count];
                planilla[1][ply_count] = 'x';
            }
        }
        if (EnJaque(negras))                    //si la jugada fue un jaque
        {
            i = 0;
            while (planilla[i][ply_count] != 0) //busco el final de la jugada
            {
                i++;
            };
            planilla[i][ply_count] = '+';       //agrego el simbolo de jaque
        }
    }
    else    //lo mismo si movieron las negras
    {
        if (bien == corona)
        {
            planilla[0][ply_count] = a;
            if (Columna(inicio) != Columna(fin))    //si capturo
            {
                planilla[1][ply_count] = 'x';
                planilla[2][ply_count] = c;
                planilla[3][ply_count] = d;
                planilla[4][ply_count] = '=';
                planilla[5][ply_count] = 'Q';
            }
            else
            {
                planilla[1][ply_count] = d;
                planilla[2][ply_count] = '=';
                planilla[3][ply_count] = 'Q';
            }
        }
        else
        {
            if (peones_n & SetMask(fin))
            {
                planilla[0][ply_count] = a;
                if (Columna(inicio) != Columna(fin))
                {
                    planilla[1][ply_count] = 'x';
                    planilla[2][ply_count] = c;
                    planilla[3][ply_count] = d;
                    if (SetMask(fin) & fila_mask[fila1])
                    {
                        planilla[4][ply_count] = '=';
                        planilla[5][ply_count] = 'Q';
                    }
                }
                else
                {
                    planilla[1][ply_count] = d;
                    if (SetMask(fin) & fila_mask[fila1])
                    {
                        planilla[2][ply_count] = '=';
                        planilla[3][ply_count] = 'Q';
                    }
                }
            }
            if (caballos_n & SetMask(fin))
            {
                planilla[0][ply_count] = 'N';
                if ((POPCNT(ataques_caballos[fin] & caballos_n)) != 0)
                {
                    aclara = 1;
                    if ((POPCNT(caballos_n & columna_mask[Columna(inicio)]) != 0))
                    {
                        planilla[1][ply_count] = b;
                    }
                    else
                    {
                        planilla[1][ply_count] = a;
                    }
                    planilla[2][ply_count] = c;
                    planilla[3][ply_count] = d;
                }
                else
                {
                    planilla[1][ply_count] = c;
                    planilla[2][ply_count] = d;
                }
            }
            if (alfiles_n & SetMask(fin))
            {
                planilla[0][ply_count] = 'B';
                if ((POPCNT(ataques_alfiles[fin] & alfiles_n)) != 0)
                {
                    aclara = 1;
                    if ((POPCNT(alfiles_n & columna_mask[Columna(inicio)]) != 0))
                    {
                        planilla[1][ply_count] = b;
                    }
                    else
                    {
                        planilla[1][ply_count] = a;
                    }
                    planilla[2][ply_count] = c;
                    planilla[3][ply_count] = d;
                }
                else
                {
                    planilla[1][ply_count] = c;
                    planilla[2][ply_count] = d;
                }
            }
            if (torres_n & SetMask(fin))
            {
                planilla[0][ply_count] = 'R';
                if ((POPCNT(ataques_torres[fin] & torres_n)) != 0)
                {
                    aclara = 1;
                    if ((POPCNT(torres_n & columna_mask[Columna(inicio)]) != 0))
                    {
                        planilla[1][ply_count] = b;
                    }
                    else
                    {
                        planilla[1][ply_count] = a;
                    }
                    planilla[2][ply_count] = c;
                    planilla[3][ply_count] = d;
                }
                else
                {
                    planilla[1][ply_count] = c;
                    planilla[2][ply_count] = d;
                }
            }
            if ((damas_n & SetMask(fin)))
            {
                planilla[0][ply_count] = 'Q';
                if (((POPCNT(ataques_alfiles[fin] & damas_n)) != 0) || (POPCNT(ataques_torres[fin] & damas_n)) != 0)
                {
                    aclara = 1;
                    if ((POPCNT(damas_n & columna_mask[Columna(inicio)]) != 0))
                    {
                        planilla[1][ply_count] = b;
                    }
                    else
                    {
                        planilla[1][ply_count] = a;
                    }
                    planilla[2][ply_count] = c;
                    planilla[3][ply_count] = d;
                }
                else
                {
                    planilla[1][ply_count] = c;
                    planilla[2][ply_count] = d;
                }
            }
            if (rey_n & SetMask(fin))
            {
                if (bien == enroque)
                {
                    if (fin == G8)
                    {
                        planilla[0][ply_count] = '0';
                        planilla[1][ply_count] = '-';
                        planilla[2][ply_count] = '0';
                    }
                    else
                    {
                        planilla[0][ply_count] = '0';
                        planilla[1][ply_count] = '-';
                        planilla[2][ply_count] = '0';
                        planilla[3][ply_count] = '-';
                        planilla[4][ply_count] = '0';
                    }
                }
                else
                {
                    planilla[0][ply_count] = 'K';
                    planilla[1][ply_count] = c;
                    planilla[2][ply_count] = d;
                }
            }
        }

        if (comio[0] && !(peones_n & SetMask(fin)) && (bien != corona))
        {
            planilla[6][ply_count] = planilla[5][ply_count];
            planilla[5][ply_count] = planilla[4][ply_count];
            planilla[4][ply_count] = planilla[3][ply_count];
            planilla[3][ply_count] = planilla[2][ply_count];
            if (aclara)
            {
                planilla[2][ply_count] = 'x';
            }
            else
            {
                planilla[2][ply_count] = planilla[1][ply_count];
                planilla[1][ply_count] = 'x';
            }
        }
        if (EnJaque(blancas))
        {
            i = 0;
            while (planilla[i][ply_count] != 0)
            {
                i++;
            };
            planilla[i][ply_count] = '+';
        }
    }
    ply_count++;                            //por ultimo incremento en 1 el contador de ply o medias jugadas
}

void ExportarPartida()
{
    char comillas = 34;
    unsigned int i;

    wcout << "[Event " << comillas << "?" << comillas << "]" << endl;  //primero hago el encabezado con los datos de la partida
    wcout << "[Site " << comillas << "?" << comillas << "]" << endl;
    wcout << "[Date " << comillas << "????.??.??" << comillas << "]" << endl;
    wcout << "[Round " << comillas << "?" << comillas << "]" << endl;
    if (colorpic == blancas)
    {
        wcout << "[White " << comillas << "Quasar" << comillas << "]" << endl;
        wcout << "[Black " << comillas << "Beta" << comillas << "]" << endl;
    }
    else
    {
        wcout << "[White " << comillas << "Beta" << comillas << "]" << endl;
        wcout << "[Black " << comillas << "Quasar" << comillas << "]" << endl;
    }
    switch (resultado)
    {
        case BLANCASGANAN:
        {
            wcout << "[Result " << comillas << "1-0" << comillas << "]" << endl;
        }break;
        case NEGRASGANAN:
        {
            wcout << "[Result " << comillas << "0-1" << comillas << "]" << endl;
        }break;
        case TABLAS:
        {
            wcout << "[Result " << comillas << "1/2-1/2" << comillas << "]" << endl;
        }break;
    }
    wcout << "[ECO " << comillas << "?" << comillas << "]" << endl;
    wcout << "[PlyCount " << comillas << ply_count << comillas << "]" << endl;
    if (strcmp(fen,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) //strcmp() devuelve 1 si son distintos los argum
        wcout << "[FEN " << comillas << fen << comillas << "]" << endl;         //si no se parte de la pos inicial indico la fen

    for (i=0;i<ply_count;i++)   //por ultimo escribo las jugadas de la partida
    {
        unsigned int j=0;
        while ((planilla[j][i] != 0))
        {
            j++;
            if (j == 7)
                break;
        };
        switch (j)
        {
            case 2:
            {
                wcout << planilla[0][i] << planilla[1][i] << endl;
            }break;
            case 3:
            {
                wcout << planilla[0][i] << planilla[1][i] << planilla[2][i] << endl;
            }break;
            case 4:
            {
                wcout << planilla[0][i] << planilla[1][i] << planilla[2][i] << planilla[3][i] << endl;
            }break;
            case 5:
            {
                wcout << planilla[0][i] << planilla[1][i] << planilla[2][i] << planilla[3][i] << planilla[4][i] << endl;
            }break;
            case 6:
            {
                wcout << planilla[0][i] << planilla[1][i] << planilla[2][i] << planilla[3][i] << planilla[4][i] << planilla[5][i] << endl;
            }break;
            case 7:
            {
                wcout << planilla[0][i] << planilla[1][i] << planilla[2][i] << planilla[3][i] << planilla[4][i] << planilla[5][i] << planilla[6][i] << endl;
            }break;
        }
    }
}

BYTE Legal()											//se fija si la jugada es totalmente correcta en terminos legales (FALSE si es ilegal y TRUE si es legal)
{
	semibien = SemiLegal(0);							//q llame a la comprobacion de semilegalidad (sin tener en cuenta jaques)
	if (semibien)
	{
		HacerJugada(inicio,fin,semibien,0);				//esto es para ver si luego de jugarla queda en jaque
		if (turno_c == blancas)
		{
			if (EnJaque(blancas))						//esta es la forma correcta de llamar a EnJaque() para ver si las blancas estan en jaque
			{
				DeshacerJugada(inicio,fin,semibien,0);	//despues la hago como debe ser con Jugar()
				return ilegal;
			}
			DeshacerJugada(inicio,fin,semibien,0);
			return semibien;							//no solo devuelve TRUE sino q tambien que tipo de jugada fue
		}
		else
		{
			if (EnJaque(negras))
			{
				DeshacerJugada(inicio,fin,semibien,0);
				return ilegal;
			}
			DeshacerJugada(inicio,fin,semibien,0);
			return semibien;
		}
	}
	else												//si ni siquiera es semilegal
	{
		return ilegal;
	}
}

BYTE SemiLegal(BYTE prof)		//solo se preocupa del correcto movimiento de las piezas
{
	switch (pieza)
	{
		case PEON:
		{
			if (turno_c == blancas)
			{
				if (mov_peones[blancas][inicio] & SetMask(fin))
				{
					if ((SetMask(inicio) & fila_mask[fila2]) && (SetMask(fin) & fila_mask[fila4]))	//si se cumple esto es movimiento de a 2 casillas
					{
						if (tablero & dospasos[blancas][Columna(inicio)])				//si esta ocupada la casilla de paso del peon o la de fin es ilegal y sino esta todo bien
							return ilegal;
						return doble;													//indica q se avanzo de a 2 pasos un peon por lo q hay q modificar alpaso
					}
					if (Columna(inicio) != Columna(fin))								//esto se da cuando captura (normal o alpaso)
					{
						if (SetMask(fin) & piezas_n)									//si hay una pieza contraria para comer es semilegal
						{
							if (SetMask(fin) & fila_mask[fila8])						//si el peon llego a octava
								return corona;											//que indique coronacion
							return normal;
						}
						else															//sino, falta ver si es al paso
						{
							if ((Columna(fin) == alpaso[prof]) && (Fila(inicio) == fila5))	//alpaso vale 0 para la columna A, hasta 7 en la H. Si no hay posibilidad vale 8
								return ap;															//indico q fue al paso
							return ilegal;
						}
					}
					if (tablero & SetMask(fin))	//si no es ninguno de los anteriores es un movimiento de a una casilla normal asi q miro q no este ocupada la casilla de destino
						return ilegal;
					if (SetMask(fin) & fila_mask[fila8])								//si el peon llego a octava
						return corona;													//que indique coronacion
					return normal;
				}
				return ilegal;
			}
			else
			{
				if (mov_peones[negras][inicio] & SetMask(fin))
				{
					if ((SetMask(inicio) & fila_mask[fila7]) && (SetMask(fin) & fila_mask[fila5]))
					{
						if (tablero & dospasos[negras][Columna(inicio)])	//si esta ocupada la casilla de paso del peon o la de fin es ilegal y sino esta todo bien
							return ilegal;
						return doble;
					}
					if (Columna(inicio) != Columna(fin))				//esto se da cuando captura (normal o alpaso)
					{
						if (SetMask(fin) & piezas_b)					//si hay una pieza contraria para comer es semilegal
						{
							if (SetMask(fin) & fila_mask[fila1])
								return corona;
							return normal;
						}
						else											//sino, falta ver si es al paso
						{
							if ((Columna(fin) == alpaso[prof]) && (Fila(inicio) == fila4))	//alpaso vale 0 para la columna A, hasta 7 en la H. Si no hay posibilidad vale 8
								return ap;															//indico q fue al paso
							return ilegal;
						}
					}
					if (tablero & SetMask(fin))	//si no es ninguno de los anteriores es un movimiento de a una casilla normal asi q miro q no este ocupada la casilla de destino
						return ilegal;
					if (SetMask(fin) & fila_mask[fila1])
						return corona;
					return normal;
				}
				return ilegal;
			}
		}break;
		case CABALLO:
		{
			if (turno_c == blancas)
			{
				if (ataques_caballos[inicio] & SetMask(fin) & (~piezas_b))	//si "fin" esta atacada x un caballo desde "inicio", y ademas "fin" no tiene unaa pieza blanca
					return normal;	//es semi-legal y no tiene nada raro la jugada
				return ilegal;		//sino se cumple algo es ilegal
			}
			else
			{
				if (ataques_caballos[inicio] & SetMask(fin) & (~piezas_n))
					return normal;		//es semi-legal
				return ilegal;		//es ilegal
			}
		}break;
		case ALFIL:
		{
			if (ataques_alfiles[inicio] & SetMask(fin))		//se cumple si el alfil movio como alfil
			{
				if (ChequeaDiagonal())
					return normal;
			}
			return ilegal;
		}break;
		case TORRE:
		{
			if (ataques_torres[inicio] & SetMask(fin))		//se cumple si la torre movio como torre
			{
				if (ChequeaOrtogonal())
					return movtorre;
			}
			return ilegal;
		}break;
		case DAMA:
		{
			if (ataques_torres[inicio] & SetMask(fin))	//si movio como torre
			{
				if (ChequeaOrtogonal())
					return normal;
			}
			if (ataques_alfiles[inicio] & SetMask(fin))	//si movio como alfil
			{
				if (ChequeaDiagonal())
					return normal;
			}
			return ilegal;	//si no movio de ninguna de las 2 formas es ilegal
		}break;
		case REY:
		{
			if (turno_c == blancas)
			{
				if (ataques_rey[inicio] & SetMask(fin) & (~piezas_b))			//esto contempla todo menos enroques
					return movrey;
				if ((derechos_enroque[prof] & 0x01) && (fin == G1))		//entra si es intento de enroque corto blanco y todavia es posible
				{
					if (OO[blancas] & tablero)									//si hay una pieza interponiendose en el camino es ilegal
						return ilegal;
					if(CasillaAtacada(E1,negras) || CasillaAtacada(F1,negras))	//la comprobacion de jaque luego de jugar se la dejo a EnJaque() como el resto de las jugadas
						return ilegal;
					if (torres_b & SetMask(H1))									//por ultimo miro q la torre este en el lugar (q no se la hayan comido antes sin darme cuenta)
						return enroque;
					return ilegal;
				}
				if ((derechos_enroque[prof] & 0x02) && (fin == C1))		//entra si es intento de enroque largo blanco y todavia es posible
				{
					if (OOO[blancas] & tablero)
						return 0;
					if (CasillaAtacada(E1,negras) || CasillaAtacada(D1,negras))
						return 0;
					if (torres_b & SetMask(A1))
						return enroque;
					return ilegal;
				}
				return ilegal;
			}
			else
			{
				if (ataques_rey[inicio] & SetMask(fin) & (~piezas_n))
					return movrey;
				if ((derechos_enroque[prof] & 0x04) && (fin == G8))		//entra si es intento de enroque corto negro y todavia es posible
				{
					if (OO[negras] & tablero)
						return ilegal;
					if(CasillaAtacada(E8,blancas) || CasillaAtacada(F8,blancas))
						return ilegal;
					if (torres_n & SetMask(H8))
						return enroque;
					return ilegal;
				}
				if ((derechos_enroque[prof] & 0x08) && (fin == C8))
				{
					if (OOO[negras] & tablero)
						return ilegal;
					if(CasillaAtacada(E8,blancas) || CasillaAtacada(D8,blancas))
						return ilegal;
					if (torres_n & SetMask(A8))
						return enroque;
					return ilegal;
				}
				return ilegal;
			}
		}break;
	}
	return ilegal;  //aca no se deberia llegar pero por las dudas...
}

BYTE ChequeaDiagonal()
{
	BITBOARD bloqueos, ataque_diag;
	BYTE casilla_bloqueada;

	if (Fila(inicio) < Fila(fin))						//si el movimiento es hacia arriba
	{
		if (Columna(inicio) < Columna(fin))				//si el movimiento es hacia arriba a la derecha
		{
			bloqueos = mas9dir[inicio] & tablero;		//mira en donde hay piezas bloqueando la accion del alfil en la direccion diagonal a1-h8
			casilla_bloqueada = LSB(bloqueos);			//la primera es la que importa
			ataque_diag = mas9dir[inicio];				//ataque_diag es la bitboard q tiene los datos actualizados (no voy a desconfigurar los plus que son fijos)
			ataque_diag ^= mas9dir[casilla_bloqueada];	//solo deja "vivos" los bits anteriores al bloqueo y el bloqueo mismo (son a los q realmente ataca el alfil)
		}
		else											//arriba a la izquierda
		{
			bloqueos = mas7dir[inicio] & tablero;		//lo mismo para el movimiento diagonal con sentido h1-a8
			casilla_bloqueada = LSB(bloqueos);
			ataque_diag = mas7dir[inicio];
			ataque_diag ^= mas7dir[casilla_bloqueada];
		}
	}
	else												//abajo
	{
		if (Columna(inicio) < Columna(fin))				//si el movimiento es hacia abajo a la derecha
		{
			bloqueos = menos7dir[inicio] & tablero;		//lo mismo para el movimiento diagonal con sentido a8-h1
			casilla_bloqueada = MSB(bloqueos);			//solo q aca hay q buscar el primer bit MAS significativo para q tenga sentido
			ataque_diag = menos7dir[inicio];
			ataque_diag ^= menos7dir[casilla_bloqueada];
		}
		else	//abajo a la izquierda
		{
			bloqueos = menos9dir[inicio] & tablero;		//lo mismo para el movimiento diagonal con sentido h8-a1
			casilla_bloqueada = MSB(bloqueos);
			ataque_diag = menos9dir[inicio];
			ataque_diag ^= menos9dir[casilla_bloqueada];
		}
	}
	if (turno_c == blancas)
	{
		if (ataque_diag & SetMask(fin) & (~piezas_b))
			return 1;
		return 0;
	}
	else
	{
		if (ataque_diag & SetMask(fin) & (~piezas_n))
			return 1;
		return 0;
	}
}

BYTE ChequeaOrtogonal()
{
	BITBOARD bloqueos, ataque_ortogonal;
	BYTE casilla_bloqueada;

	if (Fila(inicio) < Fila(fin))							//si el movimiento es hacia arriba (solo puede entrar en uno de estos 4 if de direccion)
	{
		bloqueos = mas8dir[inicio] & tablero;				//mira en donde hay piezas bloqueando la accion de la torre en la direccion vertical hacia arriba
		casilla_bloqueada = LSB(bloqueos);					//la primera es la que importa
		ataque_ortogonal = mas8dir[inicio];					//ataque_ortogonal es la bitboard q tiene los datos actualizados (no voy a desconfigurar los plus que son fijos)
		ataque_ortogonal ^= mas8dir[casilla_bloqueada];		//solo deja "vivos" los bits anteriores al bloqueo y el bloqueo mismo (son a los q realmente ataca la torre)
	}
	if (Fila(inicio) > Fila(fin))							//si el movimiento es hacia abajo
	{
		bloqueos = menos8dir[inicio] & tablero;
		casilla_bloqueada = MSB(bloqueos);
		ataque_ortogonal = menos8dir[inicio];
		ataque_ortogonal ^= menos8dir[casilla_bloqueada];
	}
	if (Columna(inicio) < Columna(fin))						//si el movimiento es hacia la derecha
	{
		bloqueos = mas1dir[inicio] & tablero;
		casilla_bloqueada = LSB(bloqueos);
		ataque_ortogonal = mas1dir[inicio];
		ataque_ortogonal ^= mas1dir[casilla_bloqueada];
	}
	if (Columna(inicio) > Columna(fin))						//si el movimiento es hacia la izquierda
	{
		bloqueos = menos1dir[inicio] & tablero;
		casilla_bloqueada = MSB(bloqueos);
		ataque_ortogonal = menos1dir[inicio];
		ataque_ortogonal ^= menos1dir[casilla_bloqueada];
	}
	if (turno_c == blancas)
	{
		if (ataque_ortogonal & SetMask(fin) & (~piezas_b))
			return 1;
		return 0;
	}
	else
	{
		if (ataque_ortogonal & SetMask(fin) & (~piezas_n))
			return 1;
		return 0;
	}
}

void Guardar()											//esto sirve para guardar el estado actual de la partida por si se tiene q salir con longjmp de NegaMax
{														//el resto de los registros importantes ya estan guardados en la funcion de 3 repet
	turno1 = turno;										//estos tambien los guardo pero no puedo usar registros de las 3 repet
	comio1 = comio[0];									//xq estos no influyen en esa funcion para nada
	jugadas_reversibles1 = jugadas_reversibles;
	tab90 = tablero90;
	tabA1 = tableroA1;
	tabA8 = tableroA8;
	hash_pos2 = hash_pos;                               //tambien guardo la clave hash
}

void Recuperar()
{
	peones_b = piezas_anteriores[0][0];					//recupero la posicion de las piezas del tablero actual
	caballos_b = piezas_anteriores[0][1];
	alfiles_b = piezas_anteriores[0][2];
	torres_b = piezas_anteriores[0][3];
	damas_b = piezas_anteriores[0][4];
	rey_b = piezas_anteriores[0][5];

	peones_n = piezas_anteriores[0][6];
	caballos_n = piezas_anteriores[0][7];
	alfiles_n = piezas_anteriores[0][8];
	torres_n = piezas_anteriores[0][9];
	damas_n = piezas_anteriores[0][10];
	rey_n = piezas_anteriores[0][11];

	turno_c = turnos_anteriores[0];						//tambien el turno del color
	derechos_enroque[0] = enroques_anteriores[0];		//los derechos de enroque
	alpaso[0] = alpaso_anteriores[0];					//y la posibilidad de peon al paso


	turno = turno1;										//y recupero los registros q no estan guardados en 3 repet
	comio[0] = comio1;
	jugadas_reversibles = jugadas_reversibles1;

	piezas_b = peones_b | caballos_b | alfiles_b | torres_b | damas_b | rey_b;	//por ultimo reconstruyo piezas_b,n y el tablero
	piezas_n = peones_n | caballos_n | alfiles_n | torres_n | damas_n | rey_n;
	tablero = piezas_b | piezas_n;
	tablero90 = tab90;
	tableroA1 = tabA1;
	tableroA8 = tabA8;
	hash_pos = hash_pos2;                               //recupero la clave hash
}

void BorrarVP()						//deja en 0 todas las estructuras relacionadas con la variante principal xq no se ha analizado nada aun
{
	BYTE i,j;

	for (i=0;i<MAXPROF;i++)
	{
		vp[i][0] = 64;				//como siempre 64 es la casilla prohibida q indica el fin de la variante (no se analizo mas alla de esto)
		vp[i][1] = 64;
		vp[i][2] = 64;
		for (j=0;j<MAXPROF;j++)
		{
			pv[i][j] = 0;			//para borrar este hay q usar el 0
		}
	}
}

void DescomprimirVP()					//pasa del formato de la pv al q se necesita para realizar las jugadas (que es el de vp)
{
	BYTE i = 0;

    if (salir == 0)                         //solo si no se aborto Negamax por cuestiones de tiempo, y se salio normalmente hay q actualizar vp
    {
        while (pv[0][i] != 0)				//mientras no se alcance el fin de la variante se copia
        {
            vp[i][0] = pv[0][i]>>12;		//casilla de inicio
            vp[i][1] = (pv[0][i]>>6)&0x3f;	//casilla de fin
            vp[i][2] = pv[0][i]&0x3f;		//tipo de movimiento
            i++;
        }
    }

	inicio = vp[0][0];					//tambien dejo listo para que Jugar() tenga los datos para hacer la jugada en la misma forma q el user las hace
	fin = vp[0][1];
	bien = vp[0][2];

    a = Columna(inicio)+0x61;           //y tambien dejo los registros en ASCII para la comunicacion con el user y demas
    b = Fila(inicio)+0x31;
    c = Columna(fin)+0x61;
    d = Fila(fin)+0x31;
}

void Analiza()							//busca la "mejor" jugada q se pueda (aca esta toda la magia y los errores jeje)
{
    BYTE prof_max2 = 0;
    int i;

    QueryPerformanceCounter((LARGE_INTEGER *)&t_inicio);    //inicio el contador de tiempo
//	if (Libro())						//si se dan las condiciones para q encuentre una jugada de libro q la haga
//		return;							//y retorne inmediatamente
	//inicializo todos los registros necesarios para hacer una nueva busqueda
	BorrarVP();
	eficiencia_1 = 0;
	eficiencia_2 = 0;
	tt_hits = 0;                        //cantidad de coincidencias de la tt durante la busqueda
	prof_max = 0;						//comienzan las iteraciones desde profundidad de hojas 1 (profundidad se incrementa antes de llamar a MinMax)
	nulo = 0;
	mcut = 1;
//	ext = 0;
	unica = 0;							//no se sabe todavia si la posicion raiz tiene una unica jugada o no
	for (i=0;i<MAXPROF;i++)
	{
		legales[i] = 0;
	}
	cant_analisis++;					//incremento la cantidad de veces q se entro a analiza (para tener despues el promedio de profundidad)
	Guardar();							//guardamos todos los registros necesarios en caso de que tengamos q salir precipitadamente
	//ahora la parte del control de tiempo
	salir = 0;							//el analisis recien comienza asi q no hay q salir
	setjmp(env);						//aca se retorna si por alguna razon se debe dejar de analizar
	if (salir == 1)						//si estamos aca producto de que quisimos salir retornamos (ya tenemos una jugada)
	{
		saliointerrumpido++;			//para ver cuantas veces sale asi y cuantas de la otra forma
		prof_max = prof_max2;
		proftotal += prof_max -1;		//la profundidad a la q se evaluo la jugada es una menos q la q hay xq se interrumpio la busqueda
		profmedia = proftotal/cant_analisis;		//para ver a q nivel aproximado esta jugando la partida
        valoracion = estimo;            //para q muestre correctamente la valoracion de la anterior iteracion
		Recuperar();					//ahora recuperamos todos los registros
        DescomprimirVP();
		if(eficiencia_1 != 0)			//como podemos salir por los 2 lados aca tambien calculo la eficiencia de ordenamiento
			eficiencia = (eficiencia_2*100 ) / eficiencia_1;
		return;							//nos vamos incondicionalmente
	}
	do																				//aca empieza la busqueda en profundidad iterada
	{
		prof_max++;
		prof_max2 = prof_max;           //la guardo xq prof_max puede variar dentro de Negamax debido a LMR y poda de mov nulo
		nodos = 0;
		Qnodos = 0;
		hojastt_hits = 0;
		evaltt_hits = 0;
		qpasaaca = 0;
		llamadasevaluar = 0;
		nodosnulos = 0;
		cortesnulo = 0;
		cortes_inut_inversa = 0;
		hash_pos2 = hash_pos;
//		valoracion = Raiz(-30000,30000,0);
//		nodos = Perft(0);												    		//perft para probar el generador de movimientos cuando cambio algo

		if (prof_max != 1)															//si ya se tiene una estimacion del valor de la posicion en una previa iteracion
		{
			cantventanas++;
			valoracion = Raiz(estimo-40,estimo+40,prof_max);	          			//aplico una ventana de aspiracion hacia cada lado
			if (valoracion >= estimo + 40 || valoracion <= estimo - 40)				//si se salio de la ventana
			{
				Recuperar();														//aca parece q puede fallar feo PVS junto con la ventana de aspiracion asi q recupero x si acaso
				falloventana++;
				valoracion = Raiz(-30000,30000,prof_max);	    					//mala suerte, hay q calcular de nuevo con una ventana totalmente abierta
			}
		}
		else																		//si es la primer iteracion no se puede aplicar ventana xq no hay valor estimado
		{
			valoracion = Raiz(-30000,30000,prof_max);   							//asi q llame a la busqueda Negamax con poda alfa-beta y ventana abierta (-inf,+inf)
		}
		estimo = valoracion;														//estimo se usa en la ventana de aspiracion como valor medio de referencia
		DescomprimirVP();														    //y actualizo la vp
	}while(abs(valoracion) < 29000 && !unica && prof_max < 20);
    //sale si es unica o si vio mate o si alcanza profundidad fija
	salionormal++;												//para ver cuantas veces sale asi y cuantas de la otra forma
	if(eficiencia_1 != 0)
		eficiencia = (eficiencia_2*100 ) / eficiencia_1;
	proftotal += prof_max;						//la profundidad a la q se evaluo la jugada es prof_max xq no se interrumpio la busqueda
	profmedia = proftotal/cant_analisis;		//para ver a q nivel aproximado esta jugando la partida
//    QueryPerformanceCounter((LARGE_INTEGER *)&t_fin);   //paro el reloj
    t_transcurrido = ((t_fin - t_inicio) * timerFrequency);     //miro cuanto tiempo se demoro en esta jugada
}

int Raiz(int alfa,int beta,BYTE draft)        //alfa y beta son los limites, prof siempre es 0 pq estamos en la raiz y draft
{                                             //la distancia a las hojas del arbol
	BYTE j;
	unsigned int z;
//#ifdef USAR_TT
//	int bandera_hash = TT_ALFA;                                         //es nodo alfa hasta q se demuestre lo contrario
//
//    if ((val = Probar_tt(draft, alfa, beta)) != INVALIDO)   //pruebo si hay hit en la tt para ahorrar trabajo
//    {
//        tt_hits++;
//        return val;
//    }
//#endif // USAR_TT
	GenerarTodas(0);
	Ordenar(0);
	for (z = 0 ; z < ultimajugada[0] ; z = z + 3)
	{
		HacerJugada(jugadas[0][z],jugadas[0][z+1],jugadas[0][z+2],0);
		if (!EnJaque(turno_c))												//si el rey del bando q jugo no esta en jaque es totalmente legal
		{
			if (legales[0] == 0)									//si es la primer jugada en considerar (variante principal)
				valoracion = -NegaMax(-beta,-alfa,1,draft-1);   	//examino con ventana normal (para obtener el valor verdadero de la posicion)
			else													//sino
			{
			    if (legales[0] > 3 && draft > 2 && LMR_ok(0,z))
                {
                    prof_max--;
                    valoracion = -NegaMax(-(alfa+1),-alfa,1,draft-2);    //aplico LMR con ventana minima sobre alfa
                    prof_max++;
                }
                else
                    valoracion = alfa + 1;                              //esto asegura la re busqueda cuando corresponda
                if (valoracion > alfa)                                  //aca se entra si fallo LMR o si no se aplico
                {
                    valoracion = -NegaMax(-(alfa+1),-alfa,1,draft-1);	//ventana minima sobre alfa (considero q la primer jugada buscada es mejor en base a buen ordenamiento)
                    if (valoracion > alfa)								//si supero alfa entonces hay q hacer nueva busqueda (fallo la ventana minima)
                        valoracion = -NegaMax(-beta,-alfa,1,draft-1);	//busco nuevamente con ventana normal
                }
			}
			DeshacerJugada(jugadas[0][z],jugadas[0][z+1],jugadas[0][z+2],0);
			legales[0]++; 				//hay una jugada legal posible (no es la cantidad xq alfa-beta las reduce pero si sirve para detectar mates, ahogados y unicas)
			if (valoracion >= beta)					//fail high beta cutoff
			{
			    if (!(SetMask(jugadas[0][z+1]) & tablero))        //si la jugada no es una captura
                {
                    if ((jugadas[0][z] + (jugadas[0][z+1]<<8)) != killer[0][0]) //si la killer es distinta de la actual
                    {
                        killer[0][1] = killer[0][0];                            //paso la killer anterior al nivel bajo
                        killer[0][0] = jugadas[0][z] + (jugadas[0][z+1]<<8);    //marco nueva jugada "killer"
                    }
                    historia[turno_c][jugadas[0][z]][jugadas[0][z+1]] += draft*draft;  //tambien incremento los contadores
                }
			    mejor_jugada = jugadas[0][z] + (jugadas[0][z+1]<<8);    //guardo inicio y fin
				eficiencia_1++;
				if (legales[0] == 1)		//si el beta cutoff ocurrio en la primera jugada
					eficiencia_2++;					//punto a favor para el ordenamiento
				legales[0] = 0;			//antes de hacer el corte hay q reiniciar legales para q no se produzcan conflictos posteriores
//#ifdef USAR_TT
//                Save_tt(draft,beta,TT_BETA,mejor_jugada);     //guardo la posicion en la tabla hash
//#endif // USAR_TT
				return beta;						//poda en la busqueda producto de q el nodo supero beta (por lo tanto no lo va a elegir el bando q juega)
			}
			if (valoracion > alfa)
			{
				alfa = valoracion;					//alfa funciona como max en NegaMax
//#ifdef USAR_TT
//				bandera_hash = TT_EXACTO;
//#endif // USAR_TT
				mejor_jugada = jugadas[0][z] + (jugadas[0][z+1]<<8);
                //actualizo pv
                pv[0][0] = (jugadas[0][z]<<12)|(jugadas[0][z+1]<<6)|(jugadas[0][z+2]);
                j = 1;
                while (j != prof_max)
                {
                    pv[0][j] = pv[1][j];
                    j++;
                }
			}
		}
		else										//si es ilegal se deshace y se pasa a la siguiente jugada en la lista
		{
			DeshacerJugada(jugadas[0][z],jugadas[0][z+1],jugadas[0][z+2],0);
		}
	}
//#ifdef USAR_TT
//	Save_tt(draft,alfa,bandera_hash,mejor_jugada); //guarda la posicion en la tt indicando el tipo de nodo q es
//#endif // USAR_TT
	if (legales[0] == 0)					//si a esta profundidad no se puede realizar ninguna jugada legal es ahogado o mate
	{
		if (EnJaque(turno_c^1))
			return - MATE;      			//es mate en la profundidad actual (esto hace q el mate en 1 sea mejor q el mate en 2, etc.)
		else
			return 0;						//ahogado (puede ser 0 o algun contempt ligeramente negativo)
	}
	if (legales[0] == 1)            		//si en la posicion raiz solo hay una jugada posible
		unica = 1;							//lo indico para que ya no piense mas y la realize inmediatamente
	legales[0] = 0; 						//la dejo lista para la proxima pasada a esta profundidad
//	if (cincuenta >= 100)					//si se detecta una variante en la q aplica la regla de 50 jugadas y no hay mate entonces la posicion es tablas
//		return 0;
	return alfa;
}

int NegaMax(int alfa,int beta,BYTE prof,BYTE draft)     //alfa y beta son los limites, prof es la distancia a la raiz y draft
{                                                       //la distancia a las hojas del arbol
	BYTE j;
	unsigned int z;
#ifdef PODAR_INUTILIDAD
    static int fmargen[4] = {0,200,500,900};            //el margen de poda de inutilidad (futility prunning)
    int podar = 0;
#endif // PODAR_INUTILIDAD
#ifdef PODAR_INUTILIDAD_INVERSA
//    static int fmargen[4] = {0,200,500,900};            //el margen de poda de inutilidad (futility prunning)
#endif // PODAR_INUTILIDAD_INVERSA
#ifdef USAR_TT
	int bandera_hash = TT_ALFA;                         //es nodo alfa hasta q se demuestre lo contrario
#endif // USAR_TT

	if (prof == 1 && Repite())							//primero miro si esta analizando una repeticion. Solo las considero a profundidad 1 q es cuando el pic
		return TABLAS;									//acaba de analizar su primer jugada y si es una repeticion q la valore como tablas (aunque no sea todavia)
#ifdef USAR_TT
    if ((val = Probar_tt(draft, alfa, beta)) != INVALIDO)   //pruebo si hay hit en la tt para ahorrar trabajo
    {
        tt_hits++;
        return val;
    }
#endif // USAR_TT
	if (draft == 0)// + ext)           //no tiene sentido reducir y extender una jugada asi q si ext == 1 entonces redu == 0 y viceversa
	{
		nodos++;
#ifdef USAR_RELOJ
		if ((nodos & 1023) == 0)
            ManejarReloj();
#endif // USAR_RELOJ
#ifdef USAR_TT
        if ((val = Probar_hojastt(alfa, beta)) != INVALIDO)  //miro si hay hit en la hojastt
        {
            hojastt_hits++;
            return val;
        }
#endif // USAR_TT
        val = Quies(alfa,beta,0);
#ifdef USAR_TT
        Save_hojastt(alfa,beta,val);          //esto hay q mirarlo bien xq es complejo como todo lo de las tt
#endif // USAR_TT
        return val;
//		return (Quies(alfa,beta,0));
	}
	qpasaaca++;
#ifdef PODAR_INUTILIDAD_INVERSA
	if (draft < 4 && EvaluarRapido()-fmargen[draft] >= beta && (!EnJaque(turno_c^1)) && abs(alfa) < 29000)
    {
        cortes_inut_inversa++;
        return beta;
    }
#endif // PODAR_INUTILIDAD_INVERSA
#ifdef USAR_NULO
	if (Nulo_ok(prof) && !EnJaque(turno_c^1))					//miro si es legal y deseado hacer un movimiento nulo
	{
		nulo = 1;														//advierto q ya hice un mov nulo en esta rama asi q no hay que hacer otro
		nodosnulos++;
		turno_c ^= 1;													//hago el movimiento nulo (cambiar el turno del q juega)
        alpaso[prof+1] = 8;
        hash_pos ^= Zobrist.bando;
        hash_pos ^= Zobrist.alpaso[alpaso[prof]];
        hash_pos ^= Zobrist.alpaso[alpaso[prof+1]];
        prof_max -= R;
		valoracion = -NegaMax(-beta,-beta+1,prof+1,draft-1-R);        	    //movimiento nulo con ventana m�nima alrededor de beta y reduccion R
        prof_max += R;
        hash_pos ^= Zobrist.bando;
        hash_pos ^= Zobrist.alpaso[alpaso[prof+1]];
        hash_pos ^= Zobrist.alpaso[alpaso[prof]];
		turno_c ^= 1;													//deshago el movimiento nulo
		nulo = 0;														//indico q ya no se esta analizando un mov nulo
		if (valoracion >= beta) 										//podar en caso de superar o igualar beta
		{
			cortesnulo++;
			return valoracion;
		}
	}
#endif // USAR_NULO
	GenerarTodas(prof);
	Ordenar(prof);
    //ahora aplicamos la idea de MultiCut para ver si podemos asumir sin mucho riesgo a equivocarnos q este nodo es
    //un CUT nodo es decir q supera beta y por lo tanto podemos retornar simplemente beta sin mas evaluacion.
    //parametros variables: m(cant jugadas a analizar hasta renunciar), c(la cantidad de sub-cortes necesarios), r(la reduccion)
#ifdef USAR_MULTICUT
	if (draft > 2 && mcut && ultimajugada[prof] > 60 && !EnJaque(turno_c^1))
    {
        int c = 0, m = 0;           //iniciamos el contador de nodos hijos exitosos c y el de jugadas a expandir m
        mcut = 0;                   //aclaramos q estamos haciendo multi cut para q no se aniden varios
        for (z = 0 ; z < ultimajugada[prof] ; z = z + 3)
        {
            HacerJugada(jugadas[prof][z],jugadas[prof][z+1],jugadas[prof][z+2],prof);
            if (!EnJaque(turno_c))
            {
                prof_max -= R;
                valoracion = -NegaMax(-beta,-beta+1,prof+1,draft-1-R);  //multicut con reduccion R
                prof_max += R;
                DeshacerJugada(jugadas[prof][z],jugadas[prof][z+1],jugadas[prof][z+2],prof);
                if (valoracion >= beta) //si este nodo hijo supero beta lo a�adimos al contador de exitos c
                {
                    if (++c == 3)           //como condicion al menos 3 hijos de la posicion actual deben superar beta
                    {
                        mcut = 1;           //indicamos q se puede volver a probar multicut en otra ocasion
                        return beta;        //corte del algoritmo multicut
                    }
                }
                if (++m == 10)              //solo miramos las primeras 10 jugadas legales m = 10
                {
                    mcut = 1;
                    break;                  //salimos. fallo multi cut por lo que seguimos con la busqueda normal
                }
            }
            else
            {
                DeshacerJugada(jugadas[prof][z],jugadas[prof][z+1],jugadas[prof][z+2],prof);
            }
        }
        mcut = 1;   //aca se llega en el raro caso de q no haya 10 jugadas legales (se q hay minimo 20 semilegales)
    }
#endif // USAR_MULTICUT
#ifdef PODAR_INUTILIDAD
    //ahora vemos si se cumplen las condiciones para realizar la poda de inutilidad o futility prunning
    if ((draft) <= 2 &&
        (!EnJaque(turno_c^1)) && abs(alfa) < 29000 && EvaluarRapido()+fmargen[draft] <= alfa)
        podar = 1;                  //si se desea podar entonces avisamos q las jugadas de este nodo pueden ser podadas
#endif // PODAR_INUTILIDAD
	for (z = 0 ; z < ultimajugada[prof] ; z = z + 3)
	{
#ifdef PODAR_INUTILIDAD
	    if (podar && (legales[prof] != 0) && (jugadas[prof][z+2] != corona) &&    //si la jugada califica para ser
            ((tablero & SetMask(jugadas[prof][z+1])) == 0))                       //podada entonces
            continue;                   //esto realiza la poda de la jugada (ni siquiera se llego a realizar en el tablero)
#endif // PODAR_INUTILIDAD
		HacerJugada(jugadas[prof][z],jugadas[prof][z+1],jugadas[prof][z+2],prof);
		if (!EnJaque(turno_c))												//si el rey del bando q jugo no esta en jaque es totalmente legal
		{
//			if ((prof == prof_max - 1) && EnJaque(turno_c^1))		//si es un nodo horizonte y hay jaque entonces extiendo 1 ply para ver q pasa
//				ext = 1;
			if (legales[prof] == 0)									//si es la primer jugada en considerar (variante principal)
				valoracion = -NegaMax(-beta,-alfa,prof+1,draft-1);	//examino con ventana normal (para obtener el valor verdadero de la posicion)
			else													//sino
			{
			    if (legales[prof] > 3 && draft > 2 && LMR_ok(prof,z))
                {
                    prof_max--;
                    valoracion = -NegaMax(-(alfa+1),-alfa,prof+1,draft-2);    //aplico LMR con ventana minima sobre alfa
                    prof_max++;
                }
                else
                    valoracion = alfa + 1;                              //esto asegura la re busqueda cuando corresponda
                if (valoracion > alfa)                                  //aca se entra si fallo LMR o si no se aplico
                {
                    valoracion = -NegaMax(-(alfa+1),-alfa,prof+1,draft-1);	//ventana minima sobre alfa (considero q la primer jugada buscada es mejor en base a buen ordenamiento)
                    if (valoracion > alfa)								//si supero alfa entonces hay q hacer nueva busqueda (fallo la ventana minima)
                        valoracion = -NegaMax(-beta,-alfa,prof+1,draft-1);	//busco nuevamente con ventana normal
                }
			}
//			if (prof == prof_max - 1)								//si es nodo (frontera - 1) borro las extensiones q pudiera haber (para q no extienda la siguiente)
//				ext = 0;
			DeshacerJugada(jugadas[prof][z],jugadas[prof][z+1],jugadas[prof][z+2],prof);
			legales[prof]++; 				//hay una jugada legal posible (no es la cantidad xq alfa-beta las reduce pero si sirve para detectar mates, ahogados y unicas)
			if (valoracion >= beta)					//fail high beta cutoff
			{
			    if (!(SetMask(jugadas[prof][z+1]) & tablero))        //si la jugada no es una captura
                {
                    if ((jugadas[prof][z] + (jugadas[prof][z+1]<<8)) != killer[prof][0])   //si la killer es dis
                    {                                                                                           //tinta de la actual
                        killer[prof][1] = killer[prof][0];        //paso la killer anterior al nivel bajo
                        killer[prof][0] = jugadas[prof][z] + (jugadas[prof][z+1]<<8); //marco nueva jugada "killer"
                    }
                    historia[turno_c][jugadas[prof][z]][jugadas[prof][z+1]] += draft*draft;  //tambien incremento los contadores
                }
			    mejor_jugada = jugadas[prof][z] + (jugadas[prof][z+1]<<8);    //guardo inicio y fin
				eficiencia_1++;
				if (legales[prof] == 1)		//si el beta cutoff ocurrio en la primera jugada
					eficiencia_2++;					//punto a favor para el ordenamiento
				legales[prof] = 0;			//antes de hacer el corte hay q reiniciar legales para q no se produzcan conflictos posteriores
#ifdef USAR_TT
                Save_tt(draft,beta,TT_BETA,mejor_jugada);     //guardo la posicion en la tabla hash
#endif // USAR_TT
				return beta;						//poda en la busqueda producto de q el nodo supero beta (por lo tanto no lo va a elegir el bando q juega)
			}
			if (valoracion > alfa)
			{
				alfa = valoracion;					//alfa funciona como max en NegaMax
#ifdef USAR_TT
				bandera_hash = TT_EXACTO;
#endif // USAR_TT
				mejor_jugada = jugadas[prof][z] + (jugadas[prof][z+1]<<8);
//				if (!ext)							//si no estamos en una extension actualizo la pv
//				{
					pv[prof][prof] = (jugadas[prof][z]<<12)|(jugadas[prof][z+1]<<6)|(jugadas[prof][z+2]);
					j = prof + 1;
					while (j != prof_max)
					{
						pv[prof][j] = pv[prof+1][j];
						j++;
					}
//				}
			}
		}
		else										//si es ilegal se deshace y se pasa a la siguiente jugada en la lista
		{
			DeshacerJugada(jugadas[prof][z],jugadas[prof][z+1],jugadas[prof][z+2],prof);
		}
	}
	if (legales[prof] == 0)					//si a esta profundidad no se puede realizar ninguna jugada legal es ahogado o mate
	{
		if (EnJaque(turno_c^1))
			valoracion = - MATE + prof;			//es mate en la profundidad actual (esto hace q el mate en 1 sea mejor q el mate en 2, etc.)
		else
			valoracion = 0;						//ahogado (puede ser 0 o algun contempt ligeramente negativo)
		if (valoracion >= beta)
        {
            alfa = beta;
            bandera_hash = TT_BETA;
        }
        else
        {
            if (valoracion > alfa)
            {
                alfa = valoracion;
                bandera_hash = TT_EXACTO;
            }
        }
	}
	legales[prof] = 0;						//la dejo lista para la proxima pasada a esta profundidad
#ifdef USAR_TT
	Save_tt(draft,alfa,bandera_hash,mejor_jugada); //guarda la posicion en la tt indicando el tipo de nodo q es
#endif // USAR_TT
//	if (cincuenta >= 100)							//si se detecta una variante en la q aplica la regla de 50 jugadas y no hay mate entonces la posicion es tablas
//		return 0;
	return alfa;
}

void ManejarReloj()
{
    QueryPerformanceCounter((LARGE_INTEGER *)&t_fin);
    t_transcurrido = ((t_fin - t_inicio) * timerFrequency);
	if (t_transcurrido > 3)                		//si se supero el tiempo maximo por jugada
	{
		salir = 1;									//entonces salgo del analisis
        wcout << "Tiempo transcurrido: " << t_transcurrido << endl;
		longjmp(env, 0);							//con la funcion longjmp
	}
}

BYTE Libro()
{
	int aleatorio;

	aleatorio = rand()%10;
	if (nueva)
	{
		if (total_jugadas == 0)			//si se cumple esto el pic hace su jugada de libro de blancas
		{
			if (aleatorio < 4)
			{
				inicio = 12;
				fin = 28;		//e4
				bien = doble;
			}
			if (aleatorio > 3 && aleatorio < 7)
			{
				inicio = 11;
				fin = 27;		//d4
				bien = doble;
			}
			if (aleatorio == 7)
			{
				inicio = 10;
				fin = 26;		//c4
				bien = doble;
			}
			if (aleatorio == 8)
			{
				inicio = 6;
				fin = 21;		//Cf3
				bien = normal;
			}
			if (aleatorio == 9)
			{
				inicio = 9;
				fin = 17;		//b3 (mi variante!!)
				bien = normal;
			}
			return 1;							//indica q es jugada de libro
		}
		if (total_jugadas == 1)					//si se cumple esto el pic hace su jugada de libro de negras
		{										//(si es posible en este caso ya q el user puede haber hecho una q no esta en el libro)
			if (peones_b & SetMask(E4))
			{
				if (aleatorio < 4)
				{
					inicio = 50;
					fin = 34;					//c5 siciliana
					bien = doble;
				}
				if (aleatorio > 3 && aleatorio < 7)
				{
					inicio = 52;
					fin = 36;					//e5
					bien = doble;
				}
				if (aleatorio == 7 || aleatorio == 8)
				{
					inicio = 52;
					fin = 44;					//e6 (francesa)
					bien = normal;
				}
				if (aleatorio == 9)
				{
					inicio = 50;
					fin = 42;					//c6 (caro kann)
					bien = normal;
				}
				return 1;
			}
			if (peones_b & SetMask(D4))
			{
				if (aleatorio < 5)
				{
					inicio = 51;
					fin = 35;					//d5
					bien = doble;
				}
				if (aleatorio > 4 && aleatorio < 8)
				{
					inicio = 62;
					fin = 45;					//Cf6
					bien = normal;
				}
				if (aleatorio == 8 || aleatorio == 9)
				{
					inicio = 50;
					fin = 34;					//c5 (mi variante!!)
					bien = doble;
				}
				return 1;
			}
			if (peones_b & SetMask(C4))
			{
				if (aleatorio < 5)
				{
					inicio = 52;
					fin = 36;					//e5
					bien = doble;
				}
				else
				{
					inicio = 50;
					fin = 34;					//c5
					bien = doble;
				}
				return 1;
			}
			if (peones_b & SetMask(B3))
			{
				if (aleatorio < 6)
				{
					inicio = 52;
					fin = 36;					//e5
					bien = doble;
				}
				else
				{
					inicio = 51;
					fin = 35;					//d5
					bien = doble;
				}
				return 1;
			}
			if (caballos_b & SetMask(F3))
			{
				if (aleatorio < 5)
				{
					inicio = 51;
					fin = 35;					//d5
					bien = doble;
				}
				else
				{
					inicio = 62;
					fin = 45;					//Cf6
					bien = normal;
				}
				return 1;
			}
		}
	}
	return 0;									//si no es ninguna de estas no es posicion de libro por lo que se debe pensar
}



BYTE Nulo_ok(BYTE prof)				//devuelve 1 si se puede hacer un movimiento nulo y 0 si no
{
	if(nulo == 1)							//si ya se hizo antes un movimiento nulo no se puede hacer otro
		return 0;
	if (prof + R + 1 > prof_max)		//si no hay la suficiente profundidad q no haga movimineto nulo (R por el factor de reduccion, y uno xq se aumenta la profundidad)
		return 0;
	if ((turno_c == blancas) && (piezas_b == (rey_b | peones_b)))		//el blanco no puede hacer mov nulo si solo tiene el rey y peones (puede estar en zugzwang)
		return 0;
	if ((turno_c == negras) && (piezas_n == (rey_n | peones_n)))		//lo mismo para el negro
		return 0;
//	if (prof == 0)					//no tiene sentido hacer mov nulo en la posicion raiz (creo)
//		return 0;
	return 1;
}

BYTE LMR_ok(BYTE prof,unsigned int z)
{
    if (comio[prof] != 0)       //si la jugada es una captura
        return 0;               //no le aplico LMR
    if (EnJaque(turno_c^1))     //si da jaque tampoco
        return 0;
    if (jugadas[prof][z+2] == corona)   //si es coronacion tampoco
        return 0;
    return 1;                   //aplicarle LMR a la jugada
}


int Quies(int alfa, int beta, BYTE Qprof)
{
	int stand_pat, Qvalor;
	unsigned int z;

    if (Qprof == 0)         //aca entra siempre que esta en la posicion hoja (recien entrado a Quies())
    {                       //solo aca pruebo evaltt xq quies no actualiza la hash
        if ((stand_pat = Probar_evaltt()) == INVALIDO)        //miro si se encuentra la posicion evaluada en la evaltt
        {
            llamadasevaluar++;              //si no se encontro entonces no queda otra q llamar a evaluar()
            stand_pat = Evaluar();
            Save_evaltt(stand_pat);         //guardo la evaluacion de esta posicion en la evaltt
        }
        else
        {
            evaltt_hits++;
        }
    }
    else
    {
        llamadasevaluar++;
        stand_pat = Evaluar();
    }

	if (stand_pat >= beta)
		return beta;
	if (stand_pat > alfa)
		alfa = stand_pat;
	QGenerarCapturas(Qprof);
	if (ultimacaptura[Qprof] == 0)							//si ya no hay capturas a esta Qprof entonces se alcanzo un nodo hoja de la Qsearch
		Qnodos++;											//por lo q incremento el contador
	else
	{
		QQuickSort(Qponderacion, 0, (ultimacaptura[Qprof]/2)-1, Qprof);			//si no es Qnodo hoja q ordene las capturas con algoritmo Qquicksort
		for (z = 0 ; z < ultimacaptura[Qprof] ; z = z + 2)
		{
			QHacerCaptura(Qcapturas[Qprof][z], Qcapturas[Qprof][z+1], Qprof);
			if (!EnJaque(turno_c))
			{
				Qvalor = -Quies(-beta, -alfa, Qprof + 1);
				QDeshacerCaptura(Qcapturas[Qprof][z], Qcapturas[Qprof][z+1], Qprof);
				if (Qvalor >= beta)
					return beta;
				if (Qvalor > alfa)
					alfa = Qvalor;
			}
			else
			{
				QDeshacerCaptura(Qcapturas[Qprof][z], Qcapturas[Qprof][z+1], Qprof);
			}
		}
	}
	return alfa;
}

void QQuickSort(int a[], int primero, int ultimo, BYTE Qprof)				//ordena las jugadas de cada Qprof en base al vector Qponderacion para hacer mas eficiente
{																			//alfa-beta en la busqueda Quies
	int i, j, central, pivote;

	central = (primero + ultimo)/2;
	pivote = a[central];
	i = primero;
	j = ultimo;
	do
	{
		while (a[i] > pivote) i++;
		while (a[j] < pivote) j--;
		if (i<=j)
		{
			int tmp;
			char temp;
			tmp = a[i];
			a[i] = a[j];
			a[j] = tmp; // intercambia a[i] con a[j]

			temp = Qcapturas[Qprof][2*i];
			Qcapturas[Qprof][2*i] = Qcapturas[Qprof][2*j];
			Qcapturas[Qprof][2*j] = temp;

			temp = Qcapturas[Qprof][(2*i)+1];
			Qcapturas[Qprof][(2*i)+1] = Qcapturas[Qprof][(2*j)+1];
			Qcapturas[Qprof][(2*j)+1] = temp;

			i++;
			j--;
		}
	}while (i <= j);
	if (primero < j)
		QQuickSort(a, primero, j, Qprof);// mismo proceso con sublista izqda
	if (i < ultimo)
		QQuickSort(a, i, ultimo, Qprof); // mismo proceso con sublista drcha
}

void QGenerarCapturas(BYTE Qprof)
{
	BITBOARD auxiliar,auxiliar1;

	Qpuntero = 0;
	if (turno_c == blancas)
	{
		auxiliar = peones_b;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			auxiliar1 = ataques_peones[blancas][inicio] & piezas_n;						//solo capturas (no contempla peones al paso xq en quies casi no influyen)
			while (auxiliar1)
			{
				fin = MSB(auxiliar1);
				auxiliar1 ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasBlancas() - 100;
				if (SetMask(fin) & fila_mask[7])										//si la captura es ademas una coronacion le sumo 8 puntos mas a la
					Qponderacion[Qpuntero/2] += 900;									//ponderacion (9 de la dama)
				QListaCapturas(inicio,fin,Qprof);
			};
			if (mov_peones[blancas][inicio] & SetMask(inicio + 8) & fila_mask[7] & ~tablero)	//genero ahora la posible coronacion del peon
			{
				Qponderacion[Qpuntero/2] = 800;											//9 de la dama - 1 del peon
				QListaCapturas(inicio,inicio+8,Qprof);
			}
		};
		auxiliar = caballos_b;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			auxiliar1 = ataques_caballos[inicio] & piezas_n;
			while (auxiliar1)
			{
				fin = MSB(auxiliar1);
				auxiliar1 ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasBlancas() - 300;
				QListaCapturas(inicio,fin,Qprof);
			};
		};
		auxiliar = alfiles_b;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_alfil = GeneraAlfil(inicio);
			atack_alfil &= piezas_n;
			while (atack_alfil)
			{
				fin = MSB(atack_alfil);
				atack_alfil ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasBlancas() - 315;
				QListaCapturas(inicio,fin,Qprof);
			};
		};
		auxiliar = torres_b;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_torre = GeneraTorre(inicio);
			atack_torre &= piezas_n;
			while (atack_torre)
			{
				fin = MSB(atack_torre);
				atack_torre ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasBlancas() - 500;
				QListaCapturas(inicio,fin,Qprof);
			};
		};
		auxiliar = damas_b;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_alfil = GeneraAlfil(inicio);
			atack_alfil &= piezas_n;
			while (atack_alfil)
			{
				fin = MSB(atack_alfil);
				atack_alfil ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasBlancas() - 900;
				QListaCapturas(inicio,fin,Qprof);
			};
			atack_torre = GeneraTorre(inicio);
			atack_torre &= piezas_n;
			while (atack_torre)
			{
				fin = MSB(atack_torre);
				atack_torre ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasBlancas() - 900;
				QListaCapturas(inicio,fin,Qprof);
			};
		};
		inicio = MSB(rey_b);
		auxiliar1 = ataques_rey[inicio] & piezas_n;
		while (auxiliar1)
		{
			fin = MSB(auxiliar1);
			auxiliar1 ^= SetMask(fin);
			Qponderacion[Qpuntero/2] = CapturasBlancas();
			QListaCapturas(inicio,fin,Qprof);
		};
	}
	else												//capturas de las negras
	{
		auxiliar = peones_n;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			auxiliar1 = ataques_peones[negras][inicio] & piezas_b;
			while (auxiliar1)
			{
				fin = MSB(auxiliar1);
				auxiliar1 ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasNegras() - 100;
				if (SetMask(fin) & fila_mask[0])
					Qponderacion[Qpuntero/2] += 900;
				QListaCapturas(inicio,fin,Qprof);
			};
			if (mov_peones[negras][inicio] & SetMask(inicio - 8) & fila_mask[0] & ~tablero)			//genero ahora la posible coronacion del peon
			{
				Qponderacion[Qpuntero/2] = 800;											//9 de la dama - 1 del peon
				QListaCapturas(inicio,inicio-8,Qprof);
			}
		};//end peon_n
		auxiliar = caballos_n;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			auxiliar1 = ataques_caballos[inicio] & piezas_b;
			while (auxiliar1)
			{
				fin = MSB(auxiliar1);
				auxiliar1 ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasNegras() - 300;
				QListaCapturas(inicio,fin,Qprof);
			};
		};//end caballo_n
		auxiliar = alfiles_n;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_alfil = GeneraAlfil(inicio);
			atack_alfil &= piezas_b;
			while (atack_alfil)
			{
				fin = MSB(atack_alfil);
				atack_alfil ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasNegras() - 315;
				QListaCapturas(inicio,fin,Qprof);
			};
		};//end alfil_n
		auxiliar = torres_n;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_torre = GeneraTorre(inicio);
			atack_torre &= piezas_b;
			while (atack_torre)
			{
				fin = MSB(atack_torre);
				atack_torre ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasNegras() - 500;
				QListaCapturas(inicio,fin,Qprof);
			};
		};//end torre_n
		auxiliar = damas_n;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_alfil = GeneraAlfil(inicio);
			atack_alfil &= piezas_b;
			while (atack_alfil)
			{
				fin = MSB(atack_alfil);
				atack_alfil ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasNegras() - 900;
				QListaCapturas(inicio,fin,Qprof);
			};
			atack_torre = GeneraTorre(inicio);
			atack_torre &= piezas_b;
			while (atack_torre)
			{
				fin = MSB(atack_torre);
				atack_torre ^= SetMask(fin);
				Qponderacion[Qpuntero/2] = CapturasNegras() - 900;
				QListaCapturas(inicio,fin,Qprof);
			};
		};//end dama_n
		inicio = MSB(rey_n);
		auxiliar1 = ataques_rey[inicio] & piezas_b;
		while (auxiliar1)
		{
			fin = MSB(auxiliar1);
			auxiliar1 ^= SetMask(fin);
			Qponderacion[Qpuntero/2] = CapturasNegras();
			QListaCapturas(inicio,fin,Qprof);
		};
	}
	ultimacaptura[Qprof] = Qpuntero;					//indico cual es la ultima captura de la lista en la profundidad actual
}

void QListaCapturas(BYTE inicio, BYTE fin, BYTE Qprof)
{
	Qcapturas[Qprof][Qpuntero] = inicio;
	Qcapturas[Qprof][Qpuntero+1] = fin;
	Qpuntero += 2;
}

void QHacerCaptura(BYTE inicio,BYTE fin,BYTE Qprof)		//no contempla peon al paso pero si coronaciones
{
	tablero &= ClearMask(inicio);
	tablero90 &= ClearMask90(inicio);
	tableroA1 &= ClearMaskA1(inicio);
	tableroA8 &= ClearMaskA8(inicio);
	if (turno_c == blancas)
	{
		piezas_b &= ClearMask(inicio);
		piezas_b |= SetMask(fin);
		QActualizaBlancas(inicio,fin);		//normal
		QCapturaBlancas(fin,Qprof);
		if (peones_b & fila_mask[7])		//si un peon corono al capturar o por solo promocionar
		{
			Qcorona[Qprof] = 1;				//aviso para poder deshacer
			peones_b &= ClearMask(fin);		//saco el peon q recien actualize incorrectamente
			damas_b |= SetMask(fin);		//y pongo una dama (no tiene sentido aca considerar otra pieza)
			tablero |= SetMask(fin);		//esto es xq pudiera estar vacia de antemano la casilla de fin (solo se da si fue promocion sin captura)
            tablero90 |= SetMask90(fin);
            tableroA1 |= SetMaskA1(fin);
            tableroA8 |= SetMaskA8(fin);
		}
	}//end turno blancas
	else	//si le toca a las negras
	{
		piezas_n &= ClearMask(inicio);
		piezas_n |= SetMask(fin);
		QActualizaNegras(inicio,fin);
		QCapturaNegras(fin,Qprof);
		if (peones_n & fila_mask[0])		//si un peon corono
		{
			Qcorona[Qprof] = 1;
			peones_n &= ClearMask(fin);
			damas_n |= SetMask(fin);
			tablero |= SetMask(fin);		//esto es xq pudiera estar vacia de antemano la casilla de fin (solo se da si fue promocion sin captura)
            tablero90 |= SetMask90(fin);
            tableroA1 |= SetMaskA1(fin);
            tableroA8 |= SetMaskA8(fin);
		}
	}//end turno negras
	turno_c ^= 1;
}

void QDeshacerCaptura(BYTE inicio, BYTE fin,BYTE Qprof)		//todo lo contrario a hacer captura
{
	if (turno_c == negras)									//si se cumple esto hay q deshacer una jugada del blanco.
	{
		if (Qcorona[Qprof])									//si un peon corono
		{
			Qcorona[Qprof] = 0;								//deshago la bandera de coronacion
			piezas_b &= ClearMask(fin);						//borro la pieza blanca de la casilla de fin
			piezas_b |= SetMask(inicio);					//y la pongo en la de inicio
			damas_b &= ClearMask(fin);						//saco la dama
			peones_b |= SetMask(inicio);					//y pongo el peon
		}
		else
		{
			QDesactualizaBlancas(inicio,fin);				//las restituciones normales (sin modificar la hash)
		}
		QDescapturaBlancas(fin, Qprof);						//esto se encarga de reponerle a las negras su material
	}
	else
	{
		if (Qcorona[Qprof])
		{
			Qcorona[Qprof] = 0;
			piezas_n &= ClearMask(fin);
			piezas_n |= SetMask(inicio);
			damas_n &= ClearMask(fin);
			peones_n |= SetMask(inicio);
		}
		else
		{
			QDesactualizaNegras(inicio,fin);
		}
		QDescapturaNegras(fin, Qprof);
	}
	tablero |= SetMask(inicio);								//esto se da siempre (indicar q hay una pieza en la casilla de inicio)
	tablero90 |= SetMask90(inicio);
	tableroA1 |= SetMaskA1(inicio);
	tableroA8 |= SetMaskA8(inicio);
	turno_c ^= 1;											//cambio el turno de a quien le toca jugar
}

void QActualizaBlancas(BYTE inicio, BYTE fin)           //igual que ActualizaBlancas pero sin modificar la hash
{
	if (peones_b & SetMask(inicio))						//si fue un peon el q movio
	{
		peones_b &= ClearMask(inicio);					//borro el peon de la casilla de inicio
		peones_b |= SetMask(fin);						//y lo pongo en la de fin
	}
	else												//sino sigo preguntando hasta ver que fue
	{
		if (caballos_b & SetMask(inicio))
		{
			caballos_b &= ClearMask(inicio);
			caballos_b |= SetMask(fin);
		}
		else
		{
			if (alfiles_b & SetMask(inicio))
			{
				alfiles_b &= ClearMask(inicio);
				alfiles_b |= SetMask(fin);
			}
			else
			{
				if (torres_b & SetMask(inicio))
				{
					torres_b &= ClearMask(inicio);
					torres_b |= SetMask(fin);
				}
				else
				{
					if (damas_b & SetMask(inicio))
					{
						damas_b &= ClearMask(inicio);
						damas_b |= SetMask(fin);
					}
					else								//no queda otra q el rey asi q no pregunto mas
					{
						rey_b &= ClearMask(inicio);
						rey_b |= SetMask(fin);
					}
				}
			}
		}
	}
}

void QActualizaNegras(BYTE inicio, BYTE fin)
{
	if (peones_n & SetMask(inicio))						//si fue un peon el q movio
	{
		peones_n &= ClearMask(inicio);					//borro el peon de la casilla de inicio
		peones_n |= SetMask(fin);						//y lo pongo en la de fin
	}
	else												//sino sigo preguntando hasta ver que fue
	{
		if (caballos_n & SetMask(inicio))
		{
			caballos_n &= ClearMask(inicio);
			caballos_n |= SetMask(fin);
		}
		else
		{
			if (alfiles_n & SetMask(inicio))
			{
				alfiles_n &= ClearMask(inicio);
				alfiles_n |= SetMask(fin);
			}
			else
			{
				if (torres_n & SetMask(inicio))
				{
					torres_n &= ClearMask(inicio);
					torres_n |= SetMask(fin);
				}
				else
				{
					if (damas_n & SetMask(inicio))
					{
						damas_n &= ClearMask(inicio);
						damas_n |= SetMask(fin);
					}
					else								//no queda otra q el rey asi q no pregunto mas
					{
						rey_n &= ClearMask(inicio);
						rey_n |= SetMask(fin);
					}
				}
			}
		}
	}
}

void QDesactualizaBlancas(BYTE inicio, BYTE fin)
{
	piezas_b &= ClearMask(fin);			//borro la pieza blanca de la casilla de fin
	piezas_b |= SetMask(inicio);		//y la pongo en la de inicio
	if (peones_b & SetMask(fin))		//si fue un peon el q movio
	{
		peones_b &= ClearMask(fin);		//borro el peon de la casilla de fin
		peones_b |= SetMask(inicio);	//y lo pongo en la de inicio
	}
	else
	{
		if (caballos_b & SetMask(fin))
		{
			caballos_b &= ClearMask(fin);
			caballos_b |= SetMask(inicio);
		}
		else
		{
			if (alfiles_b & SetMask(fin))
			{
				alfiles_b &= ClearMask(fin);
				alfiles_b |= SetMask(inicio);
			}
			else
			{
				if (torres_b & SetMask(fin))
				{
					torres_b &= ClearMask(fin);
					torres_b |= SetMask(inicio);
				}
				else
				{
					if (damas_b & SetMask(fin))
					{
						damas_b &= ClearMask(fin);
						damas_b |= SetMask(inicio);
					}
					else
					{
						rey_b &= ClearMask(fin);
						rey_b |= SetMask(inicio);
					}
				}
			}
		}
	}
}

void QDesactualizaNegras(BYTE inicio, BYTE fin)
{
	piezas_n &= ClearMask(fin);			//borro la pieza negra de la casilla de fin
	piezas_n |= SetMask(inicio);		//y la pongo en la de inicio
	if (peones_n & SetMask(fin))		//si fue un peon el q movio
	{
		peones_n &= ClearMask(fin);		//borro el peon de la casilla de fin
		peones_n |= SetMask(inicio);	//y lo pongo en la de inicio
	}
	else
	{
		if (caballos_n & SetMask(fin))
		{
			caballos_n &= ClearMask(fin);
			caballos_n |= SetMask(inicio);
		}
		else
		{
			if (alfiles_n & SetMask(fin))
			{
				alfiles_n &= ClearMask(fin);
				alfiles_n |= SetMask(inicio);
			}
			else
			{
				if (torres_n & SetMask(fin))
				{
					torres_n &= ClearMask(fin);
					torres_n |= SetMask(inicio);
				}
				else
				{
					if (damas_n & SetMask(fin))
					{
						damas_n &= ClearMask(fin);
						damas_n |= SetMask(inicio);
					}
					else
					{
						rey_n &= ClearMask(fin);
						rey_n |= SetMask(inicio);
					}
				}
			}
		}
	}
}

void QCapturaBlancas(BYTE fin, BYTE Qprof)			//esta es para recuperar despues la posicion (recuerda q fue lo q se comio en la Quies)
{
	piezas_n &= ClearMask(fin);						//borro el bit correspondiente en el bando negro
	if (peones_n & SetMask(fin))					//si fue un peon el comido
	{
		peones_n &= ClearMask(fin);					//borro el bit correspondiente al peon negro
		Qcomio[Qprof] = PEON;						//indico que se lastraron un peon para despues poder recuperarlo en DeshacerJugada()
	}
	else											//sino sigo preguntando hasta ver cual fue
	{
		if (caballos_n & SetMask(fin))
		{
			caballos_n &= ClearMask(fin);
			Qcomio[Qprof] = CABALLO;
		}
		else
		{
			if (alfiles_n & SetMask(fin))
			{
				alfiles_n &= ClearMask(fin);
				Qcomio[Qprof] = ALFIL;
			}
			else
			{
				if (torres_n & SetMask(fin))
				{
					torres_n &= ClearMask(fin);
					Qcomio[Qprof] = TORRE;
				}
				else
				{
					if (damas_n & SetMask(fin))
					{
						damas_n &= ClearMask(fin);
						Qcomio[Qprof] = DAMA;
					}
					else							//si no fue captura entonces la unica posibilidad es q haya sido una coronacion (tambien considerada dentro de la quies)
					{
						Qcomio[Qprof] = 0;			//por lo q indico q no fue captura
					}
				}
			}
		}
	}
}

void QCapturaNegras(BYTE fin, BYTE Qprof)			//esta es para recuperar despues la posicion (recuerda q fue lo q se comio en la Quies)
{
	piezas_b &= ClearMask(fin);						//borro el bit correspondiente en el bando blanco
	if (peones_b & SetMask(fin))					//si fue un peon el comido
	{
		peones_b &= ClearMask(fin);					//borro el bit correspondiente al peon blanco
		Qcomio[Qprof] = PEON;
	}
	else											//sino sigo preguntando hasta ver cual fue
	{
		if (caballos_b & SetMask(fin))
		{
			caballos_b &= ClearMask(fin);
			Qcomio[Qprof] = CABALLO;
		}
		else
		{
			if (alfiles_b & SetMask(fin))
			{
				alfiles_b &= ClearMask(fin);
				Qcomio[Qprof] = ALFIL;
			}
			else
			{
				if (torres_b & SetMask(fin))
				{
					torres_b &= ClearMask(fin);
					Qcomio[Qprof] = TORRE;
				}
				else								//no queda otra q la dama asi q no pregunto mas (el rey no puede ser comido)
				{
					if (damas_b & SetMask(fin))
					{
						damas_b &= ClearMask(fin);
						Qcomio[Qprof] = DAMA;
					}
					else							//si no fue captura entonces la unica posibilidad es q haya sido una coronacion (tambien considerada dentro de la quies)
					{
						Qcomio[Qprof] = 0;			//por lo q indico q no fue captura
					}
				}
			}
		}
	}
}

void QDescapturaBlancas(BYTE fin, BYTE Qprof)			//le repone a las negras las piezas comidas por las blancas en la busqueda Quies()
{
	switch (Qcomio[Qprof])
	{
		case 0:											//no fue captura (fue coronacion)
		{
			tablero &= ClearMask(fin);					//no habia nada en la casilla de fin entonces la borro (en cualquier otro caso no xq es captura osea q estaba ocupada antes)
            tablero90 &= ClearMask90(fin);
            tableroA1 &= ClearMaskA1(fin);
            tableroA8 &= ClearMaskA8(fin);
		}break;
		case PEON:										//fue captura de un peon
		{
			peones_n |= SetMask(fin);					//entonces les devuelvo el peon "descomido" a las negras
			piezas_n |= SetMask(fin);
		}break;
		case CABALLO:
		{
			caballos_n |= SetMask(fin);
			piezas_n |= SetMask(fin);
		}break;
		case ALFIL:
		{
			alfiles_n |= SetMask(fin);
			piezas_n |= SetMask(fin);
		}break;
		case TORRE:
		{
			torres_n |= SetMask(fin);
			piezas_n |= SetMask(fin);
		}break;
		case DAMA:
		{
			damas_n |= SetMask(fin);
			piezas_n |= SetMask(fin);
		}break;
	}
}

void QDescapturaNegras(BYTE fin, BYTE Qprof)			//le repone a las blancas las piezas comidas por las negras en la busqueda Quies()
{
	switch (Qcomio[Qprof])
	{
		case 0:											//no fue captura (fue coronacion)
		{
			tablero &= ClearMask(fin);					//no habia nada en la casilla de fin entonces la borro (en cualquier otro caso no xq es captura osea q estaba ocupada antes)
            tablero90 &= ClearMask90(fin);
            tableroA1 &= ClearMaskA1(fin);
            tableroA8 &= ClearMaskA8(fin);
		}break;
		case PEON:										//fue captura de un peon
		{
			peones_b |= SetMask(fin);					//entonces les devuelvo el peon "descomido" a las blancas
			piezas_b |= SetMask(fin);
		}break;
		case CABALLO:
		{
			caballos_b |= SetMask(fin);
			piezas_b |= SetMask(fin);
		}break;
		case ALFIL:
		{
			alfiles_b |= SetMask(fin);
			piezas_b |= SetMask(fin);
		}break;
		case TORRE:
		{
			torres_b |= SetMask(fin);
			piezas_b |= SetMask(fin);
		}break;
		case DAMA:
		{
			damas_b |= SetMask(fin);
			piezas_b |= SetMask(fin);
		}break;
	}
}

BITBOARD Perft(BYTE prof)		//perft para cuando cambio algo saber si esta generando bien todos los movimientos
{
	BYTE z;
	BITBOARD parcial = 0;

    if ((parcial = Probar_perfttt(prof_max - prof)) != 0)   //pruebo si hay hit en la perfttt para ahorrar trabajo
    {
        perfttt_hits++;
        return parcial;
    }

	if (prof == prof_max)   //si se llego a la hoja
		return 1;           //anoto un nodo mas

	GenerarTodas(prof);
	for (z = 0 ; z < ultimajugada[prof] ; z = z + 3)
	{
		HacerJugada(jugadas[prof][z],jugadas[prof][z+1],jugadas[prof][z+2],prof);
		if (!EnJaque(turno_c))	//si el rey del bando q jugo no esta en jaque es totalmente legal
		{
			parcial += Perft(prof + 1);
		}
		DeshacerJugada(jugadas[prof][z],jugadas[prof][z+1],jugadas[prof][z+2],prof);
/*
		if (prof == 0)       //si terminamos de analizar una jugada de la posicion raiz
        {
            divide[z/3] = nodos;    //anotamos la cantidad de nodos de esa jugada (funcion divide)
            nodos = 0;              //y reiniciamos para la proxima
        }
*/
	}
    Save_perfttt(parcial,prof_max-prof);
/*
	if (prof == 0)   //si estamos terminando es tiempo de recuperar la cantidad total de nodos
    {
        nodos = 0;
        for (z=0;z<ultimajugada[0];z=z+3)
        {
            nodos += divide[z/3];       //sumamos todos los resultados parciales de cada jugada de la posicion raiz
        }
    }
*/
	return parcial;
}

BITBOARD GeneraAlfil(BYTE inicio)										//genera todos los ataques diagonales posibles desde la casilla inicio en la posicion actual
{
	BITBOARD bloqueos;
	BYTE casilla_bloqueada;

	bloqueos = mas7dir[inicio] & tablero;
	casilla_bloqueada = LSB(bloqueos);
	atack_alfil = mas7dir[inicio] ^ mas7dir[casilla_bloqueada];

	bloqueos = mas9dir[inicio] & tablero;
	casilla_bloqueada = LSB(bloqueos);
	atack_alfil |= mas9dir[inicio] ^ mas9dir[casilla_bloqueada];

	bloqueos = menos7dir[inicio] & tablero;
	casilla_bloqueada = MSB(bloqueos);
	atack_alfil |= menos7dir[inicio] ^ menos7dir[casilla_bloqueada];

	bloqueos = menos9dir[inicio] & tablero;
	casilla_bloqueada = MSB(bloqueos);
	atack_alfil |= menos9dir[inicio] ^ menos9dir[casilla_bloqueada];	//atack_alfil ahora tiene todas las casillas atacadas en diagonal desde inicio en esta posicion

	return atack_alfil;
}

BITBOARD GeneraTorre(BYTE inicio)										//genera todos los ataques ortogonales posibles desde la casilla inicio en la posicion actual
{
	BITBOARD indice;
	BYTE inicio90;

	indice = (tablero >> (inicio & 0xf8)) & 0xff;           //indice contiene la configuracion de piezas a lo largo de la fila
	atack_torre = ataques_filas[inicio][indice];            //o columna segun corresponda

	inicio90 = (((inicio >> 3) | (inicio << 3)) & 63) ^ 7;  //roto 90 grados inicio
	//inicio90 = MSB(SetMask90(inicio));                    //esta es tambien una posibilidad q hay q ver si es mas rapida o no
	indice = (tablero90 >> (inicio90 & 0xf8)) & 0xff;       //y aplico lo mismo para los movimientos x la fila
	atack_torre |= ataques_columnas[inicio][indice];

	return atack_torre;
}

void GenerarTodas(BYTE prof)						//en base a la posicion actual del tablero lista todas las posibles jugadas semilegales
{
	BITBOARD auxiliar,auxiliar1;

	puntero = 0;										//inicializo el puntero que despues utiliza ListaJugadas() para acomodarlas en su lugar
	if (turno_c == blancas)
	{
		auxiliar = peones_b;							//auxiliar sirve para no modificar las bitboard q tiene las piezas del tablero actual
		pieza = PEON;									//las piezas q necesitan llamar a SemiLegal() tienen q indicar de cual se trata (para torre, alfil y dama no es necesario)
		while (auxiliar)                                //si no quedan peones salgo
		{
			inicio = MSB(auxiliar);						//busco los peones en el tablero (uso MSB porque es mas rapida q LSB y xq encuentra los mas avanzados primero)
			auxiliar ^= SetMask(inicio);				//borro el primer "1" encontrado para q al pasar otra vez MSB encuentre el siguiente peon
			auxiliar1 = mov_peones[blancas][inicio];	//auxiliar1 tiene ahora "1�s" en todas las casillas a las q un peon blanco puede mover desde inicio
			while (auxiliar1)                           //si no hay mas salgo y sigo con otro peon xq este esta agotado
			{
				fin = MSB(auxiliar1);					//busco las casillas de destino (fin) y ademas busca las mas agresivas primero (las q avanzan)
				auxiliar1 ^= SetMask(fin);				//borro el primer "1" encontrado para q al pasar otra vez MSB encuentre la siguiente casilla de fin (y no la misma siempre)
				semibien = SemiLegal(prof);
                if (semibien)
				{
					if (tablero & SetMask(fin))		//si fue una captura
						ponderacion[puntero/3] = CapturasBlancas() - 100;
					else							//si no fue una captura la ponderacion es -1
						ponderacion[puntero/3] = - 100;
					ponderacion[puntero/3] += peon_pos[blancas][fin] - peon_pos[blancas][inicio];
					ListaJugadas(inicio,fin,semibien,prof);	//si es semilegal que la anote en la lista de jugadas
				}
			};
		};
		auxiliar = caballos_b;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			auxiliar1 = ataques_caballos[inicio] & ~piezas_b;
			while (auxiliar1)
			{
				fin = MSB(auxiliar1);
				auxiliar1 ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasBlancas() - 300;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += caballo_pos[blancas][fin] - caballo_pos[blancas][inicio];
				ListaJugadas(inicio,fin,normal,prof);
			};
		};
		auxiliar = alfiles_b;
		while (auxiliar)										//como las piezas deslizantes (alfil, torre y dama) no llaman a SemiLegal() no es necesario indicar q pieza es
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_alfil = GeneraAlfil(inicio);			//pone en atack_alfil todas las casillas q ataca en alfil desde inicio
			atack_alfil &= ~piezas_b;					//atak_alfil ahora tiene todas las casillas semilegales a las q puede ir el alfil desde inicio
			while (atack_alfil)
			{
				fin = MSB(atack_alfil);
				atack_alfil ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasBlancas() - 310;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += alfil_pos[blancas][fin] - alfil_pos[blancas][inicio];
				ListaJugadas(inicio,fin,normal,prof);	//movimientos de alfiles y damas son siempre tipo 1 (normales)
			};
		};
		auxiliar = torres_b;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_torre = GeneraTorre(inicio);			//pone en atack_torre todas las casillas q ataca la torre desde inicio
			atack_torre &= ~piezas_b;					//atack_torre ahora tiene todas las casillas semilegales a las q puede ir la torre desde inicio
			while (atack_torre)
			{
				fin = MSB(atack_torre);
				atack_torre ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasBlancas() - 500;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += torre_pos[blancas][fin] - torre_pos[blancas][inicio];
				ListaJugadas(inicio,fin,movtorre,prof);		//como no paso por semilegal() le aclaro yo q es un movimiento de torre
			};
		};
		auxiliar = damas_b;
		while (auxiliar)										//la dama tiene la combnacion de movimientos del alfil y la torre asi q se generan ambos por separado
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_alfil = GeneraAlfil(inicio);
			atack_alfil &= ~piezas_b;
			while (atack_alfil)
			{
				fin = MSB(atack_alfil);
				atack_alfil ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasBlancas() - 900;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += dama_pos[fin] - dama_pos[inicio];
				ListaJugadas(inicio,fin,normal,prof);
			};
			atack_torre = GeneraTorre(inicio);
			atack_torre &= ~piezas_b;
			while (atack_torre)
			{
				fin = MSB(atack_torre);
				atack_torre ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasBlancas() - 900;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += dama_pos[fin] - dama_pos[inicio];
				ListaJugadas(inicio,fin,normal,prof);
			};
		};
		pieza = REY;
		inicio = MSB(rey_b);							//solo puede haber un rey blanco por lo q no hago loop para encontrar otros
		auxiliar1 = ataques_rey[inicio] & ~piezas_b;
		while (auxiliar1)
		{
			fin = MSB(auxiliar1);
			auxiliar1 ^= SetMask(fin);
			if (tablero & SetMask(fin))
				ponderacion[puntero/3] = CapturasBlancas() - 1000;
			else
				ponderacion[puntero/3] = -200;			//esto desalienta al programa a mover el rey
			ponderacion[puntero/3] += rey_pos[blancas][fin] - rey_pos[blancas][inicio];
			ListaJugadas(inicio,fin,movrey,prof);
		};
		if (derechos_enroque[prof] & 0x01)		//genero los enroques blancos (si son posibles)
		{
			fin = G1;									//primero el 0-0
			semibien = SemiLegal(prof);
            if (semibien)
			{
				ponderacion[puntero/3] = 100;			//esto alienta al programa a enrocarse
				ListaJugadas(inicio,fin,semibien,prof);
			}
		}
		if (derechos_enroque[prof] & 0x02)
		{
			fin = C1;									//y despues el 0-0-0
			semibien = SemiLegal(prof);
            if (semibien)
			{
				ponderacion[puntero/3] = 100;
				ListaJugadas(inicio,fin,semibien,prof);
			}
		}
	}
	else												//lo mismo pero para las negras (no cambia casi nada)
	{
		auxiliar = peones_n;
		pieza = PEON;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			auxiliar1 = mov_peones[negras][inicio];
			while (auxiliar1)
			{
				fin = MSB(auxiliar1);
				auxiliar1 ^= SetMask(fin);
				semibien = SemiLegal(prof);
                if (semibien)
				{
					if (tablero & SetMask(fin))
						ponderacion[puntero/3] = CapturasNegras() - 100;
					else
						ponderacion[puntero/3] = -100;
					ponderacion[puntero/3] += peon_pos[negras][fin] - peon_pos[negras][inicio];
					ListaJugadas(inicio,fin,semibien,prof);
				}
			};
		};//end peon_n
		auxiliar = caballos_n;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			auxiliar1 = ataques_caballos[inicio] & ~piezas_n;
			while (auxiliar1)
			{
				fin = MSB(auxiliar1);
				auxiliar1 ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasNegras() - 300;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += caballo_pos[negras][fin] - caballo_pos[negras][inicio];
				ListaJugadas(inicio,fin,normal,prof);
			};
		};//end caballo_n
		auxiliar = alfiles_n;							//para no estropear el bitboard q tiene el tablero actual
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_alfil = GeneraAlfil(inicio);			//pone en atack_alfil todas las casillas q ataca en alfil desde inicio
			atack_alfil &= ~piezas_n;					//atak_alfil ahora tiene todas las casillas semilegales a las q puede ir el alfil desde inicio
			while (atack_alfil)
			{
				fin = MSB(atack_alfil);
				atack_alfil ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasNegras() - 310;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += alfil_pos[negras][fin] - alfil_pos[negras][inicio];
				ListaJugadas(inicio,fin,normal,prof);
			};
		};//end alfil_n
		auxiliar = torres_n;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_torre = GeneraTorre(inicio);
			atack_torre &= ~piezas_n;
			while (atack_torre)
			{
				fin = MSB(atack_torre);
				atack_torre ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasNegras() - 500;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += torre_pos[negras][fin] - torre_pos[negras][inicio];
				ListaJugadas(inicio,fin,movtorre,prof);
			};
		};//end torre_n
		auxiliar = damas_n;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_alfil = GeneraAlfil(inicio);
			atack_alfil &= ~piezas_n;
			while (atack_alfil)
			{
				fin = MSB(atack_alfil);
				atack_alfil ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasNegras() - 900;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += dama_pos[fin] - dama_pos[inicio];
				ListaJugadas(inicio,fin,normal,prof);
			};
			atack_torre = GeneraTorre(inicio);
			atack_torre &= ~piezas_n;
			while (atack_torre)
			{
				fin = MSB(atack_torre);
				atack_torre ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasNegras() - 900;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += dama_pos[fin] - dama_pos[inicio];
				ListaJugadas(inicio,fin,normal,prof);
			};
		};//end dama_n
		pieza = REY;
		inicio = MSB(rey_n);
		auxiliar1 = ataques_rey[inicio] & ~piezas_n;
		while (auxiliar1)
		{
			fin = MSB(auxiliar1);
			auxiliar1 ^= SetMask(fin);
			if (tablero & SetMask(fin))
				ponderacion[puntero/3] = CapturasNegras() - 1000;
			else
				ponderacion[puntero/3] = -200;
			ponderacion[puntero/3] += rey_pos[negras][fin] - rey_pos[negras][inicio];
			ListaJugadas(inicio,fin,movrey,prof);
		};
		if (derechos_enroque[prof] & 0x04)		//genero los enroques negros (si son posibles)
		{
			fin = G8;
			semibien = SemiLegal(prof);
            if (semibien)
			{
				ponderacion[puntero/3] = 100;
				ListaJugadas(inicio,fin,semibien,prof);
			}
		}
		if (derechos_enroque[prof] & 0x08)
		{
			fin = C8;
			semibien = SemiLegal(prof);
            if (semibien)
			{
				ponderacion[puntero/3] = 100;
				ListaJugadas(inicio,fin,semibien,prof);
			}
		}
	}
	ultimajugada[prof] = puntero;				//indico cual es la ultima de la lista en la profundidad actual
}

void ListaJugadas(BYTE inicio,BYTE fin,BYTE semibien,BYTE prof)		//va llenando los registros en forma ordenada con las jugadas semilegales generadas
{
	jugadas[prof][puntero] = inicio;
	jugadas[prof][puntero+1] = fin;
	jugadas[prof][puntero+2] = semibien;
	puntero += 3;
}

int CapturasBlancas()				//esta la uso para ver q pieza comio el blanco al negro
{
	if (peones_n & SetMask(fin))
		return 100;
	if (caballos_n & SetMask(fin))
		return 300;
	if (alfiles_n & SetMask(fin))
		return 310;
	if (torres_n & SetMask(fin))
		return 500;
	if (damas_n & SetMask(fin))
		return 900;
	return 0;						//hasta aca no se deberia llegar pero por las dudas
}

int CapturasNegras()				//esta la uso para ver q pieza comio el negro al blanco
{
	if (peones_b & SetMask(fin))
		return 100;
	if (caballos_b & SetMask(fin))
		return 300;
	if (alfiles_b & SetMask(fin))
		return 310;
	if (torres_b & SetMask(fin))
		return 500;
	if (damas_b & SetMask(fin))
		return 900;
	return 0;
}


void Ordenar(BYTE prof)						//ordena las jugadas para hacer mas eficiente alfa-beta
{
	unsigned int i;

	if (prof_max != 1)																//entra si ya se ha establecido la VP (aunque sea una iteracion previa)
	{
	    if (tt_hit)     //entra si hay disponible una jugada de la tt.
        {
            for (i = 0 ; i < ultimajugada[prof] ; i=i+3)							//se busca la pv en el listado actual (tiene q estar xq es la misma posicion raiz)
            {
                if ((vp[prof][0] == jugadas[prof][i]) && (vp[prof][1] == jugadas[prof][i+1])) 	//si la jugada forma parte de la VP de la anterior iteracion
                {
                    ponderacion[i/3] += 10000;												//si o si se la situa primera de la lista en este nuevo analisis
//    				continue;
                }
                if (((killer[prof][0] & 0xff) == jugadas[prof][i]) && (((killer[prof][0] & 0xff00)>>8) == jugadas[prof][i+1]))
                {
                    ponderacion[i/3] = -10;
//                    continue;
                }
                if (((killer[prof][1] & 0xff) == jugadas[prof][i]) && (((killer[prof][1] & 0xff00)>>8) == jugadas[prof][i+1]))
                {
                    ponderacion[i/3] = -20;
//                    continue;
                }
                if (((tt_mejor & 0xff) == jugadas[prof][i]) && (((tt_mejor & 0xff00)>>8) == jugadas[prof][i+1]))
                {
                    mejortt++;
                    ponderacion[i/3] += 5000;
//                    continue;
                }

            }
        }
        else    //si no entonces no pregunto por la jugada de la tt
        {
            for (i = 0 ; i < ultimajugada[prof] ; i=i+3)							//se busca la pv en el listado actual (tiene q estar xq es la misma posicion raiz)
            {
                if ((vp[prof][0] == jugadas[prof][i]) && (vp[prof][1] == jugadas[prof][i+1])) 	//si la jugada forma parte de la VP de la anterior iteracion
                {
                    ponderacion[i/3] += 10000;												//si o si se la situa primera de la lista en este nuevo analisis
//                    continue;
                }
                if (((killer[prof][0] & 0xff) == jugadas[prof][i]) && (((killer[prof][0] & 0xff00)>>8) == jugadas[prof][i+1]))
                {
                    ponderacion[i/3] = -10;
//                    continue;
                }
                if (((killer[prof][1] & 0xff) == jugadas[prof][i]) && (((killer[prof][1] & 0xff00)>>8) == jugadas[prof][i+1]))
                {
                    ponderacion[i/3] = -20;
//                    continue;
                }
            }
        }
	}
    tt_hit = 0;     //reinicio el contador para la proxima
	//aca todas las jugadas tienen su ponderacion correspondiente asi q resta ordenarlas
	QuickSort(ponderacion, 0, (ultimajugada[prof]/3)-1, prof);		//llamada al algoritmo de ordenamiento quicksort
}

void QuickSort(int a[], int primero, int ultimo, BYTE prof)					//ordena las jugadas de cada profundidad en base al vector ponderacion
{
	int i,j,central,pivote;

	central = (primero + ultimo)/2;
	pivote = a[central];
	i = primero;
	j = ultimo;
	do
	{
		while (a[i] > pivote) i++;
		while (a[j] < pivote) j--;
		if (i<=j)
		{
			int tmp;
			char temp;
			tmp = a[i];
			a[i] = a[j];
			a[j] = tmp; /* intercambia a[i] con a[j] */

			temp = jugadas[prof][3*i];
			jugadas[prof][3*i] = jugadas[prof][3*j];
			jugadas[prof][3*j] = temp;

			temp = jugadas[prof][(3*i)+1];
			jugadas[prof][(3*i)+1] = jugadas[prof][(3*j)+1];
			jugadas[prof][(3*j)+1] = temp;

			temp = jugadas[prof][(3*i)+2];
			jugadas[prof][(3*i)+2] = jugadas[prof][(3*j)+2];
			jugadas[prof][(3*j)+2] = temp;
			i++;
			j--;
		}
	}while (i <= j);
	if (primero < j)
		QuickSort(a, primero, j, prof);/* mismo proceso con sublista izqda */
	if (i < ultimo)
		QuickSort(a, i, ultimo, prof); /* mismo proceso con sublista drcha */
}

void HacerJugada(BYTE inicio,BYTE fin,BYTE semibien,BYTE prof)	//actualiza los reg necesarios para q la jugada se realice en la forma de representacion del programa
{
	alpaso[prof+1] = 8;											//limpio el registro (solo se setea si avanza un peon de a 2) ya q columnas validas van de 0 a 7
	derechos_enroque[prof+1] = derechos_enroque[prof];	//en principio supongo q no se modifican los enroques (luego en los case q corresponda se modifican)
	tablero &= ClearMask(inicio);										//borro la pieza de la casilla de salida del tablero
	tablero |= SetMask(fin);											//y la pongo en la de destino
	tablero90 &= ClearMask90(inicio);
	tablero90 |= SetMask90(fin);
	tableroA1 &= ClearMaskA1(inicio);
	tableroA1 |= SetMaskA1(fin);
	tableroA8 &= ClearMaskA8(inicio);
	tableroA8 |= SetMaskA8(fin);
	if (turno_c == blancas)
	{
		piezas_b &= ClearMask(inicio);									//borro la pieza de la casilla de inicio
		piezas_b |= SetMask(fin);										//y la pongo en la de destino (comun a todo tipo de movimiento blanco)
		switch (semibien)
		{
			case normal:
			{
				ActualizaBlancas(inicio,fin);							//realizan las acciones normales de un movimiento
				CapturaBlancas(fin,prof);
			}break;
			case corona:
			{
				peones_b &= ClearMask(inicio);							//borro el peon de la casilla de inicio
				damas_b |= SetMask(fin);								//y pongo una DAMA en la casilla de fin (solo considera esto en principio)
				hash_pos ^= Zobrist.escaques[Peon_b][inicio];           //lo mismo con la llave zobrist
				hash_pos ^= Zobrist.escaques[Dama_b][fin];
				CapturaBlancas(fin,prof);
			}break;
			case enroque:
			{
				derechos_enroque[prof+1] &= 0xFC;				//borro bit 1 y 2 impidiendo asi cualquier otro enroque blanco en el futuro
				rey_b &= ClearMask(inicio);								//lo mismo con el rey
				rey_b |= SetMask(fin);
				hash_pos ^= Zobrist.escaques[Rey_b][inicio];
				hash_pos ^= Zobrist.escaques[Rey_b][fin];
				if (fin == G1)											//si fue 0-0
				{
					piezas_b &= ClearMask(H1);							//saco la pieza de H1
					piezas_b |= SetMask(F1);							//y la pongo en F1
					torres_b &= ClearMask(H1);							//lo mismo con la bitboard de las torres
					torres_b |= SetMask(F1);
					tablero &= ClearMask(H1);							//y de igual forma con el tablero
					tablero |= SetMask(F1);
					tablero90 &= ClearMask90(H1);						//y los rotados tambien
					tablero90 |= SetMask90(F1);
					tableroA1 &= ClearMaskA1(H1);
					tableroA1 |= SetMaskA1(F1);
					tableroA8 &= ClearMaskA8(H1);
					tableroA8 |= SetMaskA8(F1);
					hash_pos ^= Zobrist.escaques[Torre_b][H1];
					hash_pos ^= Zobrist.escaques[Torre_b][F1];
				}
				else													//0-0-0
				{
					piezas_b &= ClearMask(A1);							//saco la pieza de A1
					piezas_b |= SetMask(D1);							//y la pongo en D1
					torres_b &= ClearMask(A1);							//lo mismo con la bitboard de las torres
					torres_b |= SetMask(D1);
					tablero &= ClearMask(A1);							//y de igual forma con el tablero
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
				comio[prof] = 0;									//no puede haber sido captura si es un enroque
			}break;
			case ap:
			{
				peones_b &= ClearMask(inicio);							//borro el peon de la casilla de inicio
				peones_b |= SetMask(fin);								//y lo pongo en la de fin
				piezas_n &= ClearMask(fin-8);							//borro la pieza y
				peones_n &= ClearMask(fin-8);							//el peon negro comido al paso
				tablero &= ClearMask(fin-8);							//tambien lo saco del tablero
				tablero90 &= ClearMask90(fin-8);
				tableroA1 &= ClearMaskA1(fin-8);
				tableroA8 &= ClearMaskA8(fin-8);
				comio[prof] = PEON;								//fue al paso asi q es una captura de peon
                hash_pos ^= Zobrist.escaques[Peon_b][inicio];
                hash_pos ^= Zobrist.escaques[Peon_b][fin];
                hash_pos ^= Zobrist.escaques[Peon_n][fin-8];
			}break;
			case doble:
			{
				peones_b &= ClearMask(inicio);							//borro el peon de la casilla de inicio
				peones_b |= SetMask(fin);								//y lo pongo en la de fin
				comio[prof] = 0;									//no puede haber sido captura si es un doble mov de peon
				alpaso[prof+1] = Columna(inicio);				//aviso q es posible capturar al paso en esa columna (en el ply siguiente)
                hash_pos ^= Zobrist.escaques[Peon_b][inicio];
                hash_pos ^= Zobrist.escaques[Peon_b][fin];
			}break;
			case movrey:
			{
				ActualizaBlancas(inicio,fin);							//todo normal
				CapturaBlancas(fin,prof);
				derechos_enroque[prof+1] &= 0xFC;				//excepto q le quito los derechos de enroque a las blancas de aqui en adelante
			}break;
			case movtorre:
			{
				ActualizaBlancas(inicio,fin);							//todo normal
				CapturaBlancas(fin,prof);
				if (inicio == H1)										//si se movio la torre de H1
					derechos_enroque[prof+1] &= 0xFE;			//le quito los derechos de enroque corto a las blancas de aqui en adelante
				if (inicio == A1)										//si se movio la torre de H1
					derechos_enroque[prof+1] &= 0xFD;			//le quito los derechos de enroque largo a las blancas de aqui en adelante
			}break;														//en cualquier otro caso no hay q modificar nada de los enroques
		}
	}//end turno blancas
	else	//si le toca a las negras
	{
		piezas_n &= ClearMask(inicio);
		piezas_n |= SetMask(fin);
		switch (semibien)
		{
			case normal:
			{
				ActualizaNegras(inicio,fin);
				CapturaNegras(fin,prof);
			}break;
			case corona:
			{
				peones_n &= ClearMask(inicio);
				damas_n |= SetMask(fin);
				hash_pos ^= Zobrist.escaques[Peon_n][inicio];           //lo mismo con la llave zobrist
				hash_pos ^= Zobrist.escaques[Dama_n][fin];
				CapturaNegras(fin,prof);
			}break;
			case enroque:
			{
				derechos_enroque[prof+1] &= 0xF3;
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
					hash_pos ^= Zobrist.escaques[Torre_n][H8];
					hash_pos ^= Zobrist.escaques[Torre_n][F8];
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
				comio[prof] = 0;
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
				comio[prof] = PEON;
                hash_pos ^= Zobrist.escaques[Peon_n][inicio];
                hash_pos ^= Zobrist.escaques[Peon_n][fin];
                hash_pos ^= Zobrist.escaques[Peon_b][fin+8];
			}break;
			case doble:
			{
				peones_n &= ClearMask(inicio);
				peones_n |= SetMask(fin);
				comio[prof] = 0;
				alpaso[prof+1] = Columna(inicio);
                hash_pos ^= Zobrist.escaques[Peon_n][inicio];
                hash_pos ^= Zobrist.escaques[Peon_n][fin];
			}break;
			case movrey:
			{
				ActualizaNegras(inicio,fin);							//todo normal
				CapturaNegras(fin,prof);
				derechos_enroque[prof+1] &= 0xF3;				//excepto q le quito los derechos de enroque a las negras de aqui en adelante
			}break;
			case movtorre:
			{
				ActualizaNegras(inicio,fin);
				CapturaNegras(fin,prof);
				if (inicio == H8)
					derechos_enroque[prof+1] &= 0xFB;			//le quito los derechos de enroque corto a las negras de aqui en adelante
				if (inicio == A8)
					derechos_enroque[prof+1] &= 0xF7;			//le quito los derechos de enroque largo a las negras de aqui en adelante
			}break;
		}
	}//end turno negras
	//ahora actualizo la llave hash de la nueva posicion
    hash_pos ^= Zobrist.enroques[derechos_enroque[prof]];    //saco los anteriores derechos de enroque
    hash_pos ^= Zobrist.enroques[derechos_enroque[prof+1]];  //y pongo los nuevos en la llave (pueden ser los mismos)

    hash_pos ^= Zobrist.alpaso[alpaso[prof]];                //saco el viejo valor
    hash_pos ^= Zobrist.alpaso[alpaso[prof+1]];              //pongo el nuevo valor

	hash_pos ^= Zobrist.bando;                          //el bando siempre se togglea
	turno_c ^= 1;										//y por ultimo cambio el turno de a quien le toca jugar
}

void ActualizaBlancas(BYTE inicio, BYTE fin)
{
	if (peones_b & SetMask(inicio))						//si fue un peon el q movio
	{
		peones_b &= ClearMask(inicio);					//borro el peon de la casilla de inicio
		peones_b |= SetMask(fin);						//y lo pongo en la de fin
		hash_pos ^= Zobrist.escaques[Peon_b][inicio];   //actualizo de paso la llave Zobrist con lo que corresponde
		hash_pos ^= Zobrist.escaques[Peon_b][fin];
	}
	else												//sino sigo preguntando hasta ver que fue
	{
		if (caballos_b & SetMask(inicio))
		{
			caballos_b &= ClearMask(inicio);
			caballos_b |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Caballo_b][inicio];
			hash_pos ^= Zobrist.escaques[Caballo_b][fin];
		}
		else
		{
			if (alfiles_b & SetMask(inicio))
			{
				alfiles_b &= ClearMask(inicio);
				alfiles_b |= SetMask(fin);
                hash_pos ^= Zobrist.escaques[Alfil_b][inicio];
                hash_pos ^= Zobrist.escaques[Alfil_b][fin];
			}
			else
			{
				if (torres_b & SetMask(inicio))
				{
					torres_b &= ClearMask(inicio);
					torres_b |= SetMask(fin);
					hash_pos ^= Zobrist.escaques[Torre_b][inicio];
                    hash_pos ^= Zobrist.escaques[Torre_b][fin];
				}
				else
				{
					if (damas_b & SetMask(inicio))
					{
						damas_b &= ClearMask(inicio);
						damas_b |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Dama_b][inicio];
                        hash_pos ^= Zobrist.escaques[Dama_b][fin];
					}
					else								//no queda otra q el rey asi q no pregunto mas
					{
						rey_b &= ClearMask(inicio);
						rey_b |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Rey_b][inicio];
                        hash_pos ^= Zobrist.escaques[Rey_b][fin];
					}
				}
			}
		}
	}
}

void CapturaBlancas(BYTE fin, BYTE prof)			//esta es para recuperar despues la posicion (recuerda q fue lo q se comio en Negamax)
{
	if (piezas_n & SetMask(fin))						//si la jugada fue una captura
	{
		piezas_n &= ClearMask(fin);						//borro el bit correspondiente en el bando negro
		if (peones_n & SetMask(fin))					//si fue un peon el comido
		{
			peones_n &= ClearMask(fin);					//borro el bit correspondiente al peon negro
			comio[prof] = PEON;					//indico que se lastraron un peon para despues poder recuperarlo en DeshacerJugada()
            hash_pos ^= Zobrist.escaques[Peon_n][fin];  //tambien hay q actualizar la llave Zobrist (saco el peon negro)
		}
		else											//sino sigo preguntando hasta ver cual fue
		{
			if (caballos_n & SetMask(fin))
			{
				caballos_n &= ClearMask(fin);
				comio[prof] = CABALLO;
				hash_pos ^= Zobrist.escaques[Caballo_n][fin];
			}
			else
			{
				if (alfiles_n & SetMask(fin))
				{
					alfiles_n &= ClearMask(fin);
					comio[prof] = ALFIL;
					hash_pos ^= Zobrist.escaques[Alfil_n][fin];
				}
				else
				{
					if (torres_n & SetMask(fin))
					{
						torres_n &= ClearMask(fin);
						comio[prof] = TORRE;
						hash_pos ^= Zobrist.escaques[Torre_n][fin];
					}
					else								//no queda otra q la dama asi q no pregunto mas (el rey no puede ser comido)
					{
						damas_n &= ClearMask(fin);
						comio[prof] = DAMA;
						hash_pos ^= Zobrist.escaques[Dama_n][fin];
					}
				}
			}
		}
	}//end si fue una captura
	else												//si no pongo a 0 indicando q no fue captura
	{
		comio[prof] = 0;
	}
}

void ActualizaNegras(BYTE inicio, BYTE fin)
{
	if (peones_n & SetMask(inicio))						//si fue un peon el q movio
	{
		peones_n &= ClearMask(inicio);					//borro el peon de la casilla de inicio
		peones_n |= SetMask(fin);						//y lo pongo en la de fin
        hash_pos ^= Zobrist.escaques[Peon_n][inicio];   //actualizo la llave Zobrist sacando el peon negro de inicio
		hash_pos ^= Zobrist.escaques[Peon_n][fin];      //y poniendolo en fin
	}
	else												//sino sigo preguntando hasta ver que fue
	{
		if (caballos_n & SetMask(inicio))
		{
			caballos_n &= ClearMask(inicio);
			caballos_n |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Caballo_n][inicio];
            hash_pos ^= Zobrist.escaques[Caballo_n][fin];
		}
		else
		{
			if (alfiles_n & SetMask(inicio))
			{
				alfiles_n &= ClearMask(inicio);
				alfiles_n |= SetMask(fin);
                hash_pos ^= Zobrist.escaques[Alfil_n][inicio];
                hash_pos ^= Zobrist.escaques[Alfil_n][fin];
			}
			else
			{
				if (torres_n & SetMask(inicio))
				{
					torres_n &= ClearMask(inicio);
					torres_n |= SetMask(fin);
					hash_pos ^= Zobrist.escaques[Torre_n][inicio];
                    hash_pos ^= Zobrist.escaques[Torre_n][fin];
				}
				else
				{
					if (damas_n & SetMask(inicio))
					{
						damas_n &= ClearMask(inicio);
						damas_n |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Dama_n][inicio];
                        hash_pos ^= Zobrist.escaques[Dama_n][fin];
					}
					else								//no queda otra q el rey asi q no pregunto mas
					{
						rey_n &= ClearMask(inicio);
						rey_n |= SetMask(fin);
                        hash_pos ^= Zobrist.escaques[Rey_n][inicio];
                        hash_pos ^= Zobrist.escaques[Rey_n][fin];
					}
				}
			}
		}
	}
}

void CapturaNegras(BYTE fin, BYTE prof)			//esta es para recuperar despues la posicion (recuerda q fue lo q se comio en Negamax)
{
	if (piezas_b & SetMask(fin))						//si la jugada fue una captura
	{
		piezas_b &= ClearMask(fin);						//borro el bit correspondiente en el bando blanco
		if (peones_b & SetMask(fin))					//si fue un peon el comido
		{
			peones_b &= ClearMask(fin);					//borro el bit correspondiente al peon blanco
			comio[prof] = PEON;
			hash_pos ^= Zobrist.escaques[Peon_b][fin];
		}
		else											//sino sigo preguntando hasta ver cual fue
		{
			if (caballos_b & SetMask(fin))
			{
				caballos_b &= ClearMask(fin);
				comio[prof] = CABALLO;
				hash_pos ^= Zobrist.escaques[Caballo_b][fin];
			}
			else
			{
				if (alfiles_b & SetMask(fin))
				{
					alfiles_b &= ClearMask(fin);
					comio[prof] = ALFIL;
					hash_pos ^= Zobrist.escaques[Alfil_b][fin];
				}
				else
				{
					if (torres_b & SetMask(fin))
					{
						torres_b &= ClearMask(fin);
						comio[prof] = TORRE;
						hash_pos ^= Zobrist.escaques[Torre_b][fin];
					}
					else								//no queda otra q la dama asi q no pregunto mas (el rey no puede ser comido)
					{
						damas_b &= ClearMask(fin);
						comio[prof] = DAMA;
						hash_pos ^= Zobrist.escaques[Dama_b][fin];
					}
				}
			}
		}
	}//end si fue una captura
	else												//si no pongo a 0 indicando q no fue captura
	{
		comio[prof] = 0;
	}
}

void DeshacerJugada(BYTE inicio, BYTE fin,BYTE semibien,BYTE prof)	//todo lo contrario a hacer jugada
{
	if (turno_c == negras)									//si se cumple esto hay q deshacer una jugada del blanco. Todo al reves xq ya se paso una vez por HacerJugada()
	{
		switch (semibien)
		{
			case normal:
			{
				DesactualizaBlancas(inicio,fin);			//las restituciones normales
				DescapturaBlancas(fin, prof);		//esto se encarga de reponerle a las negras su material (si la jugada fue una captura)
			}break;
			case corona:
			{
				piezas_b &= ClearMask(fin);					//borro la pieza blanca de la casilla de fin
				piezas_b |= SetMask(inicio);				//y la pongo en la de inicio
				peones_b |= SetMask(inicio);				//pongo el peon q habia coronado
				damas_b &= ClearMask(fin);					//saco la dama q se habia coronado (por ahora solo se puede coronar dama)
				DescapturaBlancas(fin, prof);
                hash_pos ^= Zobrist.escaques[Peon_b][inicio];
                hash_pos ^= Zobrist.escaques[Dama_b][fin];
			}break;
			case enroque:
			{
				piezas_b &= ClearMask(fin);
				piezas_b |= SetMask(inicio);
				rey_b &= ClearMask(fin);					//devuelvo el rey a
				rey_b |= SetMask(inicio);					//su posicion de inicio
                hash_pos ^= Zobrist.escaques[Rey_b][inicio];
                hash_pos ^= Zobrist.escaques[Rey_b][fin];
				tablero &= ClearMask(fin);					//borro la casilla de fin q siempre va a quedar vacia al deshacer un enroque (no es captura)
				tablero90 &= ClearMask90(fin);
				tableroA1 &= ClearMaskA1(fin);
				tableroA8 &= ClearMaskA8(fin);
				if (fin == G1)								//0-0
				{
					piezas_b &= ClearMask(F1);
					piezas_b |= SetMask(H1);
					torres_b &= ClearMask(F1);
					torres_b |= SetMask(H1);
					tablero &= ClearMask(F1);
					tablero |= SetMask(H1);
					tablero90 &= ClearMask90(F1);
					tablero90 |= SetMask90(H1);
					tableroA1 &= ClearMaskA1(F1);
					tableroA1 |= SetMaskA1(H1);
					tableroA8 &= ClearMaskA8(F1);
					tableroA8 |= SetMaskA8(H1);
                    hash_pos ^= Zobrist.escaques[Torre_b][F1];
                    hash_pos ^= Zobrist.escaques[Torre_b][H1];
				}
				else										//0-0-0
				{
					piezas_b &= ClearMask(D1);
					piezas_b |= SetMask(A1);
					torres_b &= ClearMask(D1);
					torres_b |= SetMask(A1);
					tablero &= ClearMask(D1);
					tablero |= SetMask(A1);
					tablero90 &= ClearMask90(D1);
					tablero90 |= SetMask90(A1);
					tableroA1 &= ClearMaskA1(D1);
					tableroA1 |= SetMaskA1(A1);
					tableroA8 &= ClearMaskA8(D1);
					tableroA8 |= SetMaskA8(A1);
                    hash_pos ^= Zobrist.escaques[Torre_b][D1];
                    hash_pos ^= Zobrist.escaques[Torre_b][A1];
				}
			}break;
			case ap:
			{
				piezas_b &= ClearMask(fin);					//borro la pieza blanca de la casilla de fin
				piezas_b |= SetMask(inicio);				//y la pongo en la de inicio
				peones_b &= ClearMask(fin);					//borro el peon de la casilla de fin
				peones_b |= SetMask(inicio);				//y lo pongo en la de inicio
				piezas_n |= SetMask(fin-8);					//les devuelvo a las negras el peon capturado al paso
				peones_n |= SetMask(fin-8);
				tablero |= SetMask(fin-8);					//y tambien recompongo el tablero
				tablero &= ClearMask(fin);					//borro del tablero el peoncito
                tablero90 |= SetMask90(fin-8);
				tablero90 &= ClearMask90(fin);
				tableroA1 |= SetMaskA1(fin-8);
				tableroA1 &= ClearMaskA1(fin);
				tableroA8 |= SetMaskA8(fin-8);
				tableroA8 &= ClearMaskA8(fin);
                hash_pos ^= Zobrist.escaques[Peon_b][inicio];
                hash_pos ^= Zobrist.escaques[Peon_b][fin];
                hash_pos ^= Zobrist.escaques[Peon_n][fin-8];
			}break;
			case doble:
			{
				piezas_b &= ClearMask(fin);					//borro la pieza blanca de la casilla de fin
				piezas_b |= SetMask(inicio);				//y la pongo en la de inicio
				peones_b &= ClearMask(fin);					//borro el peon de la casilla de fin
				peones_b |= SetMask(inicio);				//y lo pongo en la de inicio
				tablero &= ClearMask(fin);					//vacio la casilla de fin en el tablero
                tablero90 &= ClearMask90(fin);
                tableroA1 &= ClearMaskA1(fin);
                tableroA8 &= ClearMaskA8(fin);
                hash_pos ^= Zobrist.escaques[Peon_b][inicio];
                hash_pos ^= Zobrist.escaques[Peon_b][fin];
			}break;
			case movrey:
			{
				DesactualizaBlancas(inicio,fin);			//las restituciones normales (no se necesita nada mas)
				DescapturaBlancas(fin, prof);
			}break;
			case movtorre:
			{
				DesactualizaBlancas(inicio,fin);			//lo mismo para el movimiento de una torre
				DescapturaBlancas(fin, prof);
			}break;
		}
	}//fin de si le toca a las blancas
	else	//si le toca a las negras
	{
		switch (semibien)
		{
			case normal:
			{
				DesactualizaNegras(inicio,fin);				//las restituciones normales
				DescapturaNegras(fin, prof);
			}break;
			case corona:
			{
				piezas_n &= ClearMask(fin);
				piezas_n |= SetMask(inicio);
				peones_n |= SetMask(inicio);
				damas_n &= ClearMask(fin);					//saco la dama q se habia coronado (por ahora solo se puede coronar dama)
				DescapturaNegras(fin, prof);
                hash_pos ^= Zobrist.escaques[Peon_n][inicio];
                hash_pos ^= Zobrist.escaques[Dama_n][fin];
			}break;
			case enroque:
			{
				piezas_n &= ClearMask(fin);
				piezas_n |= SetMask(inicio);
				rey_n &= ClearMask(fin);
				rey_n |= SetMask(inicio);
                hash_pos ^= Zobrist.escaques[Rey_n][inicio];
                hash_pos ^= Zobrist.escaques[Rey_n][fin];
				tablero &= ClearMask(fin);
				tablero90 &= ClearMask90(fin);
				tableroA1 &= ClearMaskA1(fin);
				tableroA8 &= ClearMaskA8(fin);
				if (fin == G8)								//0-0
				{
					piezas_n &= ClearMask(F8);
					piezas_n |= SetMask(H8);
					torres_n &= ClearMask(F8);
					torres_n |= SetMask(H8);
					tablero &= ClearMask(F8);
					tablero |= SetMask(H8);
					tablero90 &= ClearMask90(F8);
					tablero90 |= SetMask90(H8);
					tableroA1 &= ClearMaskA1(F8);
					tableroA1 |= SetMaskA1(H8);
					tableroA8 &= ClearMaskA8(F8);
					tableroA8 |= SetMaskA8(H8);
                    hash_pos ^= Zobrist.escaques[Torre_n][F8];
                    hash_pos ^= Zobrist.escaques[Torre_n][H8];
				}
				else										//0-0-0
				{
					piezas_n &= ClearMask(D8);
					piezas_n |= SetMask(A8);
					torres_n &= ClearMask(D8);
					torres_n |= SetMask(A8);
					tablero &= ClearMask(D8);
					tablero |= SetMask(A8);
					tablero90 &= ClearMask90(D8);
					tablero90 |= SetMask90(A8);
					tableroA1 &= ClearMaskA1(D8);
					tableroA1 |= SetMaskA1(A8);
					tableroA8 &= ClearMaskA8(D8);
					tableroA8 |= SetMaskA8(A8);
                    hash_pos ^= Zobrist.escaques[Torre_n][D8];
                    hash_pos ^= Zobrist.escaques[Torre_n][A8];
				}
			}break;
			case ap:
			{
				piezas_n &= ClearMask(fin);
				piezas_n |= SetMask(inicio);
				peones_n &= ClearMask(fin);
				peones_n |= SetMask(inicio);
				piezas_b |= SetMask(fin+8);
				peones_b |= SetMask(fin+8);
				tablero |= SetMask(fin+8);
				tablero &= ClearMask(fin);
				tablero90 |= SetMask90(fin+8);
				tablero90 &= ClearMask90(fin);
				tableroA1 |= SetMaskA1(fin+8);
				tableroA1 &= ClearMaskA1(fin);
				tableroA8 |= SetMaskA8(fin+8);
				tableroA8 &= ClearMaskA8(fin);
                hash_pos ^= Zobrist.escaques[Peon_n][inicio];
                hash_pos ^= Zobrist.escaques[Peon_n][fin];
                hash_pos ^= Zobrist.escaques[Peon_b][fin+8];
			}break;
			case doble:
			{
				piezas_n &= ClearMask(fin);
				piezas_n |= SetMask(inicio);
				peones_n &= ClearMask(fin);
				peones_n |= SetMask(inicio);
				tablero &= ClearMask(fin);
				tablero90 &= ClearMask90(fin);
				tableroA1 &= ClearMaskA1(fin);
				tableroA8 &= ClearMaskA8(fin);
                hash_pos ^= Zobrist.escaques[Peon_n][inicio];
                hash_pos ^= Zobrist.escaques[Peon_n][fin];
			}break;
			case movrey:
			{
				DesactualizaNegras(inicio,fin);
				DescapturaNegras(fin, prof);
			}break;
			case movtorre:
			{
				DesactualizaNegras(inicio,fin);
				DescapturaNegras(fin, prof);
			}break;
		}
	}//fin de si le toca a las negras
	tablero |= SetMask(inicio);								//esto se da siempre y es comun a todos los casos (indicar q hay una pieza en la casilla de inicio)
	tablero90 |= SetMask90(inicio);
	tableroA1 |= SetMaskA1(inicio);
	tableroA8 |= SetMaskA8(inicio);
	//ahora actualizo la llave hash de la nueva posicion
    hash_pos ^= Zobrist.enroques[derechos_enroque[prof]];    //pongo los anteriores derechos de enroque
    hash_pos ^= Zobrist.enroques[derechos_enroque[prof+1]];  //y saco los nuevos en la llave (pueden ser los mismos)

    hash_pos ^= Zobrist.alpaso[alpaso[prof]];                //pongo el valor viejo (anterior a la jugada)
    hash_pos ^= Zobrist.alpaso[alpaso[prof+1]];              //y saco el nuevo

	hash_pos ^= Zobrist.bando;                              //el bando siempre se togglea
	turno_c ^= 1;											//cambio el turno de a quien le toca jugar
}

void DesactualizaBlancas(BYTE inicio, BYTE fin)
{
	piezas_b &= ClearMask(fin);			//borro la pieza blanca de la casilla de fin
	piezas_b |= SetMask(inicio);		//y la pongo en la de inicio
	if (peones_b & SetMask(fin))		//si fue un peon el q movio
	{
		peones_b &= ClearMask(fin);		//borro el peon de la casilla de fin
		peones_b |= SetMask(inicio);	//y lo pongo en la de inicio
        hash_pos ^= Zobrist.escaques[Peon_b][fin];      //lo mismo hago con la llave zobrist
		hash_pos ^= Zobrist.escaques[Peon_b][inicio];
	}
	else
	{
		if (caballos_b & SetMask(fin))
		{
			caballos_b &= ClearMask(fin);
			caballos_b |= SetMask(inicio);
            hash_pos ^= Zobrist.escaques[Caballo_b][fin];
            hash_pos ^= Zobrist.escaques[Caballo_b][inicio];
		}
		else
		{
			if (alfiles_b & SetMask(fin))
			{
				alfiles_b &= ClearMask(fin);
				alfiles_b |= SetMask(inicio);
                hash_pos ^= Zobrist.escaques[Alfil_b][fin];
                hash_pos ^= Zobrist.escaques[Alfil_b][inicio];
			}
			else
			{
				if (torres_b & SetMask(fin))
				{
					torres_b &= ClearMask(fin);
					torres_b |= SetMask(inicio);
                    hash_pos ^= Zobrist.escaques[Torre_b][fin];
                    hash_pos ^= Zobrist.escaques[Torre_b][inicio];
				}
				else
				{
					if (damas_b & SetMask(fin))
					{
						damas_b &= ClearMask(fin);
						damas_b |= SetMask(inicio);
                        hash_pos ^= Zobrist.escaques[Dama_b][fin];
                        hash_pos ^= Zobrist.escaques[Dama_b][inicio];
					}
					else
					{
						rey_b &= ClearMask(fin);
						rey_b |= SetMask(inicio);
                        hash_pos ^= Zobrist.escaques[Rey_b][fin];
                        hash_pos ^= Zobrist.escaques[Rey_b][inicio];
					}
				}
			}
		}
	}
}

void DescapturaBlancas(BYTE fin, BYTE prof)
{
	switch (comio[prof])
	{
		case 0:	//no fue captura
		{
			tablero &= ClearMask(fin);		//no habia nada en la casilla de fin entonces la borro (en cualquier otro caso no xq es captura osea q estaba ocupada antes)
            tablero90 &= ClearMask90(fin);
            tableroA1 &= ClearMaskA1(fin);
            tableroA8 &= ClearMaskA8(fin);
		}break;
		case PEON:							//fue captura de un peon
		{
			peones_n |= SetMask(fin);		//entonces les devuelvo el peon "descomido" a las negras
			piezas_n |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Peon_n][fin];  //tambien cambio la llave hash con lo q corresponde (agrego el peon n)
		}break;
		case CABALLO:
		{
			caballos_n |= SetMask(fin);
			piezas_n |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Caballo_n][fin];
		}break;
		case ALFIL:
		{
			alfiles_n |= SetMask(fin);
			piezas_n |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Alfil_n][fin];
		}break;
		case TORRE:
		{
			torres_n |= SetMask(fin);
			piezas_n |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Torre_n][fin];
		}break;
		case DAMA:
		{
			damas_n |= SetMask(fin);
			piezas_n |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Dama_n][fin];
		}break;
	}
}

void DesactualizaNegras(BYTE inicio, BYTE fin)
{
	piezas_n &= ClearMask(fin);			//borro la pieza negra de la casilla de fin
	piezas_n |= SetMask(inicio);		//y la pongo en la de inicio
	if (peones_n & SetMask(fin))		//si fue un peon el q movio
	{
		peones_n &= ClearMask(fin);		//borro el peon de la casilla de fin
		peones_n |= SetMask(inicio);	//y lo pongo en la de inicio
        hash_pos ^= Zobrist.escaques[Peon_n][fin];
        hash_pos ^= Zobrist.escaques[Peon_n][inicio];
	}
	else
	{
		if (caballos_n & SetMask(fin))
		{
			caballos_n &= ClearMask(fin);
			caballos_n |= SetMask(inicio);
            hash_pos ^= Zobrist.escaques[Caballo_n][fin];
            hash_pos ^= Zobrist.escaques[Caballo_n][inicio];
		}
		else
		{
			if (alfiles_n & SetMask(fin))
			{
				alfiles_n &= ClearMask(fin);
				alfiles_n |= SetMask(inicio);
                hash_pos ^= Zobrist.escaques[Alfil_n][fin];
                hash_pos ^= Zobrist.escaques[Alfil_n][inicio];
			}
			else
			{
				if (torres_n & SetMask(fin))
				{
					torres_n &= ClearMask(fin);
					torres_n |= SetMask(inicio);
                    hash_pos ^= Zobrist.escaques[Torre_n][fin];
                    hash_pos ^= Zobrist.escaques[Torre_n][inicio];
				}
				else
				{
					if (damas_n & SetMask(fin))
					{
						damas_n &= ClearMask(fin);
						damas_n |= SetMask(inicio);
                        hash_pos ^= Zobrist.escaques[Dama_n][fin];
                        hash_pos ^= Zobrist.escaques[Dama_n][inicio];
					}
					else
					{
						rey_n &= ClearMask(fin);
						rey_n |= SetMask(inicio);
                        hash_pos ^= Zobrist.escaques[Rey_n][fin];
                        hash_pos ^= Zobrist.escaques[Rey_n][inicio];
					}
				}
			}
		}
	}
}

void DescapturaNegras(BYTE fin, BYTE prof)
{
	switch (comio[prof])
	{
		case 0:								//no fue captura
		{
			tablero &= ClearMask(fin);		//no habia nada en la casilla de fin entonces la borro (en cualquier otro caso no xq es captura osea q estaba ocupada antes)
            tablero90 &= ClearMask90(fin);
            tableroA1 &= ClearMaskA1(fin);
            tableroA8 &= ClearMaskA8(fin);
		}break;
		case PEON:							//fue captura de un peon
		{
			peones_b |= SetMask(fin);		//entonces les devuelvo el peon "descomido" a las blancas
			piezas_b |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Peon_b][fin];
		}break;
		case CABALLO:
		{
			caballos_b |= SetMask(fin);
			piezas_b |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Caballo_b][fin];
		}break;
		case ALFIL:
		{
			alfiles_b |= SetMask(fin);
			piezas_b |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Alfil_b][fin];
		}break;
		case TORRE:
		{
			torres_b |= SetMask(fin);
			piezas_b |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Torre_b][fin];
		}break;
		case DAMA:
		{
			damas_b |= SetMask(fin);
			piezas_b |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Dama_b][fin];
		}break;
	}
}

void Jugar(BYTE inicio,BYTE fin,BYTE bien)									//hace la jugada verdaderamente (no dentro de un analisis)
{
    //primero elimino de la llave hash los datos de la posicion vieja
    hash_pos ^= Zobrist.enroques[derechos_enroque[0]];  //saco los viejos derechos de la llave

    hash_pos ^= Zobrist.alpaso[alpaso[0]];              //saco de la llave el viejo valor de alpaso

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
				derechos_enroque[0] &= 0xF3;							//le quito los derechos de enroque a las negras de aqui en adelante
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
    hash_pos ^= Zobrist.enroques[derechos_enroque[0]];  //pongo los nuevos derechos en la llave (pueden ser los mismos)
    hash_pos ^= Zobrist.alpaso[alpaso[0]];              //pongo el nuevo alpaso en la llave
	hash_pos ^= Zobrist.bando;                          //el bando siempre se togglea

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

int Evaluar()																	//determina el valor relativo (ajedrecistico) de las hojas del arbol de busqueda
{
	//lo primero y principal es obviamente el material q hay sobre el tablero

	total = 900 * (POPCNT(damas_b) - POPCNT(damas_n));							//la primera es = para q inicialice
	total += 500 * (POPCNT(torres_b) - POPCNT(torres_n));
	total += 310 * (POPCNT(alfiles_b) - POPCNT(alfiles_n));						//doy 310 al alfil para premiarlo un poco pero no me salgo de los valores "clasicos" convencionales
	total += 300 * (POPCNT(caballos_b) - POPCNT(caballos_n));
	total += 100 * (POPCNT(peones_b) - POPCNT(peones_n));						//un peon = 100 cp (centipeones)
	if ((alfiles_b & casillas_b) && (alfiles_b & casillas_n))					//si el blanco tiene pareja de alfiles
		total += 30;															//le doy un buen premio
	if ((alfiles_n & casillas_b) && (alfiles_n & casillas_n))					//lo mismo para el negro
		total -= 30;

	if (!(damas_b|damas_n)||POPCNT(tablero)<11)									//si no quedan damas o quedan 10 piezas o menos estamos en un final
		total += EvaluarFinal();
	else
		total += EvaluarMJ();													//sino q evalue el medio juego

	if (turno_c == blancas)
		return total;
	return -total;
}

int EvaluarFinal()
{
	int subtotal = 0;
	BYTE sqcorona;

	switch (POPCNT(tablero))
	{
        case 2:
        {
            return -total;
        }break;
		case 3:
		{
			if (alfiles_b || alfiles_n || caballos_b || caballos_n)
				return -total;					//esto deja a la valoracion en 0 como debe ser ya q no hay material para ganar
			if (peones_b)
			{
				subtotal -= Distancia(MSB(rey_b),MSB(peones_b));
				subtotal += 4 * Distancia(MSB(rey_n),MSB(peones_b));
                if (!(rey_n & regladelcuadrado[negras][turno_c][inicio]))	//si el rey no entra en el cuadrado
                    return subtotal + 900;                                  //la coronacion es inevitable
                //si esta dentro no asegura que sea tablas

				return subtotal;
			}
			if (peones_n)
			{
				subtotal -= 4 * Distancia(MSB(rey_b),MSB(peones_n));
				subtotal += Distancia(MSB(rey_n),MSB(peones_n));
				if (!(rey_b & regladelcuadrado[blancas][turno_c][inicio]))
                    return subtotal - 900;
				return subtotal;
			}
			if (damas_b || torres_b)
			{
				subtotal += mate[MSB(rey_n)];
				subtotal -= Distancia(MSB(rey_b),MSB(rey_n));
				return subtotal;
			}
			if (damas_n || torres_n)
			{
				subtotal -= mate[MSB(rey_b)];
				subtotal += Distancia(MSB(rey_b),MSB(rey_n));
				return subtotal;
			}
		}break;
		case 4:
		{
		    if (peones_b|peones_n)
            {
                if (peones_b && peones_n)
                {
                    sqcorona = MSB(fila_mask[0] & columna_mask[Columna(MSB(peones_n))]);
                    if (Distancia(MSB(rey_b),sqcorona) <= Distancia(MSB(peones_n),sqcorona))
                        subtotal -= 3 * Distancia(MSB(rey_b),sqcorona);							//q ambos reyes intenten parar los peones rivales (si llegan)
                    else
                        subtotal -= 800;														//si no llega entonces hay coronacion de las negras
                    sqcorona = MSB(fila_mask[7] & columna_mask[Columna(MSB(peones_b))]);
                    if (Distancia(MSB(rey_n),sqcorona) <= Distancia(MSB(peones_b),sqcorona))
                        subtotal += 3 * Distancia(MSB(rey_n),sqcorona);
                    else
                        subtotal += 800;														//si no llega entonces hay coronacion
                }
            }
            if (total > 640)    //cubre AA,AT,AD,CT,CD,TT,TD y DD contra el rey negro solo los cuales son todos finales
			{                   //facilmente ganados con una minima heuristica de progreso comun a todos
				subtotal += mate[MSB(rey_n)];   //llevar el rey indefenso a los bordes y esquinas del tablero
				subtotal -= Distancia(MSB(rey_b),MSB(rey_n));   //mantener el rey atacante cerca del rey contrario
				return subtotal;                                //solo con esto una busqueda de 2 ply encuentra el mate
			}
			if (total < -640)   //el complemento de lo anterior si son las negras las q estan ganando
			{
				subtotal -= mate[MSB(rey_b)];
				subtotal += Distancia(MSB(rey_b),MSB(rey_n));
				return subtotal;
			}
            if (abs(total) < 20)    //C vs c, A vs a, A vs C, T vs t y D vs d los que son evaluados todos como 0
                return 0;           //(las colgadas y mates quedan a cargo de la busqueda)
            if (damas_b)            //si entra aca solo queda el final RD vs rt
            {
				subtotal += mate[MSB(rey_n)];
				subtotal -= Distancia(MSB(rey_b),MSB(rey_n));
				return subtotal;
            }
            if (damas_n)            //RT vs rd
            {
				subtotal -= mate[MSB(rey_b)];
				subtotal += Distancia(MSB(rey_b),MSB(rey_n));
				return subtotal;
            }
            if (torres_b)           //solo entra en RT vs ra o RT vs rc
            {                       //si bien este final es tablas le damos algo mas que 0 y una heuristica minima para que
                                    //intente ganarlo ya que es posible jugandose mal (o defenderlo)
				subtotal -= rey_final[MSB(rey_n)];  //IMPORTANTE: aca rey_final SIEMPRE es negativo (por lo q se hace positivo)
				subtotal -= Distancia(MSB(rey_b),MSB(rey_n));   //que los reyes se mantengan cerca siempre ayuda a molestar
				return -total + subtotal + 10;      //la valoracion varia (aproximadamente) entre 10 y 30
            }
            if (torres_n)           //RC vs rt o RA vs RT
            {
				subtotal += rey_final[MSB(rey_b)];
				subtotal += Distancia(MSB(rey_b),MSB(rey_n));
				return -total + subtotal - 10;
            }
            if (abs(total) == 600)  //esto solo se da en RCC vs r (y el opuesto con negras) los cuales indico como tablas
                return -600;        //esto hace 0 la valoracion
			if ((alfiles_b & casillas_b) && caballos_b)         //ahora los 4 casos de mates de A y C
			{
				subtotal += mate_ac_b[MSB(rey_n)];
				subtotal -= Distancia(MSB(rey_b),MSB(rey_n));			//resto puntaje por estar lejos un rey de otro asi lo obligo a hacercarse
				if (Distancia(MSB(caballos_b),MSB(rey_n))>5)
					subtotal -= 1;
				return subtotal;
			}
			if ((alfiles_b & casillas_n) && caballos_b)
			{
				subtotal += mate_ac_n[MSB(rey_n)];
				subtotal -= Distancia(MSB(rey_b),MSB(rey_n));
				if (Distancia(MSB(caballos_b),MSB(rey_n))>5)
					subtotal -= 1;
				return subtotal;
			}
			if ((alfiles_n & casillas_b) && caballos_n)
			{
				subtotal -= mate_ac_b[MSB(rey_b)];
				subtotal += Distancia(MSB(rey_b),MSB(rey_n));
				if (Distancia(MSB(caballos_n),MSB(rey_b))>5)
					subtotal += 1;
				return subtotal;
			}
			if ((alfiles_n & casillas_n) && caballos_n)
			{
				subtotal -= mate_ac_n[MSB(rey_b)];
				subtotal += Distancia(MSB(rey_b),MSB(rey_n));
				if (Distancia(MSB(caballos_n),MSB(rey_b))>5)
					subtotal += 1;
				return subtotal;
			}
			if (abs(total) == 620)  //por ultimo miro si se dio el final RAA vs r o viceversa con alfiles del mismo color lo q es
                return -620;        //tablas ya q no existen posiciones de mate, aunque muy raro q alguna vez se entre aca
		}break;
	}
    inicio = MSB(rey_b);
	subtotal += rey_final[inicio];
	inicio = MSB(rey_n);
	subtotal -= rey_final[inicio];
	return subtotal;
}

int EvaluarMJ()
{
	int subtotal = 0;
	BITBOARD auxiliar;

	//estructura de peones

	auxiliar = peones_b;
	while (auxiliar)
	{
		inicio = MSB(auxiliar);
		auxiliar ^= SetMask(inicio);
		total += peon_pos[blancas][inicio];									//incluyo las posiciones de los peones como un factor mas
//		if (!(peon_pasado[blancas][inicio] & peones_n))						//si el peon esta pasado le sumo a las blancas un plus
//			total += 25;
//		if (!(peon_aislado[inicio] & peones_b))								//si el peon esta aislado le resto
//			total -= 15;
//		if (!(peones_conectados[inicio] & peones_b))						//si el peon esta desconectado de la estructura
//			total -= 20; 													//lo castigo (no quiero estructuras tipo h4-g2-f4 o asi de horribles)
		if (POPCNT(columna_mask[Columna(inicio)] & peones_b) > 1)			//si hay mas de un peon en la misma columna estan doblados y se penaliza
			total -= 20;													//le quito 20 por cada peon (si hay dos doblados la perdida total es 40 y si son 3 son 60, etc.)
	};
	auxiliar = peones_n;
	while (auxiliar)
	{
		inicio = MSB(auxiliar);
		auxiliar ^= SetMask(inicio);
		total -= peon_pos[negras][inicio];
//		if (!(peon_pasado[negras][inicio] & peones_b))
//			total -= 25;
//		if (!(peon_aislado[inicio] & peones_n))
//			total += 15;
//		if (!(peones_conectados[inicio] & peones_n))
//			total += 20;
		if (POPCNT(columna_mask[Columna(inicio)] & peones_n) > 1)
			total += 20;
	};
	auxiliar = caballos_b;
	while (auxiliar)
	{
		inicio = MSB(auxiliar);
		auxiliar ^= SetMask(inicio);
		subtotal += caballo_pos[blancas][inicio];
		subtotal += POPCNT(ataques_caballos[inicio] & (!piezas_b));  //evalua la movilidad de caballos ademas
	};
	auxiliar = alfiles_b;
	while (auxiliar)
	{
		inicio = MSB(auxiliar);
		auxiliar ^= SetMask(inicio);
		subtotal += alfil_pos[blancas][inicio];
		subtotal += POPCNT(GeneraAlfil(inicio) & (!piezas_b));
	};
	auxiliar = torres_b;
	while (auxiliar)
	{
		inicio = MSB(auxiliar);
		auxiliar ^= SetMask(inicio);
		subtotal += torre_pos[blancas][inicio];
		subtotal += POPCNT(GeneraTorre(inicio) & (!piezas_b));
		if (!(columna_mask[Columna(inicio)] & peones_b))	//si es una columna semiabierta (o posiblemente abierta)
			subtotal += 25;									//le sumo por buena ubicacion
	};
	auxiliar = damas_b;
	while (auxiliar)
	{
		inicio = MSB(auxiliar);
		auxiliar ^= SetMask(inicio);
		subtotal += dama_pos[inicio];
        subtotal += POPCNT(GeneraAlfil(inicio) & (!piezas_b));
        subtotal += POPCNT(GeneraTorre(inicio) & (!piezas_b));
	};

	//lo mismo para las piezas negras

	auxiliar = caballos_n;
	while (auxiliar)
	{
		inicio = MSB(auxiliar);
		auxiliar ^= SetMask(inicio);
		subtotal -= caballo_pos[negras][inicio];
		subtotal += POPCNT(ataques_caballos[inicio] & (!piezas_n));
	};
	auxiliar = alfiles_n;
	while (auxiliar)
	{
		inicio = MSB(auxiliar);
		auxiliar ^= SetMask(inicio);
		subtotal -= alfil_pos[negras][inicio];
		subtotal -= POPCNT(GeneraAlfil(inicio) & (!piezas_n));
	};
	auxiliar = torres_n;
	while (auxiliar)
	{
		inicio = MSB(auxiliar);
		auxiliar ^= SetMask(inicio);
		subtotal -= torre_pos[negras][inicio];
		subtotal -= POPCNT(GeneraTorre(inicio) & (!piezas_n));
		if (!(columna_mask[Columna(inicio)] & peones_n))
			subtotal -= 25;
	};
	auxiliar = damas_n;
	while (auxiliar)
	{
		inicio = MSB(auxiliar);
		auxiliar ^= SetMask(inicio);
		subtotal -= dama_pos[inicio];
        subtotal -= POPCNT(GeneraAlfil(inicio) & (!piezas_n));
        subtotal -= POPCNT(GeneraTorre(inicio) & (!piezas_n));
	};

	//seguridad del rey

	inicio = MSB(rey_b);
	subtotal += rey_pos[blancas][inicio];
	inicio = MSB(rey_n);
	subtotal -= rey_pos[negras][inicio];


	//movilidad de piezas

	//dominio del centro
	return subtotal;
}

BYTE EnJaque(BYTE turno_c)														//indica si el bando q acaba de jugar esta o no en jaque
{
	BYTE sq;

	if (turno_c == blancas)														//si se da esto quiere decir q acaban de jugar las negras (ya pase por HacerJugada)
	{
		sq = LSB(rey_n);
		if ((ataques_caballos[sq] & caballos_b) || (ataques_rey[sq] & rey_b))	//se cumple si un caballo o un rey atacan sq (osea q estaria el rey negro en jaque)
			return 1;															//esta en jaque
		if (ataques_peones[negras][sq] & peones_b)								//en vez de ver si un peon blanco ataca sq miro si un peon negro en sq ataca un peon blanco
			return 1;
		atack_alfil = GeneraAlfil(sq); 											//faltan las piezas deslizantes q trato de manera similar
		if ((atack_alfil & alfiles_b) || (atack_alfil & damas_b))				//si una pieza diagonal en sq ataca una dama o alfil blanco entonces es reciproco y sq esta jaqueada
			return 1;
		atack_torre = GeneraTorre(sq);
		if ((atack_torre & torres_b) || (atack_torre & damas_b))				//lo mismo para las piezas ortogonales
			return 1;
		return 0;
	}
	else
	{
		sq = LSB(rey_b);
		if ((ataques_caballos[sq] & caballos_n) || (ataques_rey[sq] & rey_n))
			return 1;	//esta en jaque
		if (ataques_peones[blancas][sq] & peones_n)
			return 1;
		atack_alfil = GeneraAlfil(sq);
		if ((atack_alfil & alfiles_n) || (atack_alfil & damas_n))
			return 1;
		atack_torre = GeneraTorre(sq);
		if ((atack_torre & torres_n) || (atack_torre & damas_n))
			return 1;
		return 0;
	}
}

BYTE CasillaAtacada(BYTE sq,BYTE bando)						//pide como entrada la casilla a considerar, y el color del bando q se quiere ver si esta atacando sq o no
{
	if (bando == blancas)
	{
		if ((ataques_caballos[sq] & caballos_b) || (ataques_rey[sq] & rey_b))	//se cumple si un caballo o un rey blancos atacan sq
			return 1;															//sq esta amenazada por las blancas
		if (ataques_peones[negras][sq] & peones_b)								//en vez de ver si un peon blanco ataca sq miro si un peon negro en sq ataca un peon blanco
			return 1;
		atack_alfil = GeneraAlfil(sq); 											//faltan las piezas deslizantes q trato de manera similar
		if ((atack_alfil & alfiles_b) || (atack_alfil & damas_b))				//si una pieza diagonal en sq ataca una dama o alfil blanco entonces es reciproco y sq esta jaqueada
			return 1;
		atack_torre = GeneraTorre(sq);
		if ((atack_torre & torres_b) || (atack_torre & damas_b))				//lo mismo para las piezas ortogonales
			return 1;
		return 0;																//si no se da ningun caso anterior la casilla no esta atacada
	}
	else
	{
		if ((ataques_caballos[sq] & caballos_n) || (ataques_rey[sq] & rey_n))
			return 1;	//esta en jaque
		if (ataques_peones[blancas][sq] & peones_n)
			return 1;
		atack_alfil = GeneraAlfil(sq);
		if ((atack_alfil & alfiles_n) || (atack_alfil & damas_n))
			return 1;
		atack_torre = GeneraTorre(sq);
		if ((atack_torre & torres_n) || (atack_torre & damas_n))
			return 1;
		return 0;
	}
}

BYTE versitermino()													//si la partida termino por algun motivo (el q sea) devuelve TRUE y sino FALSE y se sigue jugando
{
	//primero miro si alguien esta en mate o quedo ahogado
	if (Ahogado())		//si al q le toca el turno ahora no dispone de ninguna jugada legal esta ahogado o en mate
	{
		if (EnJaque(turno_c^1))	//si esta en jaque entonces es mate solo falta ver quien esta en esa situacion
		{
			if (turno_c == blancas)
				return NEGRASGANAN;
			return BLANCASGANAN;
		}
		return TABLAS;			//es ahogado
	}

	//verifico si la partida termino por insuficiencia de material
	if (POPCNT(tablero) == 2)										//si solo quedan los reyes
		return TABLAS;												//material insuficiente
	if (POPCNT(tablero) == 3)										//si quedan los reyes y una pieza
	{
		if (alfiles_n || alfiles_b || caballos_n || caballos_b)		//y es un alfil o un caballo
			return TABLAS;											//tambien es material insuficiente
	}
	//miro si la partida termino por regla de las 50 jugadas
	if (jugadas_reversibles == 100)
		return TABLAS;
	//miro si termino por 3 repeticiones
	if (TresRepet())
		return TABLAS;
	return 0;			//si no se cumple nada de esto la partida continua
}

BYTE Ahogado()		//dice si el bando al q le toca jugar dispone o no de al menos una jugada posible
{
	BYTE z;

	GenerarTodas(0);
	for (z = 0 ; z < ultimajugada[0] ; z = z + 3)
	{
		HacerJugada(jugadas[0][z],jugadas[0][z+1],jugadas[0][z+2],0);
		if (!EnJaque(turno_c))	//si el rey del bando q jugo no esta en jaque hay al menos una jugada legal asi q
		{
			DeshacerJugada(jugadas[0][z],jugadas[0][z+1],jugadas[0][z+2],0);
			return 0;			//no esta ahogado ni en mate
		}
		DeshacerJugada(jugadas[0][z],jugadas[0][z+1],jugadas[0][z+2],0);
	}
	//si pasa aca es q esta ahogado o en mate
	return 1;
}

BYTE Repite()		//si la posicion actual se ha producido antes (se encuentra dentro de la lista de 10 ultimas posiciones) retorna 1 y sino retorna 0
{					//sirve en NegaMax() para q el programa sepa q las repeticiones son tablas (y q las evalue como tal) y aviso a la primer repeticion por las dudas
    unsigned int i;

	for (i=0;i<MAXPROF;i++)						//simplemente comparo la posicion actual contra las 10 anteriores a ver si hay coincidencias
	{
		if (turno_c == turnos_anteriores[i] && derechos_enroque[0] == enroques_anteriores[i] && alpaso[0] == alpaso_anteriores[i])
		{
		if (piezas_anteriores[i][0] == peones_b && piezas_anteriores[i][1] == caballos_b && piezas_anteriores[i][2] == alfiles_b && piezas_anteriores[i][3] == torres_b)
		{
		if (piezas_anteriores[i][4] == damas_b && piezas_anteriores[i][5] == rey_b && piezas_anteriores[i][6] == peones_n && piezas_anteriores[i][7] == caballos_n)
		{
		if(piezas_anteriores[i][8] == alfiles_n && piezas_anteriores[i][9] == torres_n && piezas_anteriores[i][10] == damas_n && piezas_anteriores[i][11] == rey_n)
		{
			return 1;						//solo si todo esta igual es una repeticion y aviso
		}
		}
		}
		}
	}
	return 0;
}

BYTE TresRepet()
{
	int i,j,coincide;

	coincide = 1;										//siempre existe una coincidencia (la posicion actual coincide con sigo misma x lo q es la primera) hay q ver si hay 2 mas
	if (jugadas_reversibles == 0)						//si la ultima jugada fue irreversible
	{
		for (i=0;i<MAXPROF;i++)
		{
			for (j=0;j<12;j++)
			{
				piezas_anteriores[i][j] = 0;			//borro todo el historial de posiciones de las piezas
			}
			turnos_anteriores[i] = 2;					//esto es borrar para turno ya q solo puede valer 0 o 1 nunca va a coincidir
			enroques_anteriores[i] = 20;				//derechos_enroques solo llega hasta 0x0f o 15 en decimal
			alpaso_anteriores[i] = 9;					//alpaso solo va de 0 a 8 (8 vale cuando no existe peon al paso) por lo q 9 nunca va a coincidir
		}
		piezas_anteriores[0][0] = peones_b;				//guardo la posicion de las piezas del tablero actual en el primer lugar
		piezas_anteriores[0][1] = caballos_b;
		piezas_anteriores[0][2] = alfiles_b;
		piezas_anteriores[0][3] = torres_b;
		piezas_anteriores[0][4] = damas_b;
		piezas_anteriores[0][5] = rey_b;

		piezas_anteriores[0][6] = peones_n;
		piezas_anteriores[0][7] = caballos_n;
		piezas_anteriores[0][8] = alfiles_n;
		piezas_anteriores[0][9] = torres_n;
		piezas_anteriores[0][10] = damas_n;
		piezas_anteriores[0][11] = rey_n;

		turnos_anteriores[0] = turno_c;					//tambien el turno
		enroques_anteriores[0] = derechos_enroque[0];	//los derechos de enroque
		alpaso_anteriores[0] = alpaso[0];				//y la posibilidad de peon al paso
		return 0;										//salgo xq no pueden haber sido tres repeticiones
	}
	for (i=0;i<MAXPROF;i++)									//ahora simplemente comparo la posicion actual contra las 10 anteriores a ver si hay coincidencias
	{
		if (turno_c == turnos_anteriores[i] && derechos_enroque[0] == enroques_anteriores[i] && alpaso[0] == alpaso_anteriores[i])
		{
		if (piezas_anteriores[i][0] == peones_b && piezas_anteriores[i][1] == caballos_b && piezas_anteriores[i][2] == alfiles_b && piezas_anteriores[i][3] == torres_b)
		{
		if (piezas_anteriores[i][4] == damas_b && piezas_anteriores[i][5] == rey_b && piezas_anteriores[i][6] == peones_n && piezas_anteriores[i][7] == caballos_n)
		{
		if(piezas_anteriores[i][8] == alfiles_n && piezas_anteriores[i][9] == torres_n && piezas_anteriores[i][10] == damas_n && piezas_anteriores[i][11] == rey_n)
		{
			coincide += 1;								//solo si todo esta igual es una repeticion y la cuento
		}
		}
		}
		}
	}
	//por ultimo antes de salir corro toda la lista para hacer lugar a la pos actual q se inserta primera
	for (i=MAXPROF-2;i>-1;i--)
	{
		piezas_anteriores[i+1][0] = piezas_anteriores[i][0];		//desplazo toda la lista hacia arriba (pierdo la ultima posicion)
		piezas_anteriores[i+1][1] = piezas_anteriores[i][1];
		piezas_anteriores[i+1][2] = piezas_anteriores[i][2];
		piezas_anteriores[i+1][3] = piezas_anteriores[i][3];
		piezas_anteriores[i+1][4] = piezas_anteriores[i][4];
		piezas_anteriores[i+1][5] = piezas_anteriores[i][5];

		piezas_anteriores[i+1][6] = piezas_anteriores[i][6];
		piezas_anteriores[i+1][7] = piezas_anteriores[i][7];
		piezas_anteriores[i+1][8] = piezas_anteriores[i][8];
		piezas_anteriores[i+1][9] = piezas_anteriores[i][9];
		piezas_anteriores[i+1][10] = piezas_anteriores[i][10];
		piezas_anteriores[i+1][11] = piezas_anteriores[i][11];

		turnos_anteriores[i+1] = turnos_anteriores[i];
		enroques_anteriores[i+1] = enroques_anteriores[i];
		alpaso_anteriores[i+1] = alpaso_anteriores[i];
	}
	//ahora ya tengo lugar en los registros para poner la posicion actual
	piezas_anteriores[0][0] = peones_b;					//guardo la posicion de las piezas del tablero actual en el primer lugar
	piezas_anteriores[0][1] = caballos_b;
	piezas_anteriores[0][2] = alfiles_b;
	piezas_anteriores[0][3] = torres_b;
	piezas_anteriores[0][4] = damas_b;
	piezas_anteriores[0][5] = rey_b;

	piezas_anteriores[0][6] = peones_n;
	piezas_anteriores[0][7] = caballos_n;
	piezas_anteriores[0][8] = alfiles_n;
	piezas_anteriores[0][9] = torres_n;
	piezas_anteriores[0][10] = damas_n;
	piezas_anteriores[0][11] = rey_n;

	turnos_anteriores[0] = turno_c;						//tambien el turno
	enroques_anteriores[0] = derechos_enroque[0];		//los derechos de enroque
	alpaso_anteriores[0] = alpaso[0];					//y la posibilidad de peon al paso

	if (coincide == 3)
		return 1;										//hubieron tres repeticiones
	return 0;											//no fueron 3 repeticiones
}

void LeeFEN()			    //pide informacion del usuario acerca de la posicion inicial y deja todo listo para q comience la partida
{
	char m = 0;
	BYTE i,error,columna,espacio,indice;
	int j,k;
	BITBOARD aux;

	peones_b = 0;		    //inicializo a 0 las bitboards q tienen la posicion del tablero
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

	ply_count = 0;          //inicio los registros de la notacion
	for (k=0;k<400;k++)
    {
        for (j=0;j<7;j++)
        {
            planilla[j][k] = 0;
        }
    }
	//ahora comienza lo propio de la FEN
	do
	{
		error = 0;					//empezamos sin errores
		espacio = 0;
		indice = 56;				//la notacion FEN arranca en la casilla a8 y va bajando por filas
		columna = 0;				//arranca en la columna 0
		derechos_enroque[0] = 0;
		for (i=0;i<85;i++)			//limpio primero por las dudas el vector q va a tener los datos de la posicion
		{
			fen[i] = 0;
		}

//        printf("Nueva partida?: y,n");
//        scanf(" %c",&m);
//        if (m == 'y')
//        {
            nueva = 1;              //si se pidio iniciar una nueva partida pongo el codigo FEN correcto de una q recien empieza
            char auxfen[85] = {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
            for (i=0;i<85;i++)		//esto es para no tener una lista tan larga hacia abajo
            {
                fen[i] = auxfen[i];
            }
//        }
/*
        else				    //si se pidio introducir la posicion
        {
            nueva = 0;
            fflush(stdin);
            wcout << "Introducir Posicion en formato FEN: " << endl;
            gets(fen);	        //introduzco por teclado el codigo FEN
        }
*/
		for (i=0;i<85;i++)	//lee el vector q se recibio
		{
            if (error)
                break;
            switch (espacio)
			{
				case 0:		//ubica las piezas en el tablero
				{
					switch (fen[i])
					{
						case '1':		//solo hay q avanzar una casilla
						{
							indice++;
							columna++;
						}break;
						case '2':		//hay q avanzar 2 casillas, etc
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
						case 'p':	//peon negro
						{
							peones_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'n':	//caballo negro
						{
							caballos_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'b':	//alfil negro
						{
							alfiles_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'r':	//torre negra
						{
							torres_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'q':		//dama negra
						{
							damas_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'k':		//rey negro
						{
							rey_n |= SetMask(indice);
							indice++;
							columna++;
						}break;
						case 'P':		//peon blanco
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
							columna = 0;			//reinicio la columna
							indice -= 16;			//baja una fila
						}break;
						case ' ':
						{
							espacio++;			//indica q se pasa al siguiente campo de informacion
						}break;
						default:				//si llega cualquier otra cosa
						{
							error = 1;			//hay error en el string FEN
						}break;
					}//end switch
				}break;//end case del primer campo de info
				case 1:		//si ya se dio un espacio (cambia el significado de los caracteres)
				{
					switch (fen[i])
					{
						case 'w':
						{
							turno_c = blancas;		//empiezan las blancas
						}break;
						case 'b':
						{
							turno_c = negras;		//empiezan las negras
						}break;
						case ' ':
						{
							espacio++;				//indica q se pasa al siguiente campo de informacion
						}break;
						default:				//si llega cualquier otra cosa
						{
							error = 1;			//hay error en el string FEN
						}break;
					}//end switch
				}break;//end case de bando q mueve primero
				case 2:	//enroques posibles
				{
					switch (fen[i])
					{
						case '-':
						{
							derechos_enroque[0] = 0;	//no es posible ningun enroque
						}break;
						case 'K':
						{
							derechos_enroque[0] += 1;	//se permite el enroque corto de las blancas
							if (!(rey_b & SetMask(E1)) || !(torres_b & SetMask(H1)))		//si no esta el rey en E1 o la torre de H1 hay error
								error = 1;
						}break;
						case 'Q':
						{
							derechos_enroque[0] += 2;	//se permite el enroque largo de las blancas
							if (!(rey_b & SetMask(E1)) || !(torres_b & SetMask(A1)))
								error = 1;
						}break;
						case 'k':
						{
							derechos_enroque[0] += 4;	//se permite el enroque corto de las negras
							if (!(rey_n & SetMask(E8)) || !(torres_n & SetMask(H8)))
								error = 1;
						}break;
						case 'q':
						{
							derechos_enroque[0] += 8;	//se permite el enroque largo de las negras
							if (!(rey_n & SetMask(E8)) || !(torres_n & SetMask(A8)))
								error = 1;
						}break;
						case ' ':
						{
							espacio++;					//va al otro campo de info
						}break;
						default:				//si llega cualquier otra cosa
						{
							error = 1;			//hay error en el string FEN
						}break;
					}//end switch
				}break;	//end case de enroques
				case 3:		//peon al paso
				{
					switch (fen[i])
					{
						case '-':
						{
							alpaso[0] = 8;	//no es posible ningun peon al paso (en la jugada anterior ningun peon movio de a 2 casillas)
						}break;
						case 'a':	//el peon de "a" (no importa si blanco o negro) movio de a dos casillas en la jugada anterior (podria comerse al paso si se diera el caso)
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
					}//end switch
				}break;//end case 3 y ultimo (el del peon al paso)
				case 4:		//no hago nada xq termino la lectura (espero a q termine el for)
				{

				}break;
			}//end switch de los campos a llenar
		}//end for
		comio[0] = 0;		//antes de la posicion inicial no pudo haber una captura (no es muy necesario esto de todas formas)
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
