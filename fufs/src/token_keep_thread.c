#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#define ARRAY_MAX_SIZE 64
#define FUFS_MAX_BUF 4096


#ifdef DEBUG
#define FUFS_LOG(format,args...)	fprintf(stdout, format, ## args)
#else
#define FUFS_LOG(format,args...)
#endif

#define URL_KEEP_TOKEN "http://openapi.vdisk.me/?m=user&a=keep_token"
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

int main()
{
	int fd;
	int number;
	char *buf=NULL;
	char token[ARRAY_MAX_SIZE];
	char param[ARRAY_MAX_SIZE];
	while(1)
	{	
		FUFS_LOG("IN FUNCTION\n");
		fd = open("/tmp/token.log",O_RDONLY);
		if(-1 == fd)
		{
			continue;
		}
		memset(token,0,ARRAY_MAX_SIZE);
		number = read(fd,token,ARRAY_MAX_SIZE);	
		if(0 == number)
		{
			close(fd);
			continue;
		}
		
		snprintf(param,ARRAY_MAX_SIZE,"token=%s",token);
		buf = fufs_curl_fecth(URL_KEEP_TOKEN,param);
		close(fd);
		FUFS_LOG("data buf is %s\n",buf);
		FUFS_LOG("token is %s\n",token);	
		sleep(60);
		if(buf)
		{
			free(buf);
			buf = NULL;
		}
	}

} 
