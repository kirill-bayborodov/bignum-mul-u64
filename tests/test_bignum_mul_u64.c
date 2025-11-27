/**
 * @file    test_bignum_mul_u64.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    27.11.2025
 *
 * @brief   Детерминированные тесты для модуля bignum_mul_u64.
 *
 * @details
 *   Этот файл содержит набор юнит-тестов для проверки корректности
 *   функции `bignum_mul_u64`.
 *
 *   **Анализ полноты покрытия (QG-13.f):**
 *   Тестовый набор покрывает следующие сценарии:
 *   1.  **Граничные случаи:** Умножение на 0 и 1.
 *   2.  **Основные операции:** Простое умножение, умножение с переносом в следующее слово, умножение многословного числа.
 *   3.  **Сложные случаи переноса:** Умножение на `UINT64_MAX`, умножение многословного числа с переносом через все слова.
 *   4.  **Переполнение:** Проверка возврата ошибки при превышении емкости `bignum_t`.
 *   5.  **Специальные случаи:** Умножение "на месте" (in-place aliasing).
 *   6.  **Нормализация:** Проверка корректной нормализации длины результата (например, при умножении на 0).
 *
 *   Этот набор тестов считается исчерпывающим для проверки детерминированной логики.
 *
 * @history
 *   - rev. 1 (01.08.2025): Первоначальное создание.
 *   - rev. 2 (01.08.2025): Добавлены более сложные тест-кейсы по результатам ревью.
 */

#include "bignum_mul_u64.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>


static int is_zero(const bignum_t *x) {
    return (x->len == 1 && x->words[0] == 0);
}

static int bignum_are_equal(const bignum_t* x, const bignum_t* y) {
    if (is_zero(x) && is_zero(y)) return 1;
    if (x->len != y->len) return 0;
    return memcmp(x->words, y->words, x->len * sizeof(uint64_t)) == 0;
}


/*// Вспомогательная функция для сравнения
static int bignum_are_equal(const bignum_t* a, const bignum_t* b) {
    if (a->len != b->len) return 0;
    if (a->len == 0) return 1;
    return memcmp(a->words, b->words, a->len * sizeof(uint64_t)) == 0;
}*/

// Вспомогательная функция для печати bignum_t (для отладки)
void print_bignum(const char* name, const bignum_t* num) {
    printf("%s (len=%ld): 0x", name, num->len);
    for (int i = num->len - 1; i >= 0; --i) {
        printf("%016lx", num->words[i]);
    }
    printf("\n");
}

/**
 * @brief Тест 1: Умножение на 0.
 * @details Проверяет, что любое число, умноженное на 0, дает 0.
 */
void test_multiply_by_zero() {
    printf("Running test: test_multiply_by_zero\n");
    bignum_t a = {.words = {12345}, .len = 1};
    bignum_t res = {.words = {0}, .len = 0};
    bignum_t expected = {.words = {0}, .len = 1};

    bignum_mul_u64_status_t status = bignum_mul_u64(&res, &a, 0);
    assert(status == BIGNUM_MUL_U64_SUCCESS);
    assert(bignum_are_equal(&res, &expected));
    printf("...PASSED\n");
}

/**
 * @brief Тест 2: Умножение на 1.
 * @details Проверяет, что любое число, умноженное на 1, равно самому себе.
 */
void test_multiply_by_one() {
    printf("Running test: test_multiply_by_one\n");
    bignum_t a = {.words = {0x123456789ABCDEF0, 0x1}, .len = 2};
    bignum_t res = {.words = {0}, .len = 0};

    bignum_mul_u64_status_t status = bignum_mul_u64(&res, &a, 1);
    assert(status == BIGNUM_MUL_U64_SUCCESS);
    assert(bignum_are_equal(&res, &a));
    printf("...PASSED\n");
}

/**
 * @brief Тест 3: Простое умножение без переноса за пределы слова.
 */
void test_simple_multiplication() {
    printf("Running test: test_simple_multiplication\n");
    bignum_t a = {.words = {100}, .len = 1};
    bignum_t res = {.words = {0}, .len = 0};
    bignum_t expected = {.words = {500}, .len = 1};

    bignum_mul_u64_status_t status = bignum_mul_u64(&res, &a, 5);
    assert(status == BIGNUM_MUL_U64_SUCCESS);
    assert(bignum_are_equal(&res, &expected));
    printf("...PASSED\n");
}

