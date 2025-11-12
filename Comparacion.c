#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include "metodosOrdenamiento.h"

typedef struct Datos{
    int* arrBurbuja;
    int* arrQuickSort;
    int* arrShellSort;
    int* arrRudix;
    int n;
} Datos;

typedef struct Porcentajes {
    volatile double burbuja;
    volatile double quickSort;
    volatile double shellSort;
    volatile double radix;
} Porcentajes;

typedef struct Tiempos{
    volatile double burbujaTiempo;
    volatile double quickSortTiempo;
    volatile double shellSortTiempo;
    volatile double rudixTiempo;
    clock_t inicio;
    volatile int terminado; // Flag para terminar el hilo de impresión
    Porcentajes porcentajes;
    CRITICAL_SECTION csTiempos; // Para sincronización de tiempos
    CRITICAL_SECTION csPorcentajes; // Para sincronización de porcentajes
} Tiempos;

void menu();
int* crearArreglo(int tamano);
void hilos(Datos* datos);
DWORD WINAPI burbujaHilo(LPVOID param);
DWORD WINAPI quickSortHilo(LPVOID param);
DWORD WINAPI shellSortHilo(LPVOID param);
DWORD WINAPI rudixHilo(LPVOID param);
DWORD WINAPI imprimirHilo(LPVOID param);
double calcularTiempo(clock_t inicio, clock_t fin);

// Estructura para pasar datos a cada hilo
typedef struct HiloParam {
    Datos* datos;
    Tiempos* tiempos;
    volatile double* tiempoResultado;
} HiloParam;

int main() {
    while(1){
        system("cls");
        printf("=== MENU DE ORDENAMIENTO ===\n");
        printf("1. Iniciar comparacion de metodos de ordenamiento\n");
        printf("2. Salir\n");
        printf("Seleccione una opcion: ");
        int opcion;
        scanf("%d", &opcion);
        if(opcion == 2){
            printf("Saliendo del programa...\n");
            break;
        } else if(opcion != 1){
            printf("Opcion invalida. Intente de nuevo.\n\n");
            continue;
        }
        printf("Ingrese tamano del arreglo: ");
        int tamano;
        scanf("%d", &tamano);
        menu(tamano);
        printf("Presione Enter para continuar...");
        getchar(); // Consumir el salto de línea pendiente
        getchar(); // Esperar a que el usuario presione Enter
    }
    return 0;
}

void menu(int n) {
    printf("Generando arreglo de %d elementos...\n", n);
    
    int* numeros = crearArreglo(n);
    int* copiaBurbuja = malloc(n * sizeof(int));
    int* copiaQuickSort = malloc(n * sizeof(int));
    int* copiaShellSort = malloc(n * sizeof(int));
    int* copiaRudix = malloc(n * sizeof(int));
    
    if (!copiaBurbuja || !copiaQuickSort || !copiaShellSort || !copiaRudix) {
        printf("Error: No se pudo asignar memoria\n");
        return;
    }
    
    memcpy(copiaBurbuja, numeros, n * sizeof(int));
    memcpy(copiaQuickSort, numeros, n * sizeof(int));
    memcpy(copiaShellSort, numeros, n * sizeof(int));
    memcpy(copiaRudix, numeros, n * sizeof(int));
    
    Datos datos = {copiaBurbuja, copiaQuickSort, copiaShellSort, copiaRudix, n};
    
    printf("Arreglo generado. Iniciando ordenamiento...\n\n");

    hilos(&datos);

    free(numeros);
    free(copiaBurbuja);
    free(copiaQuickSort);
    free(copiaShellSort);
    free(copiaRudix);
}

int* crearArreglo(int tamano){
    srand(time(NULL));
    int *nuevoArreglo = (int*) malloc(tamano * sizeof(int));

    if(nuevoArreglo == NULL){
        return NULL;
    }

    for(int i = 0; i < tamano; i++){
        nuevoArreglo[i] = 1 + rand() % 100000;
    }

    return nuevoArreglo;
}

