#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

#define CHUNK_SIZE (64 * 1024 * 1024)

int main(void) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        perror("sysconf");
        return 1;
    }

    size_t total_touched = 0;
    printf("[PID %d] Page size: %ld bytes\n", getpid(), page_size);

    while (1) {
        // Анонимное отображение памяти, не привязанное к файлу
        char *addr = mmap(
            NULL,
            CHUNK_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0
        );

        if (addr == MAP_FAILED) {
            perror("mmap failed");
            break;
        }

        // Проходим с шагом ровно в 1 страницу и пишем 0.
        // Это инициирует page fault и заставляет ядро выделить физ. фрейм.
        for (size_t offset = 0; offset < CHUNK_SIZE; offset += page_size) {
            addr[offset] = 0;
        }

        total_touched += CHUNK_SIZE;
        printf("Committed to RAM: %zu MiB\n", total_touched / (1024 * 1024));
        fflush(stdout);

        // Пауза 50 мс для сбора плавных метрик
        usleep(50000);
    }

    return 0;
}