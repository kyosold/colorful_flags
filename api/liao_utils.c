#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <libmemcached/memcached.h>

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
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT, mc_timeout);
    
    mc_servers= memcached_server_list_append(NULL, mc_ip, atoi(mc_port), &mrc);
    log_debug("memcached_server_list_append[%d]:%s:%d", mrc, mc_ip, mc_port);
    
    if (MEMCACHED_SUCCESS == mrc) {
    	mrc= memcached_server_push(memc, mc_servers);
    	memcached_server_list_free(mc_servers);
    	if (MEMCACHED_SUCCESS == mrc) {
    		char mc_value_len = strlen(value);
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
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT, mc_timeout);
    
    mc_servers= memcached_server_list_append(NULL, mc_ip, atoi(mc_port), &mrc);
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
