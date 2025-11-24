#include <stdlib.h>
#include <windows.h>

// Forward declaration de la estructura Tiempos
typedef struct Tiempos Tiempos;

// Necesitas incluir esto en tu header o definirlo aquí
typedef struct Porcentajes {
    volatile double burbuja;
    volatile double quickSort;
    volatile double shellSort;
    volatile double radix;
} Porcentajes;

struct Tiempos {
    volatile double burbujaTiempo;
    volatile double quickSortTiempo;
    volatile double shellSortTiempo;
    volatile double rudixTiempo;
    volatile int terminado;
    Porcentajes porcentajes;
    CRITICAL_SECTION csTiempos;
    CRITICAL_SECTION csPorcentajes;
};

// ==================== BURBUJA ====================
void burbuja(int arr[], int n, Tiempos* tiempos){
    int aux;
    int cambio = 1;
    int nOriginal = n;  // Guardar tamaño original
    int pasadas = 0;
    
    while (cambio){
        cambio = 0;
        for(int j = 0; j < n - 1; j++){
            if(arr[j] > arr[j+1]) {
                aux = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = aux;
                cambio = 1;
            }
        }
        n--;
        pasadas++;
        
        // Actualizar porcentaje cada 50 pasadas para mejor rendimiento
        if(pasadas % 50 == 0 || !cambio) {
            double porcentaje = ((double)pasadas / nOriginal) * 100.0;
            if(porcentaje > 100.0) porcentaje = 100.0;
            
            if(tiempos != NULL) {
                EnterCriticalSection(&tiempos->csPorcentajes);
                tiempos->porcentajes.burbuja = porcentaje;
                LeaveCriticalSection(&tiempos->csPorcentajes);
            }
        }
    }
    
    // Asegurar 100% al final
    if(tiempos != NULL) {
        EnterCriticalSection(&tiempos->csPorcentajes);
        tiempos->porcentajes.burbuja = 100.0;
        LeaveCriticalSection(&tiempos->csPorcentajes);
    }
}

// ==================== QUICKSORT ====================
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return i + 1;
}

// Estructura auxiliar para quicksort
typedef struct QuickSortProgreso {
    volatile int* elementosProcesados;
    int totalElementos;
    Tiempos* tiempos;
    int contador;  // Para actualizar cada N operaciones
} QuickSortProgreso;

void quicksortRecursivo(int arr[], int low, int high, QuickSortProgreso* progreso) {
    if (low < high) {
        int pi = partition(arr, low, high);
        
        if(progreso != NULL && progreso->tiempos != NULL) {
            progreso->contador++;
            
            // Aproximación simple: cada partición es progreso
            // Total de particiones ≈ 2*n (en árbol balanceado)
            double porcentaje = ((double)progreso->contador / (progreso->totalElementos * 2)) * 100.0;
                if(porcentaje > 100.0) porcentaje = 100.0;
                
            if(progreso->contador % 10 == 0) {
                EnterCriticalSection(&progreso->tiempos->csPorcentajes);
                progreso->tiempos->porcentajes.quickSort = porcentaje;
                LeaveCriticalSection(&progreso->tiempos->csPorcentajes);
            }
        }
        
        quicksortRecursivo(arr, low, pi - 1, progreso);
        quicksortRecursivo(arr, pi + 1, high, progreso);
    }
}

void quicksort(int arr[], int low, int high, Tiempos* tiempos) {
    if(tiempos != NULL) {
        volatile int procesados = 0;
        QuickSortProgreso progreso = {&procesados, high - low + 1, tiempos, 0};
        quicksortRecursivo(arr, low, high, &progreso);
        
        // Asegurar 100% al final
        EnterCriticalSection(&tiempos->csPorcentajes);
        tiempos->porcentajes.quickSort = 100.0;
        LeaveCriticalSection(&tiempos->csPorcentajes);
    } else {
        quicksortRecursivo(arr, low, high, NULL);
    }
}

// ==================== INSERTION SORT ====================
void insertion(int arr[], int n){
    int aux;
    for (int i = 0; i < n; i++)
    {
        int key = arr[i];
        int j;
        for (j = i-1; j >= 0 && key < arr[j]; j--)
        {
            arr[j + 1] = arr[j];
        }
        arr[j + 1] = key;
    }
}

// ==================== SHELLSORT ====================
void shellsort(int arr[], int n, Tiempos* tiempos) {
    // Contar intervalos totales
    int totalIntervalos = 0;
    for (int temp = n/2; temp > 0; temp /= 2) {
        totalIntervalos++;
    }
    
    int intervaloActual = 0;
    
    for (int interval = n/2; interval > 0; interval /= 2)
    {
        for (int i = interval; i < n; i += 1) {
            int temp = arr[i];
            int j;
            for (j = i; j >= interval && arr[j - interval] > temp; j -= interval) {
                arr[j] = arr[j - interval];
            }
            arr[j] = temp;
        }
        
        intervaloActual++;
        double porcentaje = ((double)intervaloActual / totalIntervalos) * 100.0;
        
        if(tiempos != NULL) {
            EnterCriticalSection(&tiempos->csPorcentajes);
            tiempos->porcentajes.shellSort = porcentaje;
            LeaveCriticalSection(&tiempos->csPorcentajes);
        }
    }
    
    // Asegurar 100% al final
    if(tiempos != NULL) {
        EnterCriticalSection(&tiempos->csPorcentajes);
        tiempos->porcentajes.shellSort = 100.0;
        LeaveCriticalSection(&tiempos->csPorcentajes);
    }
}

// ==================== RADIX SORT ====================
int getMax(int array[], int n) {
    int max = array[0];
    for (int i = 1; i < n; i++) {
        if (array[i] > max) {
            max = array[i];
        }
    }
    return max;
}

void countingSort(int array[], int n, int place) {
    int* output = malloc(n * sizeof(int));
    int count[10] = {0};
    
    for (int i = 0; i < n; i++) {
        int index = (array[i] / place) % 10;
        count[index]++;
    }
    
    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }
    
    for (int i = n - 1; i >= 0; i--) {
        int index = (array[i] / place) % 10;
        output[count[index] - 1] = array[i];
        count[index]--;
    }
    
    for (int i = 0; i < n; i++) {
        array[i] = output[i];
    }
    
    free(output);
}

void radixSort(int array[], int n, Tiempos* tiempos) {
    int maxElement = getMax(array, n);
    
    // Contar pasadas totales
    int totalPasadas = 0;
    for (int temp = maxElement; temp > 0; temp /= 10) {
        totalPasadas++;
    }
    
    int pasadaActual = 0;
    
    for (int place = 1; maxElement / place > 0; place *= 10) {
        countingSort(array, n, place);
        
        pasadaActual++;
        double porcentaje = ((double)pasadaActual / totalPasadas) * 100.0;
        
        if(tiempos != NULL) {
            EnterCriticalSection(&tiempos->csPorcentajes);
            tiempos->porcentajes.radix = porcentaje;
            LeaveCriticalSection(&tiempos->csPorcentajes);
        }
    }
    
    // Asegurar 100% al final
    if(tiempos != NULL) {
        EnterCriticalSection(&tiempos->csPorcentajes);
        tiempos->porcentajes.radix = 100.0;
        LeaveCriticalSection(&tiempos->csPorcentajes);
    }
}