#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

/* ========================================================================
   Task 2: parsearPayload
   ======================================================================== */

int parsearPayload(const char* payload, ConfigEntrada* cfg) {
    if (!payload || !cfg) return 0;
    memset(cfg, 0, sizeof(*cfg));
    cfg->quantum = 1;
    strcpy(cfg->algPlan, "FCFS");
    strcpy(cfg->algMMU, "FIFO");

    char* copia = strdup(payload);
    if (!copia) return 0;

    char* ctx = NULL;
    char* linea = strtok_r(copia, "\r\n", &ctx);
    while (linea) {
        if (strncmp(linea, "alg_plan=", 9) == 0) {
            strncpy(cfg->algPlan, linea + 9, sizeof(cfg->algPlan) - 1);
        } else if (strncmp(linea, "quantum=", 8) == 0) {
            cfg->quantum = atoi(linea + 8);
        } else if (strncmp(linea, "paginas=", 8) == 0) {
            cfg->paginas = atoi(linea + 8);
        } else if (strncmp(linea, "marcos=", 7) == 0) {
            cfg->marcos = atoi(linea + 7);
        } else if (strncmp(linea, "alg_mmu=", 8) == 0) {
            strncpy(cfg->algMMU, linea + 8, sizeof(cfg->algMMU) - 1);
        } else if (strncmp(linea, "proc=", 5) == 0 && cfg->nProcs < MAX_PROCS) {
            /* formato: id;nombre;prioridad;cpu;paginas */
            char* campos = linea + 5;
            int   id = 0, prio = 0, mem = 0;
            float cpu = 0.0f;
            char  nombre[50] = {0};
            char* c2 = NULL;
            char* tok = strtok_r(campos, ";", &c2);
            int idx = 0;
            while (tok) {
                switch (idx) {
                    case 0: id   = atoi(tok); break;
                    case 1: strncpy(nombre, tok, sizeof(nombre) - 1); break;
                    case 2: prio = atoi(tok); break;
                    case 3: cpu  = (float)atof(tok); break;
                    case 4: mem  = atoi(tok); break;
                    default: break;
                }
                idx++;
                tok = strtok_r(NULL, ";", &c2);
            }
            Proceso* p = &cfg->procs[cfg->nProcs];
            memset(p, 0, sizeof(*p));
            p->id = id;
            strncpy(p->nombre, nombre, sizeof(p->nombre) - 1);
            p->prioridad = prio;
            p->tiempoCPU = cpu;
            p->tiempoRestante = cpu;
            p->memoriaReq = mem;
            p->estado = LISTO;
            p->activo = true;
            p->siguiente = NULL;
            cfg->nProcs++;
        } else if (strncmp(linea, "refs=", 5) == 0) {
            /* secuencia manual de referencias: numeros separados por espacio/coma/; */
            char* lista = linea + 5;
            char* c2 = NULL;
            char* tok = strtok_r(lista, " ,;\t", &c2);
            while (tok && cfg->nSeqManual < MAX_SEQ) {
                cfg->seqManual[cfg->nSeqManual++] = atoi(tok);
                tok = strtok_r(NULL, " ,;\t", &c2);
            }
        }
        linea = strtok_r(NULL, "\r\n", &ctx);
    }
    free(copia);
    return 1;
}

/* ========================================================================
   Task 3: seleccionarVictima (6 algoritmos)
   ======================================================================== */

/* Proxima posicion futura en que se referencia 'pagina' (INT_MAX si nunca). Para OPT. */
static int proximaReferencia(const int* seq, int total, int desde, int pagina) {
    for (int i = desde + 1; i < total; i++)
        if (seq[i] == pagina) return i;
    return INT_MAX;
}

