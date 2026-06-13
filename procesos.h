#ifndef PROCESOS_H
#define PROCESOS_H

#include "estructuras.h"
#include "listas.h"

void menuConfigProcesos(Proceso **listaProcesos);

void agregarProceso(Proceso **listaProcesos);
void editarProceso(Proceso *listaProcesos);
void mostrarDetalleProceso(Proceso *p);

#endif /* PROCESOS_H */