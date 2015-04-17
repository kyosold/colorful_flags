#ifndef _LIAO_UTILS_H
#define _LIAO_UTILS_H

#include "confparser.h"
#include "dictionary.h"
#include "liao_server.h"

int set_to_mc(char *mc_ip, int mc_port, char *mc_key, char *value);
int get_to_mc(char *mc_ip, int mc_port, char *mc_key, char *mc_value, size_t mc_value_size);

void sig_catch(int sig, void (*f) () );

void init_clientst_item_with_idx(int idx);
int new_idx_from_client_st();
int get_idx_with_uid(char *uid);
int get_idx_with_sockfd(int sockfd);

void reinit_data_without_free(struct data_st *data_st);
int safe_write(struct clients *client, char *buf, size_t buf_size);

void online_dump(dictionary *d, int fd);
void send_apn_cmd(char *ios_token, char *fuid, char *fnick);

//FILE *open_img_with_name(char *img_name, char *tuid, char *fn, size_t fn_size, char *img_full_path, size_t img_full_path_size);
FILE *open_img_with_name(char *img_name, char *tuid, char *fn, size_t fn_size, char *img_full_path, size_t img_full_path_size, char *iname, size_t iname_size);

int has_offline_msg(char *myuid);
//int write_content_to_file_with_uid(char *tuid, char *content, size_t content_len);
int write_content_to_file_with_uid(char *tuid, char *content, size_t content_len, char *file_name, size_t file_name_size);
int write_content_to_file_with_path(char *file_path, char *content, size_t content_len, char *file_name, size_t file_name_size);

char *get_offline_msg_with_uid(char *myuid);

int image_resize_v1(char *old_img, char *new_img, int w, int h, int compress_quality);
int image_resize_without_scale(char *old_img, char *new_img, int compress_quality);

#endif
