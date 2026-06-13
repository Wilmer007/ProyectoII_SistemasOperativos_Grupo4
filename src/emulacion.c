#include "listas.h"
#include <stdio.h>
#include <stdlib.h>

void generarSecuencia(EstadoApp* app) {
    if (app->total_ejecucion == 0) return;

    int totalRefs = 0;
    for (int i = 0; i < app->total_ejecucion; i++) {
        totalRefs += (int)app->cola_ejecucion[i]->tiempoCPU;
    }

    if (app->secuencia_referencias) free(app->secuencia_referencias);
    app->secuencia_referencias = (int*)malloc(totalRefs * sizeof(int));
    app->total_refs = totalRefs;

    int k = 0;
    int offset = 0;
    for (int i = 0; i < app->total_ejecucion; i++) {
        Proceso* p = app->cola_ejecucion[i];
        int refs = (int)p->tiempoCPU;
        for (int j = 0; j < refs; j++) {
            app->secuencia_referencias[k++] = offset + (rand() % p->memoriaReq);
        }
        offset += p->memoriaReq;
    }
}

extern void ejecutarEmulacionMMU(EstadoApp* app);

void menuEmulacion(EstadoApp* app) {
    if (!app->config.configurado || app->total_ejecucion == 0) {
        printf("Error: Configure el SO y la cola de ejecución primero.\n");
        return;
    }

    generarSecuencia(app);
    printf("\nIniciando emulación MMU [%s]...\n", app->config.algoritmo_seleccionado);
    ejecutarEmulacionMMU(app);
}