/* Devuelve el indice del marco victima dentro del arreglo marcos[0..n-1]. */
int seleccionarVictima(const char* alg, MarcoSim* marcos, int n, int tick,
                       const int* seq, int totalSeq, int posActual, int* reloj) {
    (void)tick;
    if (n <= 0) return 0;       /* defensa: validarConfig impide n=0 */
    if (strcmp(alg, "FIFO") == 0) {
        int v = 0, min = marcos[0].cargado_en;
        for (int i = 1; i < n; i++) if (marcos[i].cargado_en < min) { min = marcos[i].cargado_en; v = i; }
        return v;
    }
    if (strcmp(alg, "LRU") == 0) {
        int v = 0;
        for (int i = 1; i < n; i++) {
            if (marcos[i].ultimo_uso < marcos[v].ultimo_uso ||
                (marcos[i].ultimo_uso == marcos[v].ultimo_uso &&
                 marcos[i].cargado_en < marcos[v].cargado_en)) {
                v = i;        /* menor ultimo_uso; empate -> FIFO por cargado_en */
            }
        }
        return v;
    }
    if (strcmp(alg, "NRU") == 0) {
        int v = 0, minClase = (marcos[0].bitR << 1) | marcos[0].bitM;
        for (int i = 1; i < n; i++) {
            int clase = (marcos[i].bitR << 1) | marcos[i].bitM;
            if (clase < minClase ||
                (clase == minClase && marcos[i].ultimo_uso < marcos[v].ultimo_uso)) {
                minClase = clase;
                v = i;
            }
        }
        return v;
    }
    if (strcmp(alg, "OPT") == 0) {
        int v = 0;
        int maxDist = proximaReferencia(seq, totalSeq, posActual, marcos[0].idPagina);
        for (int i = 1; i < n; i++) {
            int d = proximaReferencia(seq, totalSeq, posActual, marcos[i].idPagina);
            if (d > maxDist) {
                maxDist = d; v = i;
            } else if (d == maxDist && marcos[i].cargado_en < marcos[v].cargado_en) {
                v = i;                            /* desempate FIFO: la mas antigua */
            }
        }
        return v;
    }
    if (strcmp(alg, "SC") == 0) {
        /* Second Chance: recorre los marcos en orden FIFO (cargado_en ascendente).
           - Si la cabeza tiene R=0, esa es la victima.
           - Si tiene R=1, se le da segunda oportunidad (R=0) y se revisa la siguiente.
           Tras una pasada completa todas las R quedan en 0, asi que en la
           segunda pasada se garantiza encontrar una victima sin tocar M. */
        int orderIdx[MAX_MARCOS];
        for (int i = 0; i < n; i++) orderIdx[i] = i;
        for (int a = 0; a < n - 1; a++)
            for (int b = 0; b < n - 1 - a; b++)
                if (marcos[orderIdx[b]].cargado_en > marcos[orderIdx[b+1]].cargado_en) {
                    int tmp = orderIdx[b]; orderIdx[b] = orderIdx[b+1]; orderIdx[b+1] = tmp;
                }
        for (int pass = 0; pass < 2; pass++) {
            for (int j = 0; j < n; j++) {
                int idx = orderIdx[j];
                if (marcos[idx].bitR == 0) return idx;
                marcos[idx].bitR = 0;     /* segunda oportunidad consumida */
            }
        }
        return orderIdx[0];               /* salvaguarda inalcanzable */
    }
    if (strcmp(alg, "CLOCK") == 0) {
        /* Reloj: avanzar puntero circular limpiando bitR hasta hallar bitR==0. */
        int idx = *reloj % n;
        for (int vueltas = 0; vueltas < 2 * n; vueltas++) {
            if (marcos[idx].bitR == 0) { *reloj = (idx + 1) % n; return idx; }
            marcos[idx].bitR = 0;
            idx = (idx + 1) % n;
        }
        *reloj = (idx + 1) % n;
        return idx;
    }
    return 0; /* por defecto */
}

/* ========================================================================
   Task 4: generarCola + simularConRegistro
   ======================================================================== */

