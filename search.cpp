#include <iostream>
#include <string>
#include <windows.h>
#include <setjmp.h>
#include <math.h>

#include "search.h"
#include "global.h"
#include "definiciones.h"
#include "tt.h"
#include "movgen.h"
#include "eval.h"
#include "quies.h"

#define USAR_RELOJ
#define USAR_NULO

jmp_buf env;							//esta es para la funcion longjump q permite salir de negamax rapido

void Analiza()							//busca la "mejor" jugada q se pueda (aca esta toda la magia y los errores jeje)
{
    int i;
/*										//perft para probar el generador de movimientos cuando cambio algo
	for (i=0; i<9;i++)
	{
		nodos = 0;
		Perft(0, i);
	}
*/
    QueryPerformanceCounter((LARGE_INTEGER *)&t_inicio);    //inicio el contador de tiempo
	//inicializo todos los registros necesarios para hacer una nueva busqueda
	BorrarVP();
	eficiencia_1 = 0;
	eficiencia_2 = 0;
	tt_hits = 0;                        //cantidad de coincidencias de la tt durante la busqueda
	prof_max = 0;						//comienzan las iteraciones desde profundidad de hojas 1 (profundidad se incrementa antes de llamar a MinMax)
	prof_max2 = 0;
	nodos = 0;
	Qnodos = 0;
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
		QueryPerformanceCounter((LARGE_INTEGER *)&t_fin);
		t_transcurrido = ((t_fin - t_inicio) * timerFrequency * 1000);				//lo pongo en ms
		std::cout << "info depth " << prof_max2 << " score cp " << valoracion << " nodes " << nodos << " nps " << floor(1000 * nodos / t_transcurrido) << " time " << t_transcurrido << std::endl;
		if (limite == DEPTH)														//si la condicion de analisis es por profundidad fija
		{
			if (prof_max >= depth_jugada)											//si se alcanzó la profundidad fija por jugada salimos del analisis
			{
				QueryPerformanceCounter((LARGE_INTEGER *)&t_fin);
				t_transcurrido = ((t_fin - t_inicio) * timerFrequency * 1000);		//lo pongo en ms
				salir = 1;
				longjmp(env, 0);													//con la funcion longjmp
			}
		}
		prof_max++;
		prof_max2 = prof_max;           //la guardo xq prof_max puede variar dentro de Negamax debido a LMR y poda de mov nulo
		hojastt_hits = 0;
		evaltt_hits = 0;
		llamadasevaluar = 0;
		nodosnulos = 0;
		cortesnulo = 0;
		cortes_inut_inversa = 0;
		hash_pos2 = hash_pos;
//		valoracion = Raiz(-30000,30000,0);

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
	}while (1);
