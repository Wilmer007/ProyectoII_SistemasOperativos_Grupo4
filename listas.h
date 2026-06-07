#ifndef LISTAS_H
#define LISTAS_H



#include "estructuras.h"


Proceso* crearProceso(int id, const char *nombre, int prioridad,
                      int tiempoCPU, int memoriaReq);
void     insertarProceso(Proceso **cabeza, Proceso *nuevo);
Proceso* buscarProcesoPorId(Proceso *cabeza, int id);
void     eliminarProceso(Proceso **cabeza, int id);
void     listarProcesos(Proceso *cabeza);
int      contarProcesos(Proceso *cabeza);
void     liberarListaProcesos(Proceso **cabeza);


Pagina*  crearPagina(int numPagina, int idProceso);
void     insertarPagina(Pagina **cabeza, Pagina *nueva);
Pagina*  buscarPagina(Pagina *cabeza, int numPagina, int idProceso);
void     listarPaginas(Pagina *cabeza);
void     liberarListaPaginas(Pagina **cabeza);


Marco*   crearMarco(int numMarco);
void     insertarMarco(Marco **cabeza, Marco *nuevo);
Marco*   buscarMarcoLibre(Marco *cabeza);
Marco*   buscarMarcoPorNumero(Marco *cabeza, int numMarco);
Marco*   buscarMarcoPorPagina(Marco *cabeza, int numPagina, int idProceso);
int      contarMarcosOcupados(Marco *cabeza);
void     listarMarcos(Marco *cabeza);
void     liberarListaMarcos(Marco **cabeza);




Marco*   buscarMarcoNRU(Marco *cabeza);

void     limpiarBitsReferencia(Marco *cabeza);


Marco*   buscarMarcoReloj(Marco *cabeza, ConfigSO *cfg);


Marco*   buscarMarcoFueraVentana(Marco *cabeza, Pagina *paginas,
                                  int reloj, int ventana);

void     actualizarConjuntoTrabajo(Marco *cabeza, Pagina *paginas,
                                    int reloj, int ventana);

#endif /* LISTAS_H */


