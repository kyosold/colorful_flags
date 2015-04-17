#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wand/MagickWand.h>
//#include <libmemcached/memcached.h>
#include "liao_log.h"
#include <sys/epoll.h>  
#include "liao_utils.h"
#include "liao_server.h"

#include "confparser.h"
#include "dictionary.h"

extern char mc_timeout[512];
extern char max_works[512];
extern struct clients *client_st;
extern dictionary *online_d;
extern struct confg_st config_st;


void sig_catch(int sig, void (*f) () )
{
    struct sigaction sa; 
    sa.sa_handler = f;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(sig, &sa, (struct sigaction *) 0); 
}


/*
// return:
// 	0:succ	1:fail
int set_to_mc(char *mc_ip, int mc_port, char *mc_key, char *mc_value)
{
	size_t nval = 0;
    uint32_t flag = 0;
    char *result = NULL;
    int ret = 0;
    
    memcached_st        *memc = memcached_create(NULL);
    memcached_return    mrc;
    memcached_server_st *mc_servers;
    
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 1);
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT, atoi(mc_timeout));
    
    mc_servers= memcached_server_list_append(NULL, mc_ip, mc_port, &mrc);
    log_debug("memcached_server_list_append[%d]:%s:%d", mrc, mc_ip, mc_port);
    
    if (MEMCACHED_SUCCESS == mrc) {
    	mrc= memcached_server_push(memc, mc_servers);
    	memcached_server_list_free(mc_servers);
    	if (MEMCACHED_SUCCESS == mrc) {
    		char mc_value_len = strlen(mc_value);
    		mrc = memcached_set(memc, mc_key, strlen(mc_key), mc_value, mc_value_len, 0, (uint32_t)flag);
    		if (MEMCACHED_SUCCESS == mrc) {
    			log_debug("set mc key:%s val:%s succ", mc_key, mc_value);
    			ret = 0;;
    		} else {
    			log_error("set mc key:%s val:%s failed:%s", mc_key, mc_value, memcached_strerror(memc, mrc));
    			ret = 1;
    		}
    		
    		memcached_free(memc);
    		return ret; 
    	}
    } 
    
    log_error("set_to_mc:%s:%d connect fail:%s", mc_ip, mc_port, memcached_strerror(memc, mrc));
	
	memcached_free(memc);
	
	return 1;
}



// return:
// 	0:succ	1:fail
int get_to_mc(char *mc_ip, int mc_port, char *mc_key, char *mc_value, size_t mc_value_size)
{
	size_t nval = 0;
    uint32_t flag = 0;
    char *result = NULL;
    int ret = 0;
    
    memcached_st        *memc = memcached_create(NULL);
    memcached_return    mrc;
    memcached_server_st *mc_servers;
    
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 1);
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT, atoi(mc_timeout));
    
    mc_servers= memcached_server_list_append(NULL, mc_ip, mc_port, &mrc);
    log_debug("memcached_server_list_append[%d]:%s:%d", mrc, mc_ip, mc_port);
    
    if (MEMCACHED_SUCCESS == mrc) {
    	mrc= memcached_server_push(memc, mc_servers);
    	memcached_server_list_free(mc_servers);
    	if (MEMCACHED_SUCCESS == mrc) {
    		result = memcached_get(memc, mc_key, strlen(mc_key), (size_t *)&nval, &flag, &mrc);
    		if (MEMCACHED_SUCCESS == mrc) {
    			log_debug("get mc key:%s val:%s succ", mc_key, result);
    			
    			snprintf(mc_value, mc_value_size, "%s", result);
    			
    			ret = 0;
    		} else {
    			log_error("get mc key:%s val:%s failed:%s", mc_key, mc_value, memcached_strerror(memc, mrc));
    			
    			ret = 1;
    		}
    		
    		if (result != NULL) {
    			free(result);
    			result = NULL;
    		}
    			
    		memcached_free(memc);

    		return ret; 
    	}
    } 
    
    log_error("set_to_mc:%s:%d connect fail:%s", mc_ip, mc_port, memcached_strerror(memc, mrc));
	
	memcached_free(memc);
	
	return 1;
}
*/


