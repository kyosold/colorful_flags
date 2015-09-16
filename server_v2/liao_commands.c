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
#include <json-c/json.h>
#include "mysql.h"
#include "liao_log.h"
#include "liao_utils.h"
#include "liao_server.h"
#include "base64.h"

#include "confparser.h"
#include "dictionary.h"

extern char max_works[512];
extern struct clients *client_st;
extern dictionary *online_d;
extern struct confg_st config_st;

void quit(char *prepend, char *append);

/*
void commands(struct clients *client_t)
{
	char rspsoneBuf[MAX_LINE * 2] = {0};
	struct data_st *recv_pdt = client_t->recv_data;
	if ((recv_pdt->data_len > 0) && (recv_pdt->data != NULL)) {

		// SYSADMIN: 用于管理员调试
		if ( strncasecmp(recv_pdt->data, "SYSADMIN ", 9) == 0 ) {
			
		}

		
		// LOGIN: 用于表示客户端登录 LOGIN account:abalam password:123qwe ios_token:111 get_friend_list:1 \r\n\r\n
		if ( strncasecmp(recv_pdt->data, "LOGIN ", 6) == 0 ) {
			char *pbuf = recv_pdt->data + 5;
			char myuid[128] = {0};
			int ret = login_cmd(client_t, pbuf, strlen(pbuf));
			if (ret != 0) {
				// 认证失败
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL%s", TAG_LOGIN, DATA_END);

				safe_write(client_t, rspsoneBuf, strlen(rspsoneBuf));				
			}

			// init recv data
			reinit_data_without_free(recv_pdt);

			return ;
		}

		// HELO: 用于表示客户端信息 helo uid:0000001 ios_token:xxxxxxxxxxxxxx fid:0000220
		if ( strncasecmp(recv_pdt->data, "HELO ", 5) == 0 ) {
			char *pbuf = recv_pdt->data + 5;
			char myuid[128] = {0};
			int ret = helo_cmd(client_t, pbuf, strlen(pbuf), myuid, sizeof(myuid));
			if (ret == 0) {
				// 检查是否有离线消息
				int offline_msg_num = has_offline_msg(myuid);
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK %d%s", TAG_HELO, offline_msg_num, DATA_END);
			} else {
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL 0%s", TAG_HELO, DATA_END);
			}

			// init recv data
			reinit_data_without_free(recv_pdt);

			safe_write(client_t, rspsoneBuf, strlen(rspsoneBuf));

			return ;
		}

		// ALIVE: keep alive 保持在线
		if ( strncasecmp(recv_pdt->data, "ALIVE ", 6) == 0 ) {
			char *pbuf = recv_pdt->data + 6;
			snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK %s", TAG_ALIVE, DATA_END);	

			// init recv data
			reinit_data_without_free(recv_pdt);

			safe_write(client_t, rspsoneBuf, strlen(rspsoneBuf));

			return;
		}

		// QUIT: 用户退出
		if ( strncasecmp(recv_pdt->data, "QUIT ", 5) == 0 ) {
			char *pbuf = recv_pdt->data + 5;
			int ret = quit_cmd(client_t, pbuf, strlen(pbuf));

			// init recv data
			reinit_data_without_free(recv_pdt);

			//safe_write(client_t, rspsoneBuf, strlen(rspsoneBuf));

			return ;
		}


		// SENDADDFRDREQ: 发送请求加为好友信息
		if ( strncasecmp(recv_pdt->data, "SENDADDFRDREQ ", 14) == 0 ) {
			char *pbuf = recv_pdt->data + 14;
			char mid[MAX_LINE] = {0};
			int ret = sendreq_addfriend(client_t, pbuf, strlen(pbuf), mid, sizeof(mid));
			if (ret == 0) {
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK %s%s", TAG_SENDADDFRIENDREQ, mid, DATA_END);
			} else {
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL %s%s", TAG_SENDADDFRIENDREQ, mid, DATA_END);
			}

			// init recv data
			reinit_data_without_free(recv_pdt);

			safe_write(client_t, rspsoneBuf, strlen(rspsoneBuf));
			return;
		}
	
		// SENDTXT: 发送送信息给对方 
		if ( strncasecmp(recv_pdt->data, "SENDTXT ", 8) == 0 ) {
			char *pbuf = recv_pdt->data + 8;
			char mid[MAX_LINE] = {0};
			int ret = sendtxt_cmd(client_t, pbuf, strlen(pbuf), mid, sizeof(mid));
			if (ret == 0) {
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK %s%s", TAG_SENDTXT, mid, DATA_END);
			} else {
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL %s%s", TAG_SENDTXT, mid, DATA_END);
			}
			
			// init recv data
			reinit_data_without_free(recv_pdt);
			
			safe_write(client_t, rspsoneBuf, strlen(rspsoneBuf));
			return;
		}
		
		// SENDIMG: 发送送图片给对方
		if ( strncasecmp(recv_pdt->data, "SENDIMG ", 8) == 0 ) {
			char *pbuf = recv_pdt->data + 8;
			char mid[MAX_LINE] = {0};
			char thumb_img_url[MAX_LINE] = {0};
			char qid[MAX_LINE] = {0};
			int ret = sendimg_cmd(client_t, pbuf, strlen(pbuf), mid, sizeof(mid), thumb_img_url, sizeof(thumb_img_url), qid, sizeof(qid));
			if (ret == 0) {
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK %s %s %s%s", TAG_SENDIMG, mid, qid, thumb_img_url, DATA_END); 
			} else {
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL %s %s %s%s", TAG_SENDIMG, mid, qid, thumb_img_url, DATA_END);
			}
			
			// init recv data
			reinit_data_without_free(recv_pdt);
			
			safe_write(client_t, rspsoneBuf, strlen(rspsoneBuf));
			return;
		}

		// SENDAUDIO: 发送送音频给对方
		if ( strncasecmp(recv_pdt->data, "SENDAUDIO ", 10) == 0 ) {
			char *pbuf = recv_pdt->data + 10;
			char mid[MAX_LINE] = {0};
			char qid[MAX_LINE] = {0};
			char audio_url[MAX_LINE] = {0};
			int ret = sendaudio_cmd(client_t, pbuf, strlen(pbuf), mid, sizeof(mid), audio_url, sizeof(audio_url), qid, sizeof(qid));
			if (ret == 0) {
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s OK %s %s %s %s", TAG_SENDAUDIO, mid, qid, audio_url, DATA_END); 
			} else {
				snprintf(rspsoneBuf, sizeof(rspsoneBuf), "%s FAIL %s %s %s %s", TAG_SENDAUDIO, mid, qid, audio_url, DATA_END);
			}
			
			// init recv data
			reinit_data_without_free(recv_pdt);
			
			safe_write(client_t, rspsoneBuf, strlen(rspsoneBuf));
			return;
		}
	}
}


// System Command
//
//
//
void status_cmd(int sockfd, char *pbuf, size_t pbuf_size)
{
	log_debug("pbuf:%s", pbuf);

	// 在线dump
	//printf("online list:\n");
	write(sockfd, "online list:\n", strlen("online list:\n"));
	
	write(sockfd, "\tsock_fd => ios_token:\n", strlen("\tsock_fd => ios_token:\n"));
	online_dump(online_d, sockfd);


	// client info
	char buf[10*1024] = {0};
	write(sockfd, "client list info:\n", strlen("client list info:\n"));
	int i = 0, online_num = 0;;
	for (i=0; i<(atoi(config_st.max_works) + 1); i++) {
		if (client_st[i].used == 1) {
			snprintf(buf, sizeof(buf), "\t[%d] used[%d] fd[%d] uid[%s] ios_token[%s]\n", i, client_st[i].used, client_st[i].fd, client_st[i].uid, client_st[i].ios_token);
			write(sockfd, buf, strlen(buf));
			memset(buf, 0, sizeof(buf));

			online_num++;
		}
	}

	snprintf(buf, sizeof(buf), "online:%d all:%d\n", online_num, atoi(config_st.max_works));
	write(sockfd, buf, strlen(buf));
}





// Client Command
//
//

// LOGIN: 用于表示客户端登录 LOGIN account:abalam password:123qwe ios_token:111 get_friend_list:1
int login_cmd(struct clients *client_t, char *pbuf, size_t pbuf_size)
{
	log_debug("pbuf:%s", pbuf);
	
	int sockfd = client_t->fd;
	char account[MAX_LINE] = {0};
	char password[MAX_LINE] = {0};
	char ios_token[MAX_LINE] = {0};
	int get_friend_list = 0;
	
	char tok[MAX_LINE] = {0};
	char *ptok = tok;
	
	int n = pbuf_size;
	while ((*pbuf != '\0') && (n > 0)) {
		char *psed = (char *)memchr(pbuf, ' ', strlen(pbuf));
		if (psed != NULL) {
			int m = (psed - pbuf);
			memcpy(ptok, pbuf, m);
			pbuf += (m + 1);
			n -= (m + 1);
		} else {
			ptok = pbuf;
			n -= strlen(pbuf);
		}
		
		log_debug("ptok:[%s]", ptok);
		if (strncasecmp(ptok, "account:", 8) == 0) {
			snprintf(account, sizeof(account), "%s", ptok+8);
		} else if (strncasecmp(ptok, "password:", 9) == 0) {
			snprintf(password, sizeof(password), "%s", ptok+9);
		} else if (strncasecmp(ptok, "ios_token:", 10) == 0) {
			snprintf(ios_token, sizeof(ios_token), "%s", ptok+10);
		} else if (strncasecmp(ptok, "get_friend_list:", 16) == 0) {
			get_friend_list = atoi(ptok+16);
			
			break;
		} 
		
		memset(ptok, 0, strlen(ptok));
	}
	
	log_debug("get sockfd[%d] account[%s]", sockfd, account);
	log_debug("get sockfd[%d] account[%s]", sockfd, password);
	log_debug("get sockfd[%d] account[%s]", sockfd, ios_token);
	log_debug("get sockfd[%d] account[%d]", sockfd, get_friend_list);

	int result_size = 0;
	char *presult = NULL;
	size_t response_buf_size = 0;
	char *response_buf = NULL;
	
	
	// 调用认证接口
	char url[MAX_LINE] = {0};
	snprintf(url, sizeof(url), "%s/login.php", API_HTTP); 
	
	result_size = 0;
	presult = login_http_api(url, account, password, ios_token, get_friend_list, &result_size);
	log_debug("presult:[%d]%s", result_size, presult);
	if (result_size == 0 || presult == NULL) {
		log_error("get user info fail from API");
		return 1;
	}

	response_buf_size = 6 + 6 + result_size + 5;
	response_buf = (char *)calloc(1, response_buf_size);
	if (response_buf == NULL) {
		clean_mem_buf(presult);
		result_size = 0;

		return 1;	
	}

	n = snprintf(response_buf, response_buf_size, "%s OK %s%s", TAG_LOGIN, presult, DATA_END); 

	safe_write(client_t, response_buf, n);

	// 注册在线
	json_bool json_ret;
	struct array_list *tmp_array;
	struct json_object *tmp_obj;
	struct json_object *tmp_obj2;
	struct json_object *result_json;
	
	result_json = json_tokener_parse(presult);
	if ( result_json == NULL ) {
		log_error("get uid fail with json_tokener_parse");
		goto FINISH;
	}
	//tmp_obj = json_object_object_get(result_json, "myself");
	json_ret = json_object_object_get_ex(result_json, "myself", &tmp_obj);
	if ( json_ret == 0) {
		log_error("get uid fail with json_object_object_get key:myself");
		goto FINISH;
	}
	//tmp_obj2 = json_object_object_get(tmp_obj, "id");
	json_ret = json_object_object_get_ex(tmp_obj, "id", &tmp_obj2);
	if (json_ret == 0) {
		log_error("get uid fail with json_object_object_get key:id");
		goto FINISH;
	}

	char *uid = (char *)json_object_to_json_string(tmp_obj2);
	if (uid) {
		if (*(uid + (strlen(uid) - 1)) == '"') 
			*(uid + (strlen(uid) - 1)) = '\0';

		if (*uid == '"') 
			uid += 1;
	}
	log_debug("get uid[%s] from json", uid);

	// 注册在线	
	snprintf(client_t->uid, sizeof(client_t->uid), "%s", uid);
    snprintf(client_t->ios_token, sizeof(client_st->ios_token), "%s", ios_token);

	int ret = dictionary_set(online_d, uid, ios_token);
	if (ret != 0) {
		log_error("add to online failed uid[%s] sockfd[%d] ios_token[%s]", uid, sockfd, ios_token);
	} else {
		log_debug("registon online succ uid[%s] ios_token[%s] sockfd[%d]", uid, ios_token, sockfd);	
		//dictionary_dump(online_d, stderr);
	}


FINISH:	
	clean_mem_buf(presult);
	result_size = 0;

	clean_mem_buf(response_buf);
	response_buf_size = 0;

	return 0;
}

// HELO: 用于表示客户端信息	helo fuid:0000001 fdevtoken:xxxxxxxxxxxxxx
int helo_cmd(struct clients *client_t, char *pbuf, size_t pbuf_size, char *myuid, size_t myuid_size)
{
	int sockfd = client_t->fd;
	log_debug("pbuf:[%s]", pbuf);

	// pbuf[uid:0000001 ios_token:xxxxxxxxxxxxxx]
	char fuid[MAX_LINE] = {0};
	char fios_token[MAX_LINE] = {0};

	char tok[MAX_LINE] = {0};
	char *ptok = tok;
	
	int n = pbuf_size;
	while ((*pbuf != '\0') && (n > 0)) {
		char *psed = (char *)memchr(pbuf, ' ', strlen(pbuf));
		if (psed != NULL) {
			int m = (psed - pbuf);
			memcpy(ptok, pbuf, m);
			pbuf += (m + 1);
			n -= (m + 1);

		} else {
			ptok = pbuf;
			n -= strlen(pbuf);
		}
		
		log_debug("ptok:[%s]", ptok);
		if (strncasecmp(ptok, "fuid:", 5) == 0) {
			snprintf(fuid, sizeof(fuid), "%s", ptok+5);
			log_debug("from uid:[%s]", fuid);
		} else if (strncasecmp(ptok, "fios_token:", 11) == 0) {
			snprintf(fios_token, sizeof(fios_token), "%s", ptok+11);
			log_debug("from ios_token:[%s]", fios_token);
		}
		
		memset(ptok, 0, strlen(ptok));
	}
	log_debug("get sockfd[%d] from fuid[%s]", sockfd, fuid);
	log_debug("get sockfd[%d] from fios_token[%s]", sockfd, fios_token);

	if ( ( strlen(fuid) <= 0 ) || ( strlen(fios_token) <= 0 ) ) {
		log_error("get sockfd[%d] parameter error", sockfd);
		return 1;
	}
	

	snprintf(client_t->uid, sizeof(client_t->uid), "%s", fuid);
	snprintf(client_t->ios_token, sizeof(client_st->ios_token), "%s", fios_token);
	
	// 注册为在线	(uid => ios_token)
	int ret = dictionary_set(online_d, fuid, fios_token);
	if (ret != 0) {
		log_error("add to online failed uid[%s] sockfd[%d] ios_token[%s]", fuid, sockfd, fios_token);
	}
	log_debug("registon online succ uid[%s] ios_token[%s] sockfd[%d]", fuid, fios_token, sockfd);

	snprintf(myuid, myuid_size, "%s", fuid);

	return 0;
	
}


int sendreq_addfriend(struct clients *client_t, char *pbuf, size_t pbuf_size, char *mid, size_t mid_size)
{
	log_debug("pbuf:%s", pbuf);
	char m_type[] = "SYS";

	int sockfd = client_t->fd;
    char fuid[MAX_LINE] = {0};
    char fios_token[MAX_LINE] = {0};
    char fnick[MAX_LINE] = {0};
    char tuid[MAX_LINE] = {0};
    char tios_token[MAX_LINE] = {0};

	char *message = NULL;
    message = (char *)calloc(1, pbuf_size + 1); 
    if (message == NULL) {
        log_error("calloc failed:%s size:%d", strerror(errno), pbuf_size + 1); 
        goto TXT_FAIL;
    }   

    char tok[MAX_LINE] = {0};
    char *ptok = tok;
        
    int n = pbuf_size;
    while ((*pbuf != '\0') && (n > 0)) {
        char *psed = (char *)memchr(pbuf, ' ', strlen(pbuf));
        if (psed != NULL) {
            int m = (psed - pbuf);
            memcpy(ptok, pbuf, m); 
            pbuf += (m + 1); 
            n -= (m + 1); 

        } else {
            ptok = pbuf;
            n -= strlen(pbuf);
        }   
    
        log_debug("ptok:[%s]", ptok);	
		if (strncasecmp(ptok, "mid:", 4) == 0) {
            snprintf(mid, mid_size, "%s", ptok+4);
        } else if (strncasecmp(ptok, "fuid:", 5) == 0) {
            snprintf(fuid, sizeof(fuid), "%s", ptok+5);
        } else if (strncasecmp(ptok, "fios_token:", 11) == 0) {
            snprintf(fios_token, sizeof(fios_token), "%s", ptok+11);
        } else if (strncasecmp(ptok, "fnick:", 6) == 0) {
            snprintf(fnick, sizeof(fnick), "%s", ptok+6);
        } else if (strncasecmp(ptok, "tuid:", 5) == 0) {
            snprintf(tuid, sizeof(tuid), "%s", ptok+5);
        } else if (strncasecmp(ptok, "tios_token:", 11) == 0) {
            snprintf(tios_token, sizeof(tios_token), "%s", ptok+11);

            break;
        }

        memset(ptok, 0, strlen(ptok));
    }
    log_debug("get sockfd[%d] mid[%s]", sockfd, mid);
    log_debug("get sockfd[%d] fuid[%s]", sockfd, fuid);
    log_debug("get sockfd[%d] fios_token[%s]", sockfd, fios_token);
    log_debug("get sockfd[%d] fnick[%s]", sockfd, fnick);
    log_debug("get sockfd[%d] tid[%s]", sockfd, tuid);
    log_debug("get sockfd[%d] tios_token[%s]", sockfd, tios_token);


	// 写消息索引到数据库
	n = write_queue_to_db(TAG_RECVADDFRIENDREQ, mid, fuid, fnick, fios_token, tuid, tios_token, m_type, "", 0);
	if (n == -1) {
		goto TXT_FAIL;
	}
	
	// 发送通知
	log_debug("send notice message to tuid:%s", tuid);
	char ficon[MAX_LINE] = {0};
	get_avatar_url(fuid, ficon, sizeof(ficon));

	// 检查收件人是否在线
	char *pf_fd = dictionary_get(online_d, tuid, NULL);
	if (pf_fd == NULL) {
		// 收件人不在线，使用APN通知
		log_debug("tuid:%s not online, use APNS send msg", tuid);
		send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
	} else {
		// 收件人在线，使用socket发送通知
		int i = get_idx_with_uid(tuid);
		log_debug("tuid:%s is online, use socket send message", tuid);
		if ((i == -1) || (client_st[i].used != 1)) {
			log_error("logic is error, i[%d] = get_idx_with_uid(%s), client_st[i].used=%d, use APN send notice", i, tuid, client_st[i].used);
			
			send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
		} else {
			n = send_socket_cmd(&client_st[i], fuid, fnick, m_type, tuid);
			if (n != 0) {
				log_error("use send_socket_cmd send notice fail, use APN send");
				send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
			}
		}
	}
		

	goto TXT_SUCC;
	
	

TXT_FAIL:

	if (message != NULL) {
		free(message);
		message = NULL;
	}

	return 1;

TXT_SUCC:
	if (message != NULL) {
		free(message);
		message = NULL;
	}


	return 0;
}



// sendtxt mid:xxxxx fuid:0000001 fios_token:xxxxxxxxxxxxxx fnick:xxxxx tuid:0000220 tios_token:ccccccccccccccc message:32:abcabcabc
int sendtxt_cmd(struct clients *client_t, char *pbuf, size_t pbuf_size, char *mid, size_t mid_size)
{
	log_debug("pbuf:%s", pbuf);
	char m_type[] = "TXT";
	
	// pbuf[mid:xxxxx fuid:0000001 fios_token:xxxxxxxxxxxxxx fnick:xxxxxx tuid:0000220 tios_token:ccccccccccccccc message:32:abcabcabc]
	int sockfd = client_t->fd;
	char fuid[MAX_LINE] = {0};
	char fios_token[MAX_LINE] = {0};
	char fnick[MAX_LINE] = {0};
	char tuid[MAX_LINE] = {0};
	char tios_token[MAX_LINE] = {0};
	char *message = NULL;
	message = (char *)calloc(1, pbuf_size + 1);
	if (message == NULL) {
		log_error("calloc failed:%s size:%d", strerror(errno), pbuf_size + 1);
		goto TXT_FAIL;
	}

	char tok[MAX_LINE] = {0};
	char *ptok = tok;
	
	int n = pbuf_size;
	while ((*pbuf != '\0') && (n > 0)) {
		char *psed = (char *)memchr(pbuf, ' ', strlen(pbuf));
		if (psed != NULL) {
			int m = (psed - pbuf);
			memcpy(ptok, pbuf, m);
			pbuf += (m + 1);
			n -= (m + 1);

		} else {
			ptok = pbuf;
			n -= strlen(pbuf);
		}
		
		log_debug("ptok:[%s]", ptok);
		if (strncasecmp(ptok, "mid:", 4) == 0) {
			snprintf(mid, mid_size, "%s", ptok+4);
		} else if (strncasecmp(ptok, "fuid:", 5) == 0) {
			snprintf(fuid, sizeof(fuid), "%s", ptok+5);
		} else if (strncasecmp(ptok, "fios_token:", 11) == 0) {
			snprintf(fios_token, sizeof(fios_token), "%s", ptok+11);
		} else if (strncasecmp(ptok, "fnick:", 6) == 0) {
			snprintf(fnick, sizeof(fnick), "%s", ptok+6);
		} else if (strncasecmp(ptok, "tuid:", 5) == 0) {
			snprintf(tuid, sizeof(tuid), "%s", ptok+5);
		} else if (strncasecmp(ptok, "tios_token:", 11) == 0) {
			snprintf(tios_token, sizeof(tios_token), "%s", ptok+11);
		} else if (strncasecmp(ptok, "message:", 8) == 0) {
			snprintf(message, pbuf_size, "%s", ptok + 8);

			break;
		}
		
		memset(ptok, 0, strlen(ptok));
	}
	log_debug("get sockfd[%d] mid[%s]", sockfd, mid);
	log_debug("get sockfd[%d] fuid[%s]", sockfd, fuid);
	log_debug("get sockfd[%d] fios_token[%s]", sockfd, fios_token);
	log_debug("get sockfd[%d] fnick[%s]", sockfd, fnick);
	log_debug("get sockfd[%d] tid[%s]", sockfd, tuid);
	log_debug("get sockfd[%d] tios_token[%s]", sockfd, tios_token);
	log_debug("get sockfd[%d] message[%s]", sockfd, message);


	// 写消息到队列
	char queue_file[MAX_LINE] = {0};
	n = write_content_to_file_with_uid(tuid, message, strlen(message), queue_file, sizeof(queue_file));
	if (n == -1) {
		log_error("write_content_to_file_with_uid fail");
		goto TXT_FAIL;
	}
	
	// 写消息索引到数据库
	unsigned long queue_size = n;
    struct stat queue_st;
    if ( lstat(queue_file, &queue_st) == 0 )
        queue_size = queue_st.st_size;

	int qid = write_queue_to_db(TAG_RECVTXT, mid, fuid, fnick, fios_token, tuid, tios_token, m_type, queue_file, queue_size);
	if (qid == -1) {
		goto TXT_FAIL;
	}
	
	// 发送通知
	log_debug("send notice message to tuid:%s", tuid);
	char ficon[MAX_LINE] = {0};
	get_avatar_url(fuid, ficon, sizeof(ficon));

	// 检查收件人是否在线
	char *pf_fd = dictionary_get(online_d, tuid, NULL);
	if (pf_fd == NULL) {
		// 收件人不在线，使用APN通知
		log_debug("tuid:%s not online, use APNS send msg", tuid);
		send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
	} else {
		// 收件人在线，使用socket发送通知
		int i = get_idx_with_uid(tuid);
		log_debug("tuid:%s is online, use socket send message", tuid);
		if ((i == -1) || (client_st[i].used != 1)) {
			log_error("logic is error, i[%d] = get_idx_with_uid(%s), client_st[i].used=%d, use APN send notice", i, tuid, client_st[i].used);
			
			send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
		} else {
			n = send_socket_cmd(&client_st[i], fuid, fnick, m_type, tuid);
			if (n != 0) {
				log_error("use send_socket_cmd send notice fail, use APN send");
				send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
			}
		}
	}

	goto TXT_SUCC;


TXT_FAIL:
	if (strlen(queue_file) > 0) {
		unlink(queue_file);
	}

	if (message != NULL) {
		free(message);
		message = NULL;
	}


	return 1;

TXT_SUCC:
	if (message != NULL) {
		free(message);
		message = NULL;
	}


	return 0;
} 

// sendimg mid:xxxxx fuid:0000001 fios_token:xxxxxxxxxxxxxx fnick:xxxx tuid:0000220 tios_token:ccccccccccccccc file_name:abcd.JPG message:abcabcabc
int sendimg_cmd(struct clients *client_t, char *pbuf, size_t pbuf_size, char *mid, size_t mid_size, char *thumb_name_url, size_t thumb_name_url_size, char *qid, size_t qid_size)
{
	log_debug("pbuf:%s", pbuf);
	char m_type[] = "IMG";
	
	// pbuf[mid:xxxxx fuid:0000001 fios_token:xxxxxxxxxxxxxx fnick:xxxx tuid:0000220 tios_token:ccccccccccccccc file_name:abcd.JPG message:23:abcabcabc]
	int sockfd = client_t->fd;
	char fuid[MAX_LINE] = {0};
	char fios_token[MAX_LINE] = {0};
	char fnick[MAX_LINE] = {0};
	char tuid[MAX_LINE] = {0};
	char tios_token[MAX_LINE] = {0};
	char file_name[MAX_LINE] = {0};
	char *message = NULL;
	message = (char *)calloc(1, pbuf_size + 1);
	if (message == NULL) {
		log_error("calloc failed:%s size:%d", strerror(errno), pbuf_size + 1);
		goto IMG_FAIL;
	}
	
	//char tok[MAX_LINE] = {0};
	//char *ptok = tok;
	char *ptok = (char *)calloc(1, pbuf_size + 1);
	if (ptok == NULL) {
		log_error("calloc failed:%s size:%d", strerror(errno), pbuf_size + 1);
		goto IMG_FAIL;
	}
	
	int n = pbuf_size;
	while ((*pbuf != '\0') && (n > 0)) {
		char *psed = (char *)memchr(pbuf, ' ', strlen(pbuf));
		if (psed != NULL) {
			log_debug("strlen(pbuf):%d psed:%s", strlen(pbuf), psed);
			int m = (psed - pbuf);
			memcpy(ptok, pbuf, m);
			pbuf += (m + 1);
			n -= (m + 1);

		} else {
			//ptok = pbuf;
			memcpy(ptok, pbuf, strlen(pbuf));
			n -= strlen(pbuf);
		}
		
		log_debug("ptok:[%s]", ptok);
		if (strncasecmp(ptok, "mid:", 4) == 0) {
			snprintf(mid, mid_size, "%s", ptok+4);
		} else if (strncasecmp(ptok, "fuid:", 5) == 0) {
			snprintf(fuid, sizeof(fuid), "%s", ptok+5);
		} else if (strncasecmp(ptok, "fios_token:", 11) == 0) {
			snprintf(fios_token, sizeof(fios_token), "%s", ptok+11);
		} else if (strncasecmp(ptok, "fnick:", 6) == 0) {
			snprintf(fnick, sizeof(fnick), "%s", ptok+6);
		} else if (strncasecmp(ptok, "tuid:", 5) == 0) {
			snprintf(tuid, sizeof(tuid), "%s", ptok+5);
		} else if (strncasecmp(ptok, "tios_token:", 11) == 0) {
			snprintf(tios_token, sizeof(tios_token), "%s", ptok+11);
		} else if (strncasecmp(ptok, "file_name:", 10) == 0) {
			snprintf(file_name, sizeof(file_name), "%s", ptok+10);
		} else if (strncasecmp(ptok, "message:", 8) == 0) {
			snprintf(message, pbuf_size, "%s", ptok + 8);

			break;
		}
		
		memset(ptok, 0, strlen(ptok));
	}
	log_debug("get sockfd[%d] mid[%s]", sockfd, mid);
	log_debug("get sockfd[%d] fuid[%s]", sockfd, fuid);
	log_debug("get sockfd[%d] fios_token[%s]", sockfd, fios_token);
	log_debug("get sockfd[%d] fnick[%s]", sockfd, fnick);
	log_debug("get sockfd[%d] tid[%s]", sockfd, tuid);
	log_debug("get sockfd[%d] tios_token[%s]", sockfd, tios_token);
	log_debug("get sockfd[%d] file_name[%s]", sockfd, file_name);
	log_debug("get sockfd[%d] message[%s]", sockfd, message);


	// 为图片做base64解码
	int b64_message_size =  pbuf_size + 1;
	char *b64_message = NULL;
    b64_message = (char *)calloc(1, pbuf_size + 1); 
    if (b64_message == NULL) {
        log_error("calloc failed:%s size:%d", strerror(errno), b64_message_size); 

        goto IMG_FAIL;
    }
	
	int b64_n = base64_decode(message, b64_message, b64_message_size);
	if (b64_n <= 0) {
		log_error("base64_decode fail");
		goto IMG_FAIL;
	}
	log_debug("base64_decode succ[%d]", b64_n);


	// 保存原图
	char file_path[MAX_LINE] = {0};		// # /data1/htdocs/liao/api/data/1100/images/
	n = snprintf(file_path, sizeof(file_path), "%s/%s/images/", config_st.queue_path, tuid);
	if (n <= 0) {
		log_debug("create file_path fail");
		goto IMG_FAIL;
	}
	
	char img_name[MAX_LINE] = {0};		// # 142911925207076000
	n = write_content_to_file_with_path(file_path, file_name, b64_message, b64_n, img_name, sizeof(img_name));
	if (n == -1) {
		log_error("write original image to file fail");
		goto IMG_FAIL;
	}

	// 生成缩略图
	char orig_path_name[MAX_LINE] = {0};
	char thumb_path_name[MAX_LINE] = {0};
	snprintf(orig_path_name, sizeof(orig_path_name), "%s/%s", file_path, img_name);
	snprintf(thumb_path_name, sizeof(thumb_path_name), "%s/s_%s", file_path, img_name);
	
	int ret = image_resize_without_scale(orig_path_name, thumb_path_name, 95);
	if (ret != 0) {
		log_error("create thumb fail:[%s] -> [%s]", orig_path_name, thumb_path_name);
		goto IMG_FAIL;
	}
	
	// # http://chat.vmeila.com/api/data/11000/images/s_142911925207076000.jpg
	snprintf(thumb_name_url, thumb_name_url_size, "%s/%s/images/s_%s", config_st.queue_host, tuid, img_name);

	
	
	// 写消息索引到数据库
	unsigned long queue_size = n;
	struct stat queue_st;
	if (stat(thumb_path_name, &queue_st) == 0)
		queue_size = queue_st.st_size;

	int mqid = write_queue_to_db(TAG_RECVIMG, mid, fuid, fnick, fios_token, tuid, tios_token, m_type, thumb_name_url, queue_size);
	if (mqid == -1) {
		goto IMG_FAIL;
	}
	snprintf(qid, qid_size, "%d_%s_%s", mqid, fuid, tuid);
	
	// 发送通知
    log_debug("send notice message to tuid:%s", tuid);
    char ficon[MAX_LINE] = {0};
    get_avatar_url(fuid, ficon, sizeof(ficon));

    // 检查收件人是否在线
    char *pf_fd = dictionary_get(online_d, tuid, NULL);
    if (pf_fd == NULL) {
        // 收件人不在线，使用APN通知
        log_debug("tuid:%s not online, use APNS send msg", tuid);
        send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
    } else {
        // 收件人在线，使用socket发送通知
        int i = get_idx_with_uid(tuid);
        log_debug("tuid:%s is online, use socket send message", tuid);
        if ((i == -1) || (client_st[i].used != 1)) {
            log_error("logic is error, i[%d] = get_idx_with_uid(%s), client_st[i].used=%d, use APN send notice", i, tuid, client_st[i].used);
    
            send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
        } else {
        	n = send_socket_cmd(&client_st[i], fuid, fnick, m_type, tuid);
        	if (n != 0) {
            	log_error("use send_socket_cmd send notice fail, use APN send");
            	send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
			}
        }   
    }   
	
	goto IMG_SUCC;
	

IMG_FAIL:
	if (message != NULL) {
		free(message);
		message = NULL;
	}

	if (ptok != NULL) {
		free(ptok);
		ptok = NULL;
	}

	if (b64_message != NULL) {
		free(b64_message);
		b64_message = NULL;
	}
	b64_message_size = 0;


	return 1;

IMG_SUCC:
	if (message != NULL) {
		free(message);
		message = NULL;
	}

	if (b64_message != NULL) {
		free(b64_message);
		b64_message = NULL;
	}
	b64_message_size = 0;


	return 0;
} 


// sendaudio mid:xxxxx fuid:0000001 fios_token:xxxxxxxxxxxxxx fnick:xxxx tuid:0000220 tios_token:ccccccccccccccc file_name:1429860634.spx message:abcabcabc
int sendaudio_cmd(struct clients *client_t, char *pbuf, size_t pbuf_size, char *mid, size_t mid_size, char *audio_name_url, size_t audio_name_url_size, char *qid, size_t qid_size)
{
	log_debug("pbuf:%s", pbuf);
	char m_type[] = "AUDIO";
	
	// pbuf[mid:xxxxx fuid:0000001 fios_token:xxxxxxxxxxxxxx fnick:xxxx tuid:0000220 tios_token:ccccccccccccccc file_name:abcd.JPG message:23:abcabcabc]
	int sockfd = client_t->fd;
	char fuid[MAX_LINE] = {0};
	char fios_token[MAX_LINE] = {0};
	char fnick[MAX_LINE] = {0};
	char tuid[MAX_LINE] = {0};
	char tios_token[MAX_LINE] = {0};
	char file_name[MAX_LINE] = {0};
	char audio_size[MAX_LINE] = {0};
	char *message = NULL;
	message = (char *)calloc(1, pbuf_size + 1);
	if (message == NULL) {
		log_error("calloc failed:%s size:%d", strerror(errno), pbuf_size + 1);
		goto IMG_FAIL;
	}
	
	char tok[MAX_LINE] = {0};
	char *ptok = tok;
	
	int n = pbuf_size;
	while ((*pbuf != '\0') && (n > 0)) {
		char *psed = (char *)memchr(pbuf, ' ', strlen(pbuf));
		if (psed != NULL) {
			int m = (psed - pbuf);
			memcpy(ptok, pbuf, m);
			pbuf += (m + 1);
			n -= (m + 1);

		} else {
			ptok = pbuf;
			n -= strlen(pbuf);
		}
		
		log_debug("ptok:[%s]", ptok);
		if (strncasecmp(ptok, "mid:", 4) == 0) {
			snprintf(mid, mid_size, "%s", ptok+4);
		} else if (strncasecmp(ptok, "fuid:", 5) == 0) {
			snprintf(fuid, sizeof(fuid), "%s", ptok+5);
		} else if (strncasecmp(ptok, "fios_token:", 11) == 0) {
			snprintf(fios_token, sizeof(fios_token), "%s", ptok+11);
		} else if (strncasecmp(ptok, "fnick:", 6) == 0) {
			snprintf(fnick, sizeof(fnick), "%s", ptok+6);
		} else if (strncasecmp(ptok, "tuid:", 5) == 0) {
			snprintf(tuid, sizeof(tuid), "%s", ptok+5);
		} else if (strncasecmp(ptok, "tios_token:", 11) == 0) {
			snprintf(tios_token, sizeof(tios_token), "%s", ptok+11);
		} else if (strncasecmp(ptok, "file_name:", 10) == 0) {
			snprintf(file_name, sizeof(file_name), "%s", ptok+10);
		} else if (strncasecmp(ptok, "audio_size:", 11) == 0) {
			snprintf(audio_size, sizeof(audio_size), "%s", ptok+11);
		} else if (strncasecmp(ptok, "message:", 8) == 0) {
			snprintf(message, pbuf_size, "%s", ptok + 8);

			break;
		}
		
		memset(ptok, 0, strlen(ptok));
	}
	log_debug("get sockfd[%d] mid[%s]", sockfd, mid);
	log_debug("get sockfd[%d] fuid[%s]", sockfd, fuid);
	log_debug("get sockfd[%d] fios_token[%s]", sockfd, fios_token);
	log_debug("get sockfd[%d] fnick[%s]", sockfd, fnick);
	log_debug("get sockfd[%d] tid[%s]", sockfd, tuid);
	log_debug("get sockfd[%d] tios_token[%s]", sockfd, tios_token);
	log_debug("get sockfd[%d] file_name[%s]", sockfd, file_name);
	log_debug("get sockfd[%d] audio_size[%s]", sockfd, audio_size);
	log_debug("get sockfd[%d] message[%s]", sockfd, message);


	// 为音频做base64解码
	int b64_message_size =  pbuf_size + 1;
	char *b64_message = NULL;
    b64_message = (char *)calloc(1, pbuf_size + 1); 
    if (b64_message == NULL) {
        log_error("calloc failed:%s size:%d", strerror(errno), b64_message_size); 

        goto IMG_FAIL;
    }
	
	int b64_n = base64_decode(message, b64_message, b64_message_size);
	if (b64_n <= 0) {
		log_error("base64_decode fail");
		goto IMG_FAIL;
	}
	log_debug("base64_decode succ[%d]", b64_n);


	// 保存音频
	char file_path[MAX_LINE] = {0};		// # /data1/htdocs/liao/api/data/1100/audios/
	n = snprintf(file_path, sizeof(file_path), "%s/%s/audios/", config_st.queue_path, tuid);
	if (n <= 0) {
		log_debug("create file_path fail");
		goto IMG_FAIL;
	}
	
	char audio_name[MAX_LINE] = {0};		// # 142911925207076000
	n = write_content_to_file_with_path(file_path, file_name, b64_message, b64_n, audio_name, sizeof(audio_name));
	if (n == -1) {
		log_error("write original audio to file fail");
		goto IMG_FAIL;
	}
	char audio_path_name[MAX_LINE] = {0};
	snprintf(audio_path_name, sizeof(audio_path_name), "%s/%s", file_path, audio_name);

    // # http://chat.vmeila.com/api/data/11000/audios/142911925207076000.jpg
	//char audio_name_url[MAX_LINE] = {0};
    //snprintf(audio_name_url, sizeof(audio_name_url), "%s/%s/audios/%s", config_st.queue_host, tuid, audio_name);
    snprintf(audio_name_url, audio_name_url_size, "%s/%s/audios/%s", config_st.queue_host, tuid, audio_name);


	// 写消息索引到数据库
	int queue_size = atoi(audio_size);
	//struct stat queue_st;
	//if (stat(audio_path_name, &queue_st) == 0)
		//queue_size = queue_st.st_size;

	int mqid = write_queue_to_db(TAG_RECVAUDIO, mid, fuid, fnick, fios_token, tuid, tios_token, m_type, audio_name_url, queue_size);
	if (mqid == -1) {
		goto IMG_FAIL;
	}
	snprintf(qid, qid_size, "%d_%s_%s", mqid, fuid, tuid);
	
	// 发送通知
    log_debug("send notice message to tuid:%s", tuid);
    char ficon[MAX_LINE] = {0};
    get_avatar_url(fuid, ficon, sizeof(ficon));

    // 检查收件人是否在线
    char *pf_fd = dictionary_get(online_d, tuid, NULL);
    if (pf_fd == NULL) {
        // 收件人不在线，使用APN通知
        log_debug("tuid:%s not online, use APNS send msg", tuid);
        send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
    } else {
        // 收件人在线，使用socket发送通知
        int i = get_idx_with_uid(tuid);
        log_debug("tuid:%s is online, use socket send message", tuid);
        if ((i == -1) || (client_st[i].used != 1)) {
            log_error("logic is error, i[%d] = get_idx_with_uid(%s), client_st[i].used=%d, use APN send notice", i, tuid, client_st[i].used);
    
            send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
        } else {
        	n = send_socket_cmd(&client_st[i], fuid, fnick, m_type, tuid);
        	if (n != 0) {
            	log_error("use send_socket_cmd send notice fail, use APN send");
            	send_apn_cmd(tios_token, fuid, fnick, m_type, tuid);
			}
        }   
    }   
	
	goto IMG_SUCC;
	

IMG_FAIL:
	if (message != NULL) {
		free(message);
		message = NULL;
	}

	if (b64_message != NULL) {
		free(b64_message);
		b64_message = NULL;
	}
	b64_message_size = 0;

	return 1;

IMG_SUCC:
	if (message != NULL) {
		free(message);
		message = NULL;
	}

	if (b64_message != NULL) {
		free(b64_message);
		b64_message = NULL;
	}
	b64_message_size = 0;

	return 0;
} 

// quit fuid:0000001 
//int quit_cmd(int sockfd, char *pbuf, size_t pbuf_size)
int quit_cmd(struct clients *client_t, char *pbuf, size_t pbuf_size)
{
	int sockfd = client_t->fd;
	log_debug("pbuf:%s", pbuf);
	
	// pbuf[uid:0000001]
	char fuid[MAX_LINE] = {0};
	
	char tok[MAX_LINE] = {0};
	char *ptok = tok;
	
	int n = pbuf_size;
	while ((*pbuf != '\0') && (n > 0)) {
		char *psed = (char *)memchr(pbuf, ' ', strlen(pbuf));
		if (psed != NULL) {
			int m = (psed - pbuf);
			memcpy(ptok, pbuf, m);
			pbuf += (m + 1);
			n -= (m + 1);

		} else {
			ptok = pbuf;
			n -= strlen(pbuf);
		}
		
		log_debug("ptok:[%s]", ptok);
		if (strncasecmp(ptok, "fuid:", 5) == 0) {
			snprintf(fuid, sizeof(fuid), "%s", ptok+5);
		}

		memset(ptok, 0, strlen(ptok));
	}
	log_debug("get sockfd[%d] fuid[%s]", sockfd, fuid);
	
	// 设置离线
	// 查询fd
	int i = get_idx_with_sockfd(sockfd);
	if (i == -1) {
		log_error("get index failed: i[%d] = get_idx_with_sockfd(%d)", i, sockfd);
		return 1;
	}
	if (strcasecmp(client_st[i].uid, fuid) != 0) {
		log_error("logic error client_st[%d].uid=%s != fuid=%s", i, client_st[i].uid, fuid);
		return 1;
	}

	//client_st[i].used = 0;
	//client_st[i].fd = 0;
	//memset(client_st[i].uid, '0', strlen(client_st[i].uid));
	//memset(client_st[i].ios_token, '0', strlen(client_st[i].ios_token));
	
	// 注册为离线	(uid => ios_token)
	dictionary_unset(online_d, fuid);
	log_error("offline fuid[%s] sockfd[%d]", fuid, sockfd);

	// 关闭客户端fd
	close(sockfd);	

	// 回收与初始化当前item
	//init_clientst_item_with_idx(i);
	
	
	return 0;
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
    
    // HELO: 用于表示客户端信息 helo uid:0000001 ios_token:xxxxxxxxxxxxxx fid:0000220
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

*/

void quit(char *prepend, char *append)
{
	char outbuf[1024] = {0};
	int n = snprintf(outbuf, sizeof(outbuf), "%s:%s\n", prepend, append);
	int r = write(1, outbuf, n);
	_exit(0);	
}

void usage(char *prog)
{
	printf("%s [mid] [client ip] [client port]\n", prog);
}

int main(int argc, char **argv)
{
	liao_log("liao_log", LOG_PID|LOG_NDELAY, LOG_MAIL);

	if (argc != 4) {
		usage(argv[0]);
		_exit(0);
	}

	char mid[256] = {0};
	char client_ip[20] = {0};		
	char client_port[20] = {0};

	snprintf(mid, sizeof(mid), "%s", argv[1]);
	snprintf(client_ip, sizeof(mid), "%s", argv[2]);
	snprintf(client_port, sizeof(client_port), "%s", argv[3]);

	log_info("get param mid:%s cip:%s cport:%s", mid, client_ip, client_port);

	return 0;
}




