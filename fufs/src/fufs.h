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


#ifndef __FUFS_H__
#define __FUFS_H__
#define FUFS_APP_NAME			"fufs"
#define FUFS_VERSION			"0.1"



#define FUFS_MAX_PATH			(512)
#define FUFS_LOG_FILE_NAME		"fufs.log"


#define FUFS_FILE_ID "id"
#define FUFS_FILE_NAME "name"
#define FUFS_DIR_ID "dir_id"
#define FUFS_FILE_CTIME "ctime"
#define FUFS_FILE_LTIME "ltime"
#define FUFS_FILE_SIZE "size"
#define FUFS_FILE_TYPE "type"
#define FUFS_FILE_SHA1 "sha1"

#define FUFS_CURL_LOW_SPEED_LIMIT	(1)
#define FUFS_CURL_LOW_SPEED_TIMEOUT	(60)


#define FUFS_COOKIE_FILE_NAME		"cookie.txt"

#endif
