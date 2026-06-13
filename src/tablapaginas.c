#include "listas.h"
#include "TablaPaginas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void inicializarTablaPaginas(EstadoApp* app) {
    if (app->paginas_virtuales) {
        // Liberar lista existente
        Pagina* actual = app->paginas_virtuales;
        while (actual) {
            Pagina* temp = actual;
            actual = actual->siguiente;
            free(temp);
        }
        app->paginas_virtuales = NULL;
    }

    int total_paginas = app->config.tamano_memoria;

    for (int i = 0; i < total_paginas; i++) {
        Pagina* nueva = (Pagina*)malloc(sizeof(Pagina));
        if (nueva) {
            nueva->numero_pagina = i;
            nueva->id_proceso = -1;
            nueva->marco_asignado = -1;
            nueva->bit_referencia = 0;
            nueva->bit_modificacion = 0;
            nueva->tiempo_uso = 0;
            nueva->tiempo_carga = 0;
            nueva->en_disco = false;
            nueva->siguiente = NULL;

            // Insertar al final
            if (app->paginas_virtuales == NULL) {
                app->paginas_virtuales = nueva;
            } else {
                Pagina* temp = app->paginas_virtuales;
                while (temp->siguiente) temp = temp->siguiente;
                temp->siguiente = nueva;
            }
        }
    }

    printf("Tabla de paginas inicializada: %d paginas virtuales.\n", total_paginas);
}

void inicializarMarcosFisicos(EstadoApp* app) {
    if (app->marcos_fisicos) {
        Marco* actual = app->marcos_fisicos;
        while (actual) {
            Marco* temp = actual;
            actual = actual->siguiente;
            free(temp);
        }
        app->marcos_fisicos = NULL;
    }

    int total_marcos = app->config.cantidad_marcos;

    for (int i = 0; i < total_marcos; i++) {
        Marco* nuevo = (Marco*)malloc(sizeof(Marco));
        if (nuevo) {
            nuevo->numero_marco = i;
            nuevo->id_pagina = -1;
            nuevo->id_proceso = -1;
            nuevo->tiempo_carga = 0;
            nuevo->ultimo_uso = 0;
            nuevo->bit_referencia = 0;
            nuevo->bit_modificacion = 0;
            nuevo->siguiente = NULL;

            if (app->marcos_fisicos == NULL) {
                app->marcos_fisicos = nuevo;
            } else {
                Marco* temp = app->marcos_fisicos;
                while (temp->siguiente) temp = temp->siguiente;
                temp->siguiente = nuevo;
            }
        }
    }

    printf("Marcos fisicos inicializados: %d marcos.\n", total_marcos);
}

void asignarPaginasAProceso(EstadoApp* app, Proceso* p) {
    int paginas_necesarias = p->memoriaReq;
    int paginas_asignadas = 0;

    Pagina* actual = app->paginas_virtuales;
    int primera_pagina = -1;

    while (actual != NULL && paginas_asignadas < paginas_necesarias) {
        if (actual->id_proceso == -1) {
            actual->id_proceso = p->id;
            actual->en_disco = false;
            if (primera_pagina == -1) {
                primera_pagina = actual->numero_pagina;
            }
            paginas_asignadas++;
        }
        actual = actual->siguiente;
    }

    p->pagina_inicio = primera_pagina;

    if (paginas_asignadas < paginas_necesarias) {
        printf("ADVERTENCIA: No hay suficientes paginas virtuales para el proceso %s.\n", p->nombre);
    } else {
        printf("Proceso %s asignado: %d paginas (inicia en pagina %d)\n", p->nombre, paginas_asignadas, primera_pagina);
    }
}

