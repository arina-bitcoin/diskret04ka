/*
 * ЗАДАНИЕ 3: формулы и сравнение времени.
 * 3a: af(n), 3b: 115249^4183 (mod 2^n), 3c: сравнение столбик vs Карацуба.
 */
#ifndef MATH_EXPR_H
#define MATH_EXPR_H

#include "longint.h"

/* 3a: af(n) = Σ_{i=1}^{n} ((-1)^(n-i) * n!), n ∈ N */
LongInt *expression_a(unsigned int n, li_mul_fn mul_func);

/* Задание 3b: 115249^4183 (mod 2^n), n ∈ N */
LongInt *expression_b(unsigned int n, li_mul_fn mul_func);

/* Сравнение времени: умножение в столбик vs Карацуба для выражений 3a и 3b */
void compare_performance(unsigned int n);

#endif
