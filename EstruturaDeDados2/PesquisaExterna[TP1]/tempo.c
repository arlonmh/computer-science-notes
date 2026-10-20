#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "tempo.h"
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

double now_seconds(void){
#ifdef _WIN32
    LARGE_INTEGER contador, frequencia;
    if(QueryPerformanceFrequency(&frequencia) &&
        QueryPerformanceCounter(&contador)){
        return (double)contador.QuadPart / (double)frequencia.QuadPart;
    }
#endif
#if defined(CLOCK_MONOTONIC)
    struct timespec t;
    if(clock_gettime(CLOCK_MONOTONIC, &t) == 0){
        return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
    }
#endif
    return (double)clock() / (double)CLOCKS_PER_SEC;
}
