#include "listas.h"



Proceso* crearProceso(int id, const char *nombre, int prioridad,
                      int tiempoCPU, int memoriaReq) {
    Proceso *p = (Proceso*) malloc(sizeof(Proceso));
    if (!p) { fprintf(stderr, "[ERROR] crearProceso: malloc fallo\n"); return NULL; }

    p->id             = id;
    strncpy(p->nombre, nombre, MAX_NOMBRE - 1);
    p->nombre[MAX_NOMBRE - 1] = '\0';
    p->estado         = NUEVO;
    p->prioridad      = prioridad;
    p->tiempoCPU      = tiempoCPU;
    p->tiempoRestante = tiempoCPU;
    p->memoriaReq     = memoriaReq;
    p->activo         = 1;
    p->finalizado     = 0;
    p->siguiente      = NULL;
    return p;
}

void insertarProceso(Proceso **cabeza, Proceso *nuevo) {
    if (!nuevo) return;
    if (!*cabeza) { *cabeza = nuevo; return; }
    Proceso *t = *cabeza;
    while (t->siguiente) t = t->siguiente;
    t->siguiente = nuevo;
}

Proceso* buscarProcesoPorId(Proceso *cabeza, int id) {
    for (Proceso *t = cabeza; t; t = t->siguiente)
        if (t->id == id) return t;
    return NULL;
}

void eliminarProceso(Proceso **cabeza, int id) {
    if (!*cabeza) { printf("[AVISO] Lista de procesos vacia.\n"); return; }
    Proceso *actual = *cabeza, *anterior = NULL;
    while (actual && actual->id != id) { anterior = actual; actual = actual->siguiente; }
    if (!actual) { printf("[AVISO] Proceso id=%d no encontrado.\n", id); return; }
    if (!anterior) *cabeza = actual->siguiente;
    else           anterior->siguiente = actual->siguiente;
    free(actual);
    printf("[OK] Proceso id=%d eliminado.\n", id);
}

void listarProcesos(Proceso *cabeza) {
    if (!cabeza) { printf("  (No hay procesos registrados)\n"); return; }
    printf("\n  %-4s %-20s %-12s %-5s %-5s %-5s %-8s\n",
           "ID", "Nombre", "Estado", "Prio", "CPU", "Mem", "Estado");
    printf("  %s\n", "-----------------------------------------------------------");
    for (Proceso *t = cabeza; t; t = t->siguiente)
        printf("  %-4d %-20s %-12s %-5d %-5d %-5d %-8s\n",
               t->id, t->nombre, estadoTexto(t->estado),
               t->prioridad, t->tiempoCPU, t->memoriaReq,
               t->finalizado ? "LISTO" : "PENDIENTE");
    printf("\n");
}

int contarProcesos(Proceso *cabeza) {
    int n = 0;
    for (Proceso *t = cabeza; t; t = t->siguiente) n++;
    return n;
}

void liberarListaProcesos(Proceso **cabeza) {
    while (*cabeza) { Proceso *t = *cabeza; *cabeza = t->siguiente; free(t); }
}



Pagina* crearPagina(int numPagina, int idProceso) {
    Pagina *p = (Pagina*) malloc(sizeof(Pagina));
    if (!p) { fprintf(stderr, "[ERROR] crearPagina: malloc fallo\n"); return NULL; }
    p->numPagina  = numPagina;
    p->idProceso  = idProceso;
    p->enMemoria  = 0;
    p->numMarco   = PAGINA_EN_DISCO;
    p->ultimoUso  = 0;
    p->enConjunto = 0;
    p->siguiente  = NULL;
    return p;
}

void insertarPagina(Pagina **cabeza, Pagina *nueva) {
    if (!nueva) return;
    if (!*cabeza) { *cabeza = nueva; return; }
    Pagina *t = *cabeza;
    while (t->siguiente) t = t->siguiente;
    t->siguiente = nueva;
}

Pagina* buscarPagina(Pagina *cabeza, int numPagina, int idProceso) {
    for (Pagina *t = cabeza; t; t = t->siguiente)
        if (t->numPagina == numPagina && t->idProceso == idProceso) return t;
    return NULL;
}

void listarPaginas(Pagina *cabeza) {
    if (!cabeza) { printf("  (No hay paginas registradas)\n"); return; }
    printf("\n  %-8s %-9s %-12s %-8s\n", "Pagina", "Proceso", "En Memoria", "Marco");
    printf("  %s\n", "--------------------------------------");
    for (Pagina *t = cabeza; t; t = t->siguiente) {
        printf("  %-8d %-9d %-12s ",
               t->numPagina, t->idProceso,
               t->enMemoria ? "Si (RAM)" : "No (disco)");
        if (t->numMarco == PAGINA_EN_DISCO) printf("---\n");
        else                                printf("Marco %d\n", t->numMarco);
    }
    printf("\n");
}

void liberarListaPaginas(Pagina **cabeza) {
    while (*cabeza) { Pagina *t = *cabeza; *cabeza = t->siguiente; free(t); }
}


Marco* crearMarco(int numMarco) {
    Marco *m = (Marco*) malloc(sizeof(Marco));
    if (!m) { fprintf(stderr, "[ERROR] crearMarco: malloc fallo\n"); return NULL; }
    m->numMarco      = numMarco;
    m->ocupado       = 0;
    m->numPagina     = MARCO_LIBRE;
    m->idProceso     = MARCO_LIBRE;
    m->tiempoCarga   = 0;
    m->bitReferencia = 0;
    m->bitModificado = 0;
    m->enConjunto    = 0;
    m->siguiente     = NULL;
    return m;
}

