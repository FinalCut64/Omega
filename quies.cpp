#include "quies.h"
#include "global.h"
#include "tt.h"
#include "eval.h"
#include "movgen.h"
#include "bitboard.h"




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