void online_dump(dictionary *d, int fd)
{
	int     i;
	int		nwrite = 0;
	char 	buf[1024] = {0};
	if (d==NULL || fd<0) return ;
	if (d->n<1) {
		write(fd, "empty dictionary\n", strlen("empty dictionary\n"));
		return ;
	}
	for (i=0 ; i<d->size ; i++) {
		if (d->key[i]) {
			snprintf(buf, sizeof(buf), "%20s\t[%s]\n", 
														d->key[i],
														d->val[i] ? d->val[i] : "UNDEF");
			int buf_len = strlen(buf);
			int n = buf_len;
			while (n > 0) {
				nwrite = write(fd, buf + buf_len - n, n);
				if (nwrite < n) {
					if (nwrite == -1 && errno != EAGAIN) {
						log_error("write to fd[%d] failed:[%d]%s", fd, errno, strerror(errno));
					}
					break;
				}
				n -= nwrite;
			}
		}
	}
}



void init_clientst_item_with_idx(int idx)
{
	if (idx < 0 )
		return;

	client_st[idx].used = 0;
    client_st[idx].fd = 0;
    memset(client_st[idx].uid, '0', strlen(client_st[idx].uid));
    memset(client_st[idx].ios_token, '0', strlen(client_st[idx].ios_token));

	struct data_st *send_pdt = client_st[idx].send_data;
	struct data_st *recv_pdt = client_st[idx].recv_data;

	if (send_pdt->data) {
		free(send_pdt->data);
		send_pdt->data = NULL;
	}
	send_pdt->data_size = 0;
	send_pdt->data_len = 0;

	if (recv_pdt->data) {
		free(recv_pdt->data);
		recv_pdt->data = NULL;
	}
	recv_pdt->data_size = 0;
	recv_pdt->data_len = 0;

}


// 获取一个新的未使用的在client_st列表中的索引
// return: 
// 	-1 为未得到(可能满了)
int new_idx_from_client_st()
{
	int idx = -1;
	int i = 0;
	for (i=0; i<(atoi(config_st.max_works) + 1); i++) {
		if (client_st[i].used == 0) {
			idx = i;
			break;
		}
	}

	return idx;
}


// 根据uid取所在索引
// return: -1 为不存在
int get_idx_with_uid(char *uid)
{
	int idx = -1;
	int i = 0;
	for (i=0; i<(atoi(config_st.max_works) + 1); i++) {
		if (strcasecmp(client_st[i].uid, uid) == 0) {
			idx = i;
			break;
		}
	}

	return idx;
}


int get_idx_with_sockfd(int sockfd)
{
	int idx = -1;
	int i = 0;
	for (i=0; i<(atoi(config_st.max_works) + 1); i++) {
		if (client_st[i].fd == sockfd && client_st[i].used == 1) {
			idx = i;
			break;
		}
	}

	return idx;
}


void reinit_data_without_free(struct data_st *data_st)
{
	memset(data_st->data, 0, data_st->data_len);
	data_st->data_len = 0;
	data_st->data_used = 0;
}


// 说明:
// 	该函数用于使用非阻塞写数据
// 	参数:
//		client:	为被写的client sturct *
//		buf: 被写的内容指针
//		buf_size: 被写的内容长度
//
// return:
// 	-1: 系统错误
int safe_write(struct clients *client, char *buf, size_t buf_size)
{
	struct data_st *send_pdt = client->send_data;

	// 申请空间
	if (send_pdt->data == NULL) {
		send_pdt->data_size = buf_size + 1;
		send_pdt->data = (char *)calloc(1, send_pdt->data_size);
		if (send_pdt->data == NULL) {
			send_pdt->data_size = 0;
			log_error("calloc failed:%s size[%d]", strerror(errno), buf_size + 1);
			return -1;
		}
		send_pdt->data_len = 0;
	} else {
		int is_remem_succ = 1;
		if (send_pdt->data_size < (buf_size + 1)) {
			size_t new_size = (send_pdt->data_size + (buf_size + 1)) * 3;
			int fail_times = 1;
			char *tmp = send_pdt->data;
			while ( (send_pdt->data = (char *)realloc(send_pdt->data, new_size)) == NULL ) {
				if (fail_times >= 4) {
					log_error("we can't realloc memory. ");

					send_pdt->data = tmp;
					is_remem_succ = 0;
					break;
				}

				log_error("realloc fail times[%d], retry it.", fail_times);
				sleep(1);
			}

			if (is_remem_succ == 1) {
				 send_pdt->data_size = new_size;
			}
		}

		memset(send_pdt->data, 0, send_pdt->data_size);

	}

	int n = snprintf(send_pdt->data, send_pdt->data_size, "%s", buf);
	send_pdt->data_len = n;
	send_pdt->data_used = 0;

	log_debug("add send message to queue: fd[%d] msg len:%d", client->fd, send_pdt->data_len);

	epoll_event_mod(client->fd, EPOLLOUT);

	return 0;
}


