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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <errno.h>
#include <fuse.h>
#include <fuse_opt.h>
#include <oauth.h>
#include <string.h>
#define __STRICT_ANSI__
#include <json/json.h>

#include "fufs.h"
#include "fufs_node.h"
#include "fufs_conf.h"
#include "fufs_api.h"
#include "fufs_curl.h"
#include "fufs_util.h"

static int fufs_first_run = 1;


void fufs_node_dump(fufs_node * node)
{
	if (NULL == node) {
		FUFS_FILE_LOG("[%s:%d] node is NULL.\n", __FUNCTION__, __LINE__);
		return;
	}

	FUFS_FILE_LOG("node data:\n");
	FUFS_FILE_LOG("\tfullpath: %s\n", node->fullpath);
	FUFS_FILE_LOG("\tid: %s\n", node->id);
	FUFS_FILE_LOG("\tname: %s\n", node->name);
	FUFS_FILE_LOG("\tdir_id: %s\n", node->dir_id);
	FUFS_FILE_LOG("\ttype: %d (0:file, 1: folder)\n", node->type);
	FUFS_FILE_LOG("\tsha1: %d\n", node->sha1);
	FUFS_FILE_LOG("\tst.st_size: " CURL_FORMAT_OFF_T "\n", node->st.st_size);
	FUFS_FILE_LOG("\tst.st_ctime: %lu\n", node->st.st_ctime);
	FUFS_FILE_LOG("\tst.st_mtime: %lu\n", node->st.st_mtime);
}


static char *fufs_get_tmpfile(struct fuse_file_info *fi)
{
	return (char *)(uintptr_t) fi->fh;
}




static char *fufs_gen_tmp_fullpath(const char *fullpath)
{
	
	char *tmp_fullpath = NULL;
	
	char *name = NULL;
	char *p;
	if (NULL == fullpath)
		return NULL;

	tmp_fullpath = calloc(FUFS_MAX_PATH, 1);
	if (NULL == tmp_fullpath)
		return NULL;
	name = calloc(MAXLEN,1);
	if(NULL == name)
		return NULL;
	p =strrchr(fullpath,'/');
	if(NULL == p)
	{
		strcpy(name,fullpath);
	}
	else
	{
		strncpy(name,p+1,strlen(fullpath)-(p-fullpath));
	}
	
	FUFS_FILE_LOG("[%s:%d] name  %s\n", __FUNCTION__, __LINE__,name);
	
	FUFS_FILE_LOG("[%s:%d] enterpath  %s\n", __FUNCTION__, __LINE__,fullpath);
	snprintf(tmp_fullpath, FUFS_MAX_PATH, "%s/%s", "/tmp", name);

	FUFS_SAFE_FREE(name);
	return tmp_fullpath;
}

static int fufs_getattr( const char *path,struct stat *stbuf)
{
	fufs_node *node = NULL;
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	if (!path || !stbuf)
		return -1;

	if (1 == fufs_first_run) {
		fufs_node_get_root_path();
		fufs_node_parse_dir((fufs_node *) fufs_node_root_get(), "/",fufs_node_root_get_id());
		fufs_first_run = 0;
	}

	memset(stbuf, 0, sizeof(*stbuf));

	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("path: %s\n", path);

	if (0 == strcmp(path, "/.Trash") || 0 == strcmp(path, "/.Trash-1000")) {
		FUFS_FILE_LOG("we will not handle %s\n", path);
		return -1;
	}
	node = fufs_node_get_by_path((fufs_node *) fufs_node_root_get(), path);
	if (NULL == node) {
		FUFS_FILE_LOG("%s:%d, could not find: %s\n", __FUNCTION__, __LINE__, path);
		return -ENOENT;
	}
	fufs_node_dump(node);
	memcpy(stbuf, &(node->st), sizeof(node->st));
	return 0;
}

static int fufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	fufs_node *node = NULL;

	(void)offset;
	(void)fi;
	
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	node = fufs_node_get_by_path((fufs_node *) fufs_node_root_get(), path);
	if (node) {
		GHashTableIter iter;
		char *key = NULL;
		fufs_node *value = NULL;

		if (1 != strlen(path)) {
		
		}

		g_hash_table_iter_init(&iter, node->sub_nodes);
		while (g_hash_table_iter_next(&iter, (gpointer *) & key, (gpointer *) & value)) {
			FUFS_FILE_LOG("%s:%d, path: %s, filler: %s\n", __FUNCTION__, __LINE__, path, value->name);
			filler(buf, value->name, NULL, 0);
		}
	} else {
		return -1;
	}
	return 0;
}

static int fufs_access(const char *path, int mask)
{
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] path: %s, mask: %d.\n", __FUNCTION__, __LINE__, path, mask);
	return 0;
}