/* Construye la cola de ejecucion segun el algoritmo de planificacion. */
static int generarCola(const ConfigEntrada* cfg, Proceso* cola, int* origen, int maxCola) {
    int n = 0;
    if (strcmp(cfg->algPlan, "RR") == 0) {
        int q = cfg->quantum < 1 ? 1 : cfg->quantum;
        float restante[MAX_PROCS];
        for (int i = 0; i < cfg->nProcs; i++) restante[i] = cfg->procs[i].tiempoCPU;
        int quedan = 1;
        while (quedan && n < maxCola) {
            quedan = 0;
            for (int i = 0; i < cfg->nProcs && n < maxCola; i++) {
                if (restante[i] > 0) {
                    quedan = 1;
                    float turno = restante[i] < q ? restante[i] : (float)q;
                    cola[n] = cfg->procs[i];
                    cola[n].tiempoRestante = turno;
                    if (origen) origen[n] = i;
                    n++;
                    restante[i] -= turno;
                }
            }
        }
        return n;
    }
    /* copia base */
    for (int i = 0; i < cfg->nProcs && i < maxCola; i++) {
        cola[n] = cfg->procs[i];
        cola[n].tiempoRestante = cfg->procs[i].tiempoCPU;
        if (origen) origen[n] = i;
        n++;
    }
    /* ordenamientos */
    if (strcmp(cfg->algPlan, "SJF") == 0) {
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - 1 - i; j++)
                if (cola[j].tiempoCPU > cola[j+1].tiempoCPU) {
                    Proceso t = cola[j]; cola[j] = cola[j+1]; cola[j+1] = t;
                    if (origen) { int oi = origen[j]; origen[j] = origen[j+1]; origen[j+1] = oi; }
                }
    } else if (strcmp(cfg->algPlan, "PRIORIDAD") == 0) {
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - 1 - i; j++)
                if (cola[j].prioridad > cola[j+1].prioridad) {
                    Proceso t = cola[j]; cola[j] = cola[j+1]; cola[j+1] = t;
                    if (origen) { int oi = origen[j]; origen[j] = origen[j+1]; origen[j+1] = oi; }
                }
    }
    return n; /* FCFS = orden de insercion */
}

static int esPrimeraOcurrencia(const ConfigEntrada* cfg, int idx) {
    for (int i = 0; i < idx; i++)
        if (cfg->procs[i].id == cfg->procs[idx].id) return 0;
    return 1;
}

static int indiceProcesoLogico(const ConfigEntrada* cfg, int idProc) {
    for (int i = 0; i < cfg->nProcs; i++)
        if (cfg->procs[i].id == idProc) return i;
    return -1;
}

static int paginasProcesoLogico(const ConfigEntrada* cfg, int idProc) {
    int paginas = 0;
    for (int i = 0; i < cfg->nProcs; i++)
        if (cfg->procs[i].id == idProc && cfg->procs[i].memoriaReq > paginas)
            paginas = cfg->procs[i].memoriaReq;
    return paginas < 1 ? 1 : paginas;
}

static int maxPaginasPorProceso(const ConfigEntrada* cfg) {
    int mx = 1;
    for (int i = 0; i < cfg->nProcs; i++) {
        if (!esPrimeraOcurrencia(cfg, i)) continue;
        int pp = paginasProcesoLogico(cfg, cfg->procs[i].id);
        if (pp > mx) mx = pp;
    }
    return mx;
}

static int totalPaginasLogicas(const ConfigEntrada* cfg) {
    int maxId = 0;
    for (int i = 0; i < cfg->nProcs; i++)
        if (esPrimeraOcurrencia(cfg, i) && cfg->procs[i].id > maxId)
            maxId = cfg->procs[i].id;
    return (maxId + 1) * maxPaginasPorProceso(cfg);
}