void hilos(Datos* datos){
    // Inicializar estructura de tiempos
    Tiempos tiempos;
    tiempos.burbujaTiempo = -1;
    tiempos.quickSortTiempo = -1;
    tiempos.shellSortTiempo = -1;
    tiempos.rudixTiempo = -1;
    tiempos.inicio = clock();
    tiempos.terminado = 0;
    tiempos.porcentajes.burbuja = 0;
    tiempos.porcentajes.quickSort = 0;
    tiempos.porcentajes.shellSort = 0;
    tiempos.porcentajes.radix = 0;
    InitializeCriticalSection(&tiempos.csTiempos);
    InitializeCriticalSection(&tiempos.csPorcentajes);
    
    // Crear parámetros para cada hilo
    HiloParam paramBurbuja = {datos, &tiempos, &tiempos.burbujaTiempo};
    HiloParam paramQuick = {datos, &tiempos, &tiempos.quickSortTiempo};
    HiloParam paramShell = {datos, &tiempos, &tiempos.shellSortTiempo};
    HiloParam paramRudix = {datos, &tiempos, &tiempos.rudixTiempo};
    
    HANDLE hBurbuja, hQuickSort, hShellSort, hRudix, hImprimir;
    
    hBurbuja = CreateThread(NULL, 0, burbujaHilo, &paramBurbuja, 0, NULL);
    hQuickSort = CreateThread(NULL, 0, quickSortHilo, &paramQuick, 0, NULL);
    hShellSort = CreateThread(NULL, 0, shellSortHilo, &paramShell, 0, NULL);
    hRudix = CreateThread(NULL, 0, rudixHilo, &paramRudix, 0, NULL);
    hImprimir = CreateThread(NULL, 0, imprimirHilo, &tiempos, 0, NULL);

    // Esperar a que terminen todos los hilos de ordenamiento
    WaitForSingleObject(hBurbuja, INFINITE);
    WaitForSingleObject(hQuickSort, INFINITE);
    WaitForSingleObject(hShellSort, INFINITE);
    WaitForSingleObject(hRudix, INFINITE);
    
    Sleep(200); // Pequeña pausa para asegurar que el hilo de impresión capture el estado final
    // Señalar al hilo de impresión que termine
    tiempos.terminado = 1;
    WaitForSingleObject(hImprimir, INFINITE);
    
    // Limpiar
    CloseHandle(hBurbuja);
    CloseHandle(hQuickSort);
    CloseHandle(hShellSort);
    CloseHandle(hRudix);
    CloseHandle(hImprimir);
    DeleteCriticalSection(&tiempos.csTiempos);
    DeleteCriticalSection(&tiempos.csPorcentajes);
    
    printf("\n\n=== RESULTADOS FINALES ===\n");
    printf("Burbuja:   %.4f segundos\n", tiempos.burbujaTiempo);
    printf("QuickSort: %.4f segundos\n", tiempos.quickSortTiempo);
    printf("ShellSort: %.4f segundos\n", tiempos.shellSortTiempo);
    printf("RadixSort: %.4f segundos\n", tiempos.rudixTiempo);
}

DWORD WINAPI burbujaHilo(LPVOID param){
    HiloParam* p = (HiloParam*)param;
    clock_t inicio = p->tiempos->inicio;
    
    burbuja(p->datos->arrBurbuja, p->datos->n, p->tiempos);
    
    EnterCriticalSection(&p->tiempos->csTiempos);
    *(p->tiempoResultado) = calcularTiempo(inicio, clock());
    LeaveCriticalSection(&p->tiempos->csTiempos);
    
    // Ejemplo: actualizar porcentaje al terminar
    EnterCriticalSection(&p->tiempos->csPorcentajes);
    p->tiempos->porcentajes.burbuja = 100.0;
    LeaveCriticalSection(&p->tiempos->csPorcentajes);
    
    return 0;
}

DWORD WINAPI quickSortHilo(LPVOID param){
    HiloParam* p = (HiloParam*)param;
    clock_t inicio = p->tiempos->inicio;
    
    quicksort(p->datos->arrQuickSort, 0, p->datos->n - 1, p->tiempos);
    
    EnterCriticalSection(&p->tiempos->csTiempos);
    *(p->tiempoResultado) = calcularTiempo(inicio, clock());
    LeaveCriticalSection(&p->tiempos->csTiempos);
    
    EnterCriticalSection(&p->tiempos->csPorcentajes);
    p->tiempos->porcentajes.quickSort = 100.0;
    LeaveCriticalSection(&p->tiempos->csPorcentajes);
    
    return 0;
}

