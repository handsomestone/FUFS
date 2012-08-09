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

#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include "fufs_curl.h"
#include "fufs_conf.h"
#include "fufs.h"

typedef struct fufs_curl_data_t fufs_curl_data;

struct fufs_curl_data_t{
	char *buf;
	off_t count;
};
static size_t write_data(void *data, size_t size, size_t nmemb, void *stream)
{
	fufs_curl_data *param = (fufs_curl_data *) stream;
	memcpy((param->buf + param->count), data, size * nmemb);
	param->count += size * nmemb;
	return size * nmemb;
}
char *fufs_curl_fecth(const char *url,const char *param)
{
	char *buf = NULL;
	CURL *curl_handle;
	CURLcode ret ;
	fufs_curl_data data;
	memset(&data,0,sizeof(fufs_curl_data));
	/* init the curl session */  
	curl_handle = curl_easy_init(); 
	if(NULL == curl_handle)
	{
		return NULL;
	}
	//FUFS_LOG("token buf is %s\n",param);
    /* set URL to get */  
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    
	/* no progress meter please */  
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);  
	
	if(NULL != param)
	{
		curl_easy_setopt(curl_handle,CURLOPT_POST,1);
		curl_easy_setopt(curl_handle,CURLOPT_POSTFIELDS,param);
	}
	/* send all data to this function  */  
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

	buf = calloc(FUFS_MAX_BUF,1);
	if(NULL == buf)
	{
		curl_easy_cleanup(curl_handle);
		return NULL;
	}
	data.buf = buf;
	data.count = 0;
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&data);
	
	ret = curl_easy_perform(curl_handle);

	/* Check for errors */   
	if(ret != CURLE_OK)     
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret)); 
   		return NULL;
	}
	//FUFS_LOG("data buf is %s\n",data.buf);
	//FUFS_LOG("data size is %d\n",(int)data.count);
	curl_easy_cleanup(curl_handle); 
	
	return buf;     
}
int fufs_curl_range_get(const char *url, char *buf, curl_off_t start_pos, curl_off_t end_pos)
{
	CURL *curl_handle = NULL;
	CURLcode ret = CURLE_GOT_NOTHING;
	char errbuf[CURL_ERROR_SIZE];
	char range[128] = { 0 };
	char cookie_path[FUFS_MAX_PATH] = { 0 };
	fufs_curl_data data;

	if (NULL == url || NULL == buf)
		return -1;

	memset(buf, 0, end_pos - start_pos);

	data.buf = buf;
	data.count = 0;

	curl_handle = curl_easy_init();

	snprintf(range, sizeof(range), CURL_FORMAT_OFF_T "-" CURL_FORMAT_OFF_T, start_pos, end_pos);
	FUFS_FILE_LOG("range = %s.\n", range);

	curl_easy_reset(curl_handle);

	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, FUFS_APP_NAME "/" FUFS_VERSION);

	curl_easy_setopt(curl_handle, CURLOPT_URL, url);

	curl_easy_setopt(curl_handle, CURLOPT_RANGE, range);

	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);

	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

	snprintf(cookie_path, sizeof(cookie_path), "%s/%s", (char *)fufs_conf_get_writable_tmp_path(), FUFS_COOKIE_FILE_NAME);
	curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, cookie_path);

	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&data);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

	curl_easy_setopt(curl_handle, CURLOPT_LOW_SPEED_LIMIT, FUFS_CURL_LOW_SPEED_LIMIT);	// Low limit : ? bytes per seconds
	curl_easy_setopt(curl_handle, CURLOPT_LOW_SPEED_TIME, FUFS_CURL_LOW_SPEED_TIMEOUT);	// Below low limit for ? seconds

	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errbuf);

	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

	ret = curl_easy_perform(curl_handle);

	curl_easy_cleanup(curl_handle);

	if (ret != CURLE_OK) {
		FUFS_FILE_LOG("%s\n", curl_easy_strerror(ret));
		FUFS_FILE_LOG("[%s, %d] curl told us %d: %s.\n", __FUNCTION__, __LINE__, ret, errbuf);
		return -1;
	}
	FUFS_FILE_LOG("[%s:%d] enter buf is %s\n", __FUNCTION__, __LINE__,data.buf);
	return data.count;
}

int fufs_curl_upload(const char *url, char *file,char *reply,char *mtoken,char *mdir_id,char *cover)
{
	CURL *curl_handle = NULL;
	CURLcode ret = CURLE_GOT_NOTHING;
	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	struct curl_slist *headerlist = NULL;
	static const char expect_buf[] = "Expect:";
	char errbuf[CURL_ERROR_SIZE];
	char cookie_path[FUFS_MAX_PATH] = { 0 };
	fufs_curl_data data;

	if (NULL == url || NULL == reply || NULL == file )
		return -1;

	data.buf = reply;
	data.count = 0;

	curl_handle = curl_easy_init();

	if (NULL == curl_handle)
		return -1;

	curl_easy_reset(curl_handle);

	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, FUFS_APP_NAME "/" FUFS_VERSION);

	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

	snprintf(cookie_path, sizeof(cookie_path), "%s/%s", (char *)fufs_conf_get_writable_tmp_path(), FUFS_COOKIE_FILE_NAME);
	curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, cookie_path);

	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&data);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

	curl_easy_setopt(curl_handle, CURLOPT_LOW_SPEED_LIMIT, FUFS_CURL_LOW_SPEED_LIMIT);	// Low limit : ? bytes per seconds
	curl_easy_setopt(curl_handle, CURLOPT_LOW_SPEED_TIME, FUFS_CURL_LOW_SPEED_TIMEOUT);	// Below low limit for ? seconds

	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errbuf);

	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, file, CURLFORM_END);
	

	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "token",   CURLFORM_COPYCONTENTS, mtoken, CURLFORM_END); 
	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "dir_id",   CURLFORM_COPYCONTENTS, mdir_id, CURLFORM_END); 
	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "cover",	 CURLFORM_COPYCONTENTS, cover, CURLFORM_END); 
	
	headerlist = curl_slist_append(headerlist, expect_buf);

	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	

	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headerlist);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPPOST, formpost);

	ret = curl_easy_perform(curl_handle);

	curl_easy_cleanup(curl_handle);
	curl_formfree(formpost);
	curl_slist_free_all(headerlist);
	FUFS_FILE_LOG("[%s, %d] curl told us %d\n", __FUNCTION__, __LINE__, ret);
	FUFS_FILE_LOG("[%s, %d] curl told us %s\n", __FUNCTION__, __LINE__, data.buf);
	if (ret != CURLE_OK) {
		FUFS_FILE_LOG("%s\n", curl_easy_strerror(ret));
		FUFS_FILE_LOG("[%s, %d] curl told us %d: %s.\n", __FUNCTION__, __LINE__, ret, errbuf);
		return -1;
	}
	return data.count;
}


