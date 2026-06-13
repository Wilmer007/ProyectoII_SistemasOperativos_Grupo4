#ifndef LISTAS_H
#define LISTAS_H

#include "estructuras.h"

// Procesos
Proceso* crearProceso(int id, const char* nombre, int prioridad, float tiempoCPU, int memoriaReq);
void insertarProceso(Proceso** cabeza, Proceso* nuevo);
Proceso* buscarProcesoPorId(Proceso* cabeza, int id);
bool eliminarProceso(Proceso** cabeza, int id);
void liberarListaProcesos(Proceso* cabeza);
int contarProcesos(Proceso* cabeza);

// Páginas
Pagina* crearPagina(int numero, int id_proceso);
void insertarPagina(Pagina** cabeza, Pagina* nueva);
Pagina* buscarPagina(Pagina* cabeza, int numero);
void liberarListaPaginas(Pagina* cabeza);

// Marcos
Marco* crearMarco(int numero);
void insertarMarco(Marco** cabeza, Marco* nuevo);
Marco* buscarMarcoLibre(Marco* cabeza);
Marco* buscarMarcoPorNumero(Marco* cabeza, int numero);
void liberarListaMarcos(Marco* cabeza);

#endif // LISTAS_H
