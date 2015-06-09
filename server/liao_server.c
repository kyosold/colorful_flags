/*
*
*	command:
*		<命令> <内容>
*
*		命令:
*			HELO: 用于表示客户端自己的信息		helo uid:0000010 ios_token:xxxxxxxxxxxx
*
*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>  
#include <sys/epoll.h>  
#include <sys/sendfile.h>  
#include <sys/wait.h>  
#include <netinet/in.h>  
#include <netinet/tcp.h>  
#include <arpa/inet.h>  
#include <strings.h>  

#include "confparser.h"
#include "dictionary.h"

#include "liao_log.h"
#include "liao_utils.h"
#include "liao_server.h"

#define CFG_FILE		"./liao_config.ini"

dictionary *dict_conf = NULL;


struct clients *client_st;
dictionary *online_d;
struct confg_st config_st;


// global config
//int bind_port = 5050;
//char mc_server[MAX_LINE] = "127.0.0.1";
//int mc_port = 11211;
//char mc_timeout[512] = "5000";
//char max_works[512] = "512";
// default global config


// epoll
int epoll_fd = -1; 
int epoll_nfds = -1; 
int epoll_event_num = 0;
struct epoll_event *epoll_evts = NULL;
int epoll_num_running = 0;
int listenfd = -1;

// ----------------
dictionary *liao_init_cond(char *conf_file);


// 配置文件操作
dictionary *liao_init_cond(char *conf_file)
{
   dictionary *dict = open_conf_file(conf_file);
   if (dict == NULL) {
       //log_error("errror");
       return NULL;
   }
    
   return dict;
}


void send_to_client(int sockfd, char *rspsoneBuf)
{
	int nwrite = 0;
    int buf_len = strlen(rspsoneBuf);
    int n = buf_len;
    while (n > 0) {
        nwrite = write(sockfd, rspsoneBuf + buf_len - n, n);   
        if (nwrite < n) {
            if (nwrite == -1 && errno != EAGAIN) {
                log_error("write to fd[%d] failed:[%d]%s", sockfd, errno, strerror(errno));
            }   
            break;
        }   

        n -= nwrite;
    }   

	log_debug("send fd[%d]:%s", sockfd, rspsoneBuf);

}


// 逻辑处理
void command(int sockfd, char *buf, size_t buf_size, int client_idx)
{
	int ci = client_idx;
	int i = 0;

	// System Command
	if ( strcasecmp(buf, "sys_status") == 0 ) {
		status_cmd(sockfd, buf, strlen(buf));

		return;
	}


	char rspsoneBuf[512] = {0};
	
	// HELO: 用于表示客户端信息	helo uid:0000001 ios_token:xxxxxxxxxxxxxx fid:0000220
	if ( strncasecmp(buf, "helo ", 5) == 0 ) {
		char *pbuf = buf + 5;
		
		char myuid[128] = {0};
		int ret = helo_cmd(sockfd, pbuf, buf_size - 5, myuid, sizeof(myuid));
		if (ret == 0) {
			// 检查是否有离线消息
			int offline_msg_num = has_offline_msg(myuid);

			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK %d%s", TAG_HELO, offline_msg_num, DATA_END);
			send_to_client(sockfd, rspsoneBuf);

		} else {
			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL%s", TAG_HELO, DATA_END);
			send_to_client(sockfd, rspsoneBuf);

		}
	
		return;
	}

/*
	// GETMSGOFFLINE: 用于表示客户端获取离线消息
	if ( strncasecmp(buf, "getmsgoffline", 13) == 0 ) {
		char *pbuf = buf + 13;
	
		int ret = get_offline_msg(sockfd, pbuf, buf_size - 13);
		if (ret == 0) {
			
		}
	
	}
*/

	//
	if ( strncasecmp(buf, "SENDADDFRDREQ", 13) == 0 ) {
		char *pbuf = buf + 13;

		char mid[MAX_LINE] = {0};
		int mid_size = MAX_LINE;
		int ret = sendreq_addfriend(sockfd, pbuf, buf_size - 8, mid, mid_size, ci);
		if (ret == 0) {
			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK %s%s", TAG_SENDADDFRIENDREQ, mid, DATA_END);
			send_to_client(sockfd, rspsoneBuf);
		} else {
			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL %s%s", TAG_SENDADDFRIENDREQ, mid, DATA_END);
			send_to_client(sockfd, rspsoneBuf);
		}

		return;
	}


	// SENDTXT: 用于表示发送信息给对方 
	// sendtxt mid:1422868886214 uid:0000001 ios_token:xxxxxxxxxxxxxx fid:0000220 fios_token:ccccccccccccccc body:abcabcabc
	if ( strncasecmp(buf, "sendtxt ", 8) == 0 ) {
		char *pbuf = buf + 8;	
		
		char mid[MAX_LINE] = {0};
		int mid_size = MAX_LINE;
		int ret = sendtxt_cmd(sockfd, pbuf, buf_size - 8, mid, mid_size, ci);
		if (ret == 0) {

			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK %s%s", TAG_SENDTXT, mid, DATA_END);

			send_to_client(sockfd, rspsoneBuf);
		} else {
			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL %s%s", TAG_SENDTXT, mid, DATA_END);
			send_to_client(sockfd, rspsoneBuf);
		}
		
		return;
	}

	// SENDIMG: 用于表示发送图片给对方
	// sendimg mid:1422868886214 uid:0000001 ios_token:xxxxxxxxxxxxxx fid:0000220 fios_token:ccccccccccccccc message:20:abcdabcdabcd...
	if ( strncasecmp(buf, "sendimg ", 8) == 0 ) {
		char *pbuf = buf + 8;
		
		char mid[MAX_LINE] = {0};
		int mid_size = MAX_LINE;
		char img_full_host[MAX_LINE] = {0};
		char img_name[MAX_LINE] = {0};
		int ret = sendimg_cmd(sockfd, pbuf, buf_size - 8, mid, mid_size, ci, img_full_host, sizeof(img_full_host), img_name, sizeof(img_name));
		if (ret == 0) {
		
			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK %s %s %s%s", TAG_SENDIMG, mid, img_full_host, img_name, DATA_END);
			send_to_client(sockfd, rspsoneBuf);

		} else {
			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL %s%s", TAG_SENDIMG, mid, DATA_END);
			send_to_client(sockfd, rspsoneBuf);

		}

		return;
	}
	
	// QUIT: 用于表示客户端退出
	// quit uid:0000001
	if ( strncasecmp(buf, "quit ", 5) == 0 ) {
		char *pbuf = buf + 5;
		
		int ret = quit_cmd(sockfd, pbuf, buf_size - 5);
		if (ret == 0) {
			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK%s", TAG_QUIT, DATA_END);
			send_to_client(sockfd, rspsoneBuf);

		} else {
			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL%s", TAG_QUIT, DATA_END);
			send_to_client(sockfd, rspsoneBuf);
			
		}

		
		// 从epoll中删除，关闭fd
		struct epoll_event ev;
		ev.data.fd = sockfd;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, 0) == -1) {
			log_error("epoll_ctl fd[%d] EPOLL_CTL_DEL failed:[%d]%s", sockfd, errno, strerror(errno));
		}

		close(sockfd);
		epoll_num_running--;
		log_debug("sockfd[%d] close", sockfd);
		
		return ;
	}
}


