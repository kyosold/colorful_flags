#ifndef _LIAO_COMMANDS_H
#define _LIAO_COMMANDS_H

void commands(struct clients *client_t);
void status_cmd(int sockfd, char *pbuf, size_t pbuf_size);
//int helo_cmd(int sockfd, char *pbuf, size_t pbuf_size, char *myuid, size_t myuid_size);
//int sendtxt_cmd(int sockfd, char *pbuf, size_t pbuf_size);
//int sendimg_cmd(int sockfd, char *pbuf, size_t pbuf_size);

int helo_cmd(int sockfd, char *pbuf, size_t pbuf_size, char *myuid, size_t myuid_size);
int sendtxt_cmd(int sockfd, char *pbuf, size_t pbuf_size, char *mid, size_t mid_size, int client_idx);
int sendimg_cmd(int sockfd, char *pbuf, size_t pbuf_size, char *mid, size_t mid_size, int client_idx, char *img_full_path, size_t img_full_path_size, char *img_name, size_t img_name_size);
int quit_cmd(int sockfd, char *pbuf, size_t pbuf_size);

int get_offline_msg(int sockfd, char *pbuf, size_t pbuf_size);

#endif