static int fufs_delete(const char *path)
{
	fufs_node *node = NULL;
	fufs_node *parent_node = NULL;
	char *parent_path = NULL;
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	if(NULL == path)
	{
		return -1;
	}
	node= fufs_node_get_by_path((fufs_node * )fufs_node_root_get(),path);
	if(NULL == node)
	{
		return -EEXIST;
	}
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	if(FUFS_RET_FAIL == fufs_api_delete(node->type,node->id))
	{
		FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
		return -1;
	}
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	parent_path = fufs_util_get_parent_path((char *) path);
	parent_node = fufs_node_get_by_path((fufs_node *) fufs_node_root_get(), parent_path);
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_SAFE_FREE(parent_path);
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	fufs_node_rebuild(parent_node);
	FUFS_FILE_LOG("[%s:%d] enter \n", __FUNCTION__, __LINE__);
	return 0;
}
static int fufs_unlink(const char *path)
{
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] path: %s.\n", __FUNCTION__, __LINE__, path);

	return fufs_delete(path);
}
static int fufs_is_swap_file(const char *path)
{
	char *p = NULL;
	if (NULL == path)
		return 0;
	p = strrchr(path, '.');
	if (p && (0 == strcasecmp(p, ".swp"))) {
		return 1;
	}
	return 0;
}

static int fufs_open(const char *path, struct fuse_file_info *fi)
{
	int ret = 0;
	fufs_node *node = NULL;
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] path: %s.\n", __FUNCTION__, __LINE__, path);	
	if(NULL == path)
	{
		return -1;
	}
	if(1 == fufs_is_swap_file(path))
	{
		return -ENOTSUP;
	}
	node = fufs_node_get_by_path((fufs_node *)fufs_node_root_get(),path);
	if(NULL == node)
	{
		return -1;
	}
	if((fi->flags & O_ACCMODE) == O_RDONLY)
	{
		return 0;
	}
	else if(fi->flags & (O_RDWR | O_WRONLY))
	{
		char *tmpfile = fufs_gen_tmp_fullpath(path);
		fi->fh = (unsigned long)tmpfile;
	}
	else
	{
		ret = -ENOTSUP;
	}
	return ret;
}

static int fufs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	int fd = 0;
	char *tmpfile = NULL;
	

	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] path: %s, mode: %d, file info: %p.\n", __FUNCTION__, __LINE__, path, mode, fi);

	if (1 == fufs_is_swap_file(path))
		return -ENOTSUP;

	tmpfile = fufs_gen_tmp_fullpath(path);

	fi->fh = (unsigned long)tmpfile;

	fd = open(tmpfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd == -1)
		return -errno;
	
	close(fd);

	return 0;
}


static int fufs_read(const char *path,char *rbuf,size_t size,off_t offset,struct fuse_file_info *fi)
{
	char *url = NULL;
	fufs_node *node = NULL;
	int ret = 0;
	if(NULL == path || NULL == rbuf)
	{
		return -1;
	}
        if(NULL == fi )
	{

	}
	node = fufs_node_get_by_path((fufs_node *)fufs_node_root_get(),path);
	if(NULL == node)
	{
		return -1;
	}
	url = fufs_api_download_link_get(node->id);
	FUFS_FILE_LOG("[%s:%d] enter url is %s\n", __FUNCTION__, __LINE__,url);
	ret = fufs_curl_range_get(url, rbuf, offset, offset + size - 1);

	FUFS_SAFE_FREE(url);
	return ret;
}
static int fufs_write(const char *path, const char *wbuf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int fd =0 ;
	int ret = 0;
	char *tmpfile = NULL;
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] path: %s.\n", __FUNCTION__, __LINE__, path);
	if(1 == fufs_is_swap_file(path))
	{
		return -ENOTSUP;
	}
	
	tmpfile = fufs_get_tmpfile(fi);
	fd = open(tmpfile,O_WRONLY | O_CREAT | O_TRUNC,0666);
	if(-1 == fd)
	{
		return -errno;
	}
	ret = pwrite(fd,wbuf,size,offset);
	if(-1 == ret)
	{
		ret = -errno;
	}
	close(fd);
	return ret;
}
static int fufs_release(const char *path, struct fuse_file_info *fi)
{
	
	char *tmpfile = NULL;
	char *parent_path = NULL;
	char *cover="yes";
	fufs_node *parent_node = NULL;
	char *ret = NULL;
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] path: %s.\n", __FUNCTION__, __LINE__, path);
	if(1 == fufs_is_swap_file(path))
	{
		return -ENOTSUP;
	}
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] \n", __FUNCTION__, __LINE__);
	if(fi->flags & (O_RDWR | O_WRONLY))
	{
		FUFS_FILE_LOG("[%s:%d] \n", __FUNCTION__, __LINE__);
		tmpfile = fufs_get_tmpfile(fi);
		FUFS_FILE_LOG("[%s:%d] tmpfile %s \n", __FUNCTION__, __LINE__,tmpfile);
		parent_path = fufs_util_get_parent_path((char *)path);
		FUFS_FILE_LOG("[%s:%d] parent_path %s \n", __FUNCTION__, __LINE__,parent_path);
		parent_node = fufs_node_get_by_path((fufs_node *) fufs_node_root_get(), parent_path);
		FUFS_FILE_LOG("[%s:%d]parent_node is %s \n", __FUNCTION__, __LINE__,parent_node->id);
		FUFS_FILE_LOG("[%s:%d] tmpfile :%s parent_path :%s parent_node:%s \n", __FUNCTION__, __LINE__,tmpfile,parent_path,parent_node->id);
		ret = fufs_api_upload_file((char *)path,tmpfile,parent_node->id,cover);
		FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
		FUFS_FILE_LOG("[%s:%d] ret: %s.\n", __FUNCTION__, __LINE__, ret);
		unlink(tmpfile);
		FUFS_SAFE_FREE(tmpfile);
		FUFS_SAFE_FREE(parent_path);
		FUFS_SAFE_FREE(ret);
		fufs_node_rebuild(parent_node);
	}
	return 0;
}

