#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <libmemcached/memcached.h>
#include "mysql.h"
#include <sys/epoll.h>  
#include <curl/curl.h>
#include <wand/MagickWand.h>

#include "liao_log.h"
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

int get_avatar_url(char *uid, char *url, size_t url_size)
{
	int n = snprintf(url, url_size, "%s/%s/avatar/s_avatar.jpg", DEF_QUEUEHOST, uid);
	return n;
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

	log_debug("add send message to queue: fd[%d] msg len:%d:%s", client->fd, send_pdt->data_len, send_pdt->data);

	epoll_event_mod(client->fd, EPOLLOUT);

	return 0;
}


int send_socket_cmd(struct clients *client_t, char *fuid, char *fnick, char *type, char *tuid)
{
	char b64_nick[MAX_LINE] = {0};
	int b64_n = base64_decode(fnick, b64_nick, MAX_LINE);
	if (b64_n <= 0) {
		log_error("base64_decode fail");
		return 1;
	}
	

	char notice_mssage[MAX_LINE * 5] = {0};
	int n = snprintf(notice_mssage, sizeof(notice_mssage), "您收到一条来自 %s 的消息", b64_nick);
	if (n <= 0) {
		log_error("snprintf fail");	
		return 1;
	}

	int notice_msg_size = n * 5;
	char *pnotice_msg = (char *)calloc(1, notice_msg_size);
	if (pnotice_msg == NULL) {
		log_error("calloc faile");
		return 1;
	}

	int b64_size = base64_encode(notice_mssage, n, pnotice_msg, notice_msg_size);
	if (b64_size < 0) {
		log_error("base64_encode fail:%s", notice_mssage);

		clean_mem_buf(pnotice_msg);
		notice_msg_size = 0;

		return 1;		
	}

	int notice_size = strlen(type) + strlen(fuid) + strlen(fnick) + strlen(tuid) + b64_size + 512;
	char *pnotice = (char *)calloc(1, notice_size);
	if (pnotice != NULL) {
		n = snprintf(pnotice, notice_size, "MESSAGENOTICE %s %s %s %s %s %s", type, fuid, fnick, tuid, pnotice_msg, DATA_END);

		safe_write(client_t, pnotice, n);

		clean_mem_buf(pnotice);
		notice_size = 0;
	}

	clean_mem_buf(pnotice_msg);
	notice_msg_size = 0;

	return 0;
		
}


void send_apn_cmd(char *ios_token, char *fuid, char *fnick, char *type, char *tuid)
{
	char cmd[MAX_LINE] = {0};
	snprintf(cmd, sizeof(cmd), "./push.php '%s' '%s' '%s' '%s' '%s'", ios_token, fuid, fnick, type, tuid);
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
// 	 -1: 失败, other: 写入的字节数
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
		return -1;
	}

	(void)umask(033);
	srandom(time(NULL));

	for (i= 0; i< 100; i++) {
		n = snprintf(file_content, sizeof(file_content), "%s/%.5ld%.5u%.3d", file_path,  time(NULL), getpid(), seq++);
		if (n <= 0) {
			log_error("create file_content fail");
			return -1;
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
		return -1;	
	}

	fclose(fp);
	fp = NULL;

	snprintf(file_name, file_name_size, "%s", file_content);

	return content_len;
}

