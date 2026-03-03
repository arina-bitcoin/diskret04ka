#include "../include/longint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Вспомогательная функция: выделение половины числа
static LongInt *li_split(const LongInt *a, size_t split, bool high) {
    if (!a || split >= a->data[0]) return NULL;
    
    size_t new_len = high ? (a->data[0] - split) : split;
    LongInt *result = li_create(new_len);
    if (!result) return NULL;
    
    size_t start = high ? split + 1 : 1;
    memcpy(result->data + 1, a->data + start, new_len * sizeof(unsigned int));
    
    // Копируем знак
    result->msd = (a->msd & SIGN_MASK) | (result->data[new_len] & DIGIT_MASK);
    
    return result;
}

// Сложение со сдвигом
static int li_add_shifted(LongInt *a, const LongInt *b, size_t shift) {
    if (!a || !b) return -1;
    
    size_t new_len = a->data[0] > (b->data[0] + shift) ? a->data[0] : (b->data[0] + shift);
    
    // Расширяем a если нужно
    if (a->data[0] < new_len) {
        unsigned int *new_data = realloc(a->data, (new_len + 1) * sizeof(unsigned int));
        if (!new_data) return -1;
        a->data = new_data;
        memset(a->data + a->data[0] + 1, 0, (new_len - a->data[0]) * sizeof(unsigned int));
        a->data[0] = new_len;
    }
    
    unsigned int carry = 0;
    for (size_t i = 1; i <= b->data[0]; i++) {
        size_t pos = i + shift;
        uint64_t sum = (uint64_t)a->data[pos] + b->data[i] + carry;
        a->data[pos] = (unsigned int)(sum % BASE);
        carry = (unsigned int)(sum / BASE);
    }
    
    if (carry) {
        if (shift + b->data[0] + 1 > a->data[0]) {
            // Нужно расширить
            unsigned int *new_data = realloc(a->data, (a->data[0] + 2) * sizeof(unsigned int));
            if (!new_data) return -1;
            a->data = new_data;
            a->data[0]++;
        }
        a->data[shift + b->data[0] + 1] += carry;
    }
    
    return 0;
}

// Рекурсивная реализация Карацубы
LongInt *li_karatsuba_rec(const LongInt *a, const LongInt *b) {
    if (!a || !b) return NULL;
    
    // Базовый случай: маленькие числа умножаем обычным способом
    if (a->data[0] <= 4 || b->data[0] <= 4) {
        return li_mul(a, b);
    }
    
    size_t m = (a->data[0] < b->data[0]) ? a->data[0] : b->data[0];
    size_t split = m / 2;
    
    // Разбиваем числа на старшие и младшие части
    LongInt *a_low = li_split(a, split, false);
    LongInt *a_high = li_split(a, split, true);
    LongInt *b_low = li_split(b, split, false);
    LongInt *b_high = li_split(b, split, true);
    
    if (!a_low || !a_high || !b_low || !b_high) {
        li_free(a_low); li_free(a_high); li_free(b_low); li_free(b_high);
        return NULL;
    }
    
    // Рекурсивно вычисляем три произведения
    LongInt *z0 = li_karatsuba_rec(a_low, b_low);
    LongInt *z2 = li_karatsuba_rec(a_high, b_high);
    
    // Вычисляем (a_low + a_high) и (b_low + b_high)
    LongInt *a_sum = li_add(a_low, a_high);
    LongInt *b_sum = li_add(b_low, b_high);
    
    if (!a_sum || !b_sum) {
        li_free(z0); li_free(z2); li_free(a_sum); li_free(b_sum);
        li_free(a_low); li_free(a_high); li_free(b_low); li_free(b_high);
        return NULL;
    }
    
    LongInt *z1 = li_karatsuba_rec(a_sum, b_sum);
    
    // z1 = z1 - z2 - z0
    if (z1) {
        li_sub_from(z1, z2);
        li_sub_from(z1, z0);
    }
    
    // Освобождаем промежуточные результаты
    li_free(a_low); li_free(a_high); li_free(b_low); li_free(b_high);
    li_free(a_sum); li_free(b_sum);
    
    if (!z0 || !z1 || !z2) {
        li_free(z0); li_free(z1); li_free(z2);
        return NULL;
    }
    
    // Собираем результат: z2 * BASE^(2*split) + z1 * BASE^split + z0
    LongInt *result = li_copy(z2);
    if (!result) {
        li_free(z0); li_free(z1); li_free(z2);
        return NULL;
    }
    
    // Умножаем z2 на BASE^(2*split) - просто сдвиг
    result->data[0] += 2 * split;
    unsigned int *new_data = realloc(result->data, (result->data[0] + 1) * sizeof(unsigned int));
    if (!new_data) {
        li_free(result); li_free(z0); li_free(z1); li_free(z2);
        return NULL;
    }
    result->data = new_data;
    memmove(result->data + 1 + 2*split, result->data + 1, 
            (z2->data[0]) * sizeof(unsigned int));
    memset(result->data + 1, 0, 2*split * sizeof(unsigned int));
    
    // Добавляем z1 со сдвигом split
    li_add_shifted(result, z1, split);
    
    // Добавляем z0
    li_add_to(result, z0);
    
    // Устанавливаем знак
    result->msd = ((a->msd & SIGN_MASK) ^ (b->msd & SIGN_MASK)) | 
                  (result->data[result->data[0]] & DIGIT_MASK);
    
    li_free(z0); li_free(z1); li_free(z2);
    li_normalize(result);
    
    return result;
}

// Умножение Карацубой с сохранением в первое число
int li_karatsuba_mul_to(LongInt *a, const LongInt *b) {
    if (!a || !b) return -1;
    
    LongInt *result = li_karatsuba_rec(a, b);
    if (!result) return -1;
    
    // Заменяем содержимое a на результат
    li_free(a);
    *a = *result;
    free(result);
    
    return 0;
}

// Умножение Карацубой с созданием нового числа
LongInt *li_karatsuba_mul(const LongInt *a, const LongInt *b) {
    if (!a || !b) return NULL;
    return li_karatsuba_rec(a, b);
}