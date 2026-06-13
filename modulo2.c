#include "modulo2.h"

static void limpiar() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void limpiarBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void pausar() {
    printf("\n  Presione Enter para continuar...");
    limpiarBuffer();
}

static int idExiste(Proceso *cabeza, int id) {
    for (Proceso *t = cabeza; t; t = t->siguiente)
        if (t->id == id) return 1;
    return 0;
}

static void agregarProceso(Proceso **lista) {
    int id, prioridad, tiempoCPU, memoria;
    char nombre[MAX_NOMBRE];

    printf("\n  === AGREGAR PROCESO ===\n\n");

    printf("  ID: ");
    scanf("%d", &id);
    limpiarBuffer();

    if (idExiste(*lista, id)) {
        printf("\n  [!] Ya existe un proceso con ese ID.\n");
        pausar();
        return;
    }

    printf("  Nombre: ");
    fgets(nombre, MAX_NOMBRE, stdin);
    nombre[strcspn(nombre, "\n")] = '\0';

    if (strlen(nombre) == 0) {
        printf("\n  [!] El nombre no puede estar vacio.\n");
        pausar();
        return;
    }

    printf("  Prioridad (1 = mayor): ");
    scanf("%d", &prioridad);
    limpiarBuffer();

    printf("  Tiempo CPU (rafagas): ");
    scanf("%d", &tiempoCPU);
    limpiarBuffer();

    printf("  Memoria requerida (paginas): ");
    scanf("%d", &memoria);
    limpiarBuffer();

    if (memoria < 1) {
        printf("\n  [!] La memoria debe ser al menos 1 pagina.\n");
        pausar();
        return;
    }

    Proceso *nuevo = crearProceso(id, nombre, prioridad, tiempoCPU, memoria);
    insertarProceso(lista, nuevo);
    printf("\n  [OK] Proceso '%s' agregado.\n", nombre);
    pausar();
}

static void editarProceso(Proceso *lista) {
    int id;
    printf("\n  === EDITAR PROCESO ===\n\n");
    printf("  ID del proceso a editar: ");
    scanf("%d", &id);
    limpiarBuffer();

    Proceso *p = buscarProcesoPorId(lista, id);
    if (!p) {
        printf("\n  [!] Proceso con ID %d no encontrado.\n", id);
        pausar();
        return;
    }

    char nombre[MAX_NOMBRE];
    int prioridad, tiempoCPU, memoria;

    printf("  Nuevo nombre [actual: %s]: ", p->nombre);
    fgets(nombre, MAX_NOMBRE, stdin);
    nombre[strcspn(nombre, "\n")] = '\0';
    if (strlen(nombre) > 0)
        strncpy(p->nombre, nombre, MAX_NOMBRE - 1);

    printf("  Nueva prioridad [actual: %d]: ", p->prioridad);
    scanf("%d", &prioridad);
    limpiarBuffer();
    p->prioridad = prioridad;

    printf("  Nuevo tiempo CPU [actual: %d]: ", p->tiempoCPU);
    scanf("%d", &tiempoCPU);
    limpiarBuffer();
    p->tiempoCPU      = tiempoCPU;
    p->tiempoRestante = tiempoCPU;

    printf("  Nueva memoria (paginas) [actual: %d]: ", p->memoriaReq);
    scanf("%d", &memoria);
    limpiarBuffer();
    if (memoria >= 1) p->memoriaReq = memoria;

    printf("\n  [OK] Proceso ID %d actualizado.\n", id);
    pausar();
}

void menuConfigProcesos(Proceso **listaProcesos) {
    int op = 0;
    while (op != 5) {
        limpiar();
        printf("\n  === CONFIGURACION DE PROCESOS ===\n\n");
        listarProcesos(*listaProcesos);
        printf("  1. Agregar proceso\n");
        printf("  2. Editar proceso\n");
        printf("  3. Eliminar proceso\n");
        printf("  4. Listar procesos\n");
        printf("  5. Volver al menu principal\n");
        printf("\n  Opcion: ");
        scanf("%d", &op);
        limpiarBuffer();

        switch (op) {
            case 1: limpiar(); agregarProceso(listaProcesos); break;
            case 2: limpiar(); editarProceso(*listaProcesos);  break;
            case 3: {
                limpiar();
                int id;
                printf("\n  ID del proceso a eliminar: ");
                scanf("%d", &id);
                limpiarBuffer();
                eliminarProceso(listaProcesos, id);
                pausar();
                break;
            }
            case 4: limpiar(); listarProcesos(*listaProcesos); pausar(); break;
            case 5: break;
            default:
                printf("\n  [!] Opcion invalida.\n");
                pausar();
        }
    }
}

static void copiarProceso(Proceso *dest, Proceso *src) {
    dest->id             = src->id;
    strncpy(dest->nombre, src->nombre, MAX_NOMBRE - 1);
    dest->estado         = src->estado;
    dest->prioridad      = src->prioridad;
    dest->tiempoCPU      = src->tiempoCPU;
    dest->tiempoRestante = src->tiempoCPU;
    dest->memoriaReq     = src->memoriaReq;
    dest->activo         = src->activo;
    dest->finalizado     = src->finalizado;
    dest->siguiente      = NULL;
}

static void fcfs(Proceso *lista, Proceso **cola) {
    liberarListaProcesos(cola);
    for (Proceso *t = lista; t; t = t->siguiente) {
        Proceso *copia = (Proceso*)malloc(sizeof(Proceso));
        copiarProceso(copia, t);
        insertarProceso(cola, copia);
    }
    printf("\n  [OK] Cola FCFS generada.\n");
}

