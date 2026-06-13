#include "planificador.h"

Proceso* construirColaEjecucion(Proceso *listaProcesos, int algoritmo) {
    if (!listaProcesos) return NULL;

    Proceso *arr[256];
    int n = 0;

    for (Proceso *t = listaProcesos; t && n < 256; t = t->siguiente)
        arr[n++] = t;

    if (n == 0) return NULL;

    int intercambio = 1;

    while (intercambio) {
        intercambio = 0;

        for (int i = 0; i < n - 1; i++) {
            int swap = 0;

            if (algoritmo == PLANIF_SJF)
                swap = (arr[i]->tiempoCPU > arr[i+1]->tiempoCPU);

            else if (algoritmo == PLANIF_PRIORIDAD)
                swap = (arr[i]->prioridad > arr[i+1]->prioridad);

            if (swap) {
                Proceso *tmp = arr[i];
                arr[i] = arr[i+1];
                arr[i+1] = tmp;
                intercambio = 1;
            }
        }
    }

    for (int i = 0; i < n - 1; i++)
        arr[i]->siguiente = arr[i+1];

    arr[n-1]->siguiente = NULL;

    for (int i = 0; i < n; i++) {
        arr[i]->estado = LISTO;
        arr[i]->finalizado = 0;
        arr[i]->tiempoRestante = arr[i]->tiempoCPU;
    }

    return arr[0];
}

void mostrarColaEjecucion(Proceso *cola) {
    ...
}

void menuListaEjecucion(Proceso **listaProcesos,
                        Proceso **colaEjecucion) {
    ...
}