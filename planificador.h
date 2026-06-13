#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include "estructuras.h"
#include "listas.h"

#define PLANIF_FCFS      0
#define PLANIF_SJF       1
#define PLANIF_PRIORIDAD 2

void menuListaEjecucion(Proceso **listaProcesos, Proceso **colaEjecucion);

Proceso* construirColaEjecucion(Proceso *listaProcesos, int algoritmo);

void mostrarColaEjecucion(Proceso *cola);

#endif /* PLANIFICADOR_H */