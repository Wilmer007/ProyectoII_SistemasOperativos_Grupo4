#include "listas.h"
#include "TablaPaginas.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

extern void menuConfigProcesos(EstadoApp* app);
extern void menuListaEjecucion(EstadoApp* app);
extern void menuConfigSO(EstadoApp* app);
extern void menuEmulacion(EstadoApp* app);
extern void menuReporteFinal(EstadoApp* app);
extern void printHeader();

int main() {
    srand(time(NULL));

    EstadoApp app;
    app.lista_procesos = NULL;
    app.cola_ejecucion = NULL;
    app.total_ejecucion = 0;
    app.secuencia_referencias = NULL;
    app.total_refs = 0;
    app.paginas_virtuales = NULL;   // NUEVO
    app.marcos_fisicos = NULL;       // NUEVO
    app.total_fallos = 0;
    app.total_hits = 0;

    app.config.configurado = false;
    app.config.reloj_puntero = 0;
    app.config.tamano_memoria = 0;
    app.config.cantidad_marcos = 0;
    app.config.tamano_swap = 0;
    strcpy(app.config.algoritmo_seleccionado, "FIFO");

    int opcion;
    do {
        printHeader();
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║                      MENÚ PRINCIPAL                          ║\n");
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║  1. Configuración de Procesos                                ║\n");
        printf("║  2. Lista de Ejecución (Planificador)                        ║\n");
        printf("║  3. Configuración del SO                                     ║\n");
        printf("║  4. Emular MMU                                               ║\n");
        printf("║  5. Reporte Final                                            ║\n");
        printf("║  0. Salir                                                    ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        printf("Seleccione módulo: ");

        if (scanf("%d", &opcion) != 1) {
            while (getchar() != '\n');
            continue;
        }

        switch (opcion) {
            case 1: menuConfigProcesos(&app); break;
            case 2: menuListaEjecucion(&app); break;
            case 3: menuConfigSO(&app); break;
            case 4: menuEmulacion(&app); break;
            case 5: menuReporteFinal(&app); break;
            case 0: printf("\n Cerrando emulador...\n"); break;
            default: printf(" Opción no válida.\n"); break;
        }
    } while (opcion != 0);

    // Liberar memoria
    liberarListaProcesos(app.lista_procesos);
    liberarListaPaginas(app.paginas_virtuales);
    liberarListaMarcos(app.marcos_fisicos);
    if (app.cola_ejecucion) free(app.cola_ejecucion);
    if (app.secuencia_referencias) free(app.secuencia_referencias);

    return 0;
}
