

## Algoritmos implementados

|---|--------|--------|-------------|
| 0 | Óptimo | OPT | referencias futuras |
| 1 | Not Recently Used | NRU | `bitReferencia`, `bitModificado` |
| 2 | First In First Out | FIFO | `tiempoCarga` |
| 3 | Segunda Oportunidad | SC | `bitReferencia`, `tiempoCarga` |
| 4 | Reloj (Clock) | RELOJ | `bitReferencia`, `punteroReloj` |
| 5 | Least Recently Used | LRU | `ultimoUso` |
| 6 | Conjunto de Trabajo | CT | `ultimoUso`, `ventanaTrabajo`, `enConjunto` |

---



```
proyecto-mmu/
│
├── estructuras.h       # Módulo 1 — structs base + 7 algoritmos enum
├── listas.h            # Módulo 1 — firmas de todas las funciones
├── listas.c            # Módulo 1 — listas enlazadas + funciones por algoritmo
├── main.c              # Módulo 1 — menú principal
│
├── procesos.c          # Módulo 2 — CRUD de procesos        (Integrante 2)
├── planificador.c      # Módulo 2 — cola de ejecución       (Integrante 2)
│
├── configSO.c          # Módulo 3 — configuración del SO    (Integrante 3)
│
├── emulacion.c         # Módulo 4 — 7 algoritmos de paginación (Integrante 4)
│
├── visualizacion.c     # Módulo 5 — visualización y reporte (Integrante 5)
│
├── Makefile
└── README.md
```

---




|---|--------|----------|-------------|
| 1 | Arquitectura base y menú | `estructuras.h`, `listas.h`, `listas.c`, `main.c` | Integrante 1 |
| 2 | Procesos y planificador | `procesos.c`, `planificador.c` | Integrante 2 |
| 3 | Configuración del SO | `configSO.c` | Integrante 3 |
| 4 | 7 algoritmos de paginación | `emulacion.c` | Integrante 4 |
| 5 | Visualización y reporte | `visualizacion.c` | Integrante 5 |

---

## Campos nuevos respecto a la versión anterior

> **Integrantes 3, 4 y 5:** sus archivos deben usar estos campos nuevos.

### `Marco` — nuevos campos
```c
int  bitReferencia;   // NRU, Segunda Oportunidad, Reloj
int  bitModificado;   // NRU
int  enConjunto;      // Conjunto de Trabajo
```

### `Pagina` — nuevo campo
```c
int  enConjunto;      // Conjunto de Trabajo
```

### `ConfigSO` — nuevos campos
```c
int  punteroReloj;    // Reloj (Clock) — índice de la manecilla
int  ventanaTrabajo;  // Conjunto de Trabajo — tamaño de ventana
```

### `AlgoritmoMMU` — enum actualizado
```c
typedef enum { OPT=0, NRU=1, FIFO=2, SC=3, RELOJ=4, LRU=5, CT=6 } AlgoritmoMMU;
```

---

## Funciones nuevas en `listas.c` (para Integrante 4)

```c
// NRU
Marco* buscarMarcoNRU(Marco *cabeza);
void   limpiarBitsReferencia(Marco *cabeza);

// Segunda Oportunidad y Reloj (misma función para ambos)
Marco* buscarMarcoReloj(Marco *cabeza, ConfigSO *cfg);

// Conjunto de Trabajo
Marco* buscarMarcoFueraVentana(Marco *cabeza, Pagina *paginas,
                                int reloj, int ventana);
void   actualizarConjuntoTrabajo(Marco *cabeza, Pagina *paginas,
                                  int reloj, int ventana);
```

---

## Flujo de trabajo Git

```bash
# Antes de trabajar
git pull origin main

# Al terminar cambios
git add tu_archivo.c
git commit -m "Modulo X: descripción breve"
git push origin main
```

**Regla de oro:** cada integrante solo modifica sus propios archivos.  
Nadie toca `estructuras.h` sin avisar al grupo.

---

## Integrar tu módulo al Makefile

Quita el `#` de tu archivo en el `Makefile`:

```makefile
SRCS = main.c \
       listas.c \
       procesos.c \      # Integrante 2 quita este #
       planificador.c \  # Integrante 2
       configSO.c \      # Integrante 3
       emulacion.c \     # Integrante 4
       visualizacion.c   # Integrante 5
```

---

*Proyecto MMU — Sistemas Operativos I — UNITEC-CEUTEC*
