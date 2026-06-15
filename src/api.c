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
   Task 3: seleccionarVictima (7 algoritmos)
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
    if (strcmp(alg, "FIFO") == 0) {
        int v = 0, min = marcos[0].cargado_en;
        for (int i = 1; i < n; i++) if (marcos[i].cargado_en < min) { min = marcos[i].cargado_en; v = i; }
        return v;
    }
    if (strcmp(alg, "LRU") == 0) {
        int v = 0, min = marcos[0].ultimo_uso;
        for (int i = 1; i < n; i++) if (marcos[i].ultimo_uso < min) { min = marcos[i].ultimo_uso; v = i; }
        return v;
    }
    if (strcmp(alg, "MRU") == 0) {
        int v = 0, max = marcos[0].ultimo_uso;
        for (int i = 1; i < n; i++) if (marcos[i].ultimo_uso > max) { max = marcos[i].ultimo_uso; v = i; }
        return v;
    }
    if (strcmp(alg, "LFU") == 0) {
        int v = 0, min = marcos[0].frecuencia;
        for (int i = 1; i < n; i++) if (marcos[i].frecuencia < min) { min = marcos[i].frecuencia; v = i; }
        return v;
    }
    if (strcmp(alg, "OPT") == 0) {
        int v = 0, maxDist = -1;
        for (int i = 0; i < n; i++) {
            int d = proximaReferencia(seq, totalSeq, posActual, marcos[i].idPagina);
            if (d == INT_MAX) return i;          /* no se vuelve a usar -> victima ideal */
            if (d > maxDist) { maxDist = d; v = i; }
        }
        return v;
    }
    if (strcmp(alg, "SC") == 0 || strcmp(alg, "CLOCK") == 0) {
        /* Segunda oportunidad / reloj: avanzar puntero limpiando bitR hasta hallar bitR==0 */
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
static int generarCola(const ConfigEntrada* cfg, Proceso* cola, int maxCola) {
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
                    cola[n++] = cfg->procs[i];
                    restante[i] -= q;
                }
            }
        }
        return n;
    }
    /* copia base */
    for (int i = 0; i < cfg->nProcs && i < maxCola; i++) cola[n++] = cfg->procs[i];
    /* ordenamientos */
    if (strcmp(cfg->algPlan, "SJF") == 0) {
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - 1 - i; j++)
                if (cola[j].tiempoCPU > cola[j+1].tiempoCPU) {
                    Proceso t = cola[j]; cola[j] = cola[j+1]; cola[j+1] = t;
                }
    } else if (strcmp(cfg->algPlan, "PRIORIDAD") == 0) {
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - 1 - i; j++)
                if (cola[j].prioridad > cola[j+1].prioridad) {
                    Proceso t = cola[j]; cola[j] = cola[j+1]; cola[j+1] = t;
                }
    }
    return n; /* FCFS = orden de insercion */
}

/* Mapea el id de proceso a su offset global de paginas (acumulado de paginas previas). */
static int offsetProceso(const ConfigEntrada* cfg, int idProc) {
    int off = 0;
    for (int i = 0; i < cfg->nProcs; i++) {
        if (cfg->procs[i].id == idProc) return off;
        off += cfg->procs[i].memoriaReq;
    }
    return off;
}