int write_content_to_file_with_path(char *file_path, char *old_name, char *content, size_t content_len, char *file_name, size_t file_name_size)
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
		return -1;
	}

	(void)umask(033);
	srandom(time(NULL));

	for (i= 0; i< 100; i++) {
		if (*old_name && (strlen(old_name) > 0)) {
			n = snprintf(file_name, file_name_size, "%.5ld%.5u%.3d_%s", time(NULL), getpid(), seq++, old_name);
		} else {
			n = snprintf(file_name, file_name_size, "%.5ld%.5u%.3d", time(NULL), getpid(), seq++);
		}
		if (n <= 0) {
			log_error("create file_name fail");
			return -1;
		}

		n = snprintf(file_full_path, sizeof(file_full_path), "%s/%s", file_path, file_name);
		if (n <= 0) {
			log_error("create file_content fail");
			return -1;
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

	return content_len;
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


// http api -----------------------------------
struct curl_return_string {
    char *str;
    size_t len;
    size_t size;
}; // 用于存curl返回的结果

size_t _recive_data_from_http_api(void *buffer, size_t size, size_t nmemb, void *user_p)
{
    struct curl_return_string *result_t = (struct curl_return_string *)user_p;

    if (result_t->size < ((size * nmemb) + 1)) {
        result_t->str = (char *)realloc(result_t->str, (size * nmemb) + 1);
        if (result_t->str == NULL) {
            return 0;
        }
        result_t->size = (size * nmemb) + 1;
    }

    result_t->len = size * nmemb;
    memcpy(result_t->str, buffer, result_t->len);
    result_t->str[result_t->len] = '\0';

    return result_t->len;
}

void clean_mem_buf(char *buf)
{
	if (buf != NULL) {
		free(buf);
		buf = NULL;
	}
}


char *login_http_api(char *url, char *account, char *password, char *ios_token, int get_friend_list, int *result_len)
{
	// 声明保存返回 http 的结果
	struct curl_return_string curl_result_t;

	char *presult = NULL;
	*result_len = 0;

	curl_result_t.len = 0;
    curl_result_t.str = (char *)calloc(10, 1024);
	if (curl_result_t.str == NULL) {
		log_error("calloc fail for curl_result_t.str");
		return NULL;
	}	
	curl_result_t.size = 10 * 1024;

	curl_global_init(CURL_GLOBAL_ALL);

	CURL *curl;
	CURLcode ret;

	// init curl
	curl = curl_easy_init();
	if (!curl) {
		log_error("couldn't init curl");
		goto FAIL;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POST, 1);    // use post 

	// urlencode post data
	char *account_encode = curl_easy_escape(curl, account, strlen(account));
	if (!account_encode) {
		log_error("urlencode account fail, so use source data");
		account_encode = account;
	}

	char *password_encode = curl_easy_escape(curl, password, strlen(password));
	if (!password_encode) {
		log_error("urlencode password fail, so use source data");
		password_encode = password;
	}

	char *ios_token_encode = curl_easy_escape(curl, ios_token, strlen(ios_token));
	if (!ios_token_encode) {
		log_error("urlencode ios_token_encode fail, so use source data");
		ios_token_encode = ios_token;
	}

	int request_data_len = 8 + strlen(account_encode) + 10 +strlen(password_encode) + 11 + strlen(ios_token_encode) + 50;
	char *request_data = (char *)calloc(1, request_data_len) ;
	if (request_data == NULL) {
		log_error("calloc fail for request_data");
		curl_easy_cleanup(curl);
		curl_global_cleanup();

		goto FAIL;
	}

	snprintf(request_data, request_data_len, "account=%s&password=%s&ios_token=%s&get_friend_list=%d", account_encode, password_encode, ios_token_encode, get_friend_list);
	log_debug("request data:%s\n", request_data);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _recive_data_from_http_api);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_result_t);  // 传这个参数过去

	ret = curl_easy_perform(curl);
	if(ret != CURLE_OK) {
		log_error("curl_easy_perform() failed:%s, url:%s", curl_easy_strerror(ret), url);

		curl_easy_cleanup(curl);
		curl_global_cleanup();
	} else {
		if (curl_result_t.str) {
			log_info("request url:%s response:[%d]%s", url, curl_result_t.len, curl_result_t.str);

			// return is json data
			presult = (char *)calloc(1, curl_result_t.len + 1);
			if (presult == NULL) {
				log_error("calloc presult fail:%s", strerror(errno));
				goto FAIL;
			}		
			*result_len = snprintf(presult, curl_result_t.len + 1, "%s", curl_result_t.str);

			goto SUCC;
		}
	}



FAIL:
	clean_mem_buf(curl_result_t.str);
	curl_result_t.len = 0;
	curl_result_t.size = 0;

	clean_mem_buf(account_encode);
	clean_mem_buf(password_encode);
	clean_mem_buf(ios_token_encode);

	clean_mem_buf(request_data);

	return NULL;


SUCC:
	clean_mem_buf(curl_result_t.str);
	curl_result_t.len = 0;
	curl_result_t.size = 0;

	clean_mem_buf(account_encode);
	clean_mem_buf(password_encode);
	clean_mem_buf(ios_token_encode);

	clean_mem_buf(request_data);

	return presult;
}


