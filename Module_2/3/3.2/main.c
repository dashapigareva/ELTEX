#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "network.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Ошибка: неверное количество аргументов!\n");
        printf("Использование: %s <IP шлюза> <Маска подсети> <Количество пакетов>\n", argv[0]);
        printf("Пример: %s 192.168.1.1 255.255.255.0 10\n", argv[0]);
        return 1;
    }
    const char *gw_ip_str = argv[1];
    const char *mask_str = argv[2];
    int N = atoi(argv[3]);

    if (N <= 0) {
        printf("Ошибка: количество пакетов должно быть больше нуля.\n");
        return 1;
    }
    uint32_t gw_ip = parse_ip_to_uint32(gw_ip_str);
    uint32_t mask = parse_ip_to_uint32(mask_str);
    srand(time(NULL));
    int same_subnet_count = 0;
    int other_subnet_count = 0;

    printf("======================================\n");
    printf("IP шлюза: %s\n", gw_ip_str);
    printf("Маска:    %s\n", mask_str);
    printf("Имитация отправки пакетов: %d шт.\n", N);
    printf("======================================\n\n");
    for (int i = 0; i < N; i++) {
        uint32_t dest_ip = generate_random_ip();
        bool is_same = is_same_subnet(gw_ip, dest_ip, mask);
        
        if (is_same) {
            same_subnet_count++;
        } else {
            other_subnet_count++;
        }
        if (N <= 20) {
            char dest_ip_str[IP_STR_LEN];
            uint32_to_ip_str(dest_ip, dest_ip_str);
            
            printf("Пакет %2d | Назначение: %-15s | Маршрут: %s\n", 
                   i + 1, dest_ip_str, is_same ? "СВОЯ ПОДСЕТЬ" : "ДРУГАЯ СЕТЬ");
        }
    }

    if (N > 20) {
        printf("[!] Обработка завершена (детальный по-пакетный вывод скрыт для N > 20)\n\n");
    } else {
        printf("\n");
    }
    double same_pct = ((double)same_subnet_count / N) * 100.0;
    double other_pct = ((double)other_subnet_count / N) * 100.0;

    printf("============= СТАТИСТИКА =============\n");
    printf("Всего отправлено пакетов: %d\n", N);
    printf("Узлам 'своей' подсети:    %d шт. (%.2f%%)\n", same_subnet_count, same_pct);
    printf("Узлам других сетей:       %d шт. (%.2f%%)\n", other_subnet_count, other_pct);
    printf("======================================\n");

    return 0;
}