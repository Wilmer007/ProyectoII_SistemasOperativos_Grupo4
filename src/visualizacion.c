#include "listas.h"
#include "TablaPaginas.h"
#include <stdio.h>

void menuReporteFinal(EstadoApp* app) {

    printf("Reporte final                      \n");


    if (!app->config.configurado) {
        printf("\n Sin datos de simulación. Configure el SO primero.\n");
        return;
    }

    printf("\n Config del sistema:\n");

    printf("  Algoritmo MMU:      %-30s \n", app->config.algoritmo_seleccionado);
    printf("  Marcos físicos:     %-30d \n", app->config.cantidad_marcos);
    printf("  Páginas virtuales:  %-30d \n", app->config.tamano_memoria);
    printf("  Espacio Swap:       %-30d \n", app->config.tamano_swap);


    // Estadísticas de memoria
    int pag_usadas, pag_libres, mar_usados, mar_libres;
    obtenerEstadisticasMemoria(app, &pag_usadas, &pag_libres, &mar_usados, &mar_libres);

    printf("\n Estado de memoria :\n");

    printf(" Páginas virtuales usadas:  %-30d \n", pag_usadas);
    printf(" Páginas virtuales libres:  %-30d \n", pag_libres);
    printf(" Marcos físicos usados:     %-30d \n", mar_usados);
    printf(" Marcos físicos libres:     %-30d \n", mar_libres);


    // Estadísticas de ejecución
    if (app->total_refs > 0) {
        float eficiencia = (float)app->total_hits / app->total_refs * 100;
        printf("\n Rendimiento simulacion:\n");

        printf("  Total referencias:     %-30d \n", app->total_refs);
        printf("  Hits (aciertos):       %-30d \n", app->total_hits);
        printf("  Fallos (page faults):  %-30d \n", app->total_fallos);
        printf("  Eficiencia:            %-30.2f%% \n", eficiencia);

    }

    // Mostrar tabla de páginas
    imprimirTablaPaginas(app);
    imprimirEstadoMarcos(app);
}

void printHeader() {

    printf("            SISTEMA MMU - EMULADOR DE MEMORIA                  \n");
    printf("       UNIDAD DE ADMINISTRACIÓN DE MEMORIA (Paginación)       \n");

}