// Return:
//	-1: error
//	other: mysql id
int write_queue_to_db(char *tag_type, char *fuid, char *fnick, char *fios_token, char *tuid, char *tios_token, char *queue_type, char *queue_file, int queue_size)
{
    log_debug("quar_mysql:%s mysql_user:%s mysql_pass:%s mysql_db:%s mysql_port:%d", config_st.mysql_host, config_st.mysql_user, config_st.mysql_passwd, config_st.mysql_db, config_st.mysql_port);

    MYSQL mysql, *mysql_sock;
    MYSQL_RES* res = NULL;
    char sql[5 * MAX_LINE] = {0};
    
    mysql_init(&mysql);
    if (!(mysql_sock = mysql_real_connect(&mysql, config_st.mysql_host, config_st.mysql_user, config_st.mysql_passwd, config_st.mysql_db, config_st.mysql_port, NULL, 0))) {
        log_error("connect mysql fail");
        return -1;
    }   
    
    // addslash
    char mysql_tag_type[MAX_LINE] = {0}; 
    char mysql_fnick[MAX_LINE] = {0}; 
    char mysql_fios_token[MAX_LINE] = {0}; 
    char mysql_tios_token[MAX_LINE] = {0}; 
    char mysql_queue_type[MAX_LINE] = {0}; 
    char mysql_queue_file[MAX_LINE] = {0}; 

    mysql_real_escape_string(mysql_sock, mysql_tag_type, tag_type, strlen(tag_type));
    mysql_real_escape_string(mysql_sock, mysql_fnick, fnick, strlen(fnick));
    mysql_real_escape_string(mysql_sock, mysql_fios_token, fios_token, strlen(fios_token));
    mysql_real_escape_string(mysql_sock, mysql_tios_token, tios_token, strlen(tios_token));
    mysql_real_escape_string(mysql_sock, mysql_queue_type, queue_type, strlen(queue_type));
    mysql_real_escape_string(mysql_sock, mysql_queue_file, queue_file, strlen(queue_file));

	// get current time
	struct tm when;
	time_t now_time;
	time(&now_time);
	when = *localtime(&now_time);
	char cdate[21] = {0};
	snprintf(cdate, sizeof(cdate), "%04d-%02d-%02d %02d:%02d:%02d\n", when.tm_year + 1900, when.tm_mon + 1, when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec); 
    
    snprintf(sql, sizeof(sql), "insert into liao_queue (tag_type, fuid, fnick, fios_token, tuid, tios_token, queue_type, queue_file, queue_size, cdate, expire) "
                                                    "values ('%s', %d, '%s', '%s', %d, '%s', '%s', '%s', %d, '%s', 0);",
                                                    mysql_tag_type, atoi(fuid), mysql_fnick, mysql_fios_token, atoi(tuid), mysql_tios_token, mysql_queue_type, mysql_queue_file, queue_size, cdate); 
    log_debug("sql:%s", sql);
    
    if (mysql_query(mysql_sock, sql)) {
        log_error("insert mysql liao_queue fail:%s", mysql_error(mysql_sock));
        mysql_close(mysql_sock);
    
        return -1;
    }   

	int mid = mysql_insert_id(mysql_sock);
    
    mysql_close(mysql_sock);
    
    return mid;
}



