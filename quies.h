#ifndef QUIES_H_INCLUDED
#define QUIES_H_INCLUDED

#include "definiciones.h"

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

#endif // QUIES_H_INCLUDED
