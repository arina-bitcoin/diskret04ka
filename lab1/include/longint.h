/*
 * Задание 1: структура, создание/уничтожение, сложение, вычитание, умножение в столбик.
 * Задание 2: умножение Карацубой.
 */
#ifndef LONGINT_H
#define LONGINT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BASE 256
#define SIGN_MASK (1u << (8 * sizeof(unsigned int) - 1))
#define DIGIT_MASK (~SIGN_MASK)

typedef struct {
    unsigned int *data;  // data[0] = кол-во цифр, data[1..] = цифры (little-endian)
    int msd;             // старшая цифра числа (со знаком)
} LongInt;


// Создание и уничтожение
LongInt *li_create(size_t digits);
void li_free(LongInt *a);
LongInt *li_copy(const LongInt *a);

// Установка значения
LongInt *li_from_int(int value);
char *li_to_string(const LongInt *a);     

// Сравнение
int li_compare(const LongInt *a, const LongInt *b);
bool li_is_zero(const LongInt *a);
bool li_is_negative(const LongInt *a);
int li_sign(const LongInt *a);

// Нормализация
void li_normalize(LongInt *a);


// Сложение: a += b
int li_add_to(LongInt *a, const LongInt *b);
// Сложение: результат = a + b (новый объект)
LongInt *li_add(const LongInt *a, const LongInt *b);

// Вычитание: a -= b
int li_sub_from(LongInt *a, const LongInt *b);
// Вычитание: результат = a - b (новый объект)
LongInt *li_sub(const LongInt *a, const LongInt *b);

// Умножение в столбик: a *= b
int li_mul_to(LongInt *a, const LongInt *b);
// Умножение в столбик: результат = a * b (новый объект)
LongInt *li_mul(const LongInt *a, const LongInt *b);


// Умножение Карацубы: a *= b
int li_karatsuba_mul_to(LongInt *a, const LongInt *b);
// Умножение Карацубой: результат = a * b (новый объект)
LongInt *li_karatsuba_mul(const LongInt *a, const LongInt *b);

// Тип функции умножения (для задания 3 и сравнения времени; объявление нужно в task3)
typedef LongInt *(*li_mul_fn)(const LongInt *, const LongInt *);

#endif // LONGINT_H