//	}while(abs(valoracion) < 29000 && !unica && prof_max < 20);
    //sale si es unica o si vio mate o si alcanza profundidad fija
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
	if (draft == 0)// + ext)           					//no tiene sentido reducir y extender una jugada asi q si ext == 1 entonces redu == 0 y viceversa
	{
		nodos++;
#ifdef USAR_RELOJ
		if ((nodos & 1023) == 0)
		{
			switch (limite)
			{
				case NORMAL:				//si es partida normal que mire el manejo de tiempo
				{
					ManejarReloj();
				}break;
				case INFINITO:				//si es analisis infinito no hay q hacer nada (seguimos hasta q nos paren lo cual de momento es nunca xq no puedo recibir input de la GUI)
				{

				}break;
				case NODES:
				{
					if (nodos >= nodos_jugada)
					{
						QueryPerformanceCounter((LARGE_INTEGER *)&t_fin);
						t_transcurrido = ((t_fin - t_inicio) * timerFrequency * 1000);	//lo pongo en ms
						salir = 1;														//entonces salgo del analisis
						std::cout << "info depth " << prof_max2 << " score cp " << valoracion << " nodes " << nodos << " nps " << floor(1000 * nodos / t_transcurrido) << " time " << t_transcurrido << std::endl;
						longjmp(env, 0);												//con la funcion longjmp
					}
				}break;
				case MOVETIME:
				{
					QueryPerformanceCounter((LARGE_INTEGER *)&t_fin);
					t_transcurrido = ((t_fin - t_inicio) * timerFrequency * 1000);		//lo pongo en ms
					if (t_transcurrido >= tiempo_jugada)						     	//si se supero el tiempo maximo por jugada
					{
						salir = 1;														//entonces salgo del analisis
						std::cout << "info depth " << prof_max2 << " score cp " << valoracion << " nodes " << nodos << " nps " << floor(1000 * nodos / t_transcurrido) << " time " << t_transcurrido << std::endl;
						longjmp(env, 0);												//con la funcion longjmp
					}
				}break;
				case DEPTH:				//si es por profundidad no hay q hacer nada (ya se chequea la condicion de salida en analiza())
				{

				}break;
			}
		}
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
#ifdef PODAR_INUTILIDAD_INVERSA
	if (draft < 4 && EvaluarRapido()-fmargen[draft] >= beta && (!EnJaque(turno_c^1)) && abs(alfa) < 29000)
    {
        cortes_inut_inversa++;
        return beta;
    }
#endif // PODAR_INUTILIDAD_INVERSA
#ifdef USAR_NULO
	if (Nulo_ok(prof) && !EnJaque(turno_c^1))							//miro si es legal y deseado hacer un movimiento nulo
	{
		nulo = 1;														//advierto q ya hice un mov nulo en esta rama asi q no hay que hacer otro
		nodosnulos++;
		turno_c ^= 1;													//hago el movimiento nulo (cambiar el turno del q juega)
        alpaso[prof+1] = 8;
        hash_pos ^= Zobrist.bando;
        hash_pos ^= Zobrist.alpaso[alpaso[prof]];
        hash_pos ^= Zobrist.alpaso[alpaso[prof+1]];
        prof_max -= R;
		valoracion = -NegaMax(-beta,-beta+1,prof+1,draft-1-R);			//movimiento nulo con ventana m�nima alrededor de beta y reduccion R
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

void Ordenar(BYTE prof)						//ordena las jugadas para hacer mas eficiente alfa-beta
{
	unsigned int i;

	if (prof_max != 1)																			//entra si ya se ha establecido la VP (aunque sea una iteracion previa)
	{
	    if (tt_hit)     																		//entra si hay disponible una jugada de la tt.
        {
            for (i = 0 ; i < ultimajugada[prof] ; i=i+3)										//se busca la pv en el listado actual (tiene q estar xq es la misma posicion raiz)
            {
                if ((vp[prof][0] == jugadas[prof][i]) && (vp[prof][1] == jugadas[prof][i+1])) 	//si la jugada forma parte de la VP de la anterior iteracion
                {
                    ponderacion[i/3] += 10000;													//si o si se la situa primera de la lista en este nuevo analisis
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
            for (i = 0 ; i < ultimajugada[prof] ; i=i+3)										//se busca la pv en el listado actual (tiene q estar xq es la misma posicion raiz)
            {
                if ((vp[prof][0] == jugadas[prof][i]) && (vp[prof][1] == jugadas[prof][i+1])) 	//si la jugada forma parte de la VP de la anterior iteracion
                {
                    ponderacion[i/3] += 10000;													//si o si se la situa primera de la lista en este nuevo analisis
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

void QuickSort(int a[], int primero, int ultimo, BYTE prof)			//ordena las jugadas de cada profundidad en base al vector ponderacion
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
			a[j] = tmp; 					//intercambia a[i] con a[j]

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
		QuickSort(a, primero, j, prof);		//mismo proceso con sublista izqda
	if (i < ultimo)
		QuickSort(a, i, ultimo, prof);		//mismo proceso con sublista drcha
}

BYTE Repite()									//si la posicion actual se ha producido antes (se encuentra dentro de la lista de 10 ultimas posiciones) retorna 1 y sino retorna 0
{												//sirve en NegaMax() para q el programa sepa q las repeticiones son tablas (y q las evalue como tal) y aviso a la primer repeticion por las dudas
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
			return 1;							//solo si todo esta igual es una repeticion y aviso
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
	for (i=0;i<MAXPROF;i++)								//ahora simplemente comparo la posicion actual contra las 10 anteriores a ver si hay coincidencias
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

void Guardar()											//esto sirve para guardar el estado actual de la partida por si se tiene q salir con longjmp de NegaMax
{														//el resto de los registros importantes ya estan guardados en la funcion de 3 repet
	turno1 = turno;										//estos tambien los guardo pero no puedo usar registros de las 3 repet
	comio1 = comio[0];									//xq estos no influyen en esa funcion para nada
	jugadas_reversibles1 = jugadas_reversibles;
	tab90 = tablero90;
	tabA1 = tableroA1;
	tabA8 = tableroA8;
	hash_pos2 = hash_pos;                               //tambien guardo la clave hash

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

void ManejarReloj()
{
    QueryPerformanceCounter((LARGE_INTEGER *)&t_fin);
    t_transcurrido = ((t_fin - t_inicio) * timerFrequency * 1000);			//lo pongo en ms
    if (reloj == blancas)
	{
		if (winc)		//si hay incremento
		{
			if (t_transcurrido > (wtime / 40) + (winc * 0.5))      			//si se supero el tiempo maximo por jugada
			{
				salir = 1;													//entonces salgo del analisis
				std::cout << "info depth " << prof_max2 << " score cp " << valoracion << " nodes " << nodos << " nps " << floor(1000 * nodos / t_transcurrido) << " time " << t_transcurrido << std::endl;
				longjmp(env, 0);											//con la funcion longjmp
			}
		}
		else			//sin incremento
		{
			if (t_transcurrido > (wtime / 40))           					//si se supero el tiempo maximo por jugada
			{
				salir = 1;													//entonces salgo del analisis
				std::cout << "info depth " << prof_max2 << " score cp " << valoracion << " nodes " << nodos << " nps " << floor(1000 * nodos / t_transcurrido) << " time " << t_transcurrido << std::endl;
				longjmp(env, 0);											//con la funcion longjmp
			}
		}
	}
	else
	{
		if (binc)		//si hay incremento
		{
			if (t_transcurrido > (btime / 40) + (binc * 0.5))      			//si se supero el tiempo maximo por jugada
			{
				salir = 1;													//entonces salgo del analisis
				std::cout << "info depth " << prof_max2 << " score cp " << valoracion << " nodes " << nodos << " nps " << floor(1000 * nodos / t_transcurrido) << " time " << t_transcurrido << std::endl;
				longjmp(env, 0);											//con la funcion longjmp
			}
		}
		else			//sin incremento
		{
			if (t_transcurrido > (btime / 40))           					//si se supero el tiempo maximo por jugada
			{
				salir = 1;													//entonces salgo del analisis
				std::cout << "info depth " << prof_max2 << " score cp " << valoracion << " nodes " << nodos << " nps " << floor(1000 * nodos / t_transcurrido) << " time " << t_transcurrido << std::endl;
				longjmp(env, 0);											//con la funcion longjmp
			}
		}
	}
}
