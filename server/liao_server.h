#ifndef _LIAO_SERVER_H
#define _LIAO_SERVER_H

#define MAX_LINE        1024
#define BUF_SIZE        8 * 1024
#define CFG_FILE        "./liao_config.ini"

#define TAG_GREET		"GREET"
#define TAG_HELO		"HELO"
#define TAG_ALIVE		"ALIVE"
#define TAG_LOGIN		"LOGIN"
#define TAG_SENDTXT		"SENDTXT"
#define TAG_RECVTXT		"RECVTXT"
#define TAG_SENDIMG		"SENDIMG"
#define TAG_RECVIMG		"RECVIMG"
#define TAG_SENDAUDIO	"SENDAUDIO"
#define TAG_RECVAUDIO	"RECVAUDIO"
#define TAG_SENDADDFRIENDREQ "SENDADDFRDREQ"
#define TAG_RECVADDFRIENDREQ "RECVADDFRDREQ"
#define TAG_QUIT		"QUIT"


//#define CRLF			(u_char *) "\r\n"
#define DATA_END		(u_char *) "\r\n"

#define IMG_PATH		"/data1/htdocs/liao/api/data/"
#define API_HTTP		"http://chat.vmeila.com/api"
#define IMG_HOST		"http://chat.vmeila.com/api/data/"

// default global config
#define DEF_BINDPORT    "5050"
#define DEF_LOGLEVEL	"info"
//#define DEF_USEMC       "1"
#define DEF_MCSERVER    "127.0.0.1"
#define DEF_MCPORT      "11211"
#define DEF_MCTIMEOUT   "5000"
#define DEF_MAXWORKS    "512"
#define DEF_QUEUEPATH	"/data1/htdocs/colorful_flags/api/data/"
#define DEF_QUEUEHOST	"http://chat.vmeila.com/api/data/"

#define DEF_MYSQLSERVER "localhost"
#define DEF_MYSQLPORT	"3306"
#define DEF_MYSQLUSER	"admin"
#define DEF_MYSQLPASSWD	"1234qwer"
#define DEF_MYSQLDB		"liao"
#define DEF_MYSQLTIMEOUT	"2"

#define OFFLINE_MSG_CONTENT	"content.txt"
#define OFFLINE_MSG_CONENT_IDX	"content_idx.txt"

#define THUMB_SIZE_W	139
#define THUMB_SIZE_H	139


static char fb_pid[8192] = {0};

//struct cfg_st
struct confg_st
{
	char bind_port[10];
	char log_level[512];

	// ---- mc -----
	char use_mc[10];
	char mc_server[512];
	int mc_port;
	char mc_timeout[512];

	// ---- mysql ----
	char mysql_host[512];
	int mysql_port;
	char mysql_user[512];
	char mysql_passwd[512];
	char mysql_db[512];
	char mysql_timeout[512];

	char max_works[512];

	char queue_path[512];
	char queue_host[512];
};

struct data_st
{
	size_t data_size;
	size_t data_len;
	size_t data_used;
	char *data;
} data_t;

struct clients
{
    int used;
    int fd; 
	char ip[20];
	char port[20];
    char uid[MAX_LINE];
    char ios_token[MAX_LINE];
	struct data_st *recv_data;
	struct data_st *send_data;
};



#endif
