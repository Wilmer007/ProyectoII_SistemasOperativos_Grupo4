#include "estructuras.h"
#include "listas.h"

 //Cuando el equipo integre sus modulos, reemplazar los stubs
 //de abajo con las implementaciones reales.
 * ─────────────────────────────────────────────────────────── */
void menuConfigProcesos(Proceso **listaProcesos);
void menuListaEjecucion(Proceso **listaProcesos, Proceso **colaEjecucion);
void menuConfigSO(ConfigSO *config, Marco **listaMarcos, Pagina **listaPaginas,
                  Proceso *listaProcesos);
void menuEmularMMU(Proceso *colaEjecucion, Marco **listaMarcos,
                   Pagina **listaPaginas, ConfigSO *config);

 //Borrar cuando cada integrante entregue su modulo real.
void menuConfigProcesos(Proceso **lp) {
    (void)lp;
    printf("\n  [Modulo 2 pendiente - Integrante 2]\n");
    printf("  Presione Enter..."); getchar(); getchar();
}
void menuListaEjecucion(Proceso **lp, Proceso **ce) {
    (void)lp; (void)ce;
    printf("\n  [Modulo 2 pendiente - Integrante 2]\n");
    printf("  Presione Enter..."); getchar(); getchar();
}
void menuConfigSO(ConfigSO *c, Marco **lm, Pagina **lpag, Proceso *lp) {
    (void)c; (void)lm; (void)lpag; (void)lp;
    printf("\n  [Modulo 3 pendiente - Integrante 3]\n");
    printf("  Presione Enter..."); getchar(); getchar();
}
void menuEmularMMU(Proceso *ce, Marco **lm, Pagina **lpag, ConfigSO *c) {
    (void)ce; (void)lm; (void)lpag; (void)c;
    printf("\n  [Modulo 4 pendiente - Integrante 4]\n");
    printf("  Presione Enter..."); getchar(); getchar();
}



void limpiarPantalla(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void imprimirBienvenida(void) {
    printf("  +====================================================+\n");
    printf("  |    SISTEMA MMU -       ALGORITMOS DE PAGINACION   |\n");
    printf("  |                                                      |\n");
    printf("  |                                                      |\n");
    printf("  +====================================================+\n\n");
}

void imprimirEstadoSistema(Proceso *procs, ConfigSO *cfg,
                            Proceso *cola) {
    printf("  Estado del sistema:\n");
    printf("    Procesos registrados : %d\n", contarProcesos(procs));
    printf("    Cola de ejecucion    : %s\n",
           cola ? "definida" : "no definida");
    if (cfg->totalMarcos > 0) {
        printf("    Paginas virtuales    : %d\n", cfg->totalPaginas);
        printf("    Marcos fisicos       : %d\n", cfg->totalMarcos);
        printf("    Algoritmo MMU        : %s\n",
               algoritmoTexto(cfg->algoritmoMMU));
    } else {
        printf("    Configuracion SO     : pendiente\n");
    }
    printf("\n");
}

int mostrarMenuPrincipal(void) {
    int op = 0;
    printf("  +---------------------------------------------+\n");
    printf("  |              MENU PRINCIPAL                 |\n");
    printf("  +---------------------------------------------+\n");
    printf("  |  1. Configuracion de programas              |\n");
    printf("  |  2. Definicion de lista de ejecucion        |\n");
    printf("  |  3. Configuracion SO                        |\n");
    printf("  |  4. Emular MMU (algoritmos de paginacion)   |\n");
    printf("  |  5. Salir                                   |\n");
    printf("  +---------------------------------------------+\n");
    printf("\n  Seleccione una opcion [1-5]: ");
    scanf("%d", &op);
    return op;
}

int validarSistema(Proceso *cola, ConfigSO *cfg, Marco *marcos) {
    int ok = 1;
    if (!cola) {
        printf("\n  [!] Sin lista de ejecucion. Use la opcion 2.\n");
        ok = 0;
    }
    if (cfg->totalMarcos == 0) {
        printf("\n  [!] SO no configurado. Use la opcion 3.\n");
        ok = 0;
    }
    if (!marcos) {
        printf("\n  [!] Memoria fisica no inicializada. Use la opcion 3.\n");
        ok = 0;
    }
    return ok;
}


int main(void) {
    Proceso  *listaProcesos = NULL;
    Proceso  *colaEjecucion = NULL;
    Pagina   *listaPaginas  = NULL;
    Marco    *listaMarcos   = NULL;
    ConfigSO  configSO;
    iniciarConfigSO(&configSO);      

    int opcion = 0;
    int salir  = 0;

    limpiarPantalla();
    imprimirBienvenida();
    printf("  Presione Enter para continuar...");
    getchar();

    while (!salir) {
        limpiarPantalla();
        imprimirBienvenida();
        imprimirEstadoSistema(listaProcesos, &configSO, colaEjecucion);
        opcion = mostrarMenuPrincipal();

        switch (opcion) {

            case 1:
                limpiarPantalla();
                printf("\n  === CONFIGURACION DE PROGRAMAS ===\n\n");
                menuConfigProcesos(&listaProcesos);
                break;

            case 2:
                limpiarPantalla();
                printf("\n  === LISTA DE EJECUCION ===\n\n");
                if (!listaProcesos) {
                    printf("  [!] Primero registre procesos (opcion 1).\n\n");
                    printf("  Presione Enter..."); getchar(); getchar();
                } else {
                    menuListaEjecucion(&listaProcesos, &colaEjecucion);
                }
                break;

            case 3:
                limpiarPantalla();
                printf("\n  === CONFIGURACION DEL SISTEMA OPERATIVO ===\n\n");
                menuConfigSO(&configSO, &listaMarcos,
                             &listaPaginas, listaProcesos);
                break;

            case 4:
                limpiarPantalla();
                printf("\n  === EMULACION MMU ===\n\n");
                if (validarSistema(colaEjecucion, &configSO, listaMarcos))
                    menuEmularMMU(colaEjecucion, &listaMarcos,
                                  &listaPaginas, &configSO);
                else {
                    printf("\n  Presione Enter..."); getchar(); getchar();
                }
                break;

            case 5:
                salir = 1;
                break;

            default:
                printf("\n  [!] Opcion invalida. Ingrese 1-5.\n");
                printf("  Presione Enter..."); getchar(); getchar();
                break;
        }
    }

    liberarListaProcesos(&listaProcesos);
    liberarListaProcesos(&colaEjecucion);
    liberarListaPaginas(&listaPaginas);
    liberarListaMarcos(&listaMarcos);

    limpiarPantalla();
    printf("\n  Proyecto MMU — UNITEC-CEUTEC. Hasta luego.\n\n");
    return 0;
}

