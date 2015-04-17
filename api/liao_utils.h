#ifndef _LIAO_UTILS_H
#define _LIAO_UTILS_H

int set_to_mc(char *mc_ip, char *mc_port, char *mc_key, char *value);
int get_to_mc(char *mc_ip, int mc_port, char *mc_key, char *mc_value, size_t mc_value_size);


#endif