void send_apn_cmd(char *ios_token, char *fuid, char *fnick)
{
	char cmd[MAX_LINE] = {0};
	snprintf(cmd, sizeof(cmd), "./push.php '%s' '%s' '%s'", ios_token, fuid, fnick);
	log_debug("APN CMD: %s", cmd);

	FILE *fp;
	char buf[MAX_LINE] = {0};

	fp = popen(cmd, "r");
	if (fp == NULL) {
		log_error("popen failed:%s", strerror(errno));
		return;
	}

	while ( fgets(buf, sizeof(buf), fp) != NULL ) {
		log_info("%s", buf);
	}

	pclose(fp);
}



// 检查目录是否存在,如不存在则创建
int is_dir_exits(char *path)
{
	char dir_name[MAX_LINE] = {0};
	strcpy(dir_name, path);

	int i, len = strlen(dir_name);
	if (dir_name[len - 1] != '/') 
		strcat(dir_name, "/");

	for (i = 1; i<len; i++) {
		if (dir_name[i] == '/') {
			dir_name[i] = '\0';
			if ( access(dir_name, F_OK) != 0 ) {
				umask(0);
				if ( mkdir(dir_name, 0777) == -1) {
					log_error("create dir:%s fail:%s", dir_name, strerror(errno));
					return 1;
				}
			}

			dir_name[i] = '/';
		}
	}

	return 0;
}


// return fd of fn
FILE *open_img_with_name(char *img_name, char *tuid, char *fn, size_t fn_size, char *img_full_path, size_t img_full_path_size, char *img_full_host, size_t img_full_host_size)
{
	FILE *fp;
	char fn_pid[MAX_LINE] = {0};
    int i=0, fd = -1;
    static int seq = 0; 

	snprintf(img_full_path, img_full_path_size, "%s/%s/images/", IMG_PATH, tuid);
	snprintf(img_full_host, img_full_host_size, "%s/%s/images/", IMG_HOST, tuid);

	if ( is_dir_exits(img_full_path) != 0 ) {
        log_error("is_dir_exit fail");
        return NULL;
	}

    (void)umask(033);
    srandom(time(NULL));

    for (i= 0; i< 100; i++) 
    {    
		snprintf(fn, fn_size, "%.5ld%.5u%.3d_%s", time(NULL), getpid(), seq++, img_name);

        snprintf(fn_pid, sizeof(fn_pid), "%s/%s", img_full_path, fn);

		fd = open(fn_pid, O_WRONLY | O_EXCL | O_CREAT, 0755);
        if (fd != -1)
        {    
            fp = fdopen(fd, "wb");
            if(fp)
            {    
                setbuf(fp, fb_pid);
				return fp;	
            }    
            close(fd);
        }    
        log_info("open file:(%s) error:%m", fn_pid);
    }    


    return NULL;
}


