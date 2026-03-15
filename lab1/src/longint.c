/*
 * ЗАДАНИЕ 1: Длинное целое — структура и базовые операции.
 * Инициализация/деинициализация, сложение, вычитание, умножение в столбик
 * (в двух вариантах: результат в первый операнд и новый объект).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "longint.h"

// Создание числа с заданным количеством цифр
LongInt *li_create(size_t digits) {
    if (digits == 0) digits = 1; // Минимум 1 цифра
    
    LongInt *a = malloc(sizeof(LongInt));
    if (!a) return NULL;
    
    // Выделяем память: data[0] для длины + digits цифр
    a->data = calloc(digits + 1, sizeof(unsigned int));
    if (!a->data) {
        free(a);
        return NULL;
    }
    
    a->data[0] = digits;
    a->msd = 0;
    return a;
}

// Освобождение памяти
void li_free(LongInt *a) {
    if (!a) return;
    if (a->data) {
        // Затираем данные перед освобождением (безопасность) гыгы 
        memset(a->data, 0, (a->data[0] + 1) * sizeof(unsigned int));
        free(a->data);
    }
    free(a);
}

// Копирование числа
LongInt *li_copy(const LongInt *a) {
    if (!a) return NULL;
    
    LongInt *b = li_create(a->data[0]);
    if (!b) return NULL;
    
    memcpy(b->data, a->data, (a->data[0] + 1) * sizeof(unsigned int));
    b->msd = a->msd;
    return b;
}

// Нормализация числа (удаление ведущих нулей)
void li_normalize(LongInt *a) {
    if (!a || !a->data) return;
    
    // Если число ноль
    if (a->data[0] == 1 && a->data[1] == 0) {
        a->msd = 0;
        return;
    }
    
    // Находим последнюю ненулевую цифру
    size_t new_len = a->data[0];
    while (new_len > 1 && a->data[new_len] == 0) {
        new_len--;
    }
    
    if (new_len < a->data[0]) {
        // Усекаем массив
        unsigned int *new_data = realloc(a->data, (new_len + 1) * sizeof(unsigned int));
        if (new_data) {
            a->data = new_data;
            a->data[0] = new_len;
        }
    }
    
    // Обновляем msd с учетом знака
    a->msd = (a->msd & SIGN_MASK) | (a->data[a->data[0]] & DIGIT_MASK);
}

LongInt *li_from_int(int value) {
    LongInt *a = NULL;
    unsigned int abs_val;
    if (value < 0) {
        abs_val = (unsigned int)(-value);
    } else {
        abs_val = (unsigned int)value;
    }
    
    // Считаем, сколько цифр нужно при BASE=256
    unsigned int tmp = abs_val;
    size_t digits = 1;
    while (tmp >= BASE) {  // BASE теперь 256
        tmp /= BASE;
        ++digits;
    }
    
    a = li_create(digits);
    if (!a) return NULL;
    
    tmp = abs_val;
    for (size_t i = 1; i <= digits; ++i) {
        a->data[i] = tmp % BASE;  // BASE теперь 256
        tmp /= BASE;
    }
    
    // Устанавливаем знак и нормализуем
    if (value < 0) {
        a->msd = (int)(a->data[a->data[0]] | SIGN_MASK);
    } else {
        a->msd = (int)a->data[a->data[0]];
    }
    li_normalize(a);
    return a;
}


// Проверка на ноль
bool li_is_zero(const LongInt *a) {
    if (!a || !a->data) return true;
    return (a->data[0] == 1 && a->data[1] == 0);
}

// Проверка знака
bool li_is_negative(const LongInt *a) {
    if (!a) return false;
    return (a->msd & SIGN_MASK) != 0;
}

// Знак числа: -1, 0, 1
int li_sign(const LongInt *a) {
    if (li_is_zero(a)) return 0;
    return li_is_negative(a) ? -1 : 1;
}

// Сложение с сохранением в первое число
int li_add_to(LongInt *a, const LongInt *b) {
    if (!a || !b) return -1;
    
    // Проверка на ноль
    if (li_is_zero(b)) return 0;
    
    // Обработка знаков
    if (li_is_negative(a) != li_is_negative(b)) {
        // Разные знаки - вызываем вычитание
        if (li_is_negative(a)) {
            // a < 0, b > 0: a + b = b - |a|. Копируем b в a, затем a -= |a| (b const — не меняем).
            LongInt *abs_a = li_copy(a);
            if (!abs_a) return -1;
            abs_a->msd &= DIGIT_MASK;
            if (a->data[0] < b->data[0] + 1) {
                unsigned int *new_data = realloc(a->data, (b->data[0] + 2) * sizeof(unsigned int));
                if (!new_data) { li_free(abs_a); return -1; }
                a->data = new_data;
            }
            a->data[0] = b->data[0];
            memcpy(a->data + 1, b->data + 1, b->data[0] * sizeof(unsigned int));
            a->msd = b->msd;
            int result = li_sub_from(a, abs_a);
            li_free(abs_a);
            return result;
        } else {
            // a > 0, b < 0: a + b = a - |b|
            LongInt *abs_b = li_copy(b);
            if (!abs_b) return -1;
            abs_b->msd &= DIGIT_MASK;
            
            int result = li_sub_from(a, abs_b);
            li_free(abs_b);
            return result;
        }
    }
    
    // Одинаковые знаки - выполняем сложение
    size_t max_len = (a->data[0] > b->data[0]) ? a->data[0] : b->data[0];
    size_t min_len = (a->data[0] < b->data[0]) ? a->data[0] : b->data[0];
    
    // Увеличиваем массив a если нужно
    if (a->data[0] < max_len + 1) {
        unsigned int *new_data = realloc(a->data, (max_len + 2) * sizeof(unsigned int));
        if (!new_data) return -1;
        a->data = new_data;
        memset(a->data + a->data[0] + 1, 0, (max_len + 1 - a->data[0]) * sizeof(unsigned int));
        a->data[0] = max_len + 1;
    }
    
    unsigned int carry = 0;
    size_t i;
    
    // Складываем общую часть
    for (i = 1; i <= min_len; i++) {
        uint64_t sum = (uint64_t)a->data[i] + b->data[i] + carry;
        a->data[i] = (unsigned int)(sum % BASE);
        carry = (unsigned int)(sum / BASE);
    }
    
    // Продолжаем с более длинным числом
    const unsigned int *longer = (a->data[0] > b->data[0]) ? a->data : b->data;
    size_t longer_len = (a->data[0] > b->data[0]) ? a->data[0] : b->data[0];
    
    for (; i <= longer_len && carry; i++) {
        uint64_t sum = (uint64_t)longer[i] + carry;
        a->data[i] = (unsigned int)(sum % BASE);
        carry = (unsigned int)(sum / BASE);
    }
    
    // Если остался перенос
    if (carry) {
        a->data[i] = carry;
        a->data[0] = i;
    } else if (i <= longer_len) {
        // Копируем оставшиеся цифры
        for (; i <= longer_len; i++) {
            a->data[i] = longer[i];
        }
        a->data[0] = longer_len;
    }
    
    // Обновляем msd с сохранением знака
    int sign = a->msd & SIGN_MASK;
    a->msd = sign | (a->data[a->data[0]] & DIGIT_MASK);
    
    return 0;
}

// Сложение с созданием нового числа
LongInt *li_add(const LongInt *a, const LongInt *b) {
    if (!a || !b) return NULL;
    
    LongInt *result = li_copy(a);
    if (!result) return NULL;
    
    if (li_add_to(result, b) != 0) {
        li_free(result);
        return NULL;
    }
    
    return result;
}

// Вычитание с сохранением в первое число (a -= b)
int li_sub_from(LongInt *a, const LongInt *b) {
    if (!a || !b) return -1;
    
    // Проверка на ноль
    if (li_is_zero(b)) return 0;
    
    // Обработка знаков
    if (li_is_negative(a) != li_is_negative(b)) {
        // Разные знаки: a - b = a + (-b)
        LongInt *neg_b = li_copy(b);
        if (!neg_b) return -1;
        neg_b->msd ^= SIGN_MASK; // Инвертируем знак
        
        int result = li_add_to(a, neg_b);
        li_free(neg_b);
        return result;
    }
    
    // Одинаковые знаки
    int cmp = li_compare(a, b);
    
    if (cmp == 0) {
        // a == b, результат 0
        // Освобождаем старые данные a
        free(a->data);
        // Создаём новый массив для нуля
        a->data = calloc(2, sizeof(unsigned int));
        if (!a->data) return -1;
        a->data[0] = 1;
        a->data[1] = 0;
        a->msd = 0;
        return 0;
    }
    
    // Определяем, какое число больше по абсолютной величине
    int abs_cmp = li_compare_abs(a, b);
    
    if (abs_cmp < 0) {
        // |a| < |b|, результат будет с противоположным знаком
        // Вычисляем b - a, затем меняем знак
        LongInt *tmp = li_copy(b);
        if (!tmp) return -1;
        
        // tmp = tmp - a
        if (li_sub_from(tmp, a) != 0) {
            li_free(tmp);
            return -1;
        }
        
        // Меняем знак результата
        tmp->msd ^= SIGN_MASK;
        
        // Заменяем содержимое a на tmp
        free(a->data);
        a->data = tmp->data;
        a->data[0] = tmp->data[0];
        a->msd = tmp->msd;
        
        // Освобождаем структуру tmp, но не данные (они теперь в a)
        free(tmp);
        
        return 0;
    }
    
    // |a| >= |b|, выполняем вычитание
    size_t i;
    unsigned int borrow = 0;
    
    for (i = 1; i <= b->data[0]; i++) {
        uint64_t diff;
        if (a->data[i] >= b->data[i] + borrow) {
            diff = a->data[i] - b->data[i] - borrow;
            borrow = 0;
        } else {
            diff = a->data[i] + BASE - b->data[i] - borrow;
            borrow = 1;
        }
        a->data[i] = (unsigned int)diff;
    }
    
    // Продолжаем с оставшимися цифрами a
    for (; i <= a->data[0] && borrow; i++) {
        if (a->data[i] >= borrow) {
            a->data[i] -= borrow;
            borrow = 0;
        } else {
            a->data[i] = a->data[i] + BASE - borrow;
            borrow = 1;
        }
    }
    
    // Нормализуем результат (удаляем ведущие нули)
    li_normalize(a);
    
    return 0;
}


LongInt *li_sub(const LongInt *a, const LongInt *b) {
    if (!a || !b) return NULL;
    
    LongInt *result = li_copy(a);
    if (!result) return NULL;
    
    if (li_sub_from(result, b) != 0) {
        li_free(result);
        return NULL;
    }
    
    return result;
}

// Сравнение абсолютных значений
int li_compare_abs(const LongInt *a, const LongInt *b) {
    if (!a || !b) return 0;
    
    if (a->data[0] != b->data[0]) {
        return (a->data[0] > b->data[0]) ? 1 : -1;
    }
    
    for (size_t i = a->data[0]; i > 0; i--) {
        if (a->data[i] != b->data[i]) {
            return (a->data[i] > b->data[i]) ? 1 : -1;
        }
    }
    
    return 0;
}

// Полное сравнение с учётом знака: -1, 0, 1
int li_compare(const LongInt *a, const LongInt *b) {
    if (!a || !b) return 0;
    int sa = li_sign(a);
    int sb = li_sign(b);
    if (sa != sb) {
        return (sa < sb) ? -1 : 1;
    }
    if (sa == 0) return 0;
    int cmp_abs = li_compare_abs(a, b);
    return (sa > 0) ? cmp_abs : -cmp_abs;
}

// Умножение в столбик (сохранение в первое число)
int li_mul_to(LongInt *a, const LongInt *b) {
    if (!a || !b) return -1;
    
    // Проверка на ноль
    if (li_is_zero(a) || li_is_zero(b)) {
        free(a->data);
        LongInt *z = li_from_int(0);
        if (!z) return -1;
        a->data = z->data;
        a->data[0] = z->data[0];
        a->msd = z->msd;
        free(z);
        return 0;
    }
    
    size_t a_len = a->data[0];
    size_t b_len = b->data[0];
    size_t result_len = a_len + b_len;
    
    // Создаем временный массив для результата
    unsigned int *result = calloc(result_len + 1, sizeof(unsigned int));
    if (!result) return -1;
    result[0] = result_len;
    
    // Умножение в столбик
    for (size_t i = 1; i <= a_len; i++) {
        unsigned int carry = 0;
        for (size_t j = 1; j <= b_len; j++) {
            size_t pos = i + j - 1;
            uint64_t mul = (uint64_t)a->data[i] * b->data[j] + result[pos] + carry;
            result[pos] = (unsigned int)(mul % BASE);
            carry = (unsigned int)(mul / BASE);
        }
        if (carry) {
            result[i + b_len] += carry;
        }
    }
    
    // Определяем знак результата
    int sign = (li_is_negative(a) ^ li_is_negative(b)) ? SIGN_MASK : 0;
    
    // Освобождаем старые данные a
    free(a->data);
    
    // Присваиваем результат
    a->data = result;
    a->msd = sign | (result[result_len] & DIGIT_MASK);
    
    // Нормализуем
    li_normalize(a);
    
    return 0;
}

// Умножение с созданием нового числа
LongInt *li_mul(const LongInt *a, const LongInt *b) {
    if (!a || !b) return NULL;
    
    LongInt *result = li_copy(a);
    if (!result) return NULL;
    
    if (li_mul_to(result, b) != 0) {
        li_free(result);
        return NULL;
    }
    
    return result;
}


char *li_to_string(const LongInt *a) {
    if (!a || !a->data) return NULL;
    
    // Если число равно 0
    if (li_is_zero(a)) {
        char *result = malloc(2);
        if (result) strcpy(result, "0");
        return result;
    }
    
    char *result = malloc(a->data[0] * 3 + 2); // максимум 2 символа на цифру (FF) + знак + null
    if (!result) return NULL;
    
    char *ptr = result;
    if (li_is_negative(a)) {
        *ptr++ = '-';
    }
    
    for (size_t i = a->data[0]; i > 0; i--) {
        ptr += sprintf(ptr, "%02X", a->data[i]); // %02X для BASE=256 (2 шестн. цифры)
    }
    
    return result;
}

