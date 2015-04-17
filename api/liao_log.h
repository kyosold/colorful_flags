#ifndef _LIAO_LOG_H
#define _LIAO_LOG_H

#include "syslog.h"

extern int log_level;
extern char uniq_id[100];
extern char icid_id[100];

#define debug   4   
#define info    2   

#define log_emerg(fmt, ...) syslog(LOG_EMERG, "[EMERG] %s %s %s "fmt, __func__, icid_id, uniq_id, ##__VA_ARGS__)
#define log_alert(fmt, ...) syslog(LOG_ALERT, "[ALERT] %s %s %s "fmt, __func__, icid_id, uniq_id, ##__VA_ARGS__)
#define log_crit(fmt, ...) syslog(LOG_CRIT, "[CRIT] %s %s %s "fmt, __func__, icid_id, uniq_id, ##__VA_ARGS__)
#define log_error(fmt, ...) syslog(LOG_ERR, "[ERROR] %s %s %s "fmt, __func__, icid_id, uniq_id, ##__VA_ARGS__)
#define log_warning(fmt, ...) syslog(LOG_WARNING, "[WARNING] %s %s %s "fmt, __func__, icid_id, uniq_id, ##__VA_ARGS__)
#define log_notice(fmt, ...) syslog(LOG_NOTICE, "[NOTICE] %s %s %s "fmt, __func__, icid_id, uniq_id, ##__VA_ARGS__)
#define log_info(fmt, ...) {if(log_level>=info){syslog(LOG_INFO, "[INFO] %s %s %s "fmt, __func__, icid_id, uniq_id, ##__VA_ARGS__);}}
#define log_debug(fmt, ...) {if(log_level>=debug){syslog(LOG_DEBUG, "[DEBUG] %s %s %s "fmt, __func__, icid_id, uniq_id, ##__VA_ARGS__);}}

void liao_log(const char *ident, int option, int facility);

#endif