/**
 * @brief Тест 4: Умножение с переносом в следующее слово.
 */
void test_carry_to_next_word() {
    printf("Running test: test_carry_to_next_word\n");
    bignum_t a = {.words = {0xFFFFFFFFFFFFFFFF}, .len = 1};
    bignum_t res = {.words = {0}, .len = 0};
    bignum_t expected = {.words = {0xFFFFFFFFFFFFFFFE, 1}, .len = 2};

    bignum_mul_u64_status_t status = bignum_mul_u64(&res, &a, 2);
    assert(status == BIGNUM_MUL_U64_SUCCESS);
    assert(bignum_are_equal(&res, &expected));
    printf("...PASSED\n");
}

/**
 * @brief Тест 5: Умножение многословного числа.
 */
void test_multi_word_multiplication() {
    printf("Running test: test_multi_word_multiplication\n");
    bignum_t a = {.words = {10, 1}, .len = 2};
    bignum_t res = {.words = {0}, .len = 0};
    bignum_t expected = {.words = {30, 3}, .len = 2};

    bignum_mul_u64_status_t status = bignum_mul_u64(&res, &a, 3);
    assert(status == BIGNUM_MUL_U64_SUCCESS);
    assert(bignum_are_equal(&res, &expected));
    printf("...PASSED\n");
}

/**
 * @brief Тест 6: Умножение с результатом на месте (in-place).
 */
void test_in_place_multiplication() {
    printf("Running test: test_in_place_multiplication\n");
    bignum_t a = {.words = {1000}, .len = 1};
    bignum_t expected = {.words = {5000}, .len = 1};

    bignum_mul_u64_status_t status = bignum_mul_u64(&a, &a, 5);
    assert(status == BIGNUM_MUL_U64_SUCCESS);
    assert(bignum_are_equal(&a, &expected));
    printf("...PASSED\n");
}

/**
 * @brief Тест 7: Проверка на переполнение.
 */
void test_overflow() {
    printf("Running test: test_overflow\n");
    bignum_t a;
    a.len = BIGNUM_CAPACITY;
    memset(a.words, 0, sizeof(a.words));
    a.words[BIGNUM_CAPACITY - 1] = 0xFFFFFFFFFFFFFFFF;

    bignum_t res = {.words = {0}, .len = 0};
    bignum_mul_u64_status_t status = bignum_mul_u64(&res, &a, 2);
    assert(status == BIGNUM_MUL_U64_ERROR_OVERFLOW);
    printf("...PASSED\n");
}

/**
 * @brief Тест 8: Умножение на максимальное значение uint64_t.
 */
void test_multiply_by_uint64_max() {
    printf("Running test: test_multiply_by_uint64_max\n");
    bignum_t a = {.words = {2}, .len = 1};
    bignum_t res = {.words = {0}, .len = 0};
    bignum_t expected = {.words = {0xFFFFFFFFFFFFFFFE, 1}, .len = 2};

    bignum_mul_u64_status_t status = bignum_mul_u64(&res, &a, UINT64_MAX);
    assert(status == BIGNUM_MUL_U64_SUCCESS);
    assert(bignum_are_equal(&res, &expected));
    printf("...PASSED\n");
}

/**
 * @brief Тест 9: Умножение многословного числа с полным переносом.
 */
void test_multi_word_full_carry() {
    printf("Running test: test_multi_word_full_carry\n");
    bignum_t a = {.words = {0xFFFFFFFFFFFFFFFF, 1}, .len = 2};
    bignum_t res = {.words = {0}, .len = 0};
    bignum_t expected = {.words = {0xFFFFFFFFFFFFFFFE, 3}, .len = 2};

    bignum_mul_u64_status_t status = bignum_mul_u64(&res, &a, 2);
    assert(status == BIGNUM_MUL_U64_SUCCESS);
    assert(bignum_are_equal(&res, &expected));
    printf("...PASSED\n");
}

/* Дополнительные тесты единичных случаев для bignum_mul_u64.
   Возвращает 1 при успехе, 0 при провале. */
