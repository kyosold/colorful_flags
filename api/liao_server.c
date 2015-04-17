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
#include <sys/stat.h>  
#include <netinet/in.h>  
#include <netinet/tcp.h>  
#include <arpa/inet.h>  
#include <strings.h>  

#include "confparser.h"
#include "dictionary.h"

#include "liao_log.h"
#include "liao_utils.h"

#define MAX_LINE		1024
#define BUF_SIZE		1024

#define CFG_FILE		"./liao_config.ini"

dictionary *dict_conf = NULL;


struct clients
{
	int used;
	int fd;
	char uid[MAX_LINE];
	char ios_token[MAX_LINE];
};
struct clients *client_st;


// global config
int bind_port = 5050;
char mc_server[MAX_LINE] = "127.0.0.1";
int mc_port = 11211;
char mc_timeout[512] = "5000";
char max_works[512] = "512";


// epoll
int epoll_fd = -1; 
int epoll_nfds = -1; 
int epoll_event_num = 0;
struct epoll_event *epoll_evts = NULL;
int epoll_num_running = 0;

void epoll_clean();
int epoll_delete_evt(int epoll_fd, int fd);
int epoll_add_evt(int epoll_fd, int fd, int state);

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


// 逻辑处理
void command(int sockfd, char *buf, size_t buf_size)
{
	int i = 0;
	
	// HELO: 用于表示客户端信息	helo uid:0000001 ios_token:xxxxxxxxxxxxxx
	if ( strncasecmp(buf, "helo ", 5) == 0 ) {
		char uid[MAX_LINE] = {0};
		char ios_token[MAX_LINE] = {0};
		char *pbuf = buf + 5;
		
		char *ptok = strtok(pbuf, " ");
		while (ptok != NULL) {
			if (strncasecmp(ptok, "uid:", 4) == 0) {
				// 处理uid
				log_debug("get sockfd[%d] uid[%s]", sockfd, ptok+4);
				
				continue;
			}
		
			if (strncasecmp(ptok, "ios_token:", 10) == 0) {
				// 处理ios_token
				log_debug("get sockfd[%d] ios_token[%s]", sockfd, ptoken+10);
				
				continue;
			}
		
			ptok = strtok(NULL, " ");
		}
	
		return;
	}
}


// 处理配置文件
// return:
// 	0:succ	1:fail
int conf_init(char *cfg_file)
{
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
		bind_port = atoi(pbind_port);
	}

    
    char *p_log_level = dictionary_get(dict_conf, "global:log_level", NULL);
    if (p_log_level != NULL) {
        if (strcasecmp(p_log_level, "debug") == 0) {
            log_level = debug;
        } else if (strcasecmp(p_log_level, "info") == 0) {
            log_level = info;
        }   
    } 
    
	char *pmc_server = dictionary_get(dict_conf, "global:mc_server", NULL);
	if (pmc_server == NULL) {
		fprintf(stderr, "memcached server configuration failed\n");
		log_error("memcached server configuration failed");
		return 1;
	}
	snprintf(mc_server, sizeof(mc_server), "%s", pmc_server);

	char *pmc_timeout = dictionary_get(dict_conf, "global:mc_timeout", NULL);
	if (pmc_timeout == NULL) {
		log_warning("parse config mc_timeout error, use defaut:%s", mc_timeout);
	}

	char *pmc_port = (char *)memchr(mc_server, ':', strlen(mc_server));
	if (pmc_port != NULL) {
		*pmc_port = '\0';
		mc_port = atoi(pmc_port + 1);
	}

	char *pmax_works = dictionary_get(dict_conf, "global:max_works", NULL);
	if (pmax_works == NULL) {
		log_warning("parse config max_works error, use default:%s", max_works);
	} else {
		snprintf(max_works, sizeof(max_works), "%s", pmax_works);
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



void usage(char *prog)
{
	printf("Usage:\n");
	printf("%s -c[config file]\n", prog);
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
	

	// 测试mc是否正常
	int mc_ret = set_to_mc(mc_server, mc_port, "test", "1");
	if (mc_ret != 0) {
		fprintf(stderr, "memcached is error\n");
		log_error("memcached is error");
		//exit(1);
	}	

	client_st = (struct clients *)malloc((atoi(max_works) + 1) * sizeof(struct clients));
	if (!client_st) {
		log_error("malloc clients [%d] failed:[%d]%s", (atoi(max_works) + 1), errno, strerror(errno));
		exit(1);
	}
	for (i=0; i<(atoi(max_works) + 1); i++) {
		client_st[i].used = 0;
		client_st[i].fd = -1;
		client_st[i].uid = {0};
		client_st[i].ios_token = {0};
	}

	// 开始服务
	int listenfd, connfd, epfd, sockfd, n, nread;
	struct sockaddr_in local, remote;
	socklen_t addrlen;

	char buf[BUF_SIZE];

	// 创建listen socket
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

	if (listen(listenfd, atoi(max_works)) != 0) {
		log_error("listen fd[%d] max_number[%d] failed:[%d]%s", listenfd, max_works, errno, strerror(errno));
		exit(1);
	}

	// epoll create fd
	epoll_event_num = atoi(max_works) + 1;
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
				while ((connfd = accept(listenfd, (struct sockaddr *) &remote, &addrlen)) > 0) {
					char *ipaddr = inet_ntoa(remote.sin_addr);
					log_info("accept client:%s", ipaddr);
					if (setnonblocking(connfd) != 0) {
						log_error("setnonblocking fd[%d] failed", connfd);
					}
					ev.events = EPOLLIN | EPOLLET;	
					ev.data.fd = connfd;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
						log_error("epoll_ctl client fd[%d] failed:[%d]%s", connfd, errno, strerror(errno));
					}
				}

				if (connfd == -1) {
					if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR) {
						log_error("accept failed:[%d]%s", errno, strerror(errno));
				}

				continue;
				}
			}

			if (epoll_evts[epoll_i].events & EPOLLIN) {
				n = 0;
				while ((nread = read(sockfd, buf + n, BUF_SIZE - 1)) > 0) {
					n += nread;
				}

				if (nread == -1 && errno != EAGAIN) {
					log_error("read fd[%d] failed:[%d]%s", sockfd, errno, strerror(errno));

					continue;
				}
				log_debug("recv from fd[%d]:[%d]%s", sockfd, n, buf);
				
				// 逻辑处理
				command(sockfd, buf);
			
				ev.data.fd = sockfd;
				ev.events = epoll_evts[epoll_i].events | EPOLLOUT;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &ev) == -1) {
					log_error("epoll_ctl fd[%d]:[%d]%s", sockfd, errno, strerror(errno));
				}
			}

			if (epoll_evts[epoll_i].events & EPOLLOUT) {
				snprintf (buf, sizeof(buf), "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\nHello World", 11);

				int nwrite, data_size = strlen (buf); 
				n = data_size;
				while (n > 0) {
					nwrite = write (sockfd, buf + data_size - n, n);
					if (nwrite < n) {
						if (nwrite == -1 && errno != EAGAIN) {
							log_error("write to fd[%d] failed:[%d]%s", sockfd, errno, strerror(errno));
						}
						break;
					}

					n -= nwrite;
				}

				log_debug("send to client fd[%d]:[%d]%s", sockfd, data_size, buf);

				close(sockfd);
				epoll_evts[epoll_i].data.fd = -1;
			}
		}

	}

	close(epoll_fd);
	close(listenfd);
    
	return 0;
}

