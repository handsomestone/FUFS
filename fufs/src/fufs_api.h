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

#ifndef	__FUFS_API_H__
#define	__FUFS_API_H__
#include "fufs_conf.h"
#include "fufs_node.h"
#define MAXLEN 1024

#define FUFS_SAFE_FREE(a) {if (a) {free(a); a = NULL;}}

typedef struct _fufs_token{
	char token[64];
	char dologid[64];
}fufs_token;

char *fufs_api_get_token(char *username,char *password,char *app_type);
char *fufs_get_token();
char *fufs_get_dologid();


fufs_ret fufs_api_keep_token();
char *fufs_api_get_list(char *dir_id);
char *fufs_api_get_quota();
fufs_ret fufs_json_get(char *buf,char *mkey,char *array_name ,char *dest,unsigned int dest_len);
fufs_ret fufs_api_create_folder(char *name,char *parent_id);
fufs_ret fufs_api_delete(fufs_node_type node_type,char *id);
char *fufs_api_download_link_get(char *fid);
char *fufs_api_upload_file(char *dest_fullpath,char *src_fullpath,char *dir_id,char *cover);
#endif