// 处理配置文件
// return:
// 	0:succ	1:fail
int conf_init(char *cfg_file)
{
	memset(config_st.bind_port, 0, sizeof(config_st.bind_port));
	memset(config_st.log_level, 0, sizeof(config_st.log_level));

	memset(config_st.use_mc, 0, sizeof(config_st.use_mc));
	memset(config_st.mc_server, 0, sizeof(config_st.mc_server));
	config_st.mc_port = 0;
	memset(config_st.mc_timeout, 0, sizeof(config_st.mc_timeout));

	memset(config_st.mysql_host, 0, sizeof(config_st.mysql_host));
	config_st.mysql_port = 0;
	memset(config_st.mysql_user, 0, sizeof(config_st.mysql_user));
	memset(config_st.mysql_passwd, 0, sizeof(config_st.mysql_passwd));
	memset(config_st.mysql_db, 0, sizeof(config_st.mysql_db));
	memset(config_st.mysql_timeout, 0, sizeof(config_st.mysql_timeout));

	memset(config_st.max_works, 0, sizeof(config_st.max_works));
	memset(config_st.queue_path, 0, sizeof(config_st.queue_path));
	memset(config_st.queue_host, 0, sizeof(config_st.queue_host));


	// 检查配置文件是否正确
	if (*cfg_file == '\0') {
		fprintf(stderr, "config file is null\n");
		log_error("config file is null");
		return 1;
	}
	
	struct stat cfg_st;
	if (stat(cfg_file, &cfg_st) == -1) {
		fprintf(stderr, "file '%s' error:%s\n", cfg_file, strerror(errno));
		log_error("file '%s' error:%s", cfg_file, strerror(errno));
		return 1;
	}
	
	// 读取配置文件 
	dict_conf = liao_init_cond(cfg_file);
	if (dict_conf == NULL) {
        fprintf(stderr, "liao_init_cond fail\n");
        log_error("liao_init_cond fail");
        return 1;
    }   

	char *pbind_port = dictionary_get(dict_conf, "global:bind_port", NULL);
	if (pbind_port != NULL) {
		snprintf(config_st.bind_port, sizeof(config_st.bind_port), "%s", pbind_port);
	} else {
		log_warning("parse config bind_port error, use defaut:%s", DEF_BINDPORT);
		snprintf(config_st.bind_port, sizeof(config_st.bind_port), "%s", DEF_BINDPORT);
	}

    char *p_log_level = dictionary_get(dict_conf, "global:log_level", NULL);
    if (p_log_level != NULL) {
        if (strcasecmp(p_log_level, "debug") == 0) {
            log_level = debug;
        } else if (strcasecmp(p_log_level, "info") == 0) {
            log_level = info;
        }   
		snprintf(config_st.log_level, sizeof(config_st.log_level), "%s", p_log_level);
    } else {
		log_warning("parse config log_level error, use defaut:%s", DEF_LOGLEVEL);
		snprintf(config_st.log_level, sizeof(config_st.log_level), "%s", DEF_LOGLEVEL);
	}

	char *puse_mc = dictionary_get(dict_conf, "global:use_mc", NULL);
	if (puse_mc != NULL) {
		snprintf(config_st.use_mc, sizeof(config_st.use_mc), "1");
	} else {
		log_warning("parse config use_mc error, so not use mc");
		snprintf(config_st.use_mc, sizeof(config_st.use_mc), "0");
	}
    
	char *pmc_server = dictionary_get(dict_conf, "global:mc_server", NULL);
	if (pmc_server != NULL) {
		snprintf(config_st.mc_server, sizeof(config_st.mc_server), "%s", pmc_server);
	} else {
		log_warning("parse config mc_server error, use defaut:%s", DEF_MCSERVER);
		snprintf(config_st.mc_server, sizeof(config_st.mc_server), "%s", DEF_MCSERVER);
	}

	char *pmc_port = (char *)memchr(config_st.mc_server, ':', strlen(config_st.mc_server));
	if (pmc_port != NULL) {
		*pmc_port = '\0';
		config_st.mc_port = atoi(pmc_port + 1);
	} else {
		log_warning("parse config mc_port error, use defaut:%s", DEF_MCPORT);
		config_st.mc_port = atoi(DEF_MCPORT);
	}

	char *pmc_timeout = dictionary_get(dict_conf, "global:mc_timeout", NULL);
	if (pmc_timeout != NULL) {
		snprintf(config_st.mc_timeout, sizeof(config_st.mc_timeout), "%s", pmc_timeout);
	} else {
		log_warning("parse config mc_timeout error, use defaut:%s", DEF_MCTIMEOUT);
		snprintf(config_st.mc_timeout, sizeof(config_st.mc_timeout), "%s", DEF_MCTIMEOUT);
	}

	char *pmysql_host = dictionary_get(dict_conf, "global:mysql_host", NULL);
    if (pmysql_host != NULL) {
        snprintf(config_st.mysql_host, sizeof(config_st.mysql_host), "%s", pmysql_host);
    } else {
        log_warning("parse config mc_server error, use defaut:%s", DEF_MYSQLSERVER);
        snprintf(config_st.mysql_host, sizeof(config_st.mysql_host), "%s", DEF_MYSQLSERVER);
    }   

	char *pmysql_port = dictionary_get(dict_conf, "global:mysql_port", NULL);
	if (pmysql_port != NULL) {
		config_st.mysql_port = atoi(pmysql_port);
	} else {
		log_warning("parse config mysql_port error, use default:%s", DEF_MYSQLPORT);
		config_st.mysql_port = atoi(DEF_MYSQLPORT);
	}

	char *pmysql_user = dictionary_get(dict_conf, "global:mysql_user", NULL);
	if (pmysql_user != NULL) {
		snprintf(config_st.mysql_user, sizeof(config_st.mysql_user), "%s", pmysql_user);
	} else {
		log_warning("parse config mysql_user error, use defaut:%s", DEF_MYSQLUSER);
		snprintf(config_st.mysql_user, sizeof(config_st.mysql_user), "%s", DEF_MYSQLUSER);
	}

	char *pmysql_passwd = dictionary_get(dict_conf, "global:mysql_passwd", NULL);
	if (pmysql_passwd != NULL) {
		snprintf(config_st.mysql_passwd, sizeof(config_st.mysql_passwd), "%s", pmysql_passwd);
	} else {
		log_warning("parse config mysql_passwd error, use defaut:%s", DEF_MYSQLPASSWD);
		snprintf(config_st.mysql_passwd, sizeof(config_st.mysql_passwd), "%s", DEF_MYSQLPASSWD);
	}

	char *pmysql_db= dictionary_get(dict_conf, "global:mysql_db", NULL);
	if (pmysql_db != NULL) {
		snprintf(config_st.mysql_db, sizeof(config_st.mysql_db), "%s", pmysql_db);
	} else {
		log_warning("parse config mysql_db error, use defaut:%s", DEF_MYSQLDB);
		snprintf(config_st.mysql_db, sizeof(config_st.mysql_db), "%s", DEF_MYSQLDB);
	}

	char *pmysql_timeout = dictionary_get(dict_conf, "global:mysql_timeout", NULL);
	if (pmysql_timeout != NULL) {
		snprintf(config_st.mysql_timeout, sizeof(config_st.mysql_timeout), "%s", pmysql_timeout);
	} else {
		log_warning("parse config mysql_timeout error, use defaut:%s", DEF_MYSQLTIMEOUT);
		snprintf(config_st.mysql_timeout, sizeof(config_st.mysql_timeout), "%s", DEF_MYSQLTIMEOUT);
	}


	char *pmax_works = dictionary_get(dict_conf, "global:max_works", NULL);
	if (pmax_works != NULL) {
		snprintf(config_st.max_works, sizeof(config_st.max_works), "%s", pmax_works);
	} else {
		log_warning("parse config max_works error, use default:%s", DEF_MAXWORKS);
		snprintf(config_st.max_works, sizeof(config_st.max_works), "%s", DEF_MAXWORKS);
	}

	char *pqueue_path = dictionary_get(dict_conf, "global:queue_path", NULL);
	if (pqueue_path != NULL) {
		snprintf(config_st.queue_path, sizeof(config_st.queue_path), "%s", pqueue_path);
	} else {
		log_warning("parse config queue_path error, use default:%s", DEF_QUEUEPATH);
		snprintf(config_st.queue_path, sizeof(config_st.queue_path), "%s", DEF_QUEUEPATH);
	}	

	char *pqueue_host = dictionary_get(dict_conf, "global:queue_host", NULL);
	if (pqueue_host != NULL) {
		snprintf(config_st.queue_host, sizeof(config_st.queue_host), "%s", pqueue_host);
	} else {
		log_warning("parse config queue_host error, use default:%s", DEF_QUEUEHOST);
		snprintf(config_st.queue_host, sizeof(config_st.queue_host), "%s", DEF_QUEUEHOST);
	}	

	return 0;
}

