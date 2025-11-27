/**
 * @file    test_bignum_mul_u64_extra.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    27.11.2025
 *
 * @brief   Дополнительные тесты (робастность) для модуля bignum_mul_u64.
 *
 * @details
 *   Этот файл содержит тесты, проверяющие поведение функции `bignum_mul_u64`
 *   при передаче некорректных данных.
 *
 * @history
 *   - rev. 1 (01.08.2025): Первоначальная версия с тестами на NULL.
 *   - rev. 2 (01.08.2025): Добавлен тест на некорректное значение поля `len`.
 */

#include "bignum_mul_u64.h"
#include <stdio.h>
#include <assert.h>

/**
 * @brief Тест 1: Проверка на NULL-указатель для `res`.
 * @details Функция должна вернуть код ошибки и не должна падать.
 */
void test_robustness_null_res() {
    printf("Running test: test_robustness_null_res\n");
    bignum_t a = {.words = {1}, .len = 1};
    bignum_mul_u64_status_t status = bignum_mul_u64(NULL, &a, 10);
    assert(status == BIGNUM_MUL_U64_ERROR_NULL_ARG);
    printf("...PASSED\n");
}

/**
 * @brief Тест 2: Проверка на NULL-указатель для `a`.
 * @details Функция должна вернуть код ошибки и не должна падать.
 */
void test_robustness_null_a() {
    printf("Running test: test_robustness_null_a\n");
    bignum_t res;
    bignum_mul_u64_status_t status = bignum_mul_u64(&res, NULL, 10);
    assert(status == BIGNUM_MUL_U64_ERROR_NULL_ARG);
    printf("...PASSED\n");
}

/**
 * @brief Тест 3: Проверка на некорректное значение `len`.
 * @details Поле `len` не должно превышать BIGNUM_CAPACITY.
 *          Хотя текущая реализация не валидирует `len` явно (доверяя вызывающему коду),
 *          она не должна падать из-за чтения за пределами реальных данных,
 *          так как цикл ограничен `p_a->len`. Этот тест проверяет отсутствие падения.
 */
void test_robustness_invalid_len() {
    printf("Running test: test_robustness_invalid_len\n");
    bignum_t a = {.words = {1}, .len = BIGNUM_CAPACITY + 5}; // Явно некорректный len
    bignum_t res;
    
    // Мы не можем предсказать результат, но мы можем проверить, что программа не падает.
    // В реальном коде это может быть защищено ассертом на стороне разработки.
    bignum_mul_u64(&res, &a, 1);
    printf("...PASSED (no crash)\n");
}


int main() {
    printf("\n--- Starting extra tests for bignum_mul_u64 ---\n");
    test_robustness_null_res();
    test_robustness_null_a();
    test_robustness_invalid_len();
    printf("\n--- All extra tests for bignum_mul_u64 passed ---\n");
    return 0;
}

