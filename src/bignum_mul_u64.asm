; -----------------------------------------------------------------------------
; @file    bignum_mul_u64.asm
; @author  git@bayborodov.com
; @version 1.0.0
; @date    27.11.2025
;
; @brief   Ассемблерная реализация умножения bignum_t на uint64_t.
;
; @history
;   - rev. 1 (01.08.2025): Первоначальная реализация с ошибкой (порча RDX).
;   - rev. 2 (01.08.2025): Исправлена ошибка с порчей RDX.
;   - rev. 3 (01.08.2025): Добавлена проверка границ для `a->len` для
;                         предотвращения переполнения буфера (stack smashing).
;                         Восстановлена полная документация.
; -----------------------------------------------------------------------------

section .text

; =============================================================================
; @brief Умножает большое число (bignum_t) на 64-битное целое.
;
; @details
; **Протокол вызова (System V AMD64 ABI):**
;   - RDI: bignum_t *res (указатель на результат)
;   - RSI: const bignum_t *a (указатель на множимое)
;   - RDX: uint64_t b (множитель)
;   - RAX: возвращаемое значение (0 - успех, -1 - ошибка нулевых входных параметров, -2 - ошибка переполнения)
;
; **Алгоритм:**
; 1.  Проверка на NULL для `res` и `a`. Если любой из них NULL, возврат -1.
; 2.  Проверка корректности `a->len`. Если `len <= 0` или `len > BIGNUM_CAPACITY`,
;     возврат -1 для предотвращения переполнения буфера.
; 3.  Проверка тривиального случая: если множитель `b` (в RDX) равен 0,
;     записать 0 в результат и вернуть успех.
; 4.  Сохранение множителя `b` в R14, так как `mul` разрушает RDX.
; 5.  Инициализация:
;     - RCX: счетчик цикла, равен `a->len`.
;     - R8: указатель на `a->words`.
;     - R9: указатель на `res->words`.
;     - R10: переменная для хранения переноса (carry), инициализируется 0.
; 6.  Основной цикл (по словам `a`):
;     a. Загрузить очередное слово из `a->words` в RAX.
;     b. Выполнить 64x64->128-битное умножение: `mul r14`.
;        Результат: RDX:RAX.
;     c. Добавить к младшей части (RAX) перенос из предыдущей итерации (R10).
;     d. Проверить переполнение сложения (`adc rdx, 0`). Старшая часть (RDX)
;        теперь содержит новый перенос.
;     e. Сохранить результат (RAX) в `res->words`.
;     f. Сохранить новый перенос (RDX) в R10.
; 7.  После цикла, если остался ненулевой перенос (в R10), записать его
;     в следующее слово результата. Проверить на переполнение емкости.
; 8.  Установить корректное значение `res->len`.
; 9.  Вернуть 0 (успех).
;
; @abi        System V AMD64 ABI
; @param[in]  rdi: bignum_t* res (указатель на структуру)
; @param[in]  rsi: bignum_t* a (указатель на структуру)
; @param[in]  rdx: uint64_t b (множитель)
;
; @return     rax: bignum_mul_u64_status_t (0, -1 или -2)
; @retval 0 – success
; @retval -1 – null pointer
; @retval -2 – overflow
; @clobbers   rbx, r8–r15, rcx, rdx
; =============================================================================


; --- Константы ---
BIGNUM_CAPACITY         equ 32
BIGNUM_WORD_SIZE        equ 8
BIGNUM_BITS             equ BIGNUM_CAPACITY * 64
BIGNUM_OFFSET_WORDS     equ 0
BIGNUM_OFFSET_LEN       equ BIGNUM_CAPACITY * BIGNUM_WORD_SIZE
SUCCESS                 equ 0
ERROR_NULL_ARG          equ -1
ERROR_OVERFLOW          equ -2

global bignum_mul_u64

bignum_mul_u64:
    ; --- Пролог: сохраняем только rbp ---
    push    rbp
    mov     rbp, rsp
    push    rbx
    push    r11
    push    r12
    push    r13
    push    r14
    push    r15

    ; Проверка на NULL
    test    rdi, rdi
    jz      .error_1
    test    rsi, rsi
    jz      .error_1

    ; Получаем длину из a->len (смещение BIGNUM_WORD_SIZE * BIGNUM_CAPACITY)
    movsxd  rcx, dword [rsi + BIGNUM_OFFSET_LEN] ; rcx = a->len (32 - BIGNUM_CAPACITY)

    ; Проверка границ len (КЛЮЧЕВОЕ ИСПРАВЛЕНИЕ 2)
    test    rcx, rcx
    jle     .error_or_zero_len ; Если len <= 0
    cmp     rcx, BIGNUM_CAPACITY          ; 32 - BIGNUM_CAPACITY
    jg      .error_2          ; Если len > 32

    ; Проверка на b == 0
    test    rdx, rdx
    jz      .handle_zero

    ; Сохраняем указатели и множитель
    mov     r12, rdi        ; r12 = res
    mov     r13, rsi        ; r13 = a
    mov     r14, rdx        ; r14 = b

    ; Указатели на массивы words
    mov     r8, rsi         ; a_ptr = &a->words[0]
    mov     r9, rdi         ; res_ptr = &res->words[0]

    xor     r10, r10        ; r10 = carry = 0
    xor     r11, r11        ; r11 = loop counter i = 0

.loop:
    mov rax, [r8]
    mul r14
    add rax, r10
    adc rdx, 0
    mov [r9], rax
    mov r10, rdx
    add r8, BIGNUM_WORD_SIZE
    add r9, BIGNUM_WORD_SIZE
    inc r11
    cmp r11, rcx
    jl .loop

    test    r10, r10
    jz      .set_len_no_carry

.handle_final_carry:
    ; r9 уже указывает на слово после последнего результата
    cmp     rcx, BIGNUM_CAPACITY
    jge     .error_2
    mov     [r9], r10          ; записать carry в res->words[len]
    inc     rcx                ; увеличиваем длину результата


.set_len_no_carry:
    mov     [r12 + BIGNUM_OFFSET_LEN], ecx
    jmp     .success

.error_or_zero_len:
    ; Если len == 0, это валидный случай для числа 0.
    ; Но если len < 0, это ошибка. Мы трактуем и то, и то как 0.
    ; Для большей строгости можно было бы разделить.
    test    rcx, rcx
    jnz     .error_2
    ; Если len == 0, результат 0.
    mov     dword [rdi + BIGNUM_OFFSET_LEN], 1
    mov     qword [rdi], 0
    jmp     .success

.handle_zero:
    mov     dword [rdi + BIGNUM_OFFSET_LEN], 1
    mov     qword [rdi], 0
    jmp     .success

.error_1:
    mov     rax, ERROR_NULL_ARG
    jmp    .epilogue

.error_2:
    mov     rax, ERROR_OVERFLOW
    jmp    .epilogue

.success:
    xor     rax, rax ; SUCCESS

.epilogue:
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     rbx
    pop     rbp
    ret