void liberarPaginasDeProceso(EstadoApp* app, int id_proceso) {
    Pagina* actual = app->paginas_virtuales;
    int liberadas = 0;

    while (actual != NULL) {
        if (actual->id_proceso == id_proceso) {
            if (actual->marco_asignado != -1) {
                Marco* marco = app->marcos_fisicos;
                while (marco != NULL) {
                    if (marco->numero_marco == actual->marco_asignado) {
                        marco->id_pagina = -1;
                        marco->id_proceso = -1;
                        break;
                    }
                    marco = marco->siguiente;
                }
            }
            actual->id_proceso = -1;
            actual->marco_asignado = -1;
            actual->en_disco = false;
            liberadas++;
        }
        actual = actual->siguiente;
    }

    printf("Proceso %d liberado: %d paginas liberadas.\n", id_proceso, liberadas);
}

int obtenerMarcoDePagina(EstadoApp* app, int id_proceso, int num_pagina_virtual) {
    Proceso* p = app->lista_procesos;
    while (p != NULL && p->id != id_proceso) p = p->siguiente;
    if (p == NULL) return -1;

    int pagina_global = p->pagina_inicio + num_pagina_virtual;

    Pagina* actual = app->paginas_virtuales;
    while (actual != NULL) {
        if (actual->numero_pagina == pagina_global && actual->id_proceso == id_proceso) {
            return actual->marco_asignado;
        }
        actual = actual->siguiente;
    }

    return -1;
}

void cargarPaginaAMarco(EstadoApp* app, int id_proceso, int num_pagina, int num_marco, int tick) {
    Proceso* p = app->lista_procesos;
    while (p != NULL && p->id != id_proceso) p = p->siguiente;
    if (p == NULL) return;

    int pagina_global = p->pagina_inicio + num_pagina;

    Pagina* pagina = app->paginas_virtuales;
    while (pagina != NULL) {
        if (pagina->numero_pagina == pagina_global && pagina->id_proceso == id_proceso) {
            pagina->marco_asignado = num_marco;
            pagina->tiempo_carga = tick;
            pagina->tiempo_uso = tick;
            pagina->bit_referencia = 1;
            pagina->en_disco = false;
            break;
        }
        pagina = pagina->siguiente;
    }

    Marco* marco = app->marcos_fisicos;
    while (marco != NULL) {
        if (marco->numero_marco == num_marco) {
            marco->id_pagina = pagina_global;
            marco->id_proceso = id_proceso;
            marco->tiempo_carga = tick;
            marco->ultimo_uso = tick;
            break;
        }
        marco = marco->siguiente;
    }
}

bool validarCapacidadMemoriaVirtual(EstadoApp* app) {
    int paginas_requeridas = 0;
    Proceso* actual = app->lista_procesos;

    while (actual != NULL) {
        paginas_requeridas += actual->memoriaReq;
        actual = actual->siguiente;
    }

    if (paginas_requeridas > app->config.tamano_memoria) {
        printf("\nERROR: Memoria virtual insuficiente!\n");
        printf("Paginas requeridas: %d, disponibles: %d\n", paginas_requeridas, app->config.tamano_memoria);
        return false;
    }

    printf("\nValidacion de memoria: OK (%d paginas requeridas de %d disponibles)\n", paginas_requeridas, app->config.tamano_memoria);
    return true;
}

void obtenerEstadisticasMemoria(EstadoApp* app, int* paginas_usadas, int* paginas_libres,
                                 int* marcos_usados, int* marcos_libres) {
    *paginas_usadas = 0;
    *paginas_libres = 0;
    *marcos_usados = 0;
    *marcos_libres = 0;

    Pagina* p = app->paginas_virtuales;
    while (p != NULL) {
        if (p->id_proceso != -1) (*paginas_usadas)++;
        else (*paginas_libres)++;
        p = p->siguiente;
    }

    Marco* m = app->marcos_fisicos;
    while (m != NULL) {
        if (m->id_pagina != -1) (*marcos_usados)++;
        else (*marcos_libres)++;
        m = m->siguiente;
    }
}