int simularConRegistro(const ConfigEntrada* cfg, Frame* frames, int maxFrames,
                       Proceso* cola, int* nCola) {
    srand(12345); /* reproducibilidad (gotcha del bit M) */

    /* Secuencia de referencias (paginas globales). */
    int  seq[MAX_PAGINAS * 8];
    int  total = 0;
    int  manual = (cfg->nSeqManual > 0);

    if (manual) {
        /* Modo manual: la secuencia viene dada tal cual */
        for (int i = 0; i < cfg->nSeqManual && total < (int)(sizeof(seq)/sizeof(seq[0])); i++)
            seq[total++] = cfg->seqManual[i];
        if (nCola) *nCola = 0;
    } else {
        int nc = generarCola(cfg, cola, MAX_PROCS);
        if (nCola) *nCola = nc;

        int paginaActual[MAX_PROCS];
        int apar[MAX_PROCS];
        for (int i = 0; i < MAX_PROCS; i++) { paginaActual[i] = 0; apar[i] = 0; }

        for (int i = 0; i < nc; i++)
            for (int j = 0; j < cfg->nProcs; j++)
                if (cola[i].id == cfg->procs[j].id) apar[j]++;

        for (int i = 0; i < nc && total < (int)(sizeof(seq)/sizeof(seq[0])); i++) {
            int pj = 0;
            for (int j = 0; j < cfg->nProcs; j++) if (cfg->procs[j].id == cola[i].id) pj = j;
            int N = cfg->procs[pj].memoriaReq; if (N < 1) N = 1;
            int refsTotal = (int)(cfg->procs[pj].tiempoCPU + 0.5f);
            int refsTurno = apar[pj] > 0 ? refsTotal / apar[pj] : refsTotal;
            if (refsTurno < 1) refsTurno = 1;
            int off = offsetProceso(cfg, cola[i].id);
            for (int r = 0; r < refsTurno && total < (int)(sizeof(seq)/sizeof(seq[0])); r++) {
                seq[total++] = off + (paginaActual[pj] % N);
                paginaActual[pj]++;
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
        int idProc = 0, pagVirt = 0, off = 0;
        for (int i = 0; i < cfg->nProcs; i++) {
            if (pagGlobal < off + cfg->procs[i].memoriaReq) {
                idProc = cfg->procs[i].id; pagVirt = pagGlobal - off; break;
            }
            off += cfg->procs[i].memoriaReq;
        }
        const char* nombreProc = "";
        for (int i = 0; i < cfg->nProcs; i++)
            if (cfg->procs[i].id == idProc) nombreProc = cfg->procs[i].nombre;
        if (manual) { idProc = -1; pagVirt = pagGlobal; nombreProc = "Manual"; }

        /* hit? */
        int marcoAfectado = -1, hit = 0, reemplazada = -1;
        for (int i = 0; i < nM; i++) if (m[i].idPagina == pagGlobal) {
            hit = 1; marcoAfectado = i;
            m[i].ultimo_uso = t; m[i].frecuencia++; m[i].bitR = 1;
            if (rand() % 10 == 0) m[i].bitM = 1;
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
                int o = offsetProceso(cfg, cfg->procs[i].id);
                for (int v = 0; v < cfg->procs[i].memoriaReq && pp < MAX_PAGINAS; v++) {
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
    int sumaPag = 0;
    for (int i = 0; i < cfg->nProcs; i++) sumaPag += cfg->procs[i].memoriaReq;
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
            "Las paginas virtuales (%d) deben ser >= suma de paginas de procesos (%d).",
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

char* apiSimular(const char* payload) {
    ConfigEntrada cfg;
    if (!parsearPayload(payload, &cfg)) return strdup("{\"error\":\"Payload invalido\"}");
    char err[256];
    if (!validarConfig(&cfg, err, sizeof(err))) {
        SB s; sbInit(&s);
        sbAddf(&s, "{\"error\":\"%s\"}", err);
        return s.buf;
    }
    Frame* frames = malloc(sizeof(Frame) * (MAX_PAGINAS * 8));
    if (!frames) return strdup("{\"error\":\"Sin memoria\"}");
    Proceso cola[MAX_PROCS]; int nCola = 0;
    int nf = simularConRegistro(&cfg, frames, MAX_PAGINAS * 8, cola, &nCola);

    SB s; sbInit(&s);
    sbAddf(&s, "{\"config\":{\"algoritmo\":\"%s\",\"marcos\":%d,\"paginas\":%d,\"planificador\":\"%s\"},",
           cfg.algMMU, cfg.marcos, cfg.paginas, cfg.algPlan);
    /* procesos */
    sbAdd(&s, "\"procesos\":[");
    for (int i = 0; i < cfg.nProcs; i++)
        sbAddf(&s, "%s{\"id\":%d,\"nombre\":\"%s\",\"cpu\":%.0f,\"paginas\":%d}",
               i ? "," : "", cfg.procs[i].id, cfg.procs[i].nombre,
               cfg.procs[i].tiempoCPU, cfg.procs[i].memoriaReq);
    sbAdd(&s, "],");
    /* cola */
    sbAdd(&s, "\"cola\":[");
    for (int i = 0; i < nCola; i++)
        sbAddf(&s, "%s{\"id\":%d,\"nombre\":\"%s\",\"cpu\":%.0f,\"prioridad\":%d}",
               i ? "," : "", cola[i].id, cola[i].nombre,
               cola[i].tiempoCPU, cola[i].prioridad);
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
    const char* algs[7] = {"FIFO","LRU","OPT","SC","CLOCK","MRU","LFU"};
    Frame* frames = malloc(sizeof(Frame) * (MAX_PAGINAS * 8));
    if (!frames) return strdup("{\"error\":\"Sin memoria\"}");
    Proceso cola[MAX_PROCS]; int nCola = 0;

    SB s; sbInit(&s);
    sbAdd(&s, "{\"comparativa\":[");
    for (int a = 0; a < 7; a++) {
        ConfigEntrada c = cfg;
        strncpy(c.algMMU, algs[a], sizeof(c.algMMU)-1);
        int nf = simularConRegistro(&c, frames, MAX_PAGINAS * 8, cola, &nCola);
        int h   = nf ? frames[nf-1].hits   : 0;
        int fl  = nf ? frames[nf-1].fallos : 0;
        int tot = h + fl;
        float efic       = tot ? (100.0f * h  / tot) : 0.0f;
        float tasaFallos = tot ? (100.0f * fl / tot) : 0.0f;
        sbAddf(&s, "%s{\"algoritmo\":\"%s\",\"hits\":%d,\"fallos\":%d,"
                   "\"tasaFallos\":%.1f,\"eficiencia\":%.1f}",
               a ? "," : "", algs[a], h, fl, tasaFallos, efic);
    }
    sbAdd(&s, "]}");
    free(frames);
    return s.buf;
}
