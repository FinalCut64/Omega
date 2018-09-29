#include "eval.h"
#include "global.h"
#include "definiciones.h"
#include "bitboard.h"
#include "movgen.h"

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
		subtotal += POPCNT(ataques_caballos[inicio] & (!piezas_b));  		//evalua la movilidad de caballos ademas
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
		if (!(columna_mask[Columna(inicio)] & peones_b))					//si es una columna semiabierta (o posiblemente abierta)
			subtotal += 25;													//le sumo por buena ubicacion
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
				return -total;													//esto deja a la valoracion en 0 como debe ser ya q no hay material para ganar
			if (peones_b)
			{
				subtotal -= Distancia(MSB(rey_b),MSB(peones_b));
				subtotal += 4 * Distancia(MSB(rey_n),MSB(peones_b));
                if (!(rey_n & regladelcuadrado[negras][turno_c][inicio]))		//si el rey no entra en el cuadrado
                    return subtotal + 900;                                  	//la coronacion es inevitable
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
            if (total > 640)    								//cubre AA,AT,AD,CT,CD,TT,TD y DD contra el rey negro solo los cuales son todos finales
			{                  									//facilmente ganados con una minima heuristica de progreso comun a todos
				subtotal += mate[MSB(rey_n)];   				//llevar el rey indefenso a los bordes y esquinas del tablero
				subtotal -= Distancia(MSB(rey_b),MSB(rey_n));   //mantener el rey atacante cerca del rey contrario
				return subtotal;                                //solo con esto una busqueda de 2 ply encuentra el mate
			}
			if (total < -640)   								//el complemento de lo anterior si son las negras las q estan ganando
			{
				subtotal -= mate[MSB(rey_b)];
				subtotal += Distancia(MSB(rey_b),MSB(rey_n));
				return subtotal;
			}
            if (abs(total) < 20)    							//C vs c, A vs a, A vs C, T vs t y D vs d los que son evaluados todos como 0
                return 0;           							//(las colgadas y mates quedan a cargo de la busqueda)
            if (damas_b)            							//si entra aca solo queda el final RD vs rt
            {
				subtotal += mate[MSB(rey_n)];
				subtotal -= Distancia(MSB(rey_b),MSB(rey_n));
				return subtotal;
            }
            if (damas_n)            							//RT vs rd
            {
				subtotal -= mate[MSB(rey_b)];
				subtotal += Distancia(MSB(rey_b),MSB(rey_n));
				return subtotal;
            }
            if (torres_b)           							//solo entra en RT vs ra o RT vs rc
            {                       							//si bien este final es tablas le damos algo mas que 0 y una heuristica minima para que
																//intente ganarlo ya que es posible jugandose mal (o defenderlo)
				subtotal -= rey_final[MSB(rey_n)];  			//IMPORTANTE: aca rey_final SIEMPRE es negativo (por lo q se hace positivo)
				subtotal -= Distancia(MSB(rey_b),MSB(rey_n));   //que los reyes se mantengan cerca siempre ayuda a molestar
				return -total + subtotal + 10;      			//la valoracion varia (aproximadamente) entre 10 y 30
            }
            if (torres_n)           							//RC vs rt o RA vs RT
            {
				subtotal += rey_final[MSB(rey_b)];
				subtotal += Distancia(MSB(rey_b),MSB(rey_n));
				return -total + subtotal - 10;
            }
            if (abs(total) == 600)  							//esto solo se da en RCC vs r (y el opuesto con negras) los cuales indico como tablas
                return -600;        							//esto hace 0 la valoracion
			if ((alfiles_b & casillas_b) && caballos_b)         //ahora los 4 casos de mates de A y C
			{
				subtotal += mate_ac_b[MSB(rey_n)];
				subtotal -= Distancia(MSB(rey_b),MSB(rey_n));	//resto puntaje por estar lejos un rey de otro asi lo obligo a hacercarse
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
			if (abs(total) == 620)  							//por ultimo miro si se dio el final RAA vs r o viceversa con alfiles del mismo color lo q es
                return -620;        							//tablas ya q no existen posiciones de mate, aunque muy raro q alguna vez se entre aca
		}break;
	}
    inicio = MSB(rey_b);
	subtotal += rey_final[inicio];
	inicio = MSB(rey_n);
	subtotal -= rey_final[inicio];
	return subtotal;
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