static int fufs_mkdir(const char *path,mode_t mode)
{
	fufs_node *node = NULL;
	fufs_node *parent_node = NULL;
	char parent_path[FUFS_MAX_PATH] ={0};
	char *p = NULL;
	char *tmp = NULL;
	char folder_name[FUFS_MAX_PATH] = {0};
	if(NULL == path)
	{
		return -1;
	}

	node = fufs_node_get_by_path((fufs_node *) fufs_node_root_get(),path);
	if(NULL != node)
	{
		FUFS_FILE_LOG("[%s:%d] fufs_mkdir, %s is existed.\n", __FUNCTION__, __LINE__, path);
		return -EEXIST;
	}
	p  = strrchr(path,'/');
	if(path == p)
	{
		parent_node = fufs_node_root_get();
	}
	else 
	{
		tmp = (char *)path;
		memset(parent_path,0,sizeof(parent_path));
		strncpy(parent_path,tmp,p-tmp);
		parent_node = fufs_node_get_by_path((fufs_node *) fufs_node_root_get(),parent_path);
	}
	memset(folder_name,0,sizeof(folder_name));
	p = strrchr(path,'/');
	if(NULL == p)
	{
		strncpy(folder_name,path,strlen(path));
	}
	else
	{
		strncpy(folder_name,p+1,strlen(path)-(p-path));
	}
	FUFS_FILE_LOG("[%s:%d] file name %s mode %#o\n", __FUNCTION__, __LINE__,folder_name,mode);
	if(FUFS_RET_FAIL == fufs_api_create_folder(folder_name,parent_node->id))
	{
		return -1;
	}
	FUFS_FILE_LOG("[%s:%d] parent_path: %s, fullpath: %s\n", __FUNCTION__, __LINE__, parent_path, parent_node->fullpath);

	fufs_node_rebuild(parent_node);
	
	return 0;
}

static int fufs_rmdir(const char *path)
{
	
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] path: %s.\n", __FUNCTION__, __LINE__, path);
	
	return fufs_delete(path);
}
static int fufs_truncate(const char *path, off_t offset)
{
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] path: %s, offset: %d.\n", __FUNCTION__, __LINE__, path, offset);
	return 0;
}
static int fufs_rename(const char *from, const char *to)
{
	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] from: %s, to: %s.\n", __FUNCTION__, __LINE__, from, to);
	return 0;
}
static int fufs_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	struct timeval tv[2];

	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(path, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int fufs_statfs(const char *path, struct statvfs *buf)
{
	off_t quota_total = fufs_util_quota_total_get();
	off_t quota_used = fufs_util_quota_used_get();

	FUFS_FILE_LOG("[%s:%d] enter\n", __FUNCTION__, __LINE__);
	FUFS_FILE_LOG("[%s:%d] path: %s, statvfs: %p.\n", __FUNCTION__, __LINE__, path, buf);

	

	buf->f_bsize = 1;
	buf->f_frsize = 1;
	buf->f_blocks = quota_total;
	buf->f_bfree = quota_total - quota_used;
	buf->f_bavail = quota_total - quota_used;

	return 0;
}



static struct fuse_operations fufs_oper = {
	.getattr = fufs_getattr,
	.access = fufs_access,	
	.readdir = fufs_readdir,
	.mkdir = fufs_mkdir,
	.unlink = fufs_unlink,
	.rmdir = fufs_rmdir,
	.release = fufs_release,
	.rename = fufs_rename,
	.truncate = fufs_truncate,
	.utimens = fufs_utimens,
	.open = fufs_open,
	.create = fufs_create,
	.read = fufs_read,
	.write = fufs_write,
	.statfs = fufs_statfs,
};

int main(int argc, char *argv[])
{
	int fuse_ret =0;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	/**
		fufs_api_get_token(char *account,char *password,char *app_type);
		if you want to use FUFS,you should use your own account and password.
		you can use 'local' as you default app_type,or 'sinat' as your prefer. 
	 */
	fufs_api_get_token("input your account","input your password","local");	
	fuse_ret = fuse_main(argc,argv,&fufs_oper,NULL);	
	return fuse_ret;
}


