#include "../include/longint.h"
#include <stdio.h>
#include <stdlib.h>

void print_number(const char *name, LongInt *num) {
    if (!num) {
        printf("%s: NULL\n", name);
        return;
    }
    
    printf("%s = ", name);
    if (li_is_negative(num)) {
        printf("-");
    }
    
    // Вывод в шестнадцатеричном виде для наглядности
    printf("0x");
    for (size_t i = num->data[0]; i > 0; i--) {
        printf("%08X", num->data[i]);
    }
    
    // Попытка преобразовать в строку (если число небольшое)
    char *str = li_to_string(num);
    if (str) {
        printf(" (десятичное: %s)", str);
        free(str);
    }
    
    printf("\n");
}

int main() {
    
    // Базовые операции
    printf("Тест 1: Создание и базовые операции\n");
    LongInt *a = li_from_int(12345);
    LongInt *b = li_from_int(67890);
    
    print_number("a", a);
    print_number("b", b);
    
    // Сложение
    printf("\nТест 2: Сложение\n");
    LongInt *sum = li_add(a, b);
    print_number("a + b", sum);
    li_free(sum);
    
    // Вычитание
    printf("\nТест 3: Вычитание\n");
    LongInt *diff = li_sub(a, b);
    print_number("a - b", diff);
    li_free(diff);
    
    // Умножение в столбик
    printf("\nТест 4: Умножение в столбик\n");
    LongInt *prod = li_mul(a, b);
    print_number("a * b (столбик)", prod);
    li_free(prod);
    
    // Умножение Карацубой
    printf("\nТест 5: Умножение Карацубой\n");
    LongInt *prod_karatsuba = li_karatsuba_mul(a, b);
    print_number("a * b (Карацуба)", prod_karatsuba);
    li_free(prod_karatsuba);
    
    // Работа с отрицательными числами
    printf("\nТест 6: Отрицательные числа\n");
    LongInt *neg = li_from_int(-12345);
    print_number("neg", neg);
    
    LongInt *neg_sum = li_add(neg, b);
    print_number("neg + b", neg_sum);
    li_free(neg_sum);
    
    li_free(neg);
    
    
    li_free(a);
    li_free(b);
    
    return 0;
}