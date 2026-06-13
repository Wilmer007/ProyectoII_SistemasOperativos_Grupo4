/*
 * mmu_algoritmos.c
 * implementacion de los algoritmos de reemplazo de paginas
 * algoritmos: fifo, lru, opt, nru, second chance, clock
 */

#include "listas.h"
#include "TablaPaginas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
 * funcion auxiliar: busca un marco libre en la lista de marcos fisicos
 * retorna NULL si no hay marcos libres
 */
Marco* buscarMarcoLibreDesdeLista(Marco* cabeza) {
    while (cabeza != NULL) {
        if (cabeza->id_pagina == -1) {
            return cabeza;
        }
        cabeza = cabeza->siguiente;
    }
    return NULL;
}

/*
 * funcion auxiliar: imprime el estado actual de los marcos fisicos
 * muestra que pagina esta cargada en cada marco
 */
void imprimirEstadoMemoria(EstadoApp* app) {
    printf("  [");
    Marco* m = app->marcos_fisicos;
    while(m) {
        if (m->id_pagina == -1) {
            printf(" -- ");
        } else {
            printf(" p%d ", m->id_pagina);
        }
        m = m->siguiente;
    }
    printf("]\n");
}

/*
 * algoritmo fifo (first in first out)
 * reemplaza la pagina que lleva mas tiempo en memoria
 * utiliza el campo tiempo_carga para determinar la pagina mas antigua
 */
Marco* fifo_reemplazo(EstadoApp* app, int tick) {
    Marco* victima = NULL;
    int min_tiempo = INT_MAX;
    Marco* temp = app->marcos_fisicos;

    while(temp) {
        if(temp->id_pagina != -1 && temp->tiempo_carga < min_tiempo) {
            min_tiempo = temp->tiempo_carga;
            victima = temp;
        }
        temp = temp->siguiente;
    }
    return victima ? victima : app->marcos_fisicos;
}

/*
 * algoritmo lru (least recently used)
 * reemplaza la pagina que no ha sido usada por mas tiempo
 * utiliza el campo ultimo_uso para determinar la pagina menos reciente
 */
Marco* lru_reemplazo(EstadoApp* app, int tick) {
    Marco* victima = NULL;
    int min_uso = INT_MAX;
    Marco* temp = app->marcos_fisicos;

    while(temp) {
        if(temp->id_pagina != -1 && temp->ultimo_uso < min_uso) {
            min_uso = temp->ultimo_uso;
            victima = temp;
        }
        temp = temp->siguiente;
    }
    return victima ? victima : app->marcos_fisicos;
}

/*
 * funcion auxiliar para algoritmo opt
 * busca la proxima vez que se usara una pagina en el futuro
 * retorna la posicion en la secuencia o INT_MAX si nunca se usara
 */
int buscarProximaReferencia(int* secuencia, int inicio, int total_refs, int pagina_buscada) {
    for (int i = inicio + 1; i < total_refs; i++) {
        if (secuencia[i] == pagina_buscada) {
            return i;
        }
    }
    return INT_MAX;
}

/*
 * algoritmo opt (optimo)
 * reemplaza la pagina que se usara mas lejos en el futuro
 * este es el algoritmo teorico ideal, no implementable en la realidad
 */
Marco* opt_reemplazo(EstadoApp* app, int tick, int* secuencia_global, int total_refs) {
    Marco* victima = NULL;
    int maxima_distancia = -1;
    Marco* temp = app->marcos_fisicos;

    while(temp) {
        if(temp->id_pagina != -1) {
            int distancia = buscarProximaReferencia(secuencia_global, tick, total_refs, temp->id_pagina);
            if (distancia == INT_MAX) {
                return temp;
            }
            if (distancia > maxima_distancia) {
                maxima_distancia = distancia;
                victima = temp;
            }
        }
        temp = temp->siguiente;
    }
    return victima ? victima : app->marcos_fisicos;
}

/*
 * algoritmo nru (not recently used)
 * clasifica las paginas en 4 categorias segun los bits R y M
 * clase 0: r=0,m=0 (mejor para reemplazar)
 * clase 1: r=0,m=1
 * clase 2: r=1,m=0
 * clase 3: r=1,m=1 (peor para reemplazar)
 * reemplaza una pagina de la clase mas baja disponible
 */
Marco* nru_reemplazo(EstadoApp* app) {
    Marco* clases[4] = {NULL, NULL, NULL, NULL};
    Marco* temp = app->marcos_fisicos;

    while(temp) {
        if(temp->id_pagina != -1) {
            int clase = (temp->bit_referencia << 1) | temp->bit_modificacion;
            if (!clases[clase]) {
                clases[clase] = temp;
            }
        }
        temp = temp->siguiente;
    }

    for(int i = 0; i < 4; i++) {
        if(clases[i]) {
            return clases[i];
        }
    }
    return app->marcos_fisicos;
}