static int datosPaginaGlobal(const ConfigEntrada* cfg, int pagGlobal,
                             int* idProc, int* pagVirt, const char** nombreProc) {
    int pmax = maxPaginasPorProceso(cfg);
    for (int i = 0; i < cfg->nProcs; i++) {
        if (!esPrimeraOcurrencia(cfg, i)) continue;
        int off  = cfg->procs[i].id * pmax;
        int nPag = paginasProcesoLogico(cfg, cfg->procs[i].id);
        if (pagGlobal >= off && pagGlobal < off + nPag) {
            if (idProc) *idProc = cfg->procs[i].id;
            if (pagVirt) *pagVirt = pagGlobal - off;
            if (nombreProc) *nombreProc = cfg->procs[i].nombre;
            return 1;
        }
    }
    return 0;
}

/* Offset global del proceso: id * max_paginas_por_proceso.
   Asi un proceso con id=N y 1 pagina ocupa la pagina global N,
   igual que la tabla de referencias del ejemplo de clase. */
static int offsetProceso(const ConfigEntrada* cfg, int idProc) {
    return idProc * maxPaginasPorProceso(cfg);
}

int simularConRegistro(const ConfigEntrada* cfg, Frame* frames, int maxFrames,
                       Proceso* cola, int maxCola, int* nCola) {
    /* Secuencia de referencias (paginas globales). */
    int  seq[MAX_FRAMES];
    int  total = 0;
    int  manual = (cfg->nSeqManual > 0);
    int  capCola = maxCola;
    if (capCola > MAX_COLA) capCola = MAX_COLA;
    if (capCola < 0) capCola = 0;

    if (manual) {
        /* Modo manual: la secuencia viene dada tal cual */
        for (int i = 0; i < cfg->nSeqManual && total < MAX_FRAMES; i++)
            seq[total++] = cfg->seqManual[i];
        if (nCola) *nCola = 0;
    } else {
        int origenCola[MAX_COLA];
        int nc = generarCola(cfg, cola, origenCola, capCola);
        if (nCola) *nCola = nc;

        int paginaActual[MAX_PROCS];
        for (int i = 0; i < MAX_PROCS; i++) paginaActual[i] = 0;

        for (int i = 0; i < nc && total < MAX_FRAMES; i++) {
            int pj = origenCola[i];
            int logical = indiceProcesoLogico(cfg, cfg->procs[pj].id);
            if (logical < 0) logical = pj;
            int N = cfg->procs[pj].memoriaReq; if (N < 1) N = 1;
            int refsTurno = (int)(cola[i].tiempoRestante + 0.5f);
            if (refsTurno < 1) refsTurno = 1;
            int off = offsetProceso(cfg, cola[i].id);
            for (int r = 0; r < refsTurno && total < MAX_FRAMES; r++) {
                seq[total++] = off + (paginaActual[logical] % N);
                paginaActual[logical]++;
            }
        }
    }

    /* Paginas distintas de la secuencia (para la tabla de paginas en modo manual) */
    int distintas[MAX_PAGINAS]; int nDistintas = 0;
    if (manual) {
        for (int i = 0; i < total; i++) {
            int p = seq[i], found = 0;
            for (int j = 0; j < nDistintas; j++) if (distintas[j] == p) { found = 1; break; }
            if (!found && nDistintas < MAX_PAGINAS) distintas[nDistintas++] = p;
        }
        for (int a = 0; a < nDistintas - 1; a++)
            for (int b = 0; b < nDistintas - 1 - a; b++)
                if (distintas[b] > distintas[b+1]) { int t = distintas[b]; distintas[b] = distintas[b+1]; distintas[b+1] = t; }
    }

    /* Estado de marcos del simulador */
    int nM = cfg->marcos; if (nM > MAX_MARCOS) nM = MAX_MARCOS;
    MarcoSim m[MAX_MARCOS];
    for (int i = 0; i < nM; i++) {
        m[i].idPagina = -1; m[i].idProceso = -1;
        m[i].cargado_en = 0; m[i].ultimo_uso = 0;
        m[i].frecuencia = 0; m[i].bitR = 0; m[i].bitM = 0;
    }
    int reloj = 0, hits = 0, fallos = 0;

    int nf = 0;
    for (int t = 0; t < total && nf < maxFrames; t++) {
        int pagGlobal = seq[t];
        /* que proceso/pagina virtual? */
        int idProc = 0, pagVirt = 0;
        const char* nombreProc = "";
        datosPaginaGlobal(cfg, pagGlobal, &idProc, &pagVirt, &nombreProc);
        if (manual) { idProc = -1; pagVirt = pagGlobal; nombreProc = "Manual"; }

        /* hit? */
        int marcoAfectado = -1, hit = 0, reemplazada = -1;
        for (int i = 0; i < nM; i++) if (m[i].idPagina == pagGlobal) {
            hit = 1; marcoAfectado = i;
            m[i].ultimo_uso = t; m[i].frecuencia++; m[i].bitR = 1;
            /* M no se toca: solo se enciende en operacion de escritura.
               Sin info de read/write en el payload, todas son lecturas. */
            break;
        }
        if (hit) {
            hits++;
        } else {
            fallos++;
            int libre = -1;
            for (int i = 0; i < nM; i++) if (m[i].idPagina == -1) { libre = i; break; }
            int destino;
            if (libre >= 0) {
                destino = libre;
            } else {
                destino = seleccionarVictima(cfg->algMMU, m, nM, t, seq, total, t, &reloj);
                reemplazada = m[destino].idPagina;
            }
            m[destino].idPagina = pagGlobal; m[destino].idProceso = idProc;
            m[destino].cargado_en = t; m[destino].ultimo_uso = t;
            m[destino].frecuencia = 1; m[destino].bitR = 1; m[destino].bitM = 0;
            marcoAfectado = destino;
        }

        /* Grabar el frame */
        Frame* f = &frames[nf++];
        memset(f, 0, sizeof(*f));
        f->tick = t + 1; f->total = total;
        f->procId = idProc;
        strncpy(f->procNombre, nombreProc, sizeof(f->procNombre)-1);
        f->paginaVirtual = pagVirt; f->paginaGlobal = pagGlobal;
        f->marcoAfectado = marcoAfectado; f->hit = hit;
        f->paginaReemplazada = reemplazada;
        f->hits = hits; f->fallos = fallos;
        f->nMarcos = nM;
        for (int i = 0; i < nM; i++) {
            MarcoSnap* ms = &f->marcos[i];
            ms->numero = i; ms->idPagina = m[i].idPagina; ms->idProceso = m[i].idProceso;
            ms->r = m[i].bitR; ms->m = m[i].bitM;
            ms->carga = m[i].cargado_en; ms->ultimo = m[i].ultimo_uso;
            ms->activo = (i == marcoAfectado) ? 1 : 0;
            if (m[i].idPagina == -1) {
                strcpy(ms->proceso, "LIBRE");
            } else if (manual) {
                strcpy(ms->proceso, "manual");
            } else {
                const char* nom = "";
                for (int k = 0; k < cfg->nProcs; k++)
                    if (cfg->procs[k].id == m[i].idProceso) nom = cfg->procs[k].nombre;
                strncpy(ms->proceso, nom, sizeof(ms->proceso)-1);
            }
        }
        /* tabla de paginas */
        int pp = 0;
        if (manual) {
            /* en modo manual: una fila por cada pagina distinta de la secuencia */
            for (int d = 0; d < nDistintas && pp < MAX_PAGINAS; d++) {
                int gp = distintas[d];
                PaginaSnap* ps = &f->paginas[pp++];
                ps->numero = gp; ps->idProceso = -1;
                strcpy(ps->proceso, "manual");
                ps->paginaVirtual = gp; ps->marco = -1; ps->r = 0; ps->m = 0; ps->activa = 0;
                strcpy(ps->estado, "VIRTUAL");
                for (int mi = 0; mi < nM; mi++) if (m[mi].idPagina == gp) {
                    ps->marco = mi; strcpy(ps->estado, "RAM");
                    ps->r = m[mi].bitR; ps->m = m[mi].bitM;
                    if (gp == pagGlobal) ps->activa = 1;
                    break;
                }
            }
        } else {
            for (int i = 0; i < cfg->nProcs && pp < MAX_PAGINAS; i++) {
                if (!esPrimeraOcurrencia(cfg, i)) continue;
                int o = offsetProceso(cfg, cfg->procs[i].id);
                int nPag = paginasProcesoLogico(cfg, cfg->procs[i].id);
                for (int v = 0; v < nPag && pp < MAX_PAGINAS; v++) {
                    PaginaSnap* ps = &f->paginas[pp++];
                    int gp = o + v;
                    ps->numero = gp; ps->idProceso = cfg->procs[i].id;
                    strncpy(ps->proceso, cfg->procs[i].nombre, sizeof(ps->proceso)-1);
                    ps->paginaVirtual = v; ps->marco = -1; ps->r = 0; ps->m = 0; ps->activa = 0;
                    strcpy(ps->estado, "VIRTUAL");
                    for (int mi = 0; mi < nM; mi++) if (m[mi].idPagina == gp) {
                        ps->marco = mi; strcpy(ps->estado, "RAM");
                        ps->r = m[mi].bitR; ps->m = m[mi].bitM;
                        if (gp == pagGlobal) ps->activa = 1;
                        break;
                    }
                }
            }
        }
        f->nPaginas = pp;

        /* NRU: limpia el bit R cada nM referencias, DESPUES de procesar el tick.
           El intervalo iguala la cantidad de marcos para que el envejecimiento
           de R sea proporcional al tamano de la memoria fisica. M nunca se limpia. */
        if (strcmp(cfg->algMMU, "NRU") == 0 && nM > 0 && ((t + 1) % nM) == 0) {
            for (int i = 0; i < nM; i++) m[i].bitR = 0;
        }
    }
    return nf;
}

