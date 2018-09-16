#include <iostream>
#include <string>
#include <math.h>
#include <thread>

#include "global.h"
#include "tt.h"

Limites limite;
unsigned long long wtime, btime, winc, binc;
unsigned long long nodos_jugada;
unsigned int tiempo_jugada, depth_jugada;
bool reloj;
std::string recibido;

void Uci()
{
    std::string nombre = "Omega 1.0";
    std::string autor = "Demián Mavrich";

    char alg[5];

	while(1)
    {
        std::getline (std::cin,recibido);               //espero hasta que la GUI invie un comando

        if(recibido.compare("uci") == 0)                //si recibido es igual a uci la funcion devuelve 0
        {
            std::cout << "id name " << nombre << std::endl;
            std::cout << "id author " << autor << std::endl;
            std::cout << "uciok" << std::endl;
        }
        if(recibido.compare("isready") == 0)
        {
            std::cout << "readyok" << std::endl;
        }
        if(recibido.compare("ucinewgame") == 0)
        {
            Init();
        }
        if(recibido.find("position") == 0)
        {
        	recibido.erase(0,9);						//elimino del string recibido la palabra position y el espacio entre palabras (empiezo en el 0 y borro 9 caracteres)
            if(recibido.find("startpos") == 0)			//ahora el string empieza o por startpos o por fen
            {
				recibido.erase(0,9);					//elimino la palabra startpos
				LeeFEN(startpos);						//inicio registros en la posicion inicial
            }
            else if (recibido.find("fen") == 0)			//si se quiere analizar a partir de una posición fen dada por la GUI
			{
				recibido.erase(0,4);					//elimino del string recibido la palabra fen y el espacio entre palabras (empiezo en el 0 y borro 4 caracteres)
				LeeFEN(recibido);						//inicio registros en la posicion fen almacenada en recibido
			}
			InicializaHash();		       				//una vez configurada la posición se establece la clave hash de la misma
			ReiniciaRegistros();						//al comenzar una nueva partida hay q reiniciar varios registros para q todo funcione bien (esto inicia la partida)
			if(recibido.find("moves") == 0)				//me fijo si hay jugadas que realizar luego de startpos o de la posicion fen introducida
			{
				recibido.erase(0,6);					//elimino la palabra moves
				while(recibido.size())					//realizo todas las jugadas indicadas por el GUI a travez de recibido hasta que no quede ninguna
				{
					int encontrado = recibido.find_first_of(" ");       //busco los espacios que delimitan el final de cada jugada dentro del string recibido
					if (encontrado == -1)								//compruebo que existan todavia espacios dentro de recibido
					{
						recibido.copy(alg,recibido.size(),0);			//almaceno en alg la jugada (desde el caracter 0 de recibido hasta una longitud igual a encontrado)
						recibido.clear();								//si no se encuentra ninguno más borro el string recibido
					}
					else												//entra aca si encuentra un nuevo espacio lo que significa que no hemos terminado de hacer las jugadas
					{
						recibido.copy(alg,encontrado,0);				//almaceno en alg la jugada (desde el caracter 0 de recibido hasta una longitud igual a encontrado)
						recibido.erase(0,encontrado + 1);				//elimino de recibido la jugada actual asi en la proxima pasada considero la siguiente
					}
					EsperaJugada(alg);									//llamo a la funcion pasandole alg que está en formato e2e4
					bien = Legal();										//verifico legalidad
					if(bien == 0)
					{
						std::cout << "Error ilegal" << std::endl;
					}
					Jugar(inicio,fin,bien);				//realizo la jugada
				};
			}
        }
        if(recibido.find("go") == 0)
        {
            recibido.erase(0,3);								//elimino del string recibido la palabra go y el espacio entre palabras (empiezo en el 0 y borro 3 caracteres)
			if(recibido.find("wtime") == 0)						//si entra significa q se está jugando una partida entonces hay que conocer el estado de los relojes
            {
            	char w_time[10], b_time[10], w_inc[10], b_inc[10];

				limite = NORMAL;
                recibido.erase(0,6);							//elmino del string la palabra wtime y el espacio
            	int encontrado = recibido.find_first_of(" ");	//busco la cant de caracteres que tiene wtime en milisegundos
                recibido.copy(w_time,encontrado,0);				//almaceno en wtime el tiempo en miliseg que le queda a las blancas
                recibido.erase(0,encontrado + 1);				//elmino del string la cant de milisegundos que wtime tiene

   				recibido.erase(0,6);							//elmino del string la palabra btime y el espacio
				encontrado = recibido.find_first_of(" ");		//busco la cant de caracteres que tiene btime en milisegundos
                recibido.copy(b_time,encontrado,0);				//almaceno en btime el tiempo en miliseg que le queda a las negras
                recibido.erase(0,encontrado + 1);				//elmino del string la cant de milisegundos que btime tiene

   				recibido.erase(0,5);							//elmino del string la palabra winc y el espacio
				encontrado = recibido.find_first_of(" ");		//busco la cant de caracteres que tiene winc en milisegundos
                recibido.copy(w_inc,encontrado,0);				//almaceno en winc el tiempo en miliseg que hay de incremento por jugada para las blancas
                recibido.erase(0,encontrado + 1);				//elmino del string la cant de caracteres q componen a winc

   				recibido.erase(0,5);							//elmino del string la palabra binc y el espacio
				encontrado = recibido.find_first_of(" ");		//busco la cant de caracteres que tiene binc en milisegundos
                recibido.copy(b_inc,encontrado,0);				//almaceno en binc el tiempo en miliseg que hay de incremento por jugada para las negras
                recibido.erase(0,encontrado + 1);				//elmino del string la cant de caracteres q componen a binc

				std::string aux = w_time;
				wtime = std::stoi(aux);							//convierte el string y lo guarda en wtime
				aux = b_time;
				btime = std::stoi(aux);
				aux = w_inc;
				winc = std::stoi(aux);
				aux = b_inc;
				binc = std::stoi(aux);
				if(turno_c == blancas)							//significa que se pide jugar con blancas
				{
					reloj = blancas;							//bandera para q la rutina de manejo de tiempo sepa q el tiempo q importa es el de las blancas
				}
				else
				{
					reloj = negras;
				}
            }
            else if(recibido.find("infinite") == 0)				//implementar analisis infinito
			{
				recibido.erase(0,9);							//eliminar palabra infinite y espacio
				limite = INFINITO;
			}
			else if (recibido.find("movetime") == 0)
			{
				char t_jug[10];
				limite = MOVETIME;
				recibido.erase(0,9);							//elimino del string recibido la palabra movetime y el espacio entre palabras (empiezo en el 0 y borro 9 caracteres)

				int encontrado = recibido.find_first_of(" ");	//busco la cant de caracteres que tiene
                recibido.copy(t_jug,encontrado,0);				//almaceno en t_jug (char array) el numero que indica la cant de mseg para la jugada
                recibido.erase(0,encontrado + 1);				//elmino del string encontrado la cant de caracteres
				std::string aux = t_jug;						//paso de char a string
				tiempo_jugada = std::stoi(aux);					//convierte el string aux y lo guarda en depth_jugada
			}
			else if (recibido.find("depth") == 0)
			{
				char depth[3];									//la maxima profundidad posible en este caso sería 999

				limite = DEPTH;									//indico que el limite establecido es por profundidad en plies para cada jugada
				recibido.erase(0,6);							//elimino del string recibido la palabra depth y el espacio entre palabras
				int encontrado = recibido.find_first_of(" ");	//busco la cant de caracteres que tiene depth
                recibido.copy(depth,encontrado,0);				//almaceno en depth (char array) el numero que indica la profundidad de la jugada
                recibido.erase(0,encontrado + 1);				//elmino del string encontrado la cant de caracteres q componen a la depth
				std::string aux = depth;						//paso de char a string
				depth_jugada = std::stoi(aux);					//convierte el string aux y lo guarda en depth_jugada
			}
			else if (recibido.find("nodes") == 0)
			{
				char nod[20];

				limite = NODES;
				recibido.erase(0,6);							//elimino del string recibido la palabra nodes y el espacio entre palabras
				int encontrado = recibido.find_first_of(" ");	//busco la cant de caracteres que tiene nodes
                recibido.copy(nod,encontrado,0);				//almaceno en nod (char array) el numero que indica la cant de nodos x jugada
                recibido.erase(0,encontrado + 1);				//elmino del string encontrado esa cant de caracteres
				std::string aux = nod;							//paso de char a string
				nodos_jugada = std::stoi(aux);					//convierte el string aux y lo guarda en depth_jugada
			}
			Analiza();
//			std::cout << "info depth " << prof_max << " score cp " << valoracion << " nodes " << nodos << " nps " << floor(1000 * nodos / t_transcurrido) << " time " << t_transcurrido << std::endl;
			std::cout << "bestmove " << a << b << c << d << std::endl;
        }
        if(recibido.find("stop") == 0)
		{
//			std::cout << "info depth " << prof_max << " score cp " << valoracion << " nodes " << nodos << " nps " << floor(1000 * nodos / t_transcurrido) << " time " << t_transcurrido << std::endl;
			std::cout << "bestmove " << a << b << c << d << std::endl;
			salir = 1;
		}
        if(recibido.find("quit") == 0)							//si se pide terminar el proceso
		{
			std::cout << "bye bye" << std::endl;
			break;												//salimos del loop y termina el programa
		}
    };
}