/*
 * algoritmo second chance (segunda oportunidad)
 * variante de fifo que da una segunda oportunidad a las paginas referenciadas
 * si una pagina tiene bit r=1, se limpia el bit y se mueve al final
 * si tiene bit r=0, se reemplaza
 */
Marco* second_chance_reemplazo(EstadoApp* app) {
    static Marco* puntero = NULL;

    if (puntero == NULL) {
        puntero = app->marcos_fisicos;
    }

    while (1) {
        if (puntero == NULL) {
            puntero = app->marcos_fisicos;
        }

        if (puntero->id_pagina == -1) {
            Marco* resultado = puntero;
            puntero = puntero->siguiente;
            return resultado;
        }

        if (puntero->bit_referencia == 0) {
            Marco* resultado = puntero;
            puntero = puntero->siguiente;
            return resultado;
        } else {
            printf(" (segunda oportunidad a p%d)", puntero->id_pagina);
            puntero->bit_referencia = 0;
            puntero = puntero->siguiente;
        }
    }
}

/*
 * algoritmo clock (reloj)
 * implementacion eficiente de second chance usando un puntero circular
 * evita mover elementos en la lista, solo avanza el puntero
 */
Marco* clock_reemplazo(EstadoApp* app) {
    int n = app->config.cantidad_marcos;
    Marco* temp = app->marcos_fisicos;
    int idx = 0;

    while(temp && idx < app->config.reloj_puntero) {
        temp = temp->siguiente;
        idx++;
    }
    if (!temp) {
        temp = app->marcos_fisicos;
    }

    while (1) {
        if (temp->id_pagina == -1) {
            app->config.reloj_puntero = (app->config.reloj_puntero + 1) % n;
            return temp;
        }

        if (temp->bit_referencia == 0) {
            app->config.reloj_puntero = (app->config.reloj_puntero + 1) % n;
            return temp;
        } else {
            temp->bit_referencia = 0;
            temp = temp->siguiente;
            if (!temp) {
                temp = app->marcos_fisicos;
            }
            app->config.reloj_puntero = (app->config.reloj_puntero + 1) % n;
        }
    }
}

/*
 * funcion principal de emulacion de la mmu
 * ejecuta la secuencia de referencias y aplica el algoritmo seleccionado
 * muestra el resultado de cada tick (hit o fault)
 * al final muestra estadisticas de rendimiento
 */
