#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

#define CHUNK_SIZE (32 * 1024 * 1024) // 32 MiB за одну итерацию

static double get_timestamp(struct timespec start) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)(now.tv_sec - start.tv_sec) + 
           (double)(now.tv_nsec - start.tv_nsec) / 1e9;
}

// Чтение MemAvailable и MemFree из /proc/meminfo (в МиБ)
void read_sys_mem(double *avail_mib, double *free_mib) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return;
    
    char line[256];
    long long avail_kb = 0, free_kb = 0;
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "MemAvailable: %lld kB", &avail_kb) == 1) continue;
        if (sscanf(line, "MemFree: %lld kB", &free_kb) == 1) continue;
    }
    fclose(f);

    *avail_mib = (double)avail_kb / 1024.0;
    *free_mib = (double)free_kb / 1024.0;
}

// Чтение VmRSS и VmSize процесса из /proc/self/status (в МиБ)
void read_proc_mem(double *rss_mib, double *vmsize_mib) {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return;

    char line[256];
    long long rss_kb = 0, vmsize_kb = 0;
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "VmRSS: %lld kB", &rss_kb) == 1) continue;
        if (sscanf(line, "VmSize: %lld kB", &vmsize_kb) == 1) continue;
    }
    fclose(f);

    *rss_mib = (double)rss_kb / 1024.0;
    *vmsize_mib = (double)vmsize_kb / 1024.0;
}

int main() {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096;

    FILE *f_sys = fopen("system_memory.csv", "w");
    FILE *f_proc = fopen("process_memory.csv", "w");
    if (!f_sys || !f_proc) {
        perror("Ошибка открытия файлов лога");
        return 1;
    }

    // Отключаем буферизацию, чтобы при SIGKILL от OOM данные гарантированно были на диске
    setvbuf(f_sys, NULL, _IONBF, 0);
    setvbuf(f_proc, NULL, _IONBF, 0);

    fprintf(f_sys, "timestamp,mem_available_mib,mem_free_mib\n");
    fprintf(f_proc, "timestamp,rss_mib,vmsize_mib\n");

    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    printf("[+] Старт выделения памяти (шаг страницы: %ld байт)...\n", page_size);
    printf("[+] PID: %d. Следите за dmesg: 'dmesg -T | grep -i oom'\n", getpid());

    size_t total_mapped = 0;

    while (1) {
        // Анонимное отображение памяти
        volatile char *addr = (volatile char *)mmap(
            NULL,
            CHUNK_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE,
            -1,
            0
        );

        if (addr == MAP_FAILED) {
            perror("mmap failed (лимит виртуальной памяти)");
            break;
        }

        // Заполнение нулями с шагом в размер страницы для вызова Page Fault
        for (size_t offset = 0; offset < CHUNK_SIZE; offset += page_size) {
            addr[offset] = 0;
        }

        total_mapped += CHUNK_SIZE;
        double ts = get_timestamp(start_time);

        double avail_mib = 0, free_mib = 0;
        double rss_mib = 0, vmsize_mib = 0;

        read_sys_mem(&avail_mib, &free_mib);
        read_proc_mem(&rss_mib, &vmsize_mib);

        fprintf(f_sys, "%.3f,%.2f,%.2f\n", ts, avail_mib, free_mib);
        fprintf(f_proc, "%.3f,%.2f,%.2f\n", ts, rss_mib, vmsize_mib);

        // Микропауза для равномерного графика и возможности ОС сбросить лог
        usleep(5000); // 5 ms
    }

    fclose(f_sys);
    fclose(f_proc);
    return 0;
}