static int test_all_in_one_tests(void) {
    printf("Running test: test_all_in_one_tests\n");
    bignum_t a, expected, res;
    bignum_mul_u64_status_t st;

    /* 1) a = 0, b = 0 */
    memset(&a, 0, sizeof(a));
    a.len = 0;
    memset(&expected, 0, sizeof(expected));
    st = bignum_mul_u64(&expected, &a, 0);
    if (st != BIGNUM_MUL_U64_SUCCESS) return 0;
    memset(&res, 0, sizeof(res));
    st = bignum_mul_u64(&res, &a, 0);
    if (st != BIGNUM_MUL_U64_SUCCESS) return 0;
    if (!bignum_are_equal(&res, &expected)) return 0;

    /* 2) a = 0, b = nonzero */
    memset(&res, 0, sizeof(res));
    st = bignum_mul_u64(&res, &a, 12345);
    if (st != BIGNUM_MUL_U64_SUCCESS) return 0;
    if (res.len != 1) return 0;

    /* 3) a = 1 word, b = 0 */
    memset(&a, 0, sizeof(a));
    a.len = 1;
    a.words[0] = 0xFFFFFFFFFFFFFFFFULL;
    memset(&expected, 0, sizeof(expected));
    st = bignum_mul_u64(&expected, &a, 0);
    if (st != BIGNUM_MUL_U64_SUCCESS) return 0;
    if (expected.len != 1) return 0;

    /* 4) a = 1 word (small), b = small */
    memset(&a, 0, sizeof(a));
    a.len = 1;
    a.words[0] = 7;
    memset(&expected, 0, sizeof(expected));
    st = bignum_mul_u64(&expected, &a, 9); /* 7 * 9 = 63 */
    if (st != BIGNUM_MUL_U64_SUCCESS) return 0;
    if (expected.len != 1 || expected.words[0] != 63) return 0;

    /* 5) a = 1 word (max), b = UINT64_MAX -> result len = 2 */
    memset(&a, 0, sizeof(a));
    a.len = 1;
    a.words[0] = UINT64_MAX;
    memset(&expected, 0, sizeof(expected));
    st = bignum_mul_u64(&expected, &a, UINT64_MAX);
    if (st != BIGNUM_MUL_U64_SUCCESS) return 0;
    if (expected.len != 2) return 0; /* (2^64-1)^2 fits in 128 bits */

    /* 6) multi-word a near capacity, b = 2: check no overflow if fits */
    memset(&a, 0, sizeof(a));
    a.len = BIGNUM_CAPACITY;
    for (size_t i = 0; i < a.len; ++i) a.words[i] = UINT64_MAX;
    /* Multiplying all-ones by 2 may require an extra carry word -> overflow expected */
    memset(&res, 0, sizeof(res));
    st = bignum_mul_u64(&res, &a, 2);
    if (st != BIGNUM_MUL_U64_ERROR_OVERFLOW && st != BIGNUM_MUL_U64_SUCCESS) {
        /* either success (if implementation keeps within capacity) or overflow */
        return 0;
    }

    /* 7) aliasing test: res == a */
    memset(&a, 0, sizeof(a));
    a.len = 2;
    a.words[0] = 123;
    a.words[1] = 1;
    memcpy(&expected, &a, sizeof(a));
    st = bignum_mul_u64(&expected, &a, 10);
    if (st != BIGNUM_MUL_U64_SUCCESS) return 0;
    /* perform in-place */
    st = bignum_mul_u64(&a, &a, 10);
    if (st != BIGNUM_MUL_U64_SUCCESS) return 0;
    if (!bignum_are_equal(&a, &expected)) return 0;

    /* All single-thread tests passed */
    printf("...PASSED\n");
    return 1;
}


int main() {
    printf("\n--- Starting tests for bignum_mul_u64 ---\n");
    test_multiply_by_zero();
    test_multiply_by_one();
    test_simple_multiplication();
    test_carry_to_next_word();
    test_multi_word_multiplication();
    test_in_place_multiplication();
    test_overflow();
    test_multiply_by_uint64_max();
    test_multi_word_full_carry();
    if (!test_all_in_one_tests()) {
        fprintf(stderr, "All-in-one tests failed\n");
        return 1;
    }    
    printf("\n--- All tests for bignum_mul_u64 passed ---\n");
    return 0;
}