void ejecutarEmulacionMMU(EstadoApp* app) {
    if (!app->config.configurado || app->total_refs == 0) {
        printf("error: configure el so y la cola de ejecucion primero\n");
        return;
    }

    /* inicializar contadores */
    app->total_fallos = 0;
    app->total_hits = 0;
    app->config.reloj_puntero = 0;

    /* limpiar marcos fisicos */
    Marco* reset = app->marcos_fisicos;
    while(reset) {
        reset->id_pagina = -1;
        reset->id_proceso = -1;
        reset->tiempo_carga = 0;
        reset->ultimo_uso = 0;
        reset->bit_referencia = 0;
        reset->bit_modificacion = 0;
        reset = reset->siguiente;
    }

    /* limpiar paginas virtuales */
    Pagina* pag_reset = app->paginas_virtuales;
    while(pag_reset) {
        pag_reset->marco_asignado = -1;
        pag_reset->bit_referencia = 0;
        pag_reset->tiempo_uso = 0;
        pag_reset = pag_reset->siguiente;
    }

    printf("\niniciando emulacion mmu [%s]\n\n", app->config.algoritmo_seleccionado);

    /* pre-calcular para opt: necesitamos la secuencia de paginas globales */
    int* secuencia_global = NULL;
    if (strcmp(app->config.algoritmo_seleccionado, "OPT") == 0) {
        secuencia_global = (int*)malloc(app->total_refs * sizeof(int));
        int offset = 0;
        for (int i = 0; i < app->total_refs; i++) {
            int pag_ref = app->secuencia_referencias[i];
            int temp_offset = 0;
            for (int j = 0; j < app->total_ejecucion; j++) {
                Proceso* proc = app->cola_ejecucion[j];
                if (pag_ref < temp_offset + proc->memoriaReq) {
                    secuencia_global[i] = proc->pagina_inicio + (pag_ref - temp_offset);
                    break;
                }
                temp_offset += proc->memoriaReq;
            }
        }
    }

    /* bucle principal de emulacion */
    for (int t = 0; t < app->total_refs; t++) {
        int pag_ref = app->secuencia_referencias[t];

        /* determinar que proceso y que pagina virtual */
        int offset = 0;
        Proceso* p = NULL;
        int pagina_virtual = 0;

        for (int i = 0; i < app->total_ejecucion; i++) {
            Proceso* proc = app->cola_ejecucion[i];
            if (pag_ref < offset + proc->memoriaReq) {
                p = proc;
                pagina_virtual = pag_ref - offset;
                break;
            }
            offset += proc->memoriaReq;
        }

        if (!p) continue;

        printf("tick %3d | %-8s | pag.%2d | ", t + 1, p->nombre, pagina_virtual);

        /* verificar si la pagina esta en memoria */
        int marco_num = obtenerMarcoDePagina(app, p->id, pagina_virtual);

        if (marco_num != -1) {
            /* hit: la pagina esta en memoria */
            printf("hit (marco %d)\n", marco_num);
            app->total_hits++;

            Pagina* pag = obtenerPagina(app, p->id, pagina_virtual);
            if (pag) {
                pag->bit_referencia = 1;
                pag->tiempo_uso = t;

                Marco* m = app->marcos_fisicos;
                while(m) {
                    if(m->numero_marco == marco_num) {
                        m->ultimo_uso = t;
                        m->bit_referencia = 1;
                        if (rand() % 10 == 0) {
                            m->bit_modificacion = 1;
                            if(pag) pag->bit_modificacion = 1;
                        }
                        break;
                    }
                    m = m->siguiente;
                }
            }
        } else {
            /* fault: la pagina no esta en memoria */
            printf("fault");
            app->total_fallos++;

            /* buscar marco libre */
            Marco* marco_libre = buscarMarcoLibreDesdeLista(app->marcos_fisicos);

            if (marco_libre) {
                printf(" -> marco libre %d\n", marco_libre->numero_marco);
                cargarPaginaAMarco(app, p->id, pagina_virtual, marco_libre->numero_marco, t);
            } else {
                /* no hay marcos libres, hay que reemplazar */
                Marco* victima = NULL;

                if (strcmp(app->config.algoritmo_seleccionado, "FIFO") == 0) {
                    victima = fifo_reemplazo(app, t);
                    printf(" -> fifo -> reemplaza ");
                } else if (strcmp(app->config.algoritmo_seleccionado, "LRU") == 0) {
                    victima = lru_reemplazo(app, t);
                    printf(" -> lru -> reemplaza ");
                } else if (strcmp(app->config.algoritmo_seleccionado, "OPT") == 0) {
                    victima = opt_reemplazo(app, t, secuencia_global, app->total_refs);
                    printf(" -> opt -> reemplaza ");
                } else if (strcmp(app->config.algoritmo_seleccionado, "NRU") == 0) {
                    victima = nru_reemplazo(app);
                    printf(" -> nru -> reemplaza ");
                } else if (strcmp(app->config.algoritmo_seleccionado, "SC") == 0) {
                    victima = second_chance_reemplazo(app);
                    printf(" -> second chance -> reemplaza ");
                } else {
                    victima = clock_reemplazo(app);
                    printf(" -> clock -> reemplaza ");
                }

                if (victima) {
                    printf("p%d del marco %d\n", victima->id_pagina, victima->numero_marco);

                    /* si la pagina fue modificada y hay swap, mover a disco */
                    if (victima->bit_modificacion == 1 && app->config.tamano_swap > 0) {
                        Pagina* pag_vieja = app->paginas_virtuales;
                        while(pag_vieja) {
                            if(pag_vieja->numero_pagina == victima->id_pagina) {
                                int proc_id = pag_vieja->id_proceso;
                                Proceso* proc_viejo = buscarProcesoPorId(app->lista_procesos, proc_id);
                                if(proc_viejo) {
                                    int pag_virtual_vieja = victima->id_pagina - proc_viejo->pagina_inicio;
                                    moverPaginaADisco(app, proc_id, pag_virtual_vieja);
                                }
                                break;
                            }
                            pag_vieja = pag_vieja->siguiente;
                        }
                    }

                    /* cargar la nueva pagina */
                    cargarPaginaAMarco(app, p->id, pagina_virtual, victima->numero_marco, t);
                }
            }
        }

        /* mostrar estado actual de la memoria */
        imprimirEstadoMemoria(app);
        printf("\n");

        /* resetear bits de referencia cada 15 ticks para nru y clock */
        if ((t + 1) % 15 == 0 && t > 0) {
            Marco* m = app->marcos_fisicos;
            while(m) {
                m->bit_referencia = 0;
                m = m->siguiente;
            }
            printf("  [reset de bits de referencia]\n\n");
        }
    }

    /* liberar memoria usada para opt */
    if (secuencia_global) {
        free(secuencia_global);
    }

    /* mostrar estadisticas finales */
    printf("\nresumen final:\n");
    printf("  algoritmo:        %s\n", app->config.algoritmo_seleccionado);
    printf("  total referencias: %d\n", app->total_refs);
    printf("  hits:              %d\n", app->total_hits);
    printf("  fallos:            %d\n", app->total_fallos);

    if (app->total_refs > 0) {
        float eficiencia = (float)app->total_hits / app->total_refs * 100;
        printf("  eficiencia:        %.2f%%\n", eficiencia);
    }

    /* mostrar estado final de la memoria */
    imprimirTablaPaginas(app);
    imprimirEstadoMarcos(app);
}