// 返回: 
// 		0:succ 其它: 失败
// 这里有两个文件:
// content.txt: 保存聊天消息
// content_idx.txt: 保存聊天消息的索引:
// 		每行一条消息: start_offset, length, file
//
//	queue_path/tuid/
int write_content_to_file_with_uid(char *tuid, char *content, size_t content_len, char *file_name, size_t file_name_size)
{
	FILE *fp = NULL;
	int i = 0;
	int fd = 0;
	char file_path[MAX_LINE] = {0};
	char file_content[MAX_LINE] = {0};
	int seq = 0;

	int n = snprintf(file_path, sizeof(file_path), "%s/%s/queue/", config_st.queue_path, tuid);
	log_debug("queue file path:%s", file_path);

	// 检查目录是否存在
	if ( is_dir_exits(file_path) != 0) {
		log_error("is_dir_exit fail");
		return 1;
	}

	(void)umask(033);
	srandom(time(NULL));

	for (i= 0; i< 100; i++) {
		n = snprintf(file_content, sizeof(file_content), "%s/%.5ld%.5u%.3d", file_path,  time(NULL), getpid(), seq++);
		if (n <= 0) {
			log_error("create file_content fail");
			return 1;
		}	

		if (access(file_content, F_OK) == 0 ) {
			continue;
		}

		fd = open(file_content, O_WRONLY | O_EXCL | O_CREAT, 0755);
		if (fd != -1) {
			fp = fdopen(fd, "wb");
			if(fp) {
				break;
			}
			close(fd);
		}
		log_error("open file:(%s) error:%m", file_content);
	}

	n = fwrite(content, content_len, 1, fp);
	if (n != 1) {
		log_error("fwrite to file:%s fail:%s", file_content, strerror(errno));
		
		fclose(fp);
		return 1;	
	}

	fclose(fp);
	fp = NULL;

	n = snprintf(file_name, file_name_size, "%s", file_content);

	return 0;
}

int write_content_to_file_with_path(char *file_path, char *content, size_t content_len, char *file_name, size_t file_name_size)
{
	FILE *fp = NULL;
	int i = 0;
	int n = 0;
	int fd = 0;
	char file_full_path[MAX_LINE] = {0};
	int seq = 0;

	log_debug("queue file path:%s", file_path);

	// 检查目录是否存在
	if ( is_dir_exits(file_path) != 0) {
		log_error("is_dir_exit fail");
		return 1;
	}

	(void)umask(033);
	srandom(time(NULL));

	for (i= 0; i< 100; i++) {
		n = snprintf(file_name, file_name_size, "%.5ld%.5u%.3d", time(NULL), getpid(), seq++);
		if (n <= 0) {
			log_error("create file_name fail");
			return 1;
		}

		n = snprintf(file_full_path, sizeof(file_full_path), "%s/%s", file_path, file_name);
		if (n <= 0) {
			log_error("create file_content fail");
			return 1;
		}	

		if (access(file_full_path, F_OK) == 0 ) {
			continue;
		}

		fd = open(file_full_path, O_WRONLY | O_EXCL | O_CREAT, 0755);
		if (fd != -1) {
			fp = fdopen(fd, "wb");
			if(fp) {
				break;
			}
			close(fd);
		}
		log_error("open file:(%s) error:%m", file_full_path);
	}

	n = fwrite(content, content_len, 1, fp);
	if (n != 1) {
		log_error("fwrite to file:%s fail:%s", file_full_path, strerror(errno));
		
		fclose(fp);
		return 1;	
	}

	fclose(fp);
	fp = NULL;

	return 0;
}

int has_offline_msg(char *myuid)
{
	char file_path[MAX_LINE] = {0};
    char file_content_idx[MAX_LINE] = {0};

    int n = snprintf(file_path, sizeof(file_path), "%s/%s", config_st.queue_path, myuid);
    n = snprintf(file_content_idx, sizeof(file_content_idx), "%s/%s", file_path, OFFLINE_MSG_CONENT_IDX);
    if (n <= 0) {
        log_error("create file_content_idx fail");
        return 1;
    }   

	if ( access(file_content_idx, F_OK) == 0 ) {
		return 1;
	}

	return 0;
}

// return:
// 	0: succ		1:fail
char *get_offline_msg_with_uid(char *myuid) 
{
	char file_path[MAX_LINE] = {0};
	char file_content[MAX_LINE] = {0};
	char file_content_idx[MAX_LINE] = {0};

	int n = snprintf(file_path, sizeof(file_path), "%s/%s", config_st.queue_path, myuid);

	n = snprintf(file_content, sizeof(file_content), "%s/%s", file_path, OFFLINE_MSG_CONTENT); 
	if (n <= 0) {
		log_error("create file_content fail");
		return NULL;
	}

	n = snprintf(file_content_idx, sizeof(file_content_idx), "%s/%s", file_path, OFFLINE_MSG_CONENT_IDX);
	if (n <= 0) {
		log_error("create file_content_idx fail");
		return NULL;
	}

	struct stat st;
	if ( stat(file_content, &st) == -1 ) {
		log_error("stat file:%s fail:%s", file_content, strerror(errno));
		
		return NULL;
	}

	char *pmsg = (char *)calloc(1, st.st_size + 10);
	if (pmsg == NULL) {
		log_error("calloc %d fail:%s", st.st_size + 10, strerror(errno));

		return NULL;
	}

	FILE *fpc = fopen(file_content, "r");	
	if (fpc == NULL) {
		log_error("fopen file:%s fail:%s", file_content, strerror(errno));
		return NULL;
	}

	n = fread(pmsg, st.st_size, 1, fpc);
	if (n != 1) {
		log_error("fread file:%s fail", file_content, strerror(errno));
		
		fclose(fpc);
		return NULL;
	}

	fclose(fpc);

	// 清除文件
	//unlink(file_content);
	//unlink(file_content_idx);

	return pmsg;
}



