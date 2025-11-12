#ifndef METODOS_ORDENAMIENTO_H
#define METODOS_ORDENAMIENTO_H

// Forward declaration
typedef struct Tiempos Tiempos;

// Funciones de ordenamiento con soporte para progreso
void burbuja(int arr[], int n, Tiempos* tiempos);
void quicksort(int arr[], int low, int high, Tiempos* tiempos);
void shellsort(int arr[], int n, Tiempos* tiempos);
void radixSort(int array[], int n, Tiempos* tiempos);

// Funciones auxiliares
void insertion(int arr[], int n);
int getMax(int array[], int n);

#endif