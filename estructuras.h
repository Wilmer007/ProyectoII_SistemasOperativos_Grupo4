#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

/**
 *  Todos los modulos deben incluir este archivo con:
 *      #include "estructuras.h"
 *  NO modificar los nombres de campos sin avisar al equipo.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


 // CONSTANTES GLOBALES DEL SISTEMA

#define MAX_NOMBRE       50
#define MAX_PAGINAS      64    // paginas virtuales maximas           
#define MAX_MARCOS       32    //marcos fisicos maximos              
#define MAX_REFS         256   // max referencias para OPT            
#define MARCO_LIBRE      -1    // marco sin pagina asignada           
#define PAGINA_EN_DISCO  -1    // pagina no cargada en RAM            



typedef enum {
    NUEVO      = 0,
    LISTO      = 1,
    EJECUTANDO = 2,
    BLOQUEADO  = 3,
    FINALIZADO = 4
} EstadoProceso;

static inline const char* estadoTexto(EstadoProceso e) {
    switch (e) {
        case NUEVO:      return "NUEVO";
        case LISTO:      return "LISTO";
        case EJECUTANDO: return "EJECUTANDO";
        case BLOQUEADO:  return "BLOQUEADO";
        case FINALIZADO: return "FINALIZADO";
        default:         return "DESCONOCIDO";
    }
}


typedef enum {
    OPT   = 0,   /* Optimo                      */
    NRU   = 1,   /* Not Recently Used           */
    FIFO  = 2,   /* First In First Out          */
    SC    = 3,   /* Segunda Oportunidad         */
    RELOJ = 4,   /* Reloj (Clock)               */
    LRU   = 5   /* Least Recently Used         */
} AlgoritmoMMU;

static inline const char* algoritmoTexto(AlgoritmoMMU a) {
    switch (a) {
        case OPT:   return "OPT   (Optimo)";
        case NRU:   return "NRU   (Not Recently Used)";
        case FIFO:  return "FIFO  (First In First Out)";
        case SC:    return "SC    (Segunda Oportunidad)";
        case RELOJ: return "RELOJ (Reloj / Clock)";
        case LRU:   return "LRU   (Least Recently Used)";
        default:    return "DESCONOCIDO";
    }
}

/*
 *  STRUCT: Proceso  (nodo de lista enlazada)
 *  Usado por: Modulo 2 (CRUD y planificador)
 *             Modulo 4 (emulacion MMU)
 */
typedef struct Proceso {
    int            id;                 /* identificador unico              */
    char           nombre[MAX_NOMBRE]; /* nombre del programa              */
    EstadoProceso  estado;             /* estado actual                    */
    int            prioridad;         /* 1 = mayor prioridad              */
    int            tiempoCPU;         /* total de rafagas necesarias      */
    int            tiempoRestante;    /* rafagas pendientes (Round Robin) */
    int            memoriaReq;        /* paginas que necesita             */
    int            activo;            /* 1 = activo, 0 = inactivo         */
    int            finalizado;        /* 1 = termino                      */
    struct Proceso *siguiente;        /* apuntador siguiente nodo         */
} Proceso;



typedef struct Pagina {
    int  numPagina;           /* indice logico (0, 1, 2 ...)              */
    int  idProceso;           /* proceso al que pertenece                 */
    int  enMemoria;           /* 1 = en RAM,  0 = en disco                */
    int  numMarco;            /* marco asignado o PAGINA_EN_DISCO         */

    /* LRU, Conjunto de Trabajo */
    int  ultimoUso;           /* timestamp del ultimo acceso              */


    struct Pagina *siguiente; /* apuntador siguiente nodo                 */
} Pagina;



typedef struct Marco {
    int  numMarco;            /* indice fisico (0, 1, 2 ...)              */
    int  ocupado;             /* 1 = en uso,  0 = libre                   */
    int  numPagina;           /* pagina virtual contenida                 */
    int  idProceso;           /* proceso dueno de esa pagina              */

    /* FIFO, Segunda Oportunidad, Reloj */
    int  tiempoCarga;         /* timestamp cuando se cargo la pagina      */

    /* NRU, Segunda Oportunidad, Reloj */
    int  bitReferencia;       /* 1 = fue accedida recientemente           */

    /* NRU */
    int  bitModificado;       /* 1 = fue escrita (modificada)             */


    struct Marco *siguiente;  /* apuntador siguiente nodo                 */
} Marco;


typedef struct {
    int totalPaginas;         /* paginas virtuales totales                */
    int totalMarcos;          /* marcos fisicos totales                   */
    AlgoritmoMMU algoritmoMMU;/* algoritmo elegido (enum de 7 valores)    */
    int tamanioPagina;        /* tamanio de pagina en KB                  */

    int punteroReloj;


} ConfigSO;

/*
 *  Inicializador de ConfigSO con valores por defecto
 *  Llamar en main() antes de usar ConfigSO.
 *  */
static inline void iniciarConfigSO(ConfigSO *cfg) {
    cfg->totalPaginas  = 0;
    cfg->totalMarcos   = 0;
    cfg->algoritmoMMU  = FIFO;
    cfg->tamanioPagina = 4;
    cfg->punteroReloj  = 0;
}

#endif /* ESTRUCTURAS_H */

