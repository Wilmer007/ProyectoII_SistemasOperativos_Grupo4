#ifndef API_H
#define API_H

#include "estructuras.h"

#define MAX_MARCOS    64
#define MAX_PAGINAS   256
#define MAX_PROCS     64
#define MAX_SEQ       512

// --- Snapshots por tick (calcan mmuData.frames[] de reporte_mmu.html) ---
typedef struct {
    int  numero, idPagina, idProceso;
    char proceso[50];
    int  r, m, carga, ultimo;
    int  activo;                 // 1 si es el marco afectado en este tick
} MarcoSnap;

typedef struct {
    int  numero, idProceso, paginaVirtual, marco, r, m;
    char proceso[50], estado[10];  // estado: "RAM" / "VIRTUAL" / "SWAP"
    int  activa;                   // 1 si es la pagina referenciada en este tick
} PaginaSnap;

typedef struct {
    int  tick, total;
    int  procId; char procNombre[50];
    int  paginaVirtual, paginaGlobal, marcoAfectado;
    int  hit, paginaReemplazada;   // hit 0/1 ; paginaReemplazada -1 si ninguna
    int  hits, fallos;
    MarcoSnap  marcos[MAX_MARCOS];   int nMarcos;
    PaginaSnap paginas[MAX_PAGINAS]; int nPaginas;
} Frame;

// --- Config de entrada parseada del payload ---
typedef struct {
    char  algPlan[16];   // FCFS / RR / SJF / PRIORIDAD
    int   quantum;
    int   paginas;       // paginas virtuales
    int   marcos;        // marcos fisicos
    char  algMMU[16];    // FIFO / LRU / OPT / SC / CLOCK / MRU / LFU
    Proceso procs[MAX_PROCS];
    int   nProcs;
    int   seqManual[MAX_SEQ];  // secuencia manual de referencias (nSeqManual>0 = usarla)
    int   nSeqManual;
} ConfigEntrada;

// Estructura interna del simulador (un marco fisico durante la simulacion)
typedef struct {
    int idPagina, idProceso, cargado_en, ultimo_uso, frecuencia, bitR, bitM;
} MarcoSim;

// Parseo del payload delimitado por lineas. Devuelve 1 si OK, 0 si formato invalido.
int parsearPayload(const char* payload, ConfigEntrada* cfg);

// Valida reglas de negocio. Si invalida, escribe el motivo en errBuf y devuelve 0.
int validarConfig(const ConfigEntrada* cfg, char* errBuf, int errLen);

// Seleccion de victima de los 7 algoritmos de reemplazo.
int seleccionarVictima(const char* alg, MarcoSim* marcos, int n, int tick,
                       const int* seq, int totalSeq, int posActual, int* reloj);

// Corre planificador + MMU grabando cada tick. Devuelve nro de frames (<=maxFrames).
// 'cola' / 'nCola' devuelven la cola de ejecucion para el JSON.
int simularConRegistro(const ConfigEntrada* cfg, Frame* frames, int maxFrames,
                       Proceso* cola, int* nCola);

// Punto de entrada del endpoint POST /api/simular. Devuelve JSON malloc'd (liberar con free).
char* apiSimular(const char* payload);

// Punto de entrada del endpoint POST /api/comparar. Devuelve JSON malloc'd.
char* apiComparar(const char* payload);

#endif // API_H
