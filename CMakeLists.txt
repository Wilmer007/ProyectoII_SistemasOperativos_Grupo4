cmake_minimum_required(VERSION 3.20)
project(SistemMMU C)

set(CMAKE_C_STANDARD 99)

add_executable(SistemMMU
        src/main.c
        src/listas.c
        src/tablapaginas.c
        src/configSO.c
        src/procesos.c
        src/planificador.c
        src/emulacion.c
        src/visualizacion.c
        src/algoritmos.c
        src/api.c
        src/servidor.c
)

target_link_libraries(SistemMMU ws2_32)
