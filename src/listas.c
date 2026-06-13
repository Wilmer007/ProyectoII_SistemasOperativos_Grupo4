#include "listas.h"
#include <stdlib.h>
#include <string.h>

Proceso* crearProceso(int id, const char* nombre, int prioridad, float tiempoCPU, int memoriaReq) {
    Proceso* nuevo = (Proceso*)malloc(sizeof(Proceso));
    if (nuevo) {
        nuevo->id = id;
        strncpy(nuevo->nombre, nombre, sizeof(nuevo->nombre) - 1);
        nuevo->nombre[sizeof(nuevo->nombre) - 1] = '\0';
        nuevo->estado = LISTO;
        nuevo->prioridad = prioridad;
        nuevo->tiempoCPU = tiempoCPU;
        nuevo->tiempoRestante = tiempoCPU;
        nuevo->memoriaReq = memoriaReq;
        nuevo->activo = true;
        nuevo->finalizado = false;
        nuevo->siguiente = NULL;
    }
    return nuevo;
}

void insertarProceso(Proceso** cabeza, Proceso* nuevo) {
    if (*cabeza == NULL) {
        *cabeza = nuevo;
    } else {
        Proceso* actual = *cabeza;
        while (actual->siguiente != NULL) actual = actual->siguiente;
        actual->siguiente = nuevo;
    }
}

Proceso* buscarProcesoPorId(Proceso* cabeza, int id) {
    Proceso* actual = cabeza;
    while (actual != NULL) {
        if (actual->id == id) return actual;
        actual = actual->siguiente;
    }
    return NULL;
}

bool eliminarProceso(Proceso** cabeza, int id) {
    if (*cabeza == NULL) return false;
    Proceso* temp = *cabeza;
    if (temp->id == id) {
        *cabeza = temp->siguiente;
        free(temp);
        return true;
    }
    Proceso* anterior = temp;
    temp = temp->siguiente;
    while (temp != NULL) {
        if (temp->id == id) {
            anterior->siguiente = temp->siguiente;
            free(temp);
            return true;
        }
        anterior = temp;
        temp = temp->siguiente;
    }
    return false;
}

void liberarListaProcesos(Proceso* cabeza) {
    while (cabeza != NULL) {
        Proceso* temp = cabeza;
        cabeza = cabeza->siguiente;
        free(temp);
    }
}

int contarProcesos(Proceso* cabeza) {
    int n = 0;
    while (cabeza != NULL) { n++; cabeza = cabeza->siguiente; }
    return n;
}

Pagina* crearPagina(int numero, int id_proceso) {
    Pagina* nueva = (Pagina*)malloc(sizeof(Pagina));
    if (nueva) {
        nueva->numero_pagina = numero;
        nueva->id_proceso = id_proceso;
        nueva->bit_referencia = 0;
        nueva->bit_modificacion = 0;
        nueva->tiempo_uso = 0;
        nueva->siguiente = NULL;
    }
    return nueva;
}

void insertarPagina(Pagina** cabeza, Pagina* nueva) {
    if (*cabeza == NULL) {
        *cabeza = nueva;
    } else {
        Pagina* actual = *cabeza;
        while (actual->siguiente != NULL) actual = actual->siguiente;
        actual->siguiente = nueva;
    }
}

Pagina* buscarPagina(Pagina* cabeza, int numero) {
    Pagina* actual = cabeza;
    while (actual != NULL) {
        if (actual->numero_pagina == numero) return actual;
        actual = actual->siguiente;
    }
    return NULL;
}

void liberarListaPaginas(Pagina* cabeza) {
    while (cabeza != NULL) {
        Pagina* temp = cabeza;
        cabeza = cabeza->siguiente;
        free(temp);
    }
}

Marco* crearMarco(int numero) {
    Marco* nuevo = (Marco*)malloc(sizeof(Marco));
    if (nuevo) {
        nuevo->numero_marco = numero;
        nuevo->id_pagina = -1;
        nuevo->id_proceso = -1;
        nuevo->tiempo_carga = 0;
        nuevo->ultimo_uso = 0;
        nuevo->bit_referencia = 0;
        nuevo->bit_modificacion = 0;
        nuevo->siguiente = NULL;
    }
    return nuevo;
}

void insertarMarco(Marco** cabeza, Marco* nuevo) {
    if (*cabeza == NULL) {
        *cabeza = nuevo;
    } else {
        Marco* actual = *cabeza;
        while (actual->siguiente != NULL) actual = actual->siguiente;
        actual->siguiente = nuevo;
    }
}

Marco* buscarMarcoLibre(Marco* cabeza) {
    while (cabeza != NULL) {
        if (cabeza->id_pagina == -1) return cabeza;
        cabeza = cabeza->siguiente;
    }
    return NULL;
}

Marco* buscarMarcoPorNumero(Marco* cabeza, int numero) {
    while (cabeza != NULL) {
        if (cabeza->numero_marco == numero) return cabeza;
        cabeza = cabeza->siguiente;
    }
    return NULL;
}

void liberarListaMarcos(Marco* cabeza) {
    while (cabeza != NULL) {
        Marco* temp = cabeza;
        cabeza = cabeza->siguiente;
        free(temp);
    }
}
