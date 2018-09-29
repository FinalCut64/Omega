#include "movgen.h"
#include "global.h"
#include "bitboard.h"
#include "tt.h"

void GenerarTodas(BYTE prof)								//en base a la posicion actual del tablero lista todas las posibles jugadas semilegales
{
	BITBOARD auxiliar,auxiliar1;

	puntero = 0;											//inicializo el puntero que despues utiliza ListaJugadas() para acomodarlas en su lugar
	if (turno_c == blancas)
	{
		auxiliar = peones_b;								//auxiliar sirve para no modificar las bitboard q tiene las piezas del tablero actual
		pieza = PEON;										//las piezas q necesitan llamar a SemiLegal() tienen q indicar de cual se trata (para torre, alfil y dama no es necesario)
		while (auxiliar)                                	//si no quedan peones salgo
		{
			inicio = MSB(auxiliar);							//busco los peones en el tablero (uso MSB porque es mas rapida q LSB y xq encuentra los mas avanzados primero)
			auxiliar ^= SetMask(inicio);					//borro el primer "1" encontrado para q al pasar otra vez MSB encuentre el siguiente peon
			auxiliar1 = mov_peones[blancas][inicio];		//auxiliar1 tiene ahora "1�s" en todas las casillas a las q un peon blanco puede mover desde inicio
			while (auxiliar1)                           	//si no hay mas salgo y sigo con otro peon xq este esta agotado
			{
				fin = MSB(auxiliar1);						//busco las casillas de destino (fin) y ademas busca las mas agresivas primero (las q avanzan)
				auxiliar1 ^= SetMask(fin);					//borro el primer "1" encontrado para q al pasar otra vez MSB encuentre la siguiente casilla de fin (y no la misma siempre)
				semibien = SemiLegal(prof);
                if (semibien)
				{
					if (tablero & SetMask(fin))				//si fue una captura
						ponderacion[puntero/3] = CapturasBlancas() - 100;
					else									//si no fue una captura la ponderacion es -1
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
			atack_alfil = GeneraAlfil(inicio);					//pone en atack_alfil todas las casillas q ataca en alfil desde inicio
			atack_alfil &= ~piezas_b;							//atak_alfil ahora tiene todas las casillas semilegales a las q puede ir el alfil desde inicio
			while (atack_alfil)
			{
				fin = MSB(atack_alfil);
				atack_alfil ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasBlancas() - 310;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += alfil_pos[blancas][fin] - alfil_pos[blancas][inicio];
				ListaJugadas(inicio,fin,normal,prof);			//movimientos de alfiles y damas son siempre tipo 1 (normales)
			};
		};
		auxiliar = torres_b;
		while (auxiliar)
		{
			inicio = MSB(auxiliar);
			auxiliar ^= SetMask(inicio);
			atack_torre = GeneraTorre(inicio);					//pone en atack_torre todas las casillas q ataca la torre desde inicio
			atack_torre &= ~piezas_b;							//atack_torre ahora tiene todas las casillas semilegales a las q puede ir la torre desde inicio
			while (atack_torre)
			{
				fin = MSB(atack_torre);
				atack_torre ^= SetMask(fin);
				if (tablero & SetMask(fin))
					ponderacion[puntero/3] = CapturasBlancas() - 500;
				else
					ponderacion[puntero/3] = -100;
				ponderacion[puntero/3] += torre_pos[blancas][fin] - torre_pos[blancas][inicio];
				ListaJugadas(inicio,fin,movtorre,prof);			//como no paso por semilegal() le aclaro yo q es un movimiento de torre
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
		if (derechos_enroque[prof] & 0x01)				//genero los enroques blancos (si son posibles)
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

BYTE CasillaAtacada(BYTE sq,BYTE bando)											//pide como entrada la casilla a considerar, y el color del bando q se quiere ver si esta atacando sq o no
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

void HacerJugada(BYTE inicio,BYTE fin,BYTE semibien,BYTE prof)	//actualiza los reg necesarios para q la jugada se realice en la forma de representacion del programa
{
	alpaso[prof+1] = 8;											//limpio el registro (solo se setea si avanza un peon de a 2) ya q columnas validas van de 0 a 7
	derechos_enroque[prof+1] = derechos_enroque[prof];			//en principio supongo q no se modifican los enroques (luego en los case q corresponda se modifican)
	tablero &= ClearMask(inicio);								//borro la pieza de la casilla de salida del tablero
	tablero |= SetMask(fin);									//y la pongo en la de destino
	tablero90 &= ClearMask90(inicio);
	tablero90 |= SetMask90(fin);
	tableroA1 &= ClearMaskA1(inicio);
	tableroA1 |= SetMaskA1(fin);
	tableroA8 &= ClearMaskA8(inicio);
	tableroA8 |= SetMaskA8(fin);
	if (turno_c == blancas)
	{
		piezas_b &= ClearMask(inicio);							//borro la pieza de la casilla de inicio
		piezas_b |= SetMask(fin);								//y la pongo en la de destino (comun a todo tipo de movimiento blanco)
		switch (semibien)
		{
			case normal:
			{
				ActualizaBlancas(inicio,fin);					//realizan las acciones normales de un movimiento
				CapturaBlancas(fin,prof);
			}break;
			case corona:
			{
				peones_b &= ClearMask(inicio);					//borro el peon de la casilla de inicio
				damas_b |= SetMask(fin);						//y pongo una DAMA en la casilla de fin (solo considera esto en principio)
				hash_pos ^= Zobrist.escaques[Peon_b][inicio];	//lo mismo con la llave zobrist
				hash_pos ^= Zobrist.escaques[Dama_b][fin];
				CapturaBlancas(fin,prof);
			}break;
			case enroque:
			{
				derechos_enroque[prof+1] &= 0xFC;				//borro bit 1 y 2 impidiendo asi cualquier otro enroque blanco en el futuro
				rey_b &= ClearMask(inicio);						//lo mismo con el rey
				rey_b |= SetMask(fin);
				hash_pos ^= Zobrist.escaques[Rey_b][inicio];
				hash_pos ^= Zobrist.escaques[Rey_b][fin];
				if (fin == G1)									//si fue 0-0
				{
					piezas_b &= ClearMask(H1);					//saco la pieza de H1
					piezas_b |= SetMask(F1);					//y la pongo en F1
					torres_b &= ClearMask(H1);					//lo mismo con la bitboard de las torres
					torres_b |= SetMask(F1);
					tablero &= ClearMask(H1);					//y de igual forma con el tablero
					tablero |= SetMask(F1);
					tablero90 &= ClearMask90(H1);				//y los rotados tambien
					tablero90 |= SetMask90(F1);
					tableroA1 &= ClearMaskA1(H1);
					tableroA1 |= SetMaskA1(F1);
					tableroA8 &= ClearMaskA8(H1);
					tableroA8 |= SetMaskA8(F1);
					hash_pos ^= Zobrist.escaques[Torre_b][H1];
					hash_pos ^= Zobrist.escaques[Torre_b][F1];
				}
				else											//0-0-0
				{
					piezas_b &= ClearMask(A1);					//saco la pieza de A1
					piezas_b |= SetMask(D1);					//y la pongo en D1
					torres_b &= ClearMask(A1);					//lo mismo con la bitboard de las torres
					torres_b |= SetMask(D1);
					tablero &= ClearMask(A1);					//y de igual forma con el tablero
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
				comio[prof] = 0;								//no puede haber sido captura si es un enroque
			}break;
			case ap:
			{
				peones_b &= ClearMask(inicio);					//borro el peon de la casilla de inicio
				peones_b |= SetMask(fin);						//y lo pongo en la de fin
				piezas_n &= ClearMask(fin-8);					//borro la pieza y
				peones_n &= ClearMask(fin-8);					//el peon negro comido al paso
				tablero &= ClearMask(fin-8);					//tambien lo saco del tablero
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
				peones_b &= ClearMask(inicio);					//borro el peon de la casilla de inicio
				peones_b |= SetMask(fin);						//y lo pongo en la de fin
				comio[prof] = 0;								//no puede haber sido captura si es un doble mov de peon
				alpaso[prof+1] = Columna(inicio);				//aviso q es posible capturar al paso en esa columna (en el ply siguiente)
                hash_pos ^= Zobrist.escaques[Peon_b][inicio];
                hash_pos ^= Zobrist.escaques[Peon_b][fin];
			}break;
			case movrey:
			{
				ActualizaBlancas(inicio,fin);					//todo normal
				CapturaBlancas(fin,prof);
				derechos_enroque[prof+1] &= 0xFC;				//excepto q le quito los derechos de enroque a las blancas de aqui en adelante
			}break;
			case movtorre:
			{
				ActualizaBlancas(inicio,fin);					//todo normal
				CapturaBlancas(fin,prof);
				if (inicio == H1)								//si se movio la torre de H1
					derechos_enroque[prof+1] &= 0xFE;			//le quito los derechos de enroque corto a las blancas de aqui en adelante
				if (inicio == A1)								//si se movio la torre de H1
					derechos_enroque[prof+1] &= 0xFD;			//le quito los derechos de enroque largo a las blancas de aqui en adelante
			}break;												//en cualquier otro caso no hay q modificar nada de los enroques
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
				hash_pos ^= Zobrist.escaques[Peon_n][inicio];	//lo mismo con la llave zobrist
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
				ActualizaNegras(inicio,fin);					//todo normal
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
    hash_pos ^= Zobrist.enroques[derechos_enroque[prof]];    	//saco los anteriores derechos de enroque
    hash_pos ^= Zobrist.enroques[derechos_enroque[prof+1]];  	//y pongo los nuevos en la llave (pueden ser los mismos)

    hash_pos ^= Zobrist.alpaso[alpaso[prof]];                	//saco el viejo valor
    hash_pos ^= Zobrist.alpaso[alpaso[prof+1]];              	//pongo el nuevo valor

	hash_pos ^= Zobrist.bando;                          		//el bando siempre se togglea
	turno_c ^= 1;												//y por ultimo cambio el turno de a quien le toca jugar
}

void ActualizaBlancas(BYTE inicio, BYTE fin)
{
	if (peones_b & SetMask(inicio))								//si fue un peon el q movio
	{
		peones_b &= ClearMask(inicio);							//borro el peon de la casilla de inicio
		peones_b |= SetMask(fin);								//y lo pongo en la de fin
		hash_pos ^= Zobrist.escaques[Peon_b][inicio];   		//actualizo de paso la llave Zobrist con lo que corresponde
		hash_pos ^= Zobrist.escaques[Peon_b][fin];
	}
	else														//sino sigo preguntando hasta ver que fue
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
					else										//no queda otra q el rey asi q no pregunto mas
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

void CapturaBlancas(BYTE fin, BYTE prof)						//esta es para recuperar despues la posicion (recuerda q fue lo q se comio en Negamax)
{
	if (piezas_n & SetMask(fin))								//si la jugada fue una captura
	{
		piezas_n &= ClearMask(fin);								//borro el bit correspondiente en el bando negro
		if (peones_n & SetMask(fin))							//si fue un peon el comido
		{
			peones_n &= ClearMask(fin);							//borro el bit correspondiente al peon negro
			comio[prof] = PEON;									//indico que se lastraron un peon para despues poder recuperarlo en DeshacerJugada()
            hash_pos ^= Zobrist.escaques[Peon_n][fin];  		//tambien hay q actualizar la llave Zobrist (saco el peon negro)
		}
		else													//sino sigo preguntando hasta ver cual fue
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
					else										//no queda otra q la dama asi q no pregunto mas (el rey no puede ser comido)
					{
						damas_n &= ClearMask(fin);
						comio[prof] = DAMA;
						hash_pos ^= Zobrist.escaques[Dama_n][fin];
					}
				}
			}
		}
	}//end si fue una captura
	else														//si no pongo a 0 indicando q no fue captura
	{
		comio[prof] = 0;
	}
}

void ActualizaNegras(BYTE inicio, BYTE fin)
{
	if (peones_n & SetMask(inicio))								//si fue un peon el q movio
	{
		peones_n &= ClearMask(inicio);							//borro el peon de la casilla de inicio
		peones_n |= SetMask(fin);								//y lo pongo en la de fin
        hash_pos ^= Zobrist.escaques[Peon_n][inicio];   		//actualizo la llave Zobrist sacando el peon negro de inicio
		hash_pos ^= Zobrist.escaques[Peon_n][fin];      		//y poniendolo en fin
	}
	else														//sino sigo preguntando hasta ver que fue
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
					else										//no queda otra q el rey asi q no pregunto mas
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

void CapturaNegras(BYTE fin, BYTE prof)					//esta es para recuperar despues la posicion (recuerda q fue lo q se comio en Negamax)
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
	if (turno_c == negras)											//si se cumple esto hay q deshacer una jugada del blanco. Todo al reves xq ya se paso una vez por HacerJugada()
	{
		switch (semibien)
		{
			case normal:
			{
				DesactualizaBlancas(inicio,fin);					//las restituciones normales
				DescapturaBlancas(fin, prof);						//esto se encarga de reponerle a las negras su material (si la jugada fue una captura)
			}break;
			case corona:
			{
				piezas_b &= ClearMask(fin);							//borro la pieza blanca de la casilla de fin
				piezas_b |= SetMask(inicio);						//y la pongo en la de inicio
				peones_b |= SetMask(inicio);						//pongo el peon q habia coronado
				damas_b &= ClearMask(fin);							//saco la dama q se habia coronado (por ahora solo se puede coronar dama)
				DescapturaBlancas(fin, prof);
                hash_pos ^= Zobrist.escaques[Peon_b][inicio];
                hash_pos ^= Zobrist.escaques[Dama_b][fin];
			}break;
			case enroque:
			{
				piezas_b &= ClearMask(fin);
				piezas_b |= SetMask(inicio);
				rey_b &= ClearMask(fin);							//devuelvo el rey a
				rey_b |= SetMask(inicio);							//su posicion de inicio
                hash_pos ^= Zobrist.escaques[Rey_b][inicio];
                hash_pos ^= Zobrist.escaques[Rey_b][fin];
				tablero &= ClearMask(fin);							//borro la casilla de fin q siempre va a quedar vacia al deshacer un enroque (no es captura)
				tablero90 &= ClearMask90(fin);
				tableroA1 &= ClearMaskA1(fin);
				tableroA8 &= ClearMaskA8(fin);
				if (fin == G1)										//0-0
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
				else												//0-0-0
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
				piezas_b &= ClearMask(fin);							//borro la pieza blanca de la casilla de fin
				piezas_b |= SetMask(inicio);						//y la pongo en la de inicio
				peones_b &= ClearMask(fin);							//borro el peon de la casilla de fin
				peones_b |= SetMask(inicio);						//y lo pongo en la de inicio
				piezas_n |= SetMask(fin-8);							//les devuelvo a las negras el peon capturado al paso
				peones_n |= SetMask(fin-8);
				tablero |= SetMask(fin-8);							//y tambien recompongo el tablero
				tablero &= ClearMask(fin);							//borro del tablero el peoncito
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
				piezas_b &= ClearMask(fin);							//borro la pieza blanca de la casilla de fin
				piezas_b |= SetMask(inicio);						//y la pongo en la de inicio
				peones_b &= ClearMask(fin);							//borro el peon de la casilla de fin
				peones_b |= SetMask(inicio);						//y lo pongo en la de inicio
				tablero &= ClearMask(fin);							//vacio la casilla de fin en el tablero
                tablero90 &= ClearMask90(fin);
                tableroA1 &= ClearMaskA1(fin);
                tableroA8 &= ClearMaskA8(fin);
                hash_pos ^= Zobrist.escaques[Peon_b][inicio];
                hash_pos ^= Zobrist.escaques[Peon_b][fin];
			}break;
			case movrey:
			{
				DesactualizaBlancas(inicio,fin);					//las restituciones normales (no se necesita nada mas)
				DescapturaBlancas(fin, prof);
			}break;
			case movtorre:
			{
				DesactualizaBlancas(inicio,fin);					//lo mismo para el movimiento de una torre
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
				DesactualizaNegras(inicio,fin);						//las restituciones normales
				DescapturaNegras(fin, prof);
			}break;
			case corona:
			{
				piezas_n &= ClearMask(fin);
				piezas_n |= SetMask(inicio);
				peones_n |= SetMask(inicio);
				damas_n &= ClearMask(fin);							//saco la dama q se habia coronado (por ahora solo se puede coronar dama)
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
				if (fin == G8)										//0-0
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
				else												//0-0-0
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
	tablero |= SetMask(inicio);									//esto se da siempre y es comun a todos los casos (indicar q hay una pieza en la casilla de inicio)
	tablero90 |= SetMask90(inicio);
	tableroA1 |= SetMaskA1(inicio);
	tableroA8 |= SetMaskA8(inicio);
	//ahora actualizo la llave hash de la nueva posicion
    hash_pos ^= Zobrist.enroques[derechos_enroque[prof]];    	//pongo los anteriores derechos de enroque
    hash_pos ^= Zobrist.enroques[derechos_enroque[prof+1]];  	//y saco los nuevos en la llave (pueden ser los mismos)

    hash_pos ^= Zobrist.alpaso[alpaso[prof]];                	//pongo el valor viejo (anterior a la jugada)
    hash_pos ^= Zobrist.alpaso[alpaso[prof+1]];              	//y saco el nuevo

	hash_pos ^= Zobrist.bando;                              	//el bando siempre se togglea
	turno_c ^= 1;												//cambio el turno de a quien le toca jugar
}

void DesactualizaBlancas(BYTE inicio, BYTE fin)
{
	piezas_b &= ClearMask(fin);									//borro la pieza blanca de la casilla de fin
	piezas_b |= SetMask(inicio);								//y la pongo en la de inicio
	if (peones_b & SetMask(fin))								//si fue un peon el q movio
	{
		peones_b &= ClearMask(fin);								//borro el peon de la casilla de fin
		peones_b |= SetMask(inicio);							//y lo pongo en la de inicio
        hash_pos ^= Zobrist.escaques[Peon_b][fin];      		//lo mismo hago con la llave zobrist
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
		case 0:												//no fue captura
		{
			tablero &= ClearMask(fin);						//no habia nada en la casilla de fin entonces la borro (en cualquier otro caso no xq es captura osea q estaba ocupada antes)
            tablero90 &= ClearMask90(fin);
            tableroA1 &= ClearMaskA1(fin);
            tableroA8 &= ClearMaskA8(fin);
		}break;
		case PEON:											//fue captura de un peon
		{
			peones_n |= SetMask(fin);						//entonces les devuelvo el peon "descomido" a las negras
			piezas_n |= SetMask(fin);
			hash_pos ^= Zobrist.escaques[Peon_n][fin];  	//tambien cambio la llave hash con lo q corresponde (agrego el peon n)
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
	piezas_n &= ClearMask(fin);								//borro la pieza negra de la casilla de fin
	piezas_n |= SetMask(inicio);							//y la pongo en la de inicio
	if (peones_n & SetMask(fin))							//si fue un peon el q movio
	{
		peones_n &= ClearMask(fin);							//borro el peon de la casilla de fin
		peones_n |= SetMask(inicio);						//y lo pongo en la de inicio
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
		case 0:											//no fue captura
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
