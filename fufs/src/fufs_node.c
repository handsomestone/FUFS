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
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <curl/curl.h>

#define __STRICT_ANSI__
#include <json/json.h>

#include "fufs_node.h"
#include "fufs_api.h"
#include "fufs_util.h"
#include "fufs.h"
#include "fufs_conf.h"
fufs_node *g_fufs_node_root = NULL;

fufs_node *fufs_node_root_get()
{
	return g_fufs_node_root;
}
char *fufs_node_root_get_id()
{
	return g_fufs_node_root->id;
}
static time_t fufs_node_str2time(char *str)
{
	time_t times = 0;
	int i,len;
	len = strlen(str);
	for(i=0;i<len;i++)
	{
		times = times*10+(str[i]-'0');
	}
	return times;
}
fufs_node *fufs_node_root_create(char *id,char *name,off_t size)
{
	fufs_node *root = NULL;

	if(NULL != g_fufs_node_root)
	{
		return g_fufs_node_root;
	}
	
	root = calloc(sizeof(fufs_node),1);
	if(NULL == root)
	{
		return NULL;
	}
	root->fullpath = strdup("/");
	root->id = id;
	root->name = name;
	root->type = FUFS_NODE_TYPE_FOLDER;
	root->sha1 = NULL;
	root->dir_id = NULL;
	root->st.st_mode = S_IFDIR | 0755;
	root->st.st_nlink = 2;
	root->st.st_size = size;
	pthread_mutex_init(&(root->mutex), NULL);
	root->sub_nodes = g_hash_table_new(g_str_hash, g_str_equal);
	g_fufs_node_root = root;
	
	return g_fufs_node_root;

}
off_t strToLong(char *str)
{
	int len ;
	int i;
	off_t data=0;
	len = strlen(str);
	for(i=0;i<len;i++)
	{
		data = data*10+(str[i]-'0');
	}
	return data;
}
off_t fufs_node_str2size(char *str)
{
	int len;
	int i;
	off_t data=0;
	char *ptr;
	ptr = strchr(str,' ');
	len = ptr-str;
	for(i=0;i<len;i++)
	{
		data = data*10 + (str[i]-'0');
	}
	return data;
}
int fufs_node_get_root_path()
{
	char *response = NULL;
	
	off_t quota_used = 0;
	off_t quota_total = 0;
	char *tmp = NULL;
	tmp = calloc(MAXLEN,1);
	if(NULL == tmp)
	{
		return -1;
	}
	response = fufs_api_get_quota();
	if(NULL == response)
	{
		return -1;
	}
	//FUFS_LOG("get root _path response %s\n",response);
	fufs_json_get(response,"used","data",tmp,MAXLEN);
	quota_used = strToLong(tmp);
	memset(tmp,0,MAXLEN);
	
	fufs_json_get(response,"total","data",tmp,MAXLEN);
	quota_total = strToLong(tmp);
	//FUFS_LOG("get root _path response total %s\n",tmp);
	FUFS_SAFE_FREE(response);
	FUFS_SAFE_FREE(tmp);

	fufs_util_quota_info_store(quota_total,quota_used);
	
	if(NULL == fufs_node_root_create("0","root",quota_used))
	{
		return -1;
	}
	//FUFS_LOG("get root _path quota used %lld total %lld\n",(long long)quota_used,(long long)quota_total);
	return 0;
}

