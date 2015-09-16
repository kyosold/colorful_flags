/*
*
*	server预先创建几个子进程，他们都在各自与server独立链接的pipe socket等待数据，
*	server调用accept，然后遍历子进程，将connfd通过pipe给字进程处理！
*	一旦子进程结束就返回任意内容告知已经处理结束！那么server将其状态位置为闲置！！！
*	注意：这里应该使用select或者epoll，因为server接受的不仅仅是listen的msg，还有child
*	处理完成返回的标志信息！  
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



