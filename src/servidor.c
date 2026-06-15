/* servidor.c — servidor HTTP minimo con Winsock para el emulador MMU */

#include "servidor.h"
#include "api.h"

/* Winsock2 DEBE ir antes de windows.h */
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PUERTO_BASE 8080
#define BUF_SIZE    (1024 * 1024)   /* 1 MB para payloads grandes */

/* Envia una respuesta HTTP completa al cliente */
static void enviar(SOCKET cli, const char* status, const char* tipo,
                   const char* cuerpo, int len) {
    char cab[512];
    int n = snprintf(cab, sizeof(cab),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, tipo, len);
    send(cli, cab, n, 0);
    if (cuerpo && len > 0) send(cli, cuerpo, len, 0);
}

/* Sirve un archivo estatico del disco */
static void servirArchivo(SOCKET cli, const char* ruta, const char* tipo) {
    FILE* f = fopen(ruta, "rb");
    if (!f) {
        enviar(cli, "404 Not Found", "text/plain", "no encontrado", 13);
        return;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc(sz + 1);
    if (!buf) { fclose(f); enviar(cli, "500 Internal Server Error", "text/plain", "sin memoria", 11); return; }
    fread(buf, 1, sz, f);
    fclose(f);
    enviar(cli, "200 OK", tipo, buf, (int)sz);
    free(buf);
}

/* Resuelve la ruta de web/index.html (relativa al CWD o al ejecutable) */
static char g_webIndex[MAX_PATH] = "web/index.html";

static int existeArchivo(const char* ruta) {
    FILE* f = fopen(ruta, "rb");
    if (f) { fclose(f); return 1; }
    return 0;
}

static void resolverWebRoot(void) {
    if (existeArchivo("web/index.html")) { strcpy(g_webIndex, "web/index.html"); return; }
    char exe[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, exe, MAX_PATH);
    if (n > 0 && n < MAX_PATH) {
        char* slash = strrchr(exe, '\\');
        if (slash) {
            *slash = '\0';
            char cand[MAX_PATH];
            snprintf(cand, sizeof(cand), "%s\\web\\index.html", exe);
            if (existeArchivo(cand)) { strncpy(g_webIndex, cand, sizeof(g_webIndex) - 1); return; }
            snprintf(cand, sizeof(cand), "%s\\..\\web\\index.html", exe);
            if (existeArchivo(cand)) { strncpy(g_webIndex, cand, sizeof(g_webIndex) - 1); return; }
        }
    }
    /* fallback: queda "web/index.html" */
}

void iniciarServidor(void) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error: Winsock no inicio (codigo %d).\n", WSAGetLastError());
        return;
    }

    SOCKET srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv == INVALID_SOCKET) {
        printf("Error: no se pudo crear el socket.\n");
        WSACleanup();
        return;
    }
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    struct sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family      = AF_INET;
    dir.sin_addr.s_addr = inet_addr("127.0.0.1");

    int puerto = PUERTO_BASE, ligado = 0;
    for (; puerto < PUERTO_BASE + 20; puerto++) {
        dir.sin_port = htons((u_short)puerto);
        if (bind(srv, (struct sockaddr*)&dir, sizeof(dir)) == 0) {
            ligado = 1;
            break;
        }
    }
    if (!ligado) {
        printf("Error: no se pudo ligar ningun puerto (%d..%d).\n", PUERTO_BASE, PUERTO_BASE + 19);
        closesocket(srv);
        WSACleanup();
        return;
    }
    listen(srv, 8);

    char url[64];
    snprintf(url, sizeof(url), "http://localhost:%d", puerto);
    resolverWebRoot();
    printf("Servidor MMU en %s  (abriendo navegador...)\n", url);
    printf("Interfaz: %s\n", g_webIndex);
    printf("Cierra esta ventana para detener el servidor.\n");
    fflush(stdout);

    ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);

    char* buf = malloc(BUF_SIZE);
    if (!buf) {
        printf("Error: sin memoria para el buffer de recepcion.\n");
        closesocket(srv);
        WSACleanup();
        return;
    }

    while (1) {
        SOCKET cli = accept(srv, NULL, NULL);
        if (cli == INVALID_SOCKET) continue;

        int n = recv(cli, buf, BUF_SIZE - 1, 0);
        if (n <= 0) { closesocket(cli); continue; }
        buf[n] = '\0';

        /* Parsear metodo y ruta de la primera linea HTTP */
        char metodo[8]  = {0};
        char ruta[512]  = {0};
        sscanf(buf, "%7s %511s", metodo, ruta);

        /* Localizar el cuerpo (despues del doble CRLF) */
        char* cuerpo = strstr(buf, "\r\n\r\n");
        if (cuerpo) cuerpo += 4;

        /* Si hay Content-Length, leer el resto del cuerpo si es necesario */
        char* cl_ptr = strstr(buf, "Content-Length:");
        if (cl_ptr && cuerpo) {
            int esperado = atoi(cl_ptr + 15);
            int leido    = n - (int)(cuerpo - buf);
            while (leido < esperado) {
                int espacio = BUF_SIZE - 1 - (int)(cuerpo - buf) - leido;
                if (espacio <= 0) break;
                int r2 = recv(cli, cuerpo + leido, espacio, 0);
                if (r2 <= 0) break;
                leido += r2;
            }
            cuerpo[leido] = '\0';
        }

        /* Ruteo */
        if (strcmp(ruta, "/") == 0) {
            servirArchivo(cli, g_webIndex, "text/html; charset=utf-8");
        } else if (strncmp(ruta, "/api/simular", 12) == 0) {
            char* json = apiSimular(cuerpo ? cuerpo : "");
            enviar(cli, "200 OK", "application/json", json, (int)strlen(json));
            free(json);
        } else if (strncmp(ruta, "/api/comparar", 13) == 0) {
            char* json = apiComparar(cuerpo ? cuerpo : "");
            enviar(cli, "200 OK", "application/json", json, (int)strlen(json));
            free(json);
        } else {
            enviar(cli, "404 Not Found", "text/plain", "no encontrado", 13);
        }

        closesocket(cli);
    }

    /* Inalcanzable — limpieza para claridad */
    free(buf);
    closesocket(srv);
    WSACleanup();
}
