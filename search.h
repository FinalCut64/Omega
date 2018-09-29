#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED

void Analiza();								//busca la "mejor" jugada q se pueda (aca esta toda la magia y los errores jeje)
int  Raiz(int,int,BYTE);                    //Negamax() para la posicion raiz
int  NegaMax(int,int,BYTE,BYTE);			//funcion de busqueda Negamax con poda alfa-beta y demas tecnicas de busqueda
BYTE Nulo_ok(BYTE);                         //devuelve 1 si se puede aplicar poda de movimiento nulo en el nodo actual (0 sino)
BYTE LMR_ok(BYTE,unsigned int);             //devuelve 1 si se puede aplicar LMR al movimiento actual (0 sino)
void Ordenar(BYTE);
void QuickSort(int a[],int,int, BYTE);


BYTE Repite();								//avisa si la posicion q esta analizando el pic ya se produjo para q la evalue como tablas si es asi y no repita en posiciones ganadas
BYTE TresRepet();							//permite determinar si la partida debe acabar por regla de tres repeticiones

void Guardar();								//esta junto con recuperar permite manejar la posibilidad de salir de NegaMax sin concluir
void Recuperar();							//por lo q todos los registros quedarian en cualquier estado. Estas 2 funciones solucionan eso
void BorrarVP();
void DescomprimirVP();

void ManejarReloj();

#endif // SEARCH_H_INCLUDED
