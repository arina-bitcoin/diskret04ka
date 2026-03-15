/*
 * Демонстрация: задание 1 (длинное целое), задание 2 (Карацуба), задание 3 (формулы и время).
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "longint.h"
#include "math_expr.h"

#define TITLE_FMT "\n%s\n  %s\n%s\n\n"

/* Единый формат вывода: имя = значение (десятичная строка или 0x...) */
static void print_number(const char *name, const LongInt *num) {
    if (!num) {
        printf("  %s = NULL\n", name);
        return;
    }
    char *str = li_to_string(num);
    if (str) {
        printf("  %s = %s\n", name, str);
        free(str);
    } else {
        printf("  %s = 0x", name);
        if (li_is_negative(num)) printf("-");
        for (size_t i = num->data[0]; i > 0; i--)
            printf("%08X", num->data[i]);
        printf("\n");
    }
}

/* Формат вывода для одной строки "имя = значение" */
static void print_line(const char *name, const char *value) {
    printf("  %s = %s\n", name, value ? value : "?");
}

static void section_task(int n, const char *title) {
    printf(TITLE_FMT, "\n", title, "\n");
}

int main(void) {
    LongInt *a = li_from_int(12345);
    LongInt *b = li_from_int(67890);
    if (!a || !b) return 1;

    //ЗАДАНИЕ 1: Длинное целое — структура и базовые операции

    section_task(1, "ЗАДАНИЕ 1. Длинное целое: инициализация и операции");

    printf("Создание и базовые операции:\n");
    print_number("a", a);
    print_number("b", b);

    printf("\nСложение (a + b):\n");
    LongInt *sum = li_add(a, b);
    print_number("a + b", sum);
    li_free(sum);

    printf("\nВычитание (a - b):\n");
    LongInt *diff = li_sub(a, b);
    print_number("a - b", diff);
    li_free(diff);

    printf("\nУмножение в столбик (a * b):\n");
    LongInt *prod = li_mul(a, b);
    print_number("a * b", prod);
    li_free(prod);

    printf("\nОтрицательные числа:\n");
    LongInt *neg = li_from_int(-12345);
    print_number("neg", neg);
    LongInt *neg_sum = li_add(neg, b);
    print_number("neg + b", neg_sum);
    li_free(neg_sum);
    li_free(neg);


    //ЗАДАНИЕ 2: Умножение Карацубой
    section_task(2, "ЗАДАНИЕ 2. Умножение Карацубой");

    printf("a * b (Карацуба):\n");
    LongInt *prod_karatsuba = li_karatsuba_mul(a, b);
    print_number("a * b", prod_karatsuba);
    li_free(prod_karatsuba);

    // ЗАДАНИЕ 3: Функции af(n), степень по mod 2^n, сравнение времени
    //(реализация в math_expr.c: expression_a, expression_b, compare_performance)
    
    section_task(3, "ЗАДАНИЕ 3. af(n), 115249^4183 (mod 2^n), сравнение алгоритмов");

    printf("3a. af(n) = sum_{i=1..n} ((-1)^(n-i) * n!):\n");
    for (unsigned int n = 1; n <= 5; n++) {
        LongInt *af_val = expression_a(n, li_mul);
        if (af_val) {
            char *s = li_to_string(af_val);
            char label[32];
            snprintf(label, sizeof label, "af(%u)", n);
            print_line(label, s ? s : "?");
            if (s) free(s);
            li_free(af_val);
        }
    }

    printf("\n3b. 115249^4183 (mod 2^n):\n");
    for (unsigned int n_bits = 8; n_bits <= 32; n_bits *= 2) {
        LongInt *pow_val = expression_b(n_bits, li_mul);
        if (pow_val) {
            char *s = li_to_string(pow_val);
            char label[32];
            snprintf(label, sizeof label, "mod 2^%u", n_bits);
            print_line(label, s ? s : "?");
            if (s) free(s);
            li_free(pow_val);
        }
    }

    printf("\n3c. Сравнение времени (столбик vs Карацуба):\n");
    compare_performance(12);

    li_free(a);
    li_free(b);
    return 0;
}