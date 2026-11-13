#include "network.h"
#include <stdio.h>
#include <stdlib.h>

uint32_t parse_ip_to_uint32(const char *ip_str) {
    unsigned int a, b, c, d;
    if (sscanf(ip_str, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
        fprintf(stderr, "Ошибка: неверный формат IP-адреса (%s)\n", ip_str);
        exit(1);
    }
    //Склеиваем 4 октета (байта) в одно 32-битное число. Затем объединяем их побитовым ИЛИ
    return (a << 24) | (b << 16) | (c << 8) | d;
}

void uint32_to_ip_str(uint32_t ip, char *ip_str) {
    unsigned int a = (ip >> 24) & 0xFF;
    unsigned int b = (ip >> 16) & 0xFF;
    unsigned int c = (ip >> 8) & 0xFF;
    unsigned int d = ip & 0xFF;
    sprintf(ip_str, "%u.%u.%u.%u", a, b, c, d);
}

uint32_t generate_random_ip() {
    uint32_t a = rand() % 256;
    uint32_t b = rand() % 256;
    uint32_t c = rand() % 256;
    uint32_t d = rand() % 256;
    
    return (a << 24) | (b << 16) | (c << 8) | d;
}

bool is_same_subnet(uint32_t ip1, uint32_t ip2, uint32_t mask) {
    return (ip1 & mask) == (ip2 & mask);
}