void ThrowWandException(MagickWand *wand)
{
	char *description;
	ExceptionType severity;

	description = MagickGetException(wand, &severity);
	log_error("%s %s %lu %s\n", GetMagickModule(), description);
	description = (char *)MagickRelinquishMemory(description);

}

int image_resize_v1(char *old_img, char *new_img, int w, int h, int compress_quality)
{
	if (*old_img == '\0') {
		return 1;
	}

	MagickBooleanType status;
	MagickWand *magick_wand;

	// init magick_wand.
	MagickWandGenesis();
	magick_wand = NewMagickWand();	

	// Read an image.
	status = MagickReadImage(magick_wand, old_img);
	if (status == MagickFalse) {
		ThrowWandException(magick_wand);
		goto FAIL;
	}

	// Get the image's width and height
	// int width = MagickGetImageWidth(magick_wand);
	// int height = MagickGetImageHeight(magick_wand);
	// fprintf(stderr, "get image w:%d h:%d\n", width, height);

	MagickResizeImage(magick_wand, w, h, LanczosFilter, 1.0);
	
	// set the compression quality to 95 (high quality = low compression)
	MagickSetImageCompressionQuality(magick_wand, compress_quality);

	// Write the image then destroy it.
	status = MagickWriteImages(magick_wand, new_img, MagickTrue);
	if (status == MagickFalse) {
		ThrowWandException(magick_wand);
		goto FAIL;
	}

	// Clean up
	if (magick_wand) {
		magick_wand = DestroyMagickWand(magick_wand);
	}
	MagickWandTerminus();

	return 0;

FAIL:
	// Clean up
	if (magick_wand) {
        magick_wand = DestroyMagickWand(magick_wand);
    }   
    MagickWandTerminus();	

	return 1;
	
}

int image_resize_without_scale(char *old_img, char *new_img, int compress_quality)
{
	if (*old_img == '\0') {
		return 1;
	}

	MagickBooleanType status;
	MagickWand *magick_wand;

	// init magick_wand.
	MagickWandGenesis();
	magick_wand = NewMagickWand();	

	// Read an image.
	status = MagickReadImage(magick_wand, old_img);
	if (status == MagickFalse) {
		ThrowWandException(magick_wand);
		goto FAIL;
	}

	// Get the image's width and height
	int width = MagickGetImageWidth(magick_wand);
	int height = MagickGetImageHeight(magick_wand);
	log_debug("get image w:%d h:%d", width, height);
	
	float new_w = 139.0;
	float new_h = 139.0;
	int w = 0;
	int h = 0;
	if ( (new_w / new_h) > (width / height) ) {
		w = new_h * width / height;
		h = new_h;	
	} else {
		w = new_w;
		h = new_w * height / width;
	}
	log_debug("get thumb w:%d h:%d", w, h);

	MagickResizeImage(magick_wand, w, h, LanczosFilter, 1.0);
	
	// set the compression quality to 95 (high quality = low compression)
	MagickSetImageCompressionQuality(magick_wand, compress_quality);

	// Write the image then destroy it.
	status = MagickWriteImages(magick_wand, new_img, MagickTrue);
	if (status == MagickFalse) {
		ThrowWandException(magick_wand);
		goto FAIL;
	}

	// Clean up
	if (magick_wand) {
		magick_wand = DestroyMagickWand(magick_wand);
	}
	MagickWandTerminus();

	return 0;

FAIL:
	// Clean up
	if (magick_wand) {
        magick_wand = DestroyMagickWand(magick_wand);
    }   
    MagickWandTerminus();	

	return 1;
	
}

