#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdbool.h>

// Estados de un proceso
typedef enum {
    NUEVO,
    LISTO,
    EJECUTANDO,
    BLOQUEADO,
    FINALIZADO
} EstadoProceso;

// Nodo de la lista enlazada de procesos
typedef struct Proceso {
    int id;
    char nombre[50];
    EstadoProceso estado;
    int prioridad;
    float tiempoCPU;
    float tiempoRestante;
    int memoriaReq; // Cantidad de páginas que requiere
    bool activo;
    bool finalizado;
    int pagina_inicio;  // NUEVO: primera página virtual asignada
    struct Proceso* siguiente;
} Proceso;

// Página de memoria virtual
typedef struct Pagina {
    int numero_pagina;
    int id_proceso;         // -1 si está libre
    int marco_asignado;     // NUEVO: -1 si no está en RAM
    int bit_referencia;     // 0 o 1 (usada por algoritmos)
    int bit_modificacion;   // 0 o 1 (Dirty bit)
    int tiempo_uso;         // Último tick de acceso
    int tiempo_carga;       // Tick en que se cargó a RAM
    bool en_disco;          // NUEVO: si está en swap
    struct Pagina* siguiente;
} Pagina;

// Marco de memoria física
typedef struct Marco {
    int numero_marco;
    int id_pagina;          // -1 si está libre
    int id_proceso;         // -1 si está libre
    int tiempo_carga;       // Tick en que se cargó
    int ultimo_uso;         // Tick del último acceso
    int bit_referencia;     // Para algoritmos de segunda oportunidad / reloj
    int bit_modificacion;   // Para NRU
    struct Marco* siguiente;
} Marco;

// Configuración del Sistema Operativo
typedef struct {
    char algoritmo_seleccionado[20];
    int cantidad_marcos;
    int tamano_memoria;         // En páginas virtuales
    int tamano_swap;            // NUEVO: espacio en disco (páginas)
    int reloj_puntero;          // Índice del marco para algoritmo de Reloj
    int ventana_trabajo;        // Para algoritmo de Conjunto de Trabajo
    int* referencias_futuras;   // Secuencia de páginas para algoritmo OPT
    int total_referencias;
    bool configurado;
} ConfigSO;

// Estado global de la aplicación
typedef struct {
    Proceso* lista_procesos;
    Proceso** cola_ejecucion;   // Array de punteros a procesos
    int total_ejecucion;
    ConfigSO config;
    int* secuencia_referencias;
    int total_refs;

    // NUEVOS: Tabla de páginas y marcos
    Pagina* paginas_virtuales;   // Lista de todas las páginas virtuales
    Marco* marcos_fisicos;       // Lista de todos los marcos físicos

    // Para estadísticas
    int total_fallos;
    int total_hits;
} EstadoApp;

#endif // ESTRUCTURAS_H