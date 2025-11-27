/**
 * @file    bench_bignum_mul_u64.c
 * @brief   Микробенчмарк для профилирования bignum_mul_u64.
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    27.11.2025
 *
 * @details
 *   Вызывает функцию bignum_mul_u64 на случайных
 *   больших числах многократно, чтобы perf успел
 *   собрать достаточное число сэмплов.
 *
 *   Для чистоты измерений все случайные данные (числа и сдвиги)
 *   генерируются заранее и помещаются в массив. Основной цикл,
 *   который профилируется, выполняет только копирование структуры
 *   и вызов целевой функции, исключая медленный вызов rand().
 *
 * @history
 *   - rev 1.0 (12.08.2025): Первоначальная версия.
 *   - rev 1.1 (13.08.2025): Реализована предварительная генерация данных.
 *   - rev 1.2 (13.08.2025): Добавлены локальные определения констант
 *                           BIGNUM_CAPACITY и BIGNUM_BITS для компиляции.
 *
 * # Сборка
 *  gcc -g -O2 -I include -no-pie -fno-omit-frame-pointer \
 *    benchmarks/bench_bignum_mul_u64.c build/bignum_mul_u64.o \
 *    -o bin/bench_bignum_mul_u64
 *
 * # Запуск perf с записью стека через frame-pointer
 * /usr/local/bin/perf record -F 9999 -o benchmarks/reports/report_bench_bignum_mul_u64 -g -- bin/bench_bignum_mul_u64
 *
 * # Отчёт, отфильтрованный по символу
 * /usr/local/bin/perf report -i benchmarks/reports/report_bench_bignum_mul_u64 --stdio --symbol-filter=bignum_mul_u64
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <bignum.h>
#include "bignum_mul_u64.h"

// --- Локальные определения для компиляции ---
// Эти константы должны быть синхронизированы с ассемблерным кодом.
#define BIGNUM_CAPACITY 32
#define BIGNUM_BITS (BIGNUM_CAPACITY * 64)

// Увеличиваем количество итераций для более надежных измерений
#define ITERATIONS (100000000u * 20)

// Количество предварительно сгенерированных наборов данных
#define PREGEN_DATA_COUNT 8192

// Максимальный сдвиг
#define MAX_SHIFT (BIGNUM_BITS - 1)

/** Заполняет bignum случайными словами и устанавливает len. */
static void init_random_bignum(bignum_t *num) {
    int used = (rand() % BIGNUM_CAPACITY) + 1;
    num->len = used;
    for (int i = 0; i < used; ++i) {
        num->words[i] = ((uint64_t)rand() << 32) | rand();
    }
    for (int i = used; i < BIGNUM_CAPACITY; ++i) {
        num->words[i] = 0;
    }
}

int main(void) {
    // --- Фаза 1: Предварительная генерация данных ---
    printf("Pregenerating %u data sets...\n", PREGEN_DATA_COUNT);

    
    bignum_t* a = malloc(sizeof(bignum_t) * PREGEN_DATA_COUNT);
    uint64_t* b = malloc(sizeof(uint64_t) * PREGEN_DATA_COUNT);

    if (!a || !b ) {
        perror("Failed to allocate memory for test data");
        return 1;
    }
    
    
    srand((unsigned)time(NULL));
    for (unsigned i = 0; i < PREGEN_DATA_COUNT; ++i) {
        init_random_bignum(&a[i]);
        b[i] = (uint64_t)(rand() % MAX_SHIFT);
    }

    // --- Фаза 2: "Горячий" цикл для профилирования ---
    printf("Starting benchmark with %u iterations...\n", ITERATIONS);

    for (uint32_t i = 0; i < ITERATIONS; ++i) {
        // Используем предварительно сгенерированные данные, циклически обращаясь к ним
        unsigned data_idx = i % PREGEN_DATA_COUNT;
        
        // Копируем исходное число, чтобы не портить эталон
        bignum_t res_dst = {0};
        bignum_t a_dst = a[data_idx];
        uint64_t b_dst = b[data_idx];

        // Вызываем целевую функцию
        bignum_mul_u64(&res_dst, &a_dst, b_dst);
        
        // Эта проверка не дает компилятору выбросить вызов функции
        if (a_dst.len == 0xDEADBEEF) {
            // Никогда не выполнится
            printf("Error marker hit.\n");
            return 1;
        }
    }

    printf("Benchmark finished.\n");

    // --- Фаза 3: Очистка ---
    free(a);
    free(b);

    return 0;
}