/* ========================================================================
   Task 5: validarConfig + helpers JSON (SB) + apiSimular
   ======================================================================== */

int validarConfig(const ConfigEntrada* cfg, char* errBuf, int errLen) {
    if (cfg->nSeqManual > 0) {
        /* modo manual: la secuencia viene dada, solo se exige marcos >= 1 */
        if (cfg->marcos < 1) { snprintf(errBuf, errLen, "Los marcos fisicos deben ser >= 1."); return 0; }
        return 1;
    }
    for (int i = 0; i < cfg->nProcs; i++) {
        for (int j = i + 1; j < cfg->nProcs; j++) {
            if (cfg->procs[i].id == cfg->procs[j].id &&
                strcmp(cfg->procs[i].nombre, cfg->procs[j].nombre) != 0) {
                snprintf(errBuf, errLen,
                         "El ID %d ya pertenece a %s. Reutiliza el mismo nombre para repetirlo.",
                         cfg->procs[i].id, cfg->procs[i].nombre);
                return 0;
            }
        }
    }
    int sumaPag = totalPaginasLogicas(cfg);
    if (cfg->nProcs < 1)
        { snprintf(errBuf, errLen, "Agrega al menos un proceso."); return 0; }
    if (cfg->paginas < 1)
        { snprintf(errBuf, errLen, "Las paginas virtuales deben ser >= 1."); return 0; }
    if (cfg->marcos < 1)
        { snprintf(errBuf, errLen, "Los marcos fisicos deben ser >= 1."); return 0; }
    if (cfg->marcos >= cfg->paginas)
        { snprintf(errBuf, errLen, "Los marcos deben ser MENORES que las paginas virtuales."); return 0; }
    if (cfg->paginas < sumaPag)
        { snprintf(errBuf, errLen,
            "Paginas virtuales insuficientes: tienes %d, se requieren al menos %d "
            "(cada proceso de ID N ocupa la pagina global N).",
            cfg->paginas, sumaPag); return 0; }
    return 1;
}

