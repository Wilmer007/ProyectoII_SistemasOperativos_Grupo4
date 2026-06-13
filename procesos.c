#include "procesos.h"

static void limpiarBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void mostrarDetalleProceso(Proceso *p) {
    if (!p) return;
    printf("  +----------------------------------+\n");
    printf("  | ID         : %-20d |\n", p->id);
    printf("  | Nombre     : %-20s |\n", p->nombre);
    printf("  | Estado     : %-20s |\n", estadoTexto(p->estado));
    printf("  | Prioridad  : %-20d |\n", p->prioridad);
    printf("  | Tiempo CPU : %-20d |\n", p->tiempoCPU);
    printf("  | Memoria    : %-20d |\n", p->memoriaReq);
    printf("  | Activo     : %-20s |\n", p->activo ? "Si" : "No");
    printf("  | Finalizado : %-20s |\n", p->finalizado ? "Si" : "No");
    printf("  +----------------------------------+\n");
}

void agregarProceso(Proceso **listaProcesos) {
    int   id, prioridad, tiempoCPU, memoriaReq;
    char  nombre[MAX_NOMBRE];

    printf("\n  --- Nuevo proceso ---\n");

    do {
        printf("  ID del proceso (> 0)    : ");
        scanf("%d", &id);
        limpiarBuffer();
        if (id <= 0) {
            printf("  [!] El ID debe ser mayor que 0.\n");
            continue;
        }
        if (buscarProcesoPorId(*listaProcesos, id)) {
            printf("  [!] Ya existe un proceso con ID=%d.\n", id);
            id = -1;
        }
    } while (id <= 0);

    printf("  Nombre del proceso      : ");
    fgets(nombre, MAX_NOMBRE, stdin);
    nombre[strcspn(nombre, "\n")] = '\0';
    if (strlen(nombre) == 0) {
        strcpy(nombre, "Proceso");
    }

    do {
        printf("  Prioridad (1=alta, N=baja): ");
        scanf("%d", &prioridad);
        limpiarBuffer();
        if (prioridad <= 0)
            printf("  [!] La prioridad debe ser mayor que 0.\n");
    } while (prioridad <= 0);

    do {
        printf("  Tiempo de CPU (rafagas) : ");
        scanf("%d", &tiempoCPU);
        limpiarBuffer();
        if (tiempoCPU <= 0)
            printf("  [!] El tiempo de CPU debe ser mayor que 0.\n");
    } while (tiempoCPU <= 0);

    do {
        printf("  Paginas requeridas      : ");
        scanf("%d", &memoriaReq);
        limpiarBuffer();
        if (memoriaReq <= 0)
            printf("  [!] Debe requerir al menos 1 pagina.\n");
        if (memoriaReq > MAX_PAGINAS)
            printf("  [!] No puede superar %d paginas.\n", MAX_PAGINAS);
    } while (memoriaReq <= 0 || memoriaReq > MAX_PAGINAS);

    Proceso *nuevo = crearProceso(id, nombre, prioridad, tiempoCPU, memoriaReq);
    if (!nuevo) {
        printf("  [ERROR] No se pudo crear el proceso.\n");
        return;
    }
    nuevo->estado = LISTO;
    insertarProceso(listaProcesos, nuevo);
    printf("\n  [OK] Proceso '%s' (ID=%d) registrado.\n\n", nombre, id);
}

void editarProceso(Proceso *listaProcesos) {
    if (!listaProcesos) {
        printf("  [!] No hay procesos registrados.\n");
        return;
    }

    int id;
    printf("\n  ID del proceso a editar: ");
    scanf("%d", &id);
    limpiarBuffer();

    Proceso *p = buscarProcesoPorId(listaProcesos, id);
    if (!p) {
        printf("  [!] Proceso ID=%d no encontrado.\n", id);
        return;
    }

    printf("\n  Proceso actual:\n");
    mostrarDetalleProceso(p);
    printf("\n  (Deje en blanco / ingrese 0 para no cambiar el campo)\n\n");

    char nuevo_nombre[MAX_NOMBRE];
    printf("  Nuevo nombre [%s]: ", p->nombre);
    fgets(nuevo_nombre, MAX_NOMBRE, stdin);
    nuevo_nombre[strcspn(nuevo_nombre, "\n")] = '\0';
    if (strlen(nuevo_nombre) > 0)
        strncpy(p->nombre, nuevo_nombre, MAX_NOMBRE - 1);

    int v;
    printf("  Nueva prioridad [%d]: ", p->prioridad);
    scanf("%d", &v);
    limpiarBuffer();
    if (v > 0) p->prioridad = v;

    printf("  Nuevo tiempo CPU [%d]: ", p->tiempoCPU);
    scanf("%d", &v);
    limpiarBuffer();
    if (v > 0) {
        p->tiempoCPU = v;
        p->tiempoRestante = v;
    }

    printf("  Nuevas paginas requeridas [%d]: ", p->memoriaReq);
    scanf("%d", &v);
    limpiarBuffer();
    if (v > 0 && v <= MAX_PAGINAS) p->memoriaReq = v;

    printf("\n  [OK] Proceso ID=%d actualizado.\n\n", id);
    mostrarDetalleProceso(p);
}

void menuConfigProcesos(Proceso **listaProcesos) {
    int op = 0;
    int salir = 0;

    while (!salir) {
        printf("  Procesos registrados: %d\n\n", contarProcesos(*listaProcesos));
        printf("  +----------------------------------+\n");
        printf("  |   CONFIGURACION DE PROCESOS      |\n");
        printf("  +----------------------------------+\n");
        printf("  |  1. Agregar proceso              |\n");
        printf("  |  2. Listar procesos              |\n");
        printf("  |  3. Ver detalle de un proceso    |\n");
        printf("  |  4. Editar proceso               |\n");
        printf("  |  5. Eliminar proceso             |\n");
        printf("  |  6. Volver al menu principal     |\n");
        printf("  +----------------------------------+\n");
        printf("\n  Opcion: ");
        scanf("%d", &op);
        limpiarBuffer();
        printf("\n");

        switch (op) {
            case 1:
                agregarProceso(listaProcesos);
                break;

            case 2:
                listarProcesos(*listaProcesos);
                printf("  Presione Enter...");
                getchar();
                break;

            case 3: {
                if (!*listaProcesos) {
                    printf("  [!] No hay procesos registrados.\n\n");
                    break;
                }
                int id;
                printf("  ID del proceso: ");
                scanf("%d", &id);
                limpiarBuffer();
                Proceso *p = buscarProcesoPorId(*listaProcesos, id);
                if (p) mostrarDetalleProceso(p);
                else   printf("  [!] ID=%d no encontrado.\n", id);
                printf("\n  Presione Enter...");
                getchar();
                break;
            }

            case 4:
                editarProceso(*listaProcesos);
                printf("  Presione Enter...");
                getchar();
                break;

            case 5: {
                if (!*listaProcesos) {
                    printf("  [!] No hay procesos registrados.\n\n");
                    break;
                }
                int id;
                printf("  ID del proceso a eliminar: ");
                scanf("%d", &id);
                limpiarBuffer();
                eliminarProceso(listaProcesos, id);
                printf("  Presione Enter...");
                getchar();
                break;
            }

            case 6:
                salir = 1;
                break;

            default:
                printf("  [!] Opcion invalida. Ingrese 1-6.\n\n");
                break;
        }
    }
}