void insertarMarco(Marco **cabeza, Marco *nuevo) {
    if (!nuevo) return;
    if (!*cabeza) { *cabeza = nuevo; return; }
    Marco *t = *cabeza;
    while (t->siguiente) t = t->siguiente;
    t->siguiente = nuevo;
}

Marco* buscarMarcoLibre(Marco *cabeza) {
    for (Marco *t = cabeza; t; t = t->siguiente)
        if (!t->ocupado) return t;
    return NULL;
}

Marco* buscarMarcoPorNumero(Marco *cabeza, int numMarco) {
    for (Marco *t = cabeza; t; t = t->siguiente)
        if (t->numMarco == numMarco) return t;
    return NULL;
}

Marco* buscarMarcoPorPagina(Marco *cabeza, int numPagina, int idProceso) {
    for (Marco *t = cabeza; t; t = t->siguiente)
        if (t->ocupado && t->numPagina == numPagina && t->idProceso == idProceso)
            return t;
    return NULL;
}

int contarMarcosOcupados(Marco *cabeza) {
    int n = 0;
    for (Marco *t = cabeza; t; t = t->siguiente)
        if (t->ocupado) n++;
    return n;
}

void listarMarcos(Marco *cabeza) {
    if (!cabeza) { printf("  (No hay marcos registrados)\n"); return; }
    printf("\n  %-8s %-10s %-8s %-8s\n",
           "Marco", "Estado", "Pagina", "Proceso");
    printf("  %s\n", "--------------------------------------");
    for (Marco *t = cabeza; t; t = t->siguiente) {
        if (t->ocupado)
            printf("  Marco %-2d  OCUPADO   Pag %-3d Proc %d\n",
                   t->numMarco, t->numPagina, t->idProceso);
        else
            printf("  Marco %-2d  LIBRE     ---     ---\n", t->numMarco);
    }
    printf("\n");
}

void liberarListaMarcos(Marco **cabeza) {
    while (*cabeza) { Marco *t = *cabeza; *cabeza = t->siguiente; free(t); }
}




Marco* buscarMarcoNRU(Marco *cabeza) {
    Marco *clases[4] = {NULL, NULL, NULL, NULL};

    for (Marco *t = cabeza; t; t = t->siguiente) {
        if (!t->ocupado) continue;
        int clase = (t->bitReferencia * 2) + t->bitModificado;
        if (!clases[clase]) clases[clase] = t;
    }

    for (int c = 0; c < 4; c++)
        if (clases[c]) return clases[c];

    return cabeza; /* fallback: nunca deberia llegar aqui */
}


void limpiarBitsReferencia(Marco *cabeza) {
    for (Marco *t = cabeza; t; t = t->siguiente)
        t->bitReferencia = 0;
}


Marco* buscarMarcoReloj(Marco *cabeza, ConfigSO *cfg) {
    /* Convertir la lista a un arreglo temporal para acceso circular */
    Marco *arr[MAX_MARCOS];
    int n = 0;
    for (Marco *t = cabeza; t && n < MAX_MARCOS; t = t->siguiente)
        if (t->ocupado) arr[n++] = t;

    if (n == 0) return NULL;

    /* Asegurarse de que el puntero este dentro del rango */
    cfg->punteroReloj = cfg->punteroReloj % n;

    /* Dar hasta 2*n oportunidades (dos vueltas completas) */
    for (int i = 0; i < 2 * n; i++) {
        Marco *m = arr[cfg->punteroReloj % n];
        if (m->bitReferencia == 0) {
            cfg->punteroReloj = (cfg->punteroReloj + 1) % n;
            return m;
        }
        m->bitReferencia = 0;                     /* segunda oportunidad */
        cfg->punteroReloj = (cfg->punteroReloj + 1) % n;
    }

    /* Si todos tenian R=1 (ya se limpiaron), retornar el primero */
    return arr[cfg->punteroReloj % n];
}


Marco* buscarMarcoFueraVentana(Marco *cabeza, Pagina *paginas,
                                int reloj, int ventana) {
    Marco *mayorEdad     = NULL;
    int    edadMax       = -1;

    for (Marco *t = cabeza; t; t = t->siguiente) {
        if (!t->ocupado) continue;

        Pagina *p = buscarPagina(paginas, t->numPagina, t->idProceso);
        int    ultimoAcceso = p ? p->ultimoUso : 0;
        int    edad         = reloj - ultimoAcceso;

        if (edad > ventana) return t;

        if (edad > edadMax) { edadMax = edad; mayorEdad = t; }
    }

    return mayorEdad; /* fallback si todos estan en la ventana */
}


void actualizarConjuntoTrabajo(Marco *cabeza, Pagina *paginas,
                                int reloj, int ventana) {
    for (Marco *t = cabeza; t; t = t->siguiente) {
        if (!t->ocupado) { t->enConjunto = 0; continue; }
        Pagina *p = buscarPagina(paginas, t->numPagina, t->idProceso);
        int edad  = p ? (reloj - p->ultimoUso) : (ventana + 1);
        t->enConjunto = (edad <= ventana) ? 1 : 0;
        if (p) p->enConjunto = t->enConjunto;
    }
}

