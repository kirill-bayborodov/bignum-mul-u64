/**
 * @file    bignum_mul_u64.h
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    27.11.2025
 *
 * @brief   API для умножения bignum_t на uint64_t.
 *
 *
 * @see     bignum.h
 * @since   1.0.0
 *
 * @history
 *   - rev. 1 (01.08.2025): Первоначальное создание.
 *   - rev. 2 (01.08.2025): Добавлена полная Doxygen-документация для функции.
 */

#ifndef BIGNUM_MUL_U64_H
#define BIGNUM_MUL_U64_H

#include <bignum.h>
#include <stddef.h>
#include <stdint.h>

// Проверка на наличие определения BIGNUM_CAPACITY из общего заголовка
#ifndef BIGNUM_CAPACITY
#  error "bignum.h must define BIGNUM_CAPACITY"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Коды состояния для функции bignum_mul_u64.
 */
typedef enum {
    BIGNUM_MUL_U64_SUCCESS         =  0, /**< Успешное выполнение. */
    BIGNUM_MUL_U64_ERROR_NULL_ARG  = -1, /**< Ошибка: один из входных указателей равен NULL. */
    BIGNUM_MUL_U64_ERROR_OVERFLOW  = -2  /**< Ошибка: переполнение емкости. */
    /**
     * @brief Ошибка: переполнение емкости.
     * @details Сумма длин входных чисел (a->len + b->len) превышает
     *          емкость структуры bignum_t (BIGNUM_CAPACITY). Результат
     *          гарантированно не поместится.
     */
} bignum_mul_u64_status_t;

/**
 * @brief Умножает большое число (bignum_t) на 64-битное целое.
 *
 * @param[out] res Указатель на структуру для хранения результата. Может совпадать с `a`.
 * @param[in]  a   Указатель на множимое (bignum_t).
 * @param[in]  b   Множитель (uint64_t).
 *
 * @return bignum_mul_u64_status_t (0 в случае успеха, -1 в случае переполнения).
 */
bignum_mul_u64_status_t bignum_mul_u64(bignum_t *res, const bignum_t *a, uint64_t b);

#ifdef __cplusplus
}
#endif

#endif /* BIGNUM_MUL_U64_H */
