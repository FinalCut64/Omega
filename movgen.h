#ifndef MOVGEN_H_INCLUDED
#define MOVGEN_H_INCLUDED

#include "definiciones.h"

void GenerarTodas(BYTE);					//genera todas las jugadas semilegales en la posicion y profundidad actual
BITBOARD GeneraAlfil(BYTE);					//usada por GenerarTodas para los movimientos diagonales
BITBOARD GeneraTorre(BYTE);					//usada por GenerarTodas para los movimientos ortogonales
void ListaJugadas(BYTE,BYTE,BYTE,BYTE);		//llena la lista de jugadas semilegales en base a lo que le "manda" GenerarTodas()

int  CapturasBlancas();
int  CapturasNegras();

BYTE Legal();								//se fija si la jugada es totalmente correcta en terminos legales (FALSE si es ilegal y TRUE si es legal)
BYTE SemiLegal(BYTE);						//con esta funcion sola el programa puede jugar come todo jeje
BYTE CasillaAtacada(BYTE,BYTE);
BYTE ChequeaDiagonal();						//devuelve TRUE si la jugada puede realizarse por la diogonal y FLASE sino
BYTE ChequeaOrtogonal();					//lo mismo q ChequeaDiagonal pero con los movimientos ortogonales

BYTE EnJaque(BYTE);							//dice si el rey del bando q mueve queda en jaque o no

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

#endif // MOVGEN_H_INCLUDED
