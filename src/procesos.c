#include "listas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void menuConfigProcesos(EstadoApp* app) {
    int opcion;
    do {
        printf("\n--- Módulo 1: Configuración de Procesos ---\n");
        printf("1. Agregar Proceso\n");
        printf("2. Editar Proceso\n");
        printf("3. Eliminar Proceso\n");
        printf("4. Listar Procesos\n");
        printf("0. Volver\n");
        printf("Seleccione una opción: ");
        if (scanf("%d", &opcion) != 1) {
            while (getchar() != '\n');
            continue;
        }

        if (opcion == 1) {
            int id, prioridad, memoria;
            char nombre[50];
            float cpu;
            printf("ID: "); scanf("%d", &id);
            if (buscarProcesoPorId(app->lista_procesos, id)) {
                printf("Error: ID duplicado.\n");
                continue;
            }
            printf("Nombre: "); scanf("%s", nombre);
            printf("Prioridad: "); scanf("%d", &prioridad);
            printf("Tiempo CPU: "); scanf("%f", &cpu);
            printf("Páginas: "); scanf("%d", &memoria);
            Proceso* p = crearProceso(id, nombre, prioridad, cpu, memoria);
            insertarProceso(&(app->lista_procesos), p);
            printf("Proceso agregado.\n");
        } else if (opcion == 2) {
            int id;
            printf("ID del proceso a editar: "); scanf("%d", &id);
            Proceso* p = buscarProcesoPorId(app->lista_procesos, id);
            if (p) {
                printf("Nuevo nombre (actual: %s): ", p->nombre); scanf("%s", p->nombre);
                printf("Nueva prioridad (actual: %d): ", p->prioridad); scanf("%d", &(p->prioridad));
                printf("Nuevo tiempo CPU (actual: %.1f): ", p->tiempoCPU); scanf("%f", &(p->tiempoCPU));
                p->tiempoRestante = p->tiempoCPU;
                printf("Nuevas páginas (actual: %d): ", p->memoriaReq); scanf("%d", &(p->memoriaReq));
                printf("Proceso editado.\n");
            } else {
                printf("Proceso no encontrado.\n");
            }
        } else if (opcion == 3) {
            int id;
            printf("ID del proceso a eliminar: "); scanf("%d", &id);
            if (eliminarProceso(&(app->lista_procesos), id)) {
                printf("Proceso eliminado.\n");
            } else {
                printf("Proceso no encontrado.\n");
            }
        } else if (opcion == 4) {
            printf("\nID\tNombre\tPrioridad\tCPU\tPáginas\n");
            Proceso* actual = app->lista_procesos;
            while (actual) {
                printf("%d\t%s\t%d\t\t%.1f\t%d\n", actual->id, actual->nombre, actual->prioridad, actual->tiempoCPU, actual->memoriaReq);
                actual = actual->siguiente;
            }
        }
    } while (opcion != 0);
}
