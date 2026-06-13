#include "listas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void imprimirEstadoMemoria(Marco* marcos) {
    printf(" [");
    while(marcos) {
        if (marcos->id_pagina == -1) printf(" -- ");
        else printf(" P%d ", marcos->id_pagina);
        marcos = marcos->siguiente;
    }
    printf("]\n");
}

void ejecutarEmulacionMMU(EstadoApp* app) {
    int n = app->config.cantidad_marcos;
    Marco* lista_marcos = NULL;
    for (int i = 0; i < n; i++) insertarMarco(&lista_marcos, crearMarco(i));

    int fallos = 0;
    int hits = 0;

    for (int t = 0; t < app->total_refs; t++) {
        int pag_ref = app->secuencia_referencias[t];
        printf("Tick %d: Ref P%d -> ", t + 1, pag_ref);

        bool encontrado = false;
        Marco* m = lista_marcos;
        while(m) {
            if (m->id_pagina == pag_ref) {
                encontrado = true;
                m->ultimo_uso = t;
                m->bit_referencia = 1;
                if (rand() % 4 == 0) m->bit_modificacion = 1;
                break;
            }
            m = m->siguiente;
        }

        if (encontrado) {
            printf("HIT  ");
            hits++;
        } else {
            printf("FAULT");
            fallos++;
            Marco* libre = buscarMarcoLibre(lista_marcos);
            if (libre) {
                libre->id_pagina = pag_ref;
                libre->tiempo_carga = t;
                libre->ultimo_uso = t;
                libre->bit_referencia = 1;
            } else {
                Marco* victima = NULL;
                if (strcmp(app->config.algoritmo_seleccionado, "FIFO") == 0) {
                    int min = t + 1;
                    Marco* temp = lista_marcos;
                    while(temp) { if(temp->tiempo_carga < min) { min = temp->tiempo_carga; victima = temp; } temp = temp->siguiente; }
                } else if (strcmp(app->config.algoritmo_seleccionado, "LRU") == 0) {
                    int min = t + 1;
                    Marco* temp = lista_marcos;
                    while(temp) { if(temp->ultimo_uso < min) { min = temp->ultimo_uso; victima = temp; } temp = temp->siguiente; }
                } else if (strcmp(app->config.algoritmo_seleccionado, "NRU") == 0) {
                    Marco* clases[4] = {NULL, NULL, NULL, NULL};
                    Marco* temp = lista_marcos;
                    while(temp) {
                        int c = (temp->bit_referencia << 1) | temp->bit_modificacion;
                        if (!clases[c]) clases[c] = temp;
                        temp = temp->siguiente;
                    }
                    for(int i=0; i<4; i++) { if(clases[i]) { victima = clases[i]; break; } }
                } else if (strcmp(app->config.algoritmo_seleccionado, "WS") == 0) {
                    int max_edad = -1;
                    Marco* temp = lista_marcos;
                    while(temp) {
                        int edad = t - temp->ultimo_uso;
                        if (temp->bit_referencia == 0 && edad > app->config.ventana_trabajo) {
                            if (edad > max_edad) { max_edad = edad; victima = temp; }
                        }
                        temp = temp->siguiente;
                    }
                    if (!victima) victima = lista_marcos;
                } else if (strcmp(app->config.algoritmo_seleccionado, "CLOCK") == 0) {
                    while (1) {
                        Marco* temp = buscarMarcoPorNumero(lista_marcos, app->config.reloj_puntero);
                        if (temp->bit_referencia == 0) { victima = temp; app->config.reloj_puntero = (app->config.reloj_puntero + 1) % n; break; }
                        temp->bit_referencia = 0;
                        app->config.reloj_puntero = (app->config.reloj_puntero + 1) % n;
                    }
                } else { victima = lista_marcos; }

                if (victima) {
                    printf(" (Sust P%d)", victima->id_pagina);
                    victima->id_pagina = pag_ref;
                    victima->tiempo_carga = t;
                    victima->ultimo_uso = t;
                    victima->bit_referencia = 1;
                    victima->bit_modificacion = 0;
                }
            }
        }
        imprimirEstadoMemoria(lista_marcos);
        if (t % 10 == 0) {
             Marco* temp = lista_marcos;
             while(temp) { temp->bit_referencia = 0; temp = temp->siguiente; }
        }
    }
    printf("\nResumen: Fallos: %d, Hits: %d, Eficiencia: %.1f%%\n", fallos, hits, (float)hits/app->total_refs*100);
    liberarListaMarcos(lista_marcos);
}
