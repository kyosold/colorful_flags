#include <stdio.h>
#include <string.h>
#include "liao_log.h"

int log_level = info;
char uniq_id[100] = {0};
char icid_id[100] = {0};

void liao_log(const char *ident, int option, int facility)
{
    openlog(ident, option, facility);
}