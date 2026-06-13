#include "listas.h"
#include "TablaPaginas.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void menuConfigSO(EstadoApp* app) {

    printf(" Modulo 3 Configuracion del sistema operativo \n");


    // Mostrar requerimientos actuales
    int totalPags = 0;
    Proceso* p = app->lista_procesos;
    while(p) {
        totalPags += p->memoriaReq;
        p = p->siguiente;
    }

    printf("\n Requerimientos actuales: \n");
    printf("   Procesos registrados: %d\n", contarProcesos(app->lista_procesos));
    printf("   Paginas requeridas por procesos: %d\n", totalPags);

    // Configurar memoria virtual
    printf("\n Configuracion de memoria virtual L:\n");
    do {
        printf("   Páginas virtuales totales (mínimo %d): ", totalPags);
        scanf("%d", &(app->config.tamano_memoria));

        if (app->config.tamano_memoria < totalPags) {
            printf("    Error: La memoria virtual debe ser mayor o igual a %d páginas requeridas.\n", totalPags);
            printf("   Por favor, ingrese un valor válido.\n");
        }
    } while (app->config.tamano_memoria < totalPags);

    // Configurar marcos físicos
    printf("\n CONFIGURACIÓN DE MEMORIA FÍSICA:\n");
    printf("   Marcos físicos disponibles: ");
    scanf("%d", &(app->config.cantidad_marcos));

    // Configurar espacio de swap (opcional)
    printf("\n Espacio de disco :\n");
    printf("   Tamaño del swap en paginas (0 = sin swap): ");
    scanf("%d", &(app->config.tamano_swap));

    // Seleccionar algoritmo MMU
    printf("\n Menu:\n");
    printf("   ┌─────────────────────────────────────────────────┐\n");
    printf("   │ 1. FIFO (First In First Out)                   │\n");
    printf("   │ 2. LRU (Least Recently Used)                   │\n");
    printf("   │ 3. OPT (Óptimo - Optimal)                      │\n");
    printf("   │ 4. NRU (Not Recently Used)                     │\n");
    printf("   │ 5. Second Chance (Segunda Oportunidad)         │\n");
    printf("   │ 6. CLOCK (Reloj)                               │\n");
    printf("   └─────────────────────────────────────────────────┘\n");
    printf("   Seleccione algoritmo: ");

    int alg;
    scanf("%d", &alg);

    switch(alg) {
        case 1:
            strcpy(app->config.algoritmo_seleccionado, "FIFO");
            printf("Algoritmo FIFO seleccionado.\n");
            break;
        case 2:
            strcpy(app->config.algoritmo_seleccionado, "LRU");
            printf(" Algoritmo LRU seleccionado.\n");
            break;
        case 3:
            strcpy(app->config.algoritmo_seleccionado, "OPT");
            printf("Algoritmo OPT (Optimo) seleccionado.\n");
            break;
        case 4:
            strcpy(app->config.algoritmo_seleccionado, "NRU");
            printf("Algoritmo NRU seleccionado.\n");
            break;
        case 5:
            strcpy(app->config.algoritmo_seleccionado, "SC");
            printf("Algoritmo Second Chance seleccionado.\n");
            break;
        case 6:
            strcpy(app->config.algoritmo_seleccionado, "CLOCK");
            printf("Algoritmo CLOCK seleccionado.\n");
            break;
        default:
            strcpy(app->config.algoritmo_seleccionado, "FIFO");
            printf("Opcion no válida. Usando FIFO por defecto.\n");
            break;
    }

    // Inicializar estructuras de memoria
    printf("\n Inicializar procesos de memoria\n");
    inicializarTablaPaginas(app);
    inicializarMarcosFisicos(app);

    // Validar capacidad
    if (!validarCapacidadMemoriaVirtual(app)) {
        printf("\n No es optima \n");
        printf("   Considere aumentar la memoria virtual o reducir procesos.\n");
    }

    // Asignar páginas a los procesos existentes
    printf("\n Asignado a procesos\n");
    Proceso* actual = app->lista_procesos;
    while (actual != NULL) {
        asignarPaginasAProceso(app, actual);
        actual = actual->siguiente;
    }

    app->config.configurado = true;
    app->config.reloj_puntero = 0;



    // Mostrar resumen
    printf("Resumen de configuracion \n");
    printf("Memoria Virtual: %-40d paginas \n", app->config.tamano_memoria);
    printf("Memoria Física:  %-40d marcos  \n", app->config.cantidad_marcos);
    printf("Espacio Swap:    %-40d paginas \n", app->config.tamano_swap);
    printf("Algoritmo MMU:   %-40s \n", app->config.algoritmo_seleccionado);


    // Mostrar tabla de páginas inicial
    imprimirTablaPaginas(app);
}