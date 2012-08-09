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
#ifndef __FUFS_CONF_H__
#define __FUFS_CONF_H__

#define URL_GET_TOKEN "http://openapi.vdisk.me/?m=auth&a=get_token"
#define URL_KEEP_TOKEN "http://openapi.vdisk.me/?m=user&a=keep_token"
#define URL_UPLOAD_FILE "http://openapi.vdisk.me/?m=file&a=upload_file"
#define URL_UPLOAD_SHARE_FILE "http://openapi.vdisk.me/?m=file&a=upload_share_file"
#define URL_CREATE_DIR "http://openapi.vdisk.me/?m=dir&a=create_dir"
#define URL_GET_LIST "http://openapi.vdisk.me/?m=dir&a=get_list"
#define URL_GET_QUOTA "http://openapi.vdisk.me/?m=file&a=get_quota"
#define URL_UPLOAD_WITH_SHA1 "http://openapi.vdisk.me/?m=file&a=upload_with_sha1"
#define URL_GET_FILE_INFO "http://openapi.vdisk.me/?m=file&a=get_file_info"
#define URL_DELETE_DIR "http://openapi.vdisk.me/?m=dir&a=delete_dir"
#define URL_DELETE_FILE "http://openapi.vdisk.me/?m=file&a=delete_file"
#define URL_COPY_FILE "http://openapi.vdisk.me/?m=file&a=copy_file"
#define URL_MOVE_FILE "http://openapi.vdisk.me/?m=file&a=move_file"
#define URL_RENAME_FILE "http://openapi.vdisk.me/?m=file&a=rename_file"
#define URL_RENAME_DIR "http://openapi.vdisk.me/?m=dir&a=rename_dir"
#define URL_MOVE_DIR "http://openapi.vdisk.me/?m=dir&a=move_dir"
#define URL_SHARE_FILE "http://openapi.vdisk.me/?m=file&a=share_file"
#define URL_CANCEL_SHARE_FILE "http://openapi.vdisk.me/?m=file&a=cancel_share_file"
#define URL_RECYCLE_GET_LIST "http://openapi.vdisk.me/?m=recycle&a=get_list"
#define URL_TRUNCATE_RECYCLE_GET "http://openapi.vdisk.me/?m=recycle&a=truncate_recycle"
#define URL_RECYCLE_DELETE_FILE "http://openapi.vdisk.me/?m=recycle&a=delete_file"
#define URL_RECYCLE_DELETE_DIR "http://openapi.vdisk.me/?m=recycle&a=delete_dir"
#define URL_RECYCLE_RESTORE_FILE "http://openapi.vdisk.me/?m=recycle&a=restore_file"
#define URL_RECYCLE_RESTORE_DIR "http://openapi.vdisk.me/?m=recycle&a=restore_dir"
#define URL_GET_DIRID_WITH_PATH "http://openapi.vdisk.me/?m=dir&a=get_dirid_with_path"
#define URL_EMAIL_SHARE_FILE "http://openapi.vdisk.me/?m=file&a=email_share_file"

int fufs_file_log(const char *fmt, ...);

#define FUFS_DEBUG

#ifdef FUFS_DEBUG
#define FUFS_LOG(format,args...)	fprintf(stdout, format, ## args)
#define FUFS_FILE_LOG			fufs_file_log
#else
#define FUFS_LOG(format,args...)
#define FUFS_FILE_LOG
#endif

#define APPKEY "2716459810"
#define APPSECRET "a264b1a005f245b69783e39c461aa8b0"
#define APPTYPELEN 8
#define FUFS_MAX_BUF 4096
#define ERRMESSAGE 10
#define ARRAY_MAX_SIZE 64

typedef enum {
	FUFS_RET_FAIL = -1,
	FUFS_RET_OK = 0,
} fufs_ret;

char *fufs_conf_get_writable_tmp_path();

#endif

