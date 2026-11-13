#ifndef NETWORK_H
#define NETWORK_H
#include <stdint.h>
#include <stdbool.h>
#define IP_STR_LEN 16
uint32_t parse_ip_to_uint32(const char *ip_str);
void uint32_to_ip_str(uint32_t ip, char *ip_str);
uint32_t generate_random_ip();
bool is_same_subnet(uint32_t ip1, uint32_t ip2, uint32_t mask);
#endif