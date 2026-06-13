#include "listas.h"
#include <stdio.h>
#include <stdlib.h>

void menuListaEjecucion(EstadoApp* app) {
    if (app->lista_procesos == NULL) {
        printf("Error: No hay procesos configurados.\n");
        return;
    }

    int opcion;
    printf("\n--- Módulo 2: Planificador ---\n");
    printf("1. FCFS\n");
    printf("2. SJF\n");
    printf("3. Por Prioridad\n");
    printf("Seleccione algoritmo: ");
    scanf("%d", &opcion);

    int n = contarProcesos(app->lista_procesos);
    if (app->cola_ejecucion) free(app->cola_ejecucion);
    app->cola_ejecucion = (Proceso**)malloc(n * sizeof(Proceso*));
    app->total_ejecucion = n;

    Proceso* actual = app->lista_procesos;
    for (int i = 0; i < n; i++) {
        app->cola_ejecucion[i] = actual;
        actual = actual->siguiente;
    }

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            bool swap = false;
            if (opcion == 2) { // SJF
                if (app->cola_ejecucion[j]->tiempoCPU > app->cola_ejecucion[j+1]->tiempoCPU) swap = true;
            } else if (opcion == 3) { // Prioridad
                if (app->cola_ejecucion[j]->prioridad > app->cola_ejecucion[j+1]->prioridad) swap = true;
            }
            if (swap) {
                Proceso* temp = app->cola_ejecucion[j];
                app->cola_ejecucion[j] = app->cola_ejecucion[j+1];
                app->cola_ejecucion[j+1] = temp;
            }
        }
    }

    printf("\nCola de ejecución generada:\n");
    for (int i = 0; i < n; i++) {
        printf("%d. %s (CPU: %.1f)\n", i+1, app->cola_ejecucion[i]->nombre, app->cola_ejecucion[i]->tiempoCPU);
    }
}