DWORD WINAPI shellSortHilo(LPVOID param){
    HiloParam* p = (HiloParam*)param;
    clock_t inicio = p->tiempos->inicio;
    
    shellsort(p->datos->arrShellSort, p->datos->n, p->tiempos);
    
    EnterCriticalSection(&p->tiempos->csTiempos);
    *(p->tiempoResultado) = calcularTiempo(inicio, clock());
    LeaveCriticalSection(&p->tiempos->csTiempos);
    
    EnterCriticalSection(&p->tiempos->csPorcentajes);
    p->tiempos->porcentajes.shellSort = 100.0;
    LeaveCriticalSection(&p->tiempos->csPorcentajes);
    
    return 0;
}

DWORD WINAPI rudixHilo(LPVOID param){
    HiloParam* p = (HiloParam*)param;
    clock_t inicio = p->tiempos->inicio;
    
    radixSort(p->datos->arrRudix, p->datos->n, p->tiempos);
    
    EnterCriticalSection(&p->tiempos->csTiempos);
    *(p->tiempoResultado) = calcularTiempo(inicio, clock());
    LeaveCriticalSection(&p->tiempos->csTiempos);
    
    EnterCriticalSection(&p->tiempos->csPorcentajes);
    p->tiempos->porcentajes.radix = 100.0;
    LeaveCriticalSection(&p->tiempos->csPorcentajes);
    
    return 0;
}

// Función auxiliar para dibujar barra de progreso
void dibujarBarraProgreso(double porcentaje, int ancho) {
    int lleno = (int)((porcentaje / 100.0) * ancho);
    
    printf("[");
    for (int i = 0; i < ancho; i++) {
        if (i < lleno) {
            printf("=");
        } else if (i == lleno && porcentaje < 100.0) {
            printf(">");
        } else {
            printf(" ");
        }
    }
    printf("] %.1f%%", porcentaje);
}

DWORD WINAPI imprimirHilo(LPVOID param){
    Tiempos* tiempos = (Tiempos*)param;
    clock_t inicio = tiempos->inicio;
    
    while (!tiempos->terminado) {
        clock_t ahora = clock();
        double tiempoActual = calcularTiempo(inicio, ahora);
        
        EnterCriticalSection(&tiempos->csTiempos);
        double tb = tiempos->burbujaTiempo;
        double tq = tiempos->quickSortTiempo;
        double ts = tiempos->shellSortTiempo;
        double tr = tiempos->rudixTiempo;
        LeaveCriticalSection(&tiempos->csTiempos);
        
        EnterCriticalSection(&tiempos->csPorcentajes);
        double pb = tiempos->porcentajes.burbuja;
        double pq = tiempos->porcentajes.quickSort;
        double ps = tiempos->porcentajes.shellSort;
        double pr = tiempos->porcentajes.radix;
        LeaveCriticalSection(&tiempos->csPorcentajes);
        
        system("cls");
        printf("==========================================\n");
        printf("   PROGRESO DE ORDENAMIENTO\n");
        printf("==========================================\n\n");
        
        // Burbuja
        printf("Burbuja:   ");
        if (tb == -1) {
            dibujarBarraProgreso(pb, 30);
            printf(" - %.2fs\n", tiempoActual);
        } else {
            dibujarBarraProgreso(100.0, 30);
            printf(" - %.2fs [COMPLETADO]\n", tb);
        }
        
        // QuickSort
        printf("QuickSort: ");
        if (tq == -1) {
            dibujarBarraProgreso(pq, 30);
            printf(" - %.2fs\n", tiempoActual);
        } else {
            dibujarBarraProgreso(100.0, 30);
            printf(" - %.2fs [COMPLETADO]\n", tq);
        }
        
        // ShellSort
        printf("ShellSort: ");
        if (ts == -1) {
            dibujarBarraProgreso(ps, 30);
            printf(" - %.2fs\n", tiempoActual);
        } else {
            dibujarBarraProgreso(100.0, 30);
            printf(" - %.2fs [COMPLETADO]\n", ts);
        }
        
        // RadixSort
        printf("RadixSort: ");
        if (tr == -1) {
            dibujarBarraProgreso(pr, 30);
            printf(" - %.2fs\n", tiempoActual);
        } else {
            dibujarBarraProgreso(100.0, 30);
            printf(" - %.2fs [COMPLETADO]\n", tr);
        }
        
        printf("\n==========================================\n");
        
        Sleep(100); // Actualizar cada 100ms
    }
    
    return 0;
}

double calcularTiempo(clock_t inicio, clock_t fin){
    return ((double)(fin - inicio)) / CLOCKS_PER_SEC;
}