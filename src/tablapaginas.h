#ifndef TABLA_PAGINAS_H
#define TABLA_PAGINAS_H

#include "estructuras.h"

// Inicializar la tabla de páginas virtuales
void inicializarTablaPaginas(EstadoApp* app);

// Inicializar los marcos físicos
void inicializarMarcosFisicos(EstadoApp* app);

// Asignar páginas a un proceso (reserva en memoria virtual)
void asignarPaginasAProceso(EstadoApp* app, Proceso* p);

// Liberar páginas de un proceso finalizado
void liberarPaginasDeProceso(EstadoApp* app, int id_proceso);

// Obtener el marco asignado a una página virtual (-1 si no está en RAM)
int obtenerMarcoDePagina(EstadoApp* app, int id_proceso, int num_pagina_virtual);

// Cargar una página desde disco (swap) a un marco físico
void cargarPaginaAMarco(EstadoApp* app, int id_proceso, int num_pagina, int num_marco, int tick);

// Verificar si hay suficiente memoria virtual para todos los procesos
bool validarCapacidadMemoriaVirtual(EstadoApp* app);

// Obtener estadísticas de uso de memoria
void obtenerEstadisticasMemoria(EstadoApp* app, int* paginas_usadas, int* paginas_libres, int* marcos_usados, int* marcos_libres);

// Imprimir tabla de páginas completa (para debug)
void imprimirTablaPaginas(EstadoApp* app);

// Imprimir estado de marcos físicos
void imprimirEstadoMarcos(EstadoApp* app);

// Simular swap: mover página a disco
void moverPaginaADisco(EstadoApp* app, int id_proceso, int num_pagina);

// Traer página desde disco a RAM
void traerPaginaDesdeDisco(EstadoApp* app, int id_proceso, int num_pagina, int tick);

// Obtener página por número y proceso
Pagina* obtenerPagina(EstadoApp* app, int id_proceso, int num_pagina_virtual);

#endif // TABLA_PAGINAS_H