// return: 0:succ 1:fail
int epoll_event_mod(int sockfd, int type)
{
	struct epoll_event ev;
	ev.data.fd = sockfd;
	ev.events = type;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &ev) == -1) {
    	log_error("epoll_ctl fd[%d] EPOLL_CTL_MOD EPOLLIN failed:[%d]%s", sockfd, errno, strerror(errno));
		return 1;
	} 
	return 0;
}

// 设置fd为非阻塞模式
int setnonblocking(int fd)
{
	int opts;
	opts = fcntl(fd, F_GETFL);
	if (opts < 0) {
		log_error("fcntl F_GETFL failed:[%d]:%s", errno, strerror(errno));
		return 1;
	}

	opts = (opts | O_NONBLOCK);
	if (fcntl(fd, F_SETFL, opts) < 0) {
		log_error("fcntl F_SETFL failed:[%d]:%s", errno, strerror(errno));
		return 1;
	}

	return 0;
}


// 0:succ	1:fail
int resizebuf(char *sbuf, int new_size)
{
    char *new_pbuf = (char *)realloc(sbuf, new_size);
    if (new_pbuf == NULL) {
        log_error("realloc failed:%s", strerror(errno));
            
		return 1;
    } else {
        sbuf = new_pbuf; 

		return 0;
    }  
}


