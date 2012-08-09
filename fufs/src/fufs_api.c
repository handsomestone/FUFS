/*
   (c) Copyright 2012  Junjie Shi

   All rights reserved.

   Written by Junjie Shi <handsomestone@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <curl/curl.h>
#define __STRICT_ANSI__
#include <json/json.h>

#include "fufs_api.h"
#include "fufs_curl.h"
#include "fufs_conf.h"
#include "sha2.h"
#include "hmac_sha2.h"


char token[ARRAY_MAX_SIZE];
char dologid[ARRAY_MAX_SIZE];
char *fufs_get_token()
{
	return token;
}
char *fufs_get_dologid()
{
	return dologid;
}
fufs_ret fufs_json_get(char *buf,char *mkey,char *array_name ,char *dest,unsigned int dest_len)
{
	json_object *jobj = NULL;
	json_object *tmp_jobj= NULL;

	

	if(NULL == buf || NULL == dest)
	{
		return FUFS_RET_FAIL;
	}
	jobj = json_tokener_parse(buf);
	if(NULL == jobj || is_error(jobj))
		return FUFS_RET_FAIL;
	
	memset(dest,0,dest_len);
	//FUFS_LOG("mkey %s\n",mkey);
	json_object_object_foreach(jobj,key,val){
		if(NULL == array_name)
		{
			if(strcmp(key,mkey)==0)
			{								
				//FUFS_LOG("mykey %s\n",mkey);
				//FUFS_LOG("key %s\n",key);
				if(json_type_string == json_object_get_type(val))
				{
					snprintf(dest,dest_len,"%s",json_object_get_string(val));
				}
			}
		}
		else
		{
			if(strcmp(key,array_name) != 0)
			{
				continue;
			}
			//FUFS_LOG("key %s\n",key);
			tmp_jobj = json_object_object_get(jobj,key);					
			if (tmp_jobj && !is_error(tmp_jobj)) 
			{				
				json_object_object_foreach(tmp_jobj, key2, val2) 
				{
					if (strcmp(key2, mkey) == 0) 
					{
						if(json_type_string == json_object_get_type(val2))
						{
							snprintf(dest,dest_len,"%s",json_object_get_string(val2));
						}
					}
							
				}
			}
				
			
			//FUFS_LOG("dest %s\n",dest);
		}
		
	}
	json_object_put(jobj);
	return FUFS_RET_OK;
}


char *fufs_api_get_token(char *account,char *password,char *app_type)
{
	char *acnt = NULL;
	char *pwd = NULL;
	time_t times;
	char *param = NULL;
	char *signature= NULL;
	
	char *buf = NULL;
	char *message = NULL;
	char apptype[APPTYPELEN];
	
	char *ret = NULL;
	int fd;	
	ssize_t write_num;
	unsigned int mac_256_size = SHA256_DIGEST_SIZE;
	if(NULL == account || NULL == password)
	{
		return NULL;
	}
    	time(&times);
	acnt = account;
	pwd = password;
	param = calloc(MAXLEN,1);
	if(NULL == param)
	{
		goto error_out;
	}
	signature = calloc(2 * SHA256_DIGEST_SIZE + 1,1);
	if(NULL == signature)
	{
		goto error_out;
	}
	message = calloc(MAXLEN,1);
	if(NULL == message)
	{
		goto error_out;
	}
	snprintf(message,MAXLEN,"account=%s&appkey=%s&password=%s&time=%d",
			acnt,APPKEY,pwd,(int)times);
	hmac_sha256((unsigned char *)APPSECRET,strlen(APPSECRET),(unsigned char *)message,strlen(message),
			(unsigned char *)signature,mac_256_size);
    	strToHex((unsigned char **)&signature,mac_256_size);
	//FUFS_LOG("signature %s\n",signature);
	if(NULL == app_type)
	{
		strcpy(apptype,"local");
	}
	else
	{
		strcpy(apptype,app_type);
	}
	snprintf(param,MAXLEN,"account=%s&password=%s&time=%d&appkey=%s&app_type=%s&signature=%s",
			acnt,pwd,(int)times,APPKEY,apptype,signature);
	buf = fufs_curl_fecth(URL_GET_TOKEN,param);
	
	ret = calloc(ERRMESSAGE,1);
	if(NULL == ret)
	{
		goto error_out;
	}
	fufs_json_get(buf ,"err_msg",NULL,ret,ERRMESSAGE);
	if(strcmp(ret,"success") != 0)
	{
		goto error_out;
	}
	fufs_json_get(buf,"token","data",token,64);
	fd = open("/tmp/token.log", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd == -1)
	{
		goto error_out;
	}
	write_num = write(fd,token,strlen(token));
	close(fd);
error_out:
	FUFS_SAFE_FREE(param);
	FUFS_SAFE_FREE(signature);
	FUFS_SAFE_FREE(message);
	FUFS_SAFE_FREE(buf);
	FUFS_SAFE_FREE(ret);
	return token;
}

fufs_ret fufs_api_keep_token()
{
	char *buf = NULL;
	char *ret = NULL;
	char *param = NULL;
	fufs_ret retype = FUFS_RET_OK;

	if(NULL == token)
	{
		return FUFS_RET_FAIL;
	}
	param = calloc(MAXLEN,1);
	if(NULL == param)
	{
		goto error_out;
	}
	snprintf(param,MAXLEN,"token=%s",token);
	//FUFS_LOG("token %s %s\n",__FUNCTION__,token);
	buf = fufs_curl_fecth(URL_KEEP_TOKEN,param);
	
	ret = calloc(ERRMESSAGE,1);
	if(NULL == ret)
	{
		retype = FUFS_RET_FAIL;
		goto error_out;
	}
    fufs_json_get(buf ,"err_msg",NULL,ret,ERRMESSAGE);
	if(strcmp(ret,"success") != 0)
	{
		retype = FUFS_RET_FAIL;
		goto error_out;
	}
	fufs_json_get(buf,"dologid",NULL,dologid,ARRAY_MAX_SIZE);
	//FUFS_LOG("dologid %s\n",dologid);
error_out:
	FUFS_SAFE_FREE(buf);
	FUFS_SAFE_FREE(ret);
	FUFS_SAFE_FREE(param);	

	return retype;
}
char *fufs_api_get_list(char *dir_id)
{
	char *buf = NULL;
	char *param = NULL;
	char *ret = NULL;
	if(NULL == token)
	{
		goto error_out;
	}
	param = calloc(MAXLEN,1);
	if(NULL == param)
	{
		goto error_out;
	}
	
	snprintf(param,MAXLEN,"token=%s&dir_id=%s",token,dir_id);
	buf = fufs_curl_fecth(URL_GET_LIST,param);
	ret = calloc(ERRMESSAGE,1);
	if(NULL == ret)
	{		
		goto error_out;
	}
    fufs_json_get(buf ,"err_msg",NULL,ret,ERRMESSAGE);
	if(strcmp(ret,"success") != 0)
	{
		goto error_out;
	}	
error_out:
	FUFS_SAFE_FREE(ret);	
	FUFS_SAFE_FREE(param);	

	return buf;
}
fufs_ret fufs_api_create_folder(char *name,char *parent_id)
{
	char *param = NULL;
	char *ret = NULL;
	char *buf = NULL;
	fufs_ret retype = FUFS_RET_FAIL;

	if(NULL == token)
	{
		return FUFS_RET_FAIL;
	}
	param = calloc(MAXLEN,1);
	if(NULL == param)
	{
		goto error_out;
	}
	ret = calloc(ERRMESSAGE,1);
	if(NULL == ret)
	{
		goto error_out;
	}
	snprintf(param,MAXLEN,"token=%s&create_name=%s&parent_id=%s",token,name,parent_id);
	buf = fufs_curl_fecth(URL_CREATE_DIR,param);
	fufs_json_get(buf,"err_msg",NULL,ret,ERRMESSAGE);
	if(strcmp(ret,"success") != 0)
	{
		goto error_out;
	}
	retype = FUFS_RET_OK;
error_out:
	FUFS_SAFE_FREE(param);
	FUFS_SAFE_FREE(ret);
	FUFS_SAFE_FREE(buf);
	
	return retype;
}
fufs_ret fufs_api_delete(fufs_node_type node_type,char *id)
{
	char *param = NULL;
	char *ret = NULL;
	char *buf = NULL;
	fufs_ret retype = FUFS_RET_FAIL;
	if(NULL == token)
	{
		return FUFS_RET_FAIL;
	}
	param = calloc(MAXLEN,1);
	if(NULL == param)
	{
		goto error_out;
	}
	ret = calloc(ERRMESSAGE,1);
	if(NULL == ret)
	{
		goto error_out;
	}
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	if(FUFS_NODE_TYPE_FOLDER == node_type)
	{
		if(strcmp("id","0")==0)
		{
			goto error_out;
		}
		snprintf(param,MAXLEN,"token=%s&dir_id=%s",token,id);
		buf = fufs_curl_fecth(URL_DELETE_DIR,param);
	}
	else
	{
		snprintf(param,MAXLEN,"token=%s&fid=%s",token,id);
		buf = fufs_curl_fecth(URL_DELETE_FILE,param);
	}
	FUFS_FILE_LOG("[%s:%d] enter buf %s\n", __FUNCTION__, __LINE__,buf);
	fufs_json_get(buf,"err_msg",NULL,ret,ERRMESSAGE);
	if(strcmp(ret,"success") != 0)
	{
		goto error_out;
	}
	retype = FUFS_RET_OK;
error_out:
	FUFS_SAFE_FREE(param);
	FUFS_SAFE_FREE(ret);
	FUFS_SAFE_FREE(buf);
	FUFS_FILE_LOG("[%s:%d] enter \n", __FUNCTION__, __LINE__);
	return retype;
}
char *fufs_api_download_link_get(char *fid)
{
	char *buf = NULL;
	char *url = NULL;
	char *param = NULL;
	char *ret = NULL;
	if(NULL == token || NULL == fid)
	{
		return NULL;
	}
	param = calloc(MAXLEN,1);
	if(NULL == param)
	{
		goto error_out;
	}
	snprintf(param,MAXLEN,"token=%s&fid=%s",token,fid);
	buf = fufs_curl_fecth(URL_GET_FILE_INFO,param);
	ret = calloc(ERRMESSAGE,1);
	if(NULL == ret)
	{		
		goto error_out;
	}
	fufs_json_get(buf ,"err_msg",NULL,ret,ERRMESSAGE);
	if(strcmp(ret,"success") != 0)
	{
		goto error_out;
	}	
	url = calloc(MAXLEN,1);
	if(NULL == url)
	{
		goto error_out;
	}
	fufs_json_get(buf ,"s3_url","data",url,MAXLEN);
	
error_out:
	FUFS_SAFE_FREE(param);
	FUFS_SAFE_FREE(ret);
	FUFS_SAFE_FREE(buf);

	return url;
}
char *fufs_api_upload_file(char *dest_fullpath,char *src_fullpath,char *dir_id,char *cover)
{
	
	
	char *ret = NULL;
	char *reply = NULL;
	if(NULL == dest_fullpath || NULL == src_fullpath)
	{
		return NULL;
	}

	
	FUFS_FILE_LOG("[%s:%d] enter  \n", __FUNCTION__, __LINE__);
	reply = calloc(FUFS_MAX_BUF,1);
	if(NULL == reply)
	{
		goto error_out;
	}
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	fufs_curl_upload(URL_UPLOAD_FILE,src_fullpath,reply,token,dir_id,cover);
	ret = reply;
	FUFS_FILE_LOG("[%s:%d] enter ret %s \n", __FUNCTION__, __LINE__,ret);
error_out:
	
	

	return ret;
}
char *fufs_api_get_quota()
{
	char *buf = NULL;
	char *ret = NULL;
	char *param = NULL;
	if(NULL == token)
	{
		return NULL;
	}
	param = calloc(MAXLEN,1);
	if(NULL == param)
	{
		goto error_out;
	}
	//FUFS_LOG("token %s %s \n",__FUNCTION__,token);
	snprintf(param,MAXLEN,"token=%s",token);
	buf = fufs_curl_fecth(URL_GET_QUOTA,param);
	ret = calloc(ERRMESSAGE,1);
	if(NULL == ret)
	{		
		goto error_out;
	}
    	fufs_json_get(buf ,"err_msg",NULL,ret,ERRMESSAGE);
	if(strcmp(ret,"success") != 0)
	{
		goto error_out;
	}	
error_out:
	FUFS_SAFE_FREE(ret);	
	FUFS_SAFE_FREE(param);	

	return buf;	
}