void imprimirTablaPaginas(EstadoApp* app) {
    printf("\n=== TABLA DE PAGINAS ===\n");
    printf("Pagina\tProceso\tMarco\tR\tM\tDisco\n");
    printf("------\t-------\t-----\t-\t-\t-----\n");

    Pagina* actual = app->paginas_virtuales;
    while (actual != NULL) {
        char nombre_proc[15] = "LIBRE";
        if (actual->id_proceso != -1) {
            Proceso* p = app->lista_procesos;
            while (p != NULL && p->id != actual->id_proceso) p = p->siguiente;
            if (p != NULL) snprintf(nombre_proc, sizeof(nombre_proc), "%s", p->nombre);
        }

        printf("%d\t%s\t%d\t%d\t%d\t%s\n",
               actual->numero_pagina, nombre_proc,
               actual->marco_asignado,
               actual->bit_referencia, actual->bit_modificacion,
               actual->en_disco ? "Si" : "No");

        actual = actual->siguiente;
    }
}

void imprimirEstadoMarcos(EstadoApp* app) {
    printf("\n=== MARCOS DE MEMORIA FISICA ===\n");
    printf("Marco\tPagina\tProceso\tR\tM\tUltimoUso\n");
    printf("-----\t------\t-------\t-\t-\t---------\n");

    Marco* actual = app->marcos_fisicos;
    while (actual != NULL) {
        char nombre_proc[15] = "LIBRE";
        if (actual->id_proceso != -1) {
            Proceso* p = app->lista_procesos;
            while (p != NULL && p->id != actual->id_proceso) p = p->siguiente;
            if (p != NULL) snprintf(nombre_proc, sizeof(nombre_proc), "%s", p->nombre);
        }

        printf("%d\t%d\t%s\t%d\t%d\t%d\n",
               actual->numero_marco, actual->id_pagina, nombre_proc,
               actual->bit_referencia, actual->bit_modificacion, actual->ultimo_uso);

        actual = actual->siguiente;
    }
}

void moverPaginaADisco(EstadoApp* app, int id_proceso, int num_pagina) {
    Proceso* p = app->lista_procesos;
    while (p != NULL && p->id != id_proceso) p = p->siguiente;
    if (p == NULL) return;

    int pagina_global = p->pagina_inicio + num_pagina;

    Pagina* pagina = app->paginas_virtuales;
    while (pagina != NULL) {
        if (pagina->numero_pagina == pagina_global && pagina->id_proceso == id_proceso) {
            pagina->en_disco = true;
            pagina->marco_asignado = -1;
            printf("Pagina %d del proceso %s movida a DISCO (swap)\n", num_pagina, p->nombre);
            break;
        }
        pagina = pagina->siguiente;
    }
}

void traerPaginaDesdeDisco(EstadoApp* app, int id_proceso, int num_pagina, int tick) {
    Proceso* p = app->lista_procesos;
    while (p != NULL && p->id != id_proceso) p = p->siguiente;
    if (p == NULL) return;

    int pagina_global = p->pagina_inicio + num_pagina;

    Pagina* pagina = app->paginas_virtuales;
    while (pagina != NULL) {
        if (pagina->numero_pagina == pagina_global && pagina->id_proceso == id_proceso) {
            if (pagina->en_disco) {
                pagina->en_disco = false;
                printf("Pagina %d del proceso %s traida desde DISCO (swap) en tick %d\n", num_pagina, p->nombre, tick);
            }
            break;
        }
        pagina = pagina->siguiente;
    }
}

Pagina* obtenerPagina(EstadoApp* app, int id_proceso, int num_pagina_virtual) {
    Proceso* p = app->lista_procesos;
    while (p != NULL && p->id != id_proceso) p = p->siguiente;
    if (p == NULL) return NULL;

    int pagina_global = p->pagina_inicio + num_pagina_virtual;

    Pagina* actual = app->paginas_virtuales;
    while (actual != NULL) {
        if (actual->numero_pagina == pagina_global && actual->id_proceso == id_proceso) {
            return actual;
        }
        actual = actual->siguiente;
    }

    return NULL;
}