// 0:succ	1:fail
int client_close(int sockfd)
{
	log_debug("fd[%d] closed", sockfd);

	// client close
    int i = get_idx_with_sockfd(sockfd);
    if (i == -1) {
        log_error("get index failed: i[%d] = get_idx_with_sockfd(%d)", i, sockfd);
        return 1;
    }  	
	log_debug("fd[%d] client_st[%d].uid=%s", sockfd, i, client_st[i].uid);

	// 注册为离线   (uid => ios_token)
	dictionary_unset(online_d, client_st[i].uid);
	log_debug("fd[%d] set uid[%s] offline", sockfd, client_st[i].uid);

	// 关闭客户端fd
	close(sockfd);

	// 回收与初始化当前item
	init_clientst_item_with_idx(i);

	return 0;
}




void usage(char *prog)
{
	printf("Usage:\n");
	printf("%s -c[config file]\n", prog);
}


void exit_self(int s) 
{
	log_debug("get sigint for exit self");

	if (epoll_fd != -1) {
		close(epoll_fd);
		epoll_fd = -1;
	}
	if (listenfd != -1) {
		close(listenfd);
		listenfd = -1;
	}
}


int main(int argc, char **argv)
{
	liao_log("liao_log", LOG_PID|LOG_NDELAY, LOG_MAIL);

	if (argc != 3) {
		usage(argv[0]);
		exit(0);
	}

	snprintf(icid_id, sizeof(icid_id), "00000000");
	snprintf(uniq_id, sizeof(uniq_id), "00000000");
	
	char cfg_file[MAX_LINE] = CFG_FILE;
	
	int c;
	const char *args = "c:h";
	while ((c = getopt(argc, argv, args)) != -1) {
		switch (c) {
			case 'c':
				snprintf(cfg_file, sizeof(cfg_file), "%s", optarg);
				break;
				
			case 'h':
			default:
				usage(argv[0]);
				exit(0);
		}
	}

	// 处理配置文件 
	if (conf_init(cfg_file)) {
		fprintf(stderr, "Unable to read control files:%s\n", cfg_file);
		log_error("Unable to read control files:%s", cfg_file);
		exit(1);
	}

	// 检查队列目录
	if (strlen(config_st.queue_path) <= 0) {
		fprintf(stderr, "Unable to read queue path.\n");
		log_error("Unable to read queue path.");
		exit(1);
	}
	if ( access(config_st.queue_path, F_OK) ) {
		fprintf(stderr, "Queue path:%s not exists:%s, so create it.\n", config_st.queue_path, strerror(errno));	
		log_error("Queue path:%s not exists:%s, so create it.", config_st.queue_path, strerror(errno));	

		umask(0);
		mkdir(config_st.queue_path, 0777);
	}
	

	online_d = dictionary_new(atoi(config_st.max_works) + 1);
	

	// 测试mc是否正常
	/*
	int mc_ret = set_to_mc(config_st.mc_server, config_st.mc_port, "test", "1");
	if (mc_ret != 0) {
		fprintf(stderr, "memcached is error\n");
		log_error("memcached is error");
		//exit(1);
	}*/

	client_st = (struct clients *)malloc((atoi(config_st.max_works) + 1) * sizeof(struct clients));
	if (!client_st) {
		log_error("malloc clients [%d] failed:[%d]%s", (atoi(config_st.max_works) + 1), errno, strerror(errno));
		exit(1);
	}
	
	int i = 0;
	for (i=0; i<(atoi(config_st.max_works) + 1); i++) {
		client_st[i].used = 0;
		client_st[i].fd = 0;
		memset(client_st[i].ip, 20, 0);
		memset(client_st[i].port, 20, 0);
		memset(client_st[i].uid, MAX_LINE, 0);
		memset(client_st[i].ios_token, MAX_LINE, 0);
		client_st[i].recv_data = (struct data_st *)calloc(1, sizeof(struct data_st));
		if (client_st[i].recv_data == NULL) {
			log_error("client_st[%d] init for recv_data, calloc failed size[%lld]", i, sizeof(struct data_st));
			exit(1);
		}
		struct data_st *recv_pdt = client_st[i].recv_data;
		recv_pdt->data_size = 0;
		recv_pdt->data_len = 0;
		recv_pdt->data_used = 0;
		recv_pdt->data = NULL;

		client_st[i].send_data = (struct data_st *)calloc(1, sizeof(struct data_st));
		if (client_st[i].send_data == NULL) {
			log_error("client_st[%d] init for send_data, calloc failed size[%lld]", i, sizeof(struct data_st));
			exit(1);
		}
		struct data_st *send_pdt = client_st[i].send_data;
		send_pdt->data_size = 0;
		send_pdt->data_len = 0;
		send_pdt->data_used = 0;
		send_pdt->data = NULL;
	}

	sig_catch(SIGINT, exit_self);

	// 开始服务
	int connfd, epfd, sockfd, n, nread;
	struct sockaddr_in local, remote;
	socklen_t addrlen;

	// 初始化buffer
	char buf[BUF_SIZE];

	size_t pbuf_len = 0;
	size_t pbuf_size = BUF_SIZE + 1;
	char *pbuf = (char *)calloc(1, pbuf_size);
	if (pbuf == NULL) {
		log_error("calloc fail: size[%d]", pbuf_size);
		exit(1);
	}

	// 创建listen socket
	int bind_port = atoi(config_st.bind_port);
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		log_error("socket failed:[%d]:%s", errno, strerror(errno));
		exit(1);
	}
	if (setnonblocking(listenfd) > 0) {
		exit(1);
	}

	bzero(&local, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(bind_port);
	if (bind(listenfd, (struct sockaddr *)&local, sizeof(local)) < 0) {
		log_error("bind local %d failed:[%d]%s", bind_port, errno, strerror(errno));
		exit(1);
	}
	log_info("bind local %d succ", bind_port);

	if (listen(listenfd, atoi(config_st.max_works)) != 0) {
		log_error("listen fd[%d] max_number[%d] failed:[%d]%s", listenfd, atoi(config_st.max_works), errno, strerror(errno));
		exit(1);
	}

	// epoll create fd
	epoll_event_num = atoi(config_st.max_works) + 1;
	epoll_evts = NULL;
	epoll_fd = -1;
	epoll_nfds = -1;

	int epoll_i = 0;

	epoll_evts = (struct epoll_event *)malloc(epoll_event_num * sizeof(struct epoll_event));
	if (epoll_evts == NULL) {
		log_error("alloc epoll event failed");
    	_exit(111);
    }

	epoll_fd = epoll_create(epoll_event_num);
	if (epoll_fd == -1) {
		log_error("epoll_create max_number[%d] failed:[%d]%s", epoll_event_num, errno, strerror(errno));
		exit(1);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1) {
		log_error("epoll_ctl: listen_socket failed:[%d]%s", errno, strerror(errno));
		exit(1);
	}
	epoll_num_running = 1;

	for (;;) {

		epoll_nfds = epoll_wait(epoll_fd, epoll_evts, epoll_event_num, -1);
		
		if (epoll_nfds == -1) {
			if (errno == EINTR)	{
				// 收到中断信号
				log_info("epoll_wait recive EINTR signal, continue");
				continue;
			}

			_exit(111);
		}

		log_debug("epoll_num_running:%d nfds:%d", epoll_num_running, epoll_nfds);
		for (epoll_i = 0; epoll_i < epoll_nfds; epoll_i++) {

			sockfd = epoll_evts[epoll_i].data.fd;
			if (sockfd == listenfd) {
				if ((connfd = accept(listenfd, (struct sockaddr *) &remote, &addrlen)) > 0) {
					char *ipaddr = inet_ntoa(remote.sin_addr);
					log_info("accept client:%s", ipaddr);
					if (setnonblocking(connfd) != 0) {
						log_error("setnonblocking fd[%d] failed", connfd);
					}
					ev.events = EPOLLIN | EPOLLET;	
					ev.data.fd = connfd;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
						log_error("epoll_ctl client fd[%d] EPOLL_CTL_ADD failed:[%d]%s", connfd, errno, strerror(errno));
					}

					char greetting[512] = {0};
					char hostname[1024] = {0};
					if (gethostname(hostname, sizeof(hostname)) != 0) {
						snprintf(hostname, sizeof(hostname), "unknown");
					}

					// 取客户端IP,Port
					char client_ip[20] = {0};
					char client_port[20] = {0};
					struct sockaddr_in sa;
					int len = sizeof(sa);
					if (!getpeername(connfd, (struct sockaddr *)&sa, &len)) {
						snprintf(client_ip, sizeof(client_ip), "%s", inet_ntoa(sa.sin_addr));
						snprintf(client_port, sizeof(client_port), "%d", ntohs(sa.sin_port));
					}

					
					// 取一个新的client
					int i = new_idx_from_client_st();
					if (i == -1) {
						log_error("new_idx_from_client_st faile: maybe client queue is full.");

						// send error
						snprintf(greetting, sizeof(greetting), "%s ERR %s%s", TAG_GREET, hostname, DATA_END);
						write(connfd, greetting, strlen(greetting));
						log_debug("send fd[%d]:%s", connfd, greetting);

						continue;		
					}
					client_st[i].used = 1;
					client_st[i].fd = connfd;
					snprintf( client_st[i].ip, 20, "%s", client_ip);	
					snprintf( client_st[i].port, 20, "%s", client_port);	


					// send greeting
					snprintf(greetting, sizeof(greetting), "%s OK %s%s", TAG_GREET, hostname, DATA_END);
					//write(connfd, greetting, strlen(greetting));
					safe_write(&client_st[i], greetting, strlen(greetting));
					log_debug("send fd[%d]:%s", connfd, greetting);
				}

				if (connfd == -1) {
					if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR) {
						log_error("accept failed:[%d]%s", errno, strerror(errno));
					}

					continue;
				}
			}

			if (epoll_evts[epoll_i].events & EPOLLIN) {

				// 读取内容 ----------------------
				pbuf_len = 0;
				memset(pbuf, 0, pbuf_size);
				log_debug("pbuf_size:%d", pbuf_size);

				int is_read_fail = 0;
				while ((nread = read(sockfd, pbuf + pbuf_len, (pbuf_size - pbuf_len))) > 0) {
					pbuf_len += nread;
					log_debug("read(%d, buf, %d) = nread[%d]", sockfd, (pbuf_size - pbuf_len), nread);

					if ((pbuf_size - pbuf_len) < 1024) {
						size_t new_size = pbuf_size * 3;
						int fail_times = 1;

						char *tmp = pbuf;
						while ((pbuf = (char *)realloc(pbuf, new_size)) == NULL) {

							if (fail_times >= 4) {
								log_error("we can't realloc memory. ");

								pbuf = tmp;
								is_read_fail = 1;
								break;
							}

							log_error("realloc fail times[%d], retry it.", fail_times);
							sleep(3);	
						}

						pbuf_size = new_size;
					}
					
				}

				if (is_read_fail == 1) {
					continue;
				}

                if (nread == -1 && errno != EAGAIN) {
					//log_error("read fd[%d] failed:[%d]%s", sockfd, errno, strerror(errno));
					client_close(sockfd);
					continue;
                }

				if (nread == 0) {
					// client close socket
					client_close(sockfd);
					continue;
				}

/*
				// 每次命令以\r\n\r\n结束
				if ( (pbuf[pbuf_len-4] == '\r') 
					&& (pbuf[pbuf_len-3] == '\n')
					&& (pbuf[pbuf_len-2] == '\r') 
					&& (pbuf[pbuf_len-1] == '\n') ) {

					buf[pbuf_len-4] == '\0';
				} else {
					continue;
				}
*/

				log_debug("recv from fd[%d] message body:[%d]%s", sockfd, pbuf_len, pbuf);

				// 获取发件人的index ----------------------
				int ci = get_idx_with_sockfd(sockfd);
				if ( ci < 0 ) {

					log_error("logic error, get_idx_with_sockfd(%d) fail", sockfd);

					char rspsoneBuf[MAX_LINE] = {0};
					snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%ld SYS fail%s", TAG_SENDTXT, DATA_END);
					write(sockfd, rspsoneBuf, strlen(rspsoneBuf));

					continue;

				}


				// 构造读取的数据 ----------------------
				struct data_st *send_pdt = client_st[ci].send_data;
				struct data_st *recv_pdt = client_st[ci].recv_data;

				if (recv_pdt->data == NULL) {
					recv_pdt->data_len = 0;
					recv_pdt->data_size = pbuf_len * 3;
					recv_pdt->data = (char *)calloc(1, recv_pdt->data_size + 1);
					if (recv_pdt->data == NULL) {
						recv_pdt->data_size = 0;
					
						char rspsoneBuf[MAX_LINE] = {0};
						snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%ld SYS fail%s", TAG_SENDTXT, DATA_END);
						safe_write(&client_st[ci], rspsoneBuf, strlen(rspsoneBuf));

						continue;	
					}
				}

				int is_remem_succ = 1;
				if ( (recv_pdt->data_size - recv_pdt->data_len) <= pbuf_len ) {
					size_t new_size = (recv_pdt->data_size + (recv_pdt->data_size - recv_pdt->data_len) + pbuf_len) * 3;
					int fail_times = 1;
					char *tmp = recv_pdt->data;
					while ( (recv_pdt->data = (char *)realloc(recv_pdt->data, new_size)) == NULL ) {
						
						if (fail_times >= 4) {
							log_error("we can't realloc memory. ");
	
							recv_pdt->data = tmp;
							is_remem_succ = 0;
							break;
						}

						log_error("realloc fail times[%d], retry it.", fail_times);	
						sleep(1);
					}

					if (is_remem_succ == 1)	
						recv_pdt->data_size = new_size;
				}	

				if (is_remem_succ == 0) {
					client_close(sockfd);

					continue;
				}

				log_debug("client_st[%d]: pbuf_len[%d] data len[%d] data size[%d]", ci, pbuf_len, recv_pdt->data_len, recv_pdt->data_size);
				if (pbuf_len > 0) {
					int cn = 0;
					cn = snprintf(recv_pdt->data + recv_pdt->data_len, (recv_pdt->data_size - recv_pdt->data_len), "%s", pbuf);
					recv_pdt->data_len += cn;
					*(recv_pdt->data + recv_pdt->data_len) = '\0';
				}

				// debug
				if ( strncasecmp(recv_pdt->data, "SYSADMIN ", 9) == 0 ) {
					online_dump(online_d, sockfd);
				}

				// check buffer is end
				if ( (recv_pdt->data_len >= 4)
					&& (*(recv_pdt->data + (recv_pdt->data_len - 4)) == '\r')
					&& (*(recv_pdt->data + (recv_pdt->data_len - 3)) == '\n')
					&& (*(recv_pdt->data + (recv_pdt->data_len - 2)) == '\r')
					&& (*(recv_pdt->data + (recv_pdt->data_len - 1)) == '\n') ) {
					log_debug("client_st[%d]: data len[%d] data size[%d] Command Buffer Finish", ci, recv_pdt->data_len, recv_pdt->data_size);
				
					recv_pdt->data_len -= 4;
					*(recv_pdt->data + recv_pdt->data_len) = '\0';

					// buffer end and start command
					//command(sockfd, recv_pdt->data, recv_pdt->data_len, ci);
					commands(&client_st[ci]);

					// clean client buffer
					if (recv_pdt->data) {
						free(recv_pdt->data);
						recv_pdt->data = NULL;
					}
					recv_pdt->data_size = 0;
					recv_pdt->data_len = 0;
					recv_pdt->data_used = 0;

				}

				if (send_pdt->data_len > 0) {

					// 修改可写
					epoll_event_mod(sockfd, EPOLLOUT);
				}

				continue;

			}

			if (epoll_evts[epoll_i].events & EPOLLOUT) {

				int ci = get_idx_with_sockfd(sockfd);
				if ( ci < 0 ) {
					log_error("get_idx_with_sockfd(%d) fail, so not write", sockfd);

				} else {

					struct data_st *send_pdt = client_st[ci].send_data;
					if ( (send_pdt->data != NULL)
						&& (send_pdt->data_size > 0)
						&& (send_pdt->data_len > 0)
						&& (*(send_pdt->data + (send_pdt->data_len - 2)) == '\r')
						&& (*(send_pdt->data + (send_pdt->data_len - 1)) == '\n') ) {

						char *buf = send_pdt->data + send_pdt->data_used;
						int nwrite, data_size = send_pdt->data_len - send_pdt->data_used; 
						n = data_size;
						while (n > 0) {
							nwrite = write(sockfd, buf + data_size - n, n);
							send_pdt->data_used += nwrite;
							log_debug("write(%d, buf, %d) = nwrite[%d]", sockfd, n, nwrite);

							if (nwrite < n) {
								if (nwrite == -1 && errno != EAGAIN) {
									log_error("write to fd[%d] failed:[%d]%s", sockfd, errno, strerror(errno));
								}

								// 系统缓冲区可能已满,需要等待下个可读事件
								break;
							}

							n -= nwrite;
						}

						log_debug("send to client fd[%d]: [%d/%d]", sockfd, send_pdt->data_used, send_pdt->data_len);

						if ( send_pdt->data_len > send_pdt->data_used ) {
							// 还有未发送完的数据,继续等待有可读事件
							
							continue;
						}

						// clean client buffer	
						/*
						if (send_pdt->data) {
							free(send_pdt->data);
							send_pdt->data = NULL;
						}
						send_pdt->data_size = 0;
						send_pdt->data_len = 0;
						send_pdt->data_used = 0;
						*/
						reinit_data_without_free(send_pdt);

					}
				}

				epoll_event_mod(sockfd, EPOLLIN | EPOLLET);

				continue;

			}

			if (epoll_evts[epoll_i].events & EPOLLHUP) {
				log_debug("get event EPOLLHUP from fd[%d]", sockfd);

				ev.data.fd = sockfd;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, 0) == -1) {
					log_error("epoll_ctl fd[%d] EPOLL_CTL_DEL failed:[%d]%s", sockfd, errno, strerror(errno));
				}

				close(sockfd);
				epoll_num_running--;
				log_debug("sockfd[%d] close", sockfd);	
			}

		}

	}

	close(epoll_fd);
	close(listenfd);

	log_info("i'm finish");
    
	return 0;
}