static void sjf(Proceso *lista, Proceso **cola) {
    liberarListaProcesos(cola);
    int total = contarProcesos(lista);
    if (total == 0) return;

    Proceso **arr = (Proceso**)malloc(total * sizeof(Proceso*));
    int i = 0;
    for (Proceso *t = lista; t; t = t->siguiente)
        arr[i++] = t;

    for (int a = 0; a < total - 1; a++)
        for (int b = a + 1; b < total; b++)
            if (arr[b]->tiempoCPU < arr[a]->tiempoCPU) {
                Proceso *tmp = arr[a]; arr[a] = arr[b]; arr[b] = tmp;
            }

    for (i = 0; i < total; i++) {
        Proceso *copia = (Proceso*)malloc(sizeof(Proceso));
        copiarProceso(copia, arr[i]);
        insertarProceso(cola, copia);
    }
    free(arr);
    printf("\n  [OK] Cola SJF generada.\n");
}

static void porPrioridad(Proceso *lista, Proceso **cola) {
    liberarListaProcesos(cola);
    int total = contarProcesos(lista);
    if (total == 0) return;

    Proceso **arr = (Proceso**)malloc(total * sizeof(Proceso*));
    int i = 0;
    for (Proceso *t = lista; t; t = t->siguiente)
        arr[i++] = t;

    for (int a = 0; a < total - 1; a++)
        for (int b = a + 1; b < total; b++)
            if (arr[b]->prioridad < arr[a]->prioridad) {
                Proceso *tmp = arr[a]; arr[a] = arr[b]; arr[b] = tmp;
            }

    for (i = 0; i < total; i++) {
        Proceso *copia = (Proceso*)malloc(sizeof(Proceso));
        copiarProceso(copia, arr[i]);
        insertarProceso(cola, copia);
    }
    free(arr);
    printf("\n  [OK] Cola por Prioridad generada.\n");
}

static void roundRobin(Proceso *lista, Proceso **cola, int quantum) {
    liberarListaProcesos(cola);
    int total = contarProcesos(lista);
    if (total == 0) return;

    int *restante = (int*)malloc(total * sizeof(int));
    Proceso **arr  = (Proceso**)malloc(total * sizeof(Proceso*));
    int i = 0;
    for (Proceso *t = lista; t; t = t->siguiente) {
        arr[i]      = t;
        restante[i] = t->tiempoCPU;
        i++;
    }

    int pendientes = total;
    while (pendientes > 0) {
        for (i = 0; i < total; i++) {
            if (restante[i] <= 0) continue;
            int turno = (restante[i] >= quantum) ? quantum : restante[i];
            restante[i] -= turno;
            Proceso *copia = (Proceso*)malloc(sizeof(Proceso));
            copiarProceso(copia, arr[i]);
            copia->tiempoCPU      = turno;
            copia->tiempoRestante = turno;
            insertarProceso(cola, copia);
            if (restante[i] <= 0) pendientes--;
        }
    }
    free(restante);
    free(arr);
    printf("\n  [OK] Cola Round Robin (quantum=%d) generada.\n", quantum);
}

static void mostrarCola(Proceso *cola) {
    if (!cola) { printf("\n  (Cola vacia)\n"); return; }
    printf("\n  %-6s %-4s %-20s %-8s %-5s\n",
           "Turno", "ID", "Nombre", "CPU", "Prio");
    printf("  %s\n", "------------------------------------------------");
    int turno = 1;
    for (Proceso *t = cola; t; t = t->siguiente)
        printf("  %-6d %-4d %-20s %-8d %-5d\n",
               turno++, t->id, t->nombre, t->tiempoCPU, t->prioridad);
    printf("\n");
}

void menuListaEjecucion(Proceso *listaProcesos, Proceso **colaEjecucion) {
    if (!listaProcesos) {
        printf("\n  [!] No hay procesos registrados. Use la opcion 1 primero.\n");
        pausar();
        return;
    }

    int op = 0;
    while (op != 6) {
        limpiar();
        printf("\n  === LISTA DE EJECUCION ===\n\n");
        printf("  Seleccione el algoritmo planificador:\n\n");
        printf("  1. FCFS (First Come First Served)\n");
        printf("  2. SJF  (Shortest Job First)\n");
        printf("  3. Por Prioridad\n");
        printf("  4. Round Robin\n");
        printf("  5. Ver cola actual\n");
        printf("  6. Volver al menu principal\n");
        printf("\n  Opcion: ");
        scanf("%d", &op);
        limpiarBuffer();

        switch (op) {
            case 1:
                limpiar();
                fcfs(listaProcesos, colaEjecucion);
                mostrarCola(*colaEjecucion);
                pausar();
                break;
            case 2:
                limpiar();
                sjf(listaProcesos, colaEjecucion);
                mostrarCola(*colaEjecucion);
                pausar();
                break;
            case 3:
                limpiar();
                porPrioridad(listaProcesos, colaEjecucion);
                mostrarCola(*colaEjecucion);
                pausar();
                break;
            case 4: {
                limpiar();
                int quantum;
                printf("\n  Quantum (rafagas por turno): ");
                scanf("%d", &quantum);
                limpiarBuffer();
                if (quantum < 1) {
                    printf("\n  [!] El quantum debe ser al menos 1.\n");
                    pausar();
                    break;
                }
                roundRobin(listaProcesos, colaEjecucion, quantum);
                mostrarCola(*colaEjecucion);
                pausar();
                break;
            }
            case 5:
                limpiar();
                mostrarCola(*colaEjecucion);
                pausar();
                break;
            case 6: break;
            default:
                printf("\n  [!] Opcion invalida.\n");
                pausar();
        }
    }
}
