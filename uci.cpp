#include <iostream>
#include <string>

#include "global.h"
#include "tt.h"

void Uci()
{
    std::string nombre = "Omega 1.0";
    std::string autor = "Demián Mavrich";
    std::string recibido;
    char alg[5];

//    std::getline (std::cin,name);
//    std::cout << "Hello, " << name << "!\n";
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
//			std::cout << recibido << std::endl;
            if(recibido.find("startpos") == 0)			//ahora el string empieza o por startpos o por fen
            {
				recibido.erase(0,9);					//elimino la palabra startpos
//				std::cout << recibido << std::endl;

				LeeFEN();								//inicio registros en la posicion inicial
				InicializaHash();       				//una vez configurada la posición se establece la clave hash de la misma
				ReiniciaRegistros();					//al comenzar una nueva partida hay q reiniciar varios registros para q todo funcione bien (esto inicia la partida)

            	if(recibido.find("moves") == 0)			//me fijo si hay jugadas que realizar luego de startpos
				{
					recibido.erase(0,6);				//elimino la palabra moves
//					std::cout << recibido << std::endl;

					while(recibido.size())				//realizo todas las jugadas indicadas por el GUI a travez de recibido
					{
						int encontrado = recibido.find_first_of(" ");       //busco los espacios que delimitan el final de cada jugada dentro del string recibido
//						std::cout << encontrado << std::endl;
						if (encontrado == -1)					//compruebo que existan todavia espacios dentro de recibido
						{
							recibido.copy(alg,recibido.size(),0);
							recibido.clear();					//si no se encuentra ninguno más borro el string recibido

						}
						else									//entra aca si encuentra un nuevo espacio lo que significa que no hemos terminado de hacer las jugadas
						{
							recibido.copy(alg,encontrado,0);	//almaceno en alg la jugada (desde el caracter 0 de recibido hasta una longitud igual a encontrado)
							recibido.erase(0,encontrado + 1);	//elimino de recibido la jugada actual asi en la proxima pasada considero la siguiente
						}
						EsperaJugada(alg);					//llamo a la funcion pasandole alg que está en formato e2e4
						bien = Legal();						//verifico legalidad
						if(bien == 0)
						{
							std::cout << "Error ilegal" << std::endl;
						}
						Jugar(inicio,fin,bien);				//realizo la jugada
//						std::cout << recibido << std::endl;
					};
				}
            }
            else if(recibido.find("fen") == 0)			//si se quiere analizar a partir de una posición fen dada por la GUI
			{

			}
        }
        if(recibido.find("go") == 0)
        {
			Analiza();
			std::cout << "bestmove " << a << b << c << d << std::endl;
			std::cout << "nodos " << nodos << std::endl;
        }

    };
}
