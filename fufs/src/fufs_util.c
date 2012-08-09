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
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>

#define __STRICT_ANSI__
#include <json/json.h>

#include "fufs.h"
#include "fufs_conf.h"
#include "fufs_util.h"
#include "fufs_api.h"

fufs_quota_info g_fufs_quota_info;


int fufs_file_log(const char *fmt, ...)
{
	va_list ap;
	char tmp[4096] = { 0 };
	char buf[4096] = { 0 };
	char log_file[FUFS_MAX_PATH] = { 0 };
	int fd = 0;
	char ftime[64] = { 0 };
	struct timeval tv;
	time_t curtime;
	int ret = 0;

	gettimeofday(&tv, NULL);
	curtime = tv.tv_sec;
	strftime(ftime, sizeof(ftime), "%F %T", localtime(&curtime));

	va_start(ap, fmt);
	vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);

	snprintf(buf, sizeof(buf), "[%s] %s", ftime, tmp);
	snprintf(log_file, sizeof(log_file), "%s/%s", fufs_conf_get_writable_tmp_path(), FUFS_LOG_FILE_NAME);
	fd = open(log_file, O_APPEND | O_WRONLY | O_CREAT, 0666);
	if (-1 == fd)
		return -1;
	ret = write(fd, buf, strlen(buf));
	if (-1 == ret)
		printf("fail to write: %s.\n", log_file);
	close(fd);
	return 0;
}

char *fufs_util_get_parent_path(char *path)
{
	char *p = NULL;
	char *parent_path = NULL;

	if (NULL == path)
		return NULL;
	FUFS_FILE_LOG("[%s:%d] enter path is %s\n", __FUNCTION__, __LINE__,path);
	p = strrchr(path, '/');
	if (NULL == p)
		return NULL;
	parent_path = calloc(FUFS_MAX_PATH, 1);
	if (NULL == parent_path)
		return NULL;
	if(path == p)
	{
		strcpy(parent_path,"/");
	}
	else
	{
		strncpy(parent_path, path, p - path);
	}
	FUFS_FILE_LOG("[%s:%d] enter parent_path is %s\n", __FUNCTION__, __LINE__,parent_path);
	return parent_path;
}

int fufs_util_quota_info_store(off_t quota_total,off_t quota_used)
{
	memset(&g_fufs_quota_info,0,sizeof(g_fufs_quota_info));
	
	g_fufs_quota_info.quota_total =quota_total;
	g_fufs_quota_info.quota_used = quota_used;
	
	return 0;
}

off_t fufs_util_quota_total_get()
{
	return g_fufs_quota_info.quota_total;
}

off_t fufs_util_quota_used_get()
{
	return g_fufs_quota_info.quota_used;
}