fufs_node *fufs_node_get_by_path(fufs_node *node, const char *path)
{
	fufs_node *ret = NULL;
	GHashTableIter iter;

	if (NULL == path || NULL == node)
		return NULL;

	if (1 == strlen(path) && path[0] == '/')
		return fufs_node_root_get();

	ret = g_hash_table_lookup(node->sub_nodes, path);
	if (NULL == ret) {
		char *key = NULL;
		fufs_node *value = NULL;

		//FUFS_LOG("[%s:%d] could not find %s in node->sub_nodes.\n", __FUNCTION__, __LINE__, path);
		g_hash_table_iter_init(&iter, node->sub_nodes);
		while (g_hash_table_iter_next(&iter, (gpointer *) & key, (gpointer *) & value)) {
			//FUFS_LOG("[%s:%d] iter, key: %s, path: %s\n", __FUNCTION__, __LINE__, key, path);
			ret = fufs_node_get_by_path(value, path);
			if (ret)
				return ret;
		}
	} else {
		return ret;
	}
	return ret;
}
fufs_ret fufs_node_parse_dir(fufs_node * parent_node, const char *path,char *parent_node_id)
{
	char *response = NULL;
	json_object *jobj;
	//json_object *tmp_jobj;
	json_object *tmp2_jobj;
	fufs_node *node = NULL;
	int len = 0;
	char *parent_path = NULL;
	fufs_ret ret = FUFS_RET_FAIL;
	int array_ind,array_len=0;
	int is_dir = -1;
	char str[128] = {0};
	if(NULL == parent_node)
	{
		
		return FUFS_RET_FAIL;
	}

	response = fufs_api_get_list(parent_node_id);
	//FUFS_LOG("response %s:\n", response);
	
	jobj = json_tokener_parse(response);
	if(NULL == jobj || is_error(jobj))
	{
		FUFS_FILE_LOG("%s:%d, json_tokener_parse return error.\n", __FUNCTION__, __LINE__);
		FUFS_SAFE_FREE(response);
		return FUFS_RET_FAIL;
	}
	//FUFS_LOG(" %s: %d\n", __FUNCTION__,__LINE__);
	json_object_object_foreach(jobj,key,val)
	{//FUFS_LOG(" %s: %d\n", __FUNCTION__,__LINE__);
		if(strcmp(key,"data")==0)
		{//FUFS_LOG(" %s: %d\n", __FUNCTION__,__LINE__);			
			//FUFS_LOG(" %s: %d %d\n", __FUNCTION__,__LINE__,array_len);	
			if(json_type_array == json_object_get_type(val))
			{//FUFS_LOG(" %s: %d\n", __FUNCTION__,__LINE__);
				array_ind = 0;
				array_len = 0;
				array_len =json_object_array_length(val);
				for(array_ind =0 ;array_ind<array_len;array_ind++)
				{
					tmp2_jobj = json_object_array_get_idx(val,array_ind);
					if(tmp2_jobj && !is_error(tmp2_jobj))
					{
						is_dir = -1;
						node = calloc(sizeof(fufs_node),1);
						if(NULL == node)
						{
							FUFS_FILE_LOG("%s:%d, fail to calloc node.\n", __FUNCTION__, __LINE__);
							goto error_out;
						}
						errno = pthread_mutex_init(&(node->mutex), NULL);
						if (errno) {
							FUFS_FILE_LOG("%s:%d, pthread_mutex_init fail.\n", __FUNCTION__, __LINE__);
							goto error_out;
						}
						node->sub_nodes = g_hash_table_new(g_str_hash, g_str_equal);
						len = strlen(path) + 1;
						parent_path = calloc(len, 1);
						snprintf(parent_path, len, "%s", path);
						FUFS_FILE_LOG("\n", __FUNCTION__, __LINE__);
						json_object_object_foreach(tmp2_jobj,key3,val3)	
						{
							if(!strcmp(key3,FUFS_FILE_ID))
							{
								if(json_type_string == json_object_get_type(val3))
								{
									node->id = strdup(json_object_get_string(val3));
								}
							}
							else if(!strcmp(key3,FUFS_FILE_NAME))
							{
								if(json_type_string == json_object_get_type(val3))
								{
									node->name = strdup(json_object_get_string(val3));
								}
							}
							else if(!strcmp(key3,FUFS_DIR_ID))
							{
								if(json_type_string == json_object_get_type(val3))
								{
									node->dir_id = strdup(json_object_get_string(val3));
								}
							}
							else if(!strcmp(key3,FUFS_FILE_CTIME))
							{
								if(json_type_string == json_object_get_type(val3))
								{
									memset(str,0,sizeof(str));
									snprintf(str,sizeof(str),"%s",json_object_get_string(val3));
									node->st.st_ctime = fufs_node_str2time(str);
								}
							}
							else if(!strcmp(key3,FUFS_FILE_LTIME))
							{
								if(json_type_string == json_object_get_type(val3))
								{
									memset(str,0,sizeof(str));
									snprintf(str,sizeof(str),"%s",json_object_get_string(val3));
									node->st.st_mtime = fufs_node_str2time(str);
								}
							}
							else if(!strcmp(key3,FUFS_FILE_SIZE))
							{
								if(json_type_string == json_object_get_type(val3))
								{
									memset(str,0,sizeof(str));
									snprintf(str,sizeof(str),"%s",json_object_get_string(val3));
									node->st.st_size = fufs_node_str2size(str);
								}
							}
							else if(!strcmp(key3,FUFS_FILE_TYPE))
							{
								is_dir =1;
							}
							else if(!strcmp(key3,FUFS_FILE_SHA1))
							{
								if(json_type_string == json_object_get_type(val3))
								{		
									node->sha1= strdup(json_object_get_string(val3));
								}
							}
						}	
						if(is_dir == -1)
						{
							node->type = FUFS_NODE_TYPE_FOLDER;
						}
						else
						{
							node->type = FUFS_NODE_TYPE_FILE;
						}
						if(node->name)
						{
							if(1 == strlen(parent_path)&&parent_path[0] == '/')
							{
								len = strlen(parent_path) + strlen(node->name) +1;
								node->fullpath = calloc(len,1);
								snprintf(node->fullpath,len,"%s%s",parent_path,node->name);
							}
							else
							{
								len = strlen(parent_path) + strlen("/") + strlen(node->name) + 1;
								node->fullpath = calloc(len, 1);
								snprintf(node->fullpath, len, "%s/%s", parent_path, node->name);
							}
							if(FUFS_NODE_TYPE_FOLDER == node->type)
							{
								//FUFS_LOG(" %s: %d %s %s!!!\n", __FUNCTION__,__LINE__,node->fullpath,node->id);
								fufs_node_parse_dir(node,node->fullpath,node->id);
								node->st.st_mode = S_IFDIR | 0755;
								node->st.st_nlink = 2;
								if (0 == node->st.st_size)
									node->st.st_size = getpagesize();
							}
							else
							{
								node->st.st_mode = S_IFREG | 0666;
								node->st.st_nlink = 1;
							}
							FUFS_NODE_LOCK(parent_node);
							g_hash_table_insert(parent_node->sub_nodes, node->fullpath, node);
							FUFS_NODE_UNLOCK(parent_node);
						}
		
					}
				}		
			}				
/*if (tmp_jobj && !is_error(tmp_jobj)) 
{	
	FUFS_LOG(" %s: %d\n", __FUNCTION__,__LINE__);	
	array_len =json_object_array_length(val);	
	FUFS_LOG(" %s: %d %d\n", __FUNCTION__,__LINE__,array_len);
	for(array_ind=0;array_ind<array_len;array_ind++)
	{
		
		
	}		
	json_object_object_foreach(tmp_jobj, key2, val2) 
	{
							
			FUFS_LOG(" %s: %d\n", __FUNCTION__,__LINE__);
			tmp2_jobj = NULL;	fufs_node_str2time("111");
			is_dir=0;array_len=1;array_ind=0;parent_path=NULL;node=NULL;len=0;path=NULL;node=NULL;	
			goto error_out;
	}
}*/
}
	}
	ret = FUFS_RET_OK;

error_out:
	json_object_put(jobj);
	FUFS_SAFE_FREE(response);

	return ret;
}
void fufs_node_free(gpointer p)
{
	fufs_node *node = (fufs_node *) p;

	if (NULL == p)
		return;

	FUFS_SAFE_FREE(node->fullpath);
	FUFS_SAFE_FREE(node->id);
	FUFS_SAFE_FREE(node->name);
	FUFS_SAFE_FREE(node->dir_id);
	FUFS_SAFE_FREE(node->sha1);
	pthread_mutex_destroy(&(node->mutex));
}

void fufs_node_free_sub_nodes(GHashTable * sub_nodes)
{
	GHashTableIter iter;
	char *key = NULL;
	fufs_node *value = NULL;

	if (NULL == sub_nodes)
		return;

	g_hash_table_iter_init(&iter, sub_nodes);
	while (g_hash_table_iter_next(&iter, (gpointer *) & key, (gpointer *) & value)) {
		if (value) {
			fufs_node_free_sub_nodes(value->sub_nodes);
			g_hash_table_destroy(value->sub_nodes);
			fufs_node_free(value);
			FUFS_SAFE_FREE(value);
		}
	}
}

fufs_ret fufs_node_rebuild(fufs_node * node)
{
	if (NULL == node || NULL == node->sub_nodes)
		return FUFS_RET_FAIL;

	fufs_node_free_sub_nodes(node->sub_nodes);
	g_hash_table_remove_all(node->sub_nodes);

	return fufs_node_parse_dir(node, node->fullpath,node->id);
}