/* Append seguro a un buffer dinamico */
typedef struct { char* buf; int len, cap; } SB;

static void sbInit(SB* s) {
    s->cap = 65536; s->len = 0;
    s->buf = malloc(s->cap);
    if (s->buf) s->buf[0] = 0;
}

static void sbAdd(SB* s, const char* txt) {
    int n = (int)strlen(txt);
    if (s->len + n + 1 > s->cap) {
        while (s->len + n + 1 > s->cap) s->cap *= 2;
        s->buf = realloc(s->buf, s->cap);
    }
    memcpy(s->buf + s->len, txt, n);
    s->len += n;
    s->buf[s->len] = 0;
}

static void sbAddf(SB* s, const char* fmt, ...) {
    char tmp[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    sbAdd(s, tmp);
}

static const char* nombreAlgoritmoMMU(const char* alg) {
    if (strcmp(alg, "FIFO") == 0) return "FIFO — First In First Out";
    if (strcmp(alg, "LRU") == 0) return "LRU — Least Recently Used";
    if (strcmp(alg, "OPT") == 0) return "OPT — Óptimo";
    if (strcmp(alg, "NRU") == 0) return "NRU — Not Recently Used";
    if (strcmp(alg, "SC") == 0) return "Second Chance";
    if (strcmp(alg, "CLOCK") == 0) return "Clock";
    return alg;
}

char* apiSimular(const char* payload) {
    ConfigEntrada cfg;
    if (!parsearPayload(payload, &cfg)) return strdup("{\"error\":\"Payload invalido\"}");
    char err[256];
    if (!validarConfig(&cfg, err, sizeof(err))) {
        SB s; sbInit(&s);
        sbAddf(&s, "{\"error\":\"%s\"}", err);
        return s.buf;
    }
    Frame* frames = malloc(sizeof(Frame) * MAX_FRAMES);
    if (!frames) return strdup("{\"error\":\"Sin memoria\"}");
    Proceso cola[MAX_COLA]; int nCola = 0;
    int nf = simularConRegistro(&cfg, frames, MAX_FRAMES, cola, MAX_COLA, &nCola);

    SB s; sbInit(&s);
    sbAddf(&s, "{\"config\":{\"algoritmo\":\"%s\",\"marcos\":%d,\"paginas\":%d,\"planificador\":\"%s\"},",
           nombreAlgoritmoMMU(cfg.algMMU), cfg.marcos, cfg.paginas, cfg.algPlan);
    /* procesos */
    sbAdd(&s, "\"procesos\":[");
    int escritos = 0;
    for (int i = 0; i < cfg.nProcs; i++) {
        if (!esPrimeraOcurrencia(&cfg, i)) continue;
        float cpuTotal = 0.0f;
        for (int j = 0; j < cfg.nProcs; j++)
            if (cfg.procs[j].id == cfg.procs[i].id)
                cpuTotal += cfg.procs[j].tiempoCPU;
        sbAddf(&s, "%s{\"id\":%d,\"nombre\":\"%s\",\"cpu\":%.0f,\"paginas\":%d}",
               escritos ? "," : "", cfg.procs[i].id, cfg.procs[i].nombre,
               cpuTotal, paginasProcesoLogico(&cfg, cfg.procs[i].id));
        escritos++;
    }
    sbAdd(&s, "],");
    /* cola */
    sbAdd(&s, "\"cola\":[");
    for (int i = 0; i < nCola; i++)
        sbAddf(&s, "%s{\"id\":%d,\"nombre\":\"%s\",\"cpu\":%.0f,\"prioridad\":%d}",
               i ? "," : "", cola[i].id, cola[i].nombre,
               cola[i].tiempoRestante > 0 ? cola[i].tiempoRestante : cola[i].tiempoCPU,
               cola[i].prioridad);
    sbAdd(&s, "],");
    /* frames */
    sbAdd(&s, "\"frames\":[");
    for (int t = 0; t < nf; t++) {
        Frame* f = &frames[t];
        sbAddf(&s, "%s{\"tick\":%d,\"total\":%d,\"proceso\":{\"id\":%d,\"nombre\":\"%s\"},"
                   "\"paginaVirtual\":%d,\"paginaGlobal\":%d,\"marcoAfectado\":%d,"
                   "\"hit\":%s,\"paginaReemplazada\":%d,\"hits\":%d,\"fallos\":%d,\"marcos\":[",
               t ? "," : "", f->tick, f->total, f->procId, f->procNombre,
               f->paginaVirtual, f->paginaGlobal, f->marcoAfectado,
               f->hit ? "true" : "false", f->paginaReemplazada, f->hits, f->fallos);
        for (int i = 0; i < f->nMarcos; i++) {
            MarcoSnap* ms = &f->marcos[i];
            sbAddf(&s, "%s{\"numero\":%d,\"idPagina\":%d,\"idProceso\":%d,\"proceso\":\"%s\","
                       "\"r\":%d,\"m\":%d,\"carga\":%d,\"ultimo\":%d,\"activo\":%s}",
                   i ? "," : "", ms->numero, ms->idPagina, ms->idProceso, ms->proceso,
                   ms->r, ms->m, ms->carga, ms->ultimo, ms->activo ? "true" : "false");
        }
        sbAdd(&s, "],\"paginas\":[");
        for (int i = 0; i < f->nPaginas; i++) {
            PaginaSnap* ps = &f->paginas[i];
            sbAddf(&s, "%s{\"numero\":%d,\"idProceso\":%d,\"proceso\":\"%s\",\"paginaVirtual\":%d,"
                       "\"marco\":%d,\"estado\":\"%s\",\"r\":%d,\"m\":%d,\"activa\":%s}",
                   i ? "," : "", ps->numero, ps->idProceso, ps->proceso, ps->paginaVirtual,
                   ps->marco, ps->estado, ps->r, ps->m, ps->activa ? "true" : "false");
        }
        sbAdd(&s, "]}");
    }
    sbAdd(&s, "]}");
    free(frames);
    return s.buf;
}

/* ========================================================================
   Task 6: apiComparar
   ======================================================================== */

char* apiComparar(const char* payload) {
    ConfigEntrada cfg;
    if (!parsearPayload(payload, &cfg)) return strdup("{\"error\":\"Payload invalido\"}");
    char err[256];
    if (!validarConfig(&cfg, err, sizeof(err))) {
        SB s; sbInit(&s);
        sbAddf(&s, "{\"error\":\"%s\"}", err);
        return s.buf;
    }
    const char* algs[6] = {"FIFO","LRU","OPT","NRU","SC","CLOCK"};
    Frame* frames = malloc(sizeof(Frame) * MAX_FRAMES);
    if (!frames) return strdup("{\"error\":\"Sin memoria\"}");
    Proceso cola[MAX_COLA]; int nCola = 0;

    SB s; sbInit(&s);
    sbAdd(&s, "{\"comparativa\":[");
    for (int a = 0; a < 6; a++) {
        ConfigEntrada c = cfg;
        strncpy(c.algMMU, algs[a], sizeof(c.algMMU)-1);
        int nf = simularConRegistro(&c, frames, MAX_FRAMES, cola, MAX_COLA, &nCola);
        int h   = nf ? frames[nf-1].hits   : 0;
        int fl  = nf ? frames[nf-1].fallos : 0;
        int tot = h + fl;
        float efic       = tot ? (100.0f * h  / tot) : 0.0f;
        float tasaFallos = tot ? (100.0f * fl / tot) : 0.0f;
        sbAddf(&s, "%s{\"algoritmo\":\"%s\",\"hits\":%d,\"fallos\":%d,"
                   "\"tasaFallos\":%.1f,\"eficiencia\":%.1f}",
               a ? "," : "", nombreAlgoritmoMMU(algs[a]), h, fl, tasaFallos, efic);
    }
    sbAdd(&s, "]}");
    free(frames);
    return s.buf;
}
