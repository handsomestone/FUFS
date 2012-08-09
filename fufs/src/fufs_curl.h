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


#ifndef __FUFS_CURL_H__
#define __FUFS_CURL_H__

#include <curl/curl.h>

char *fufs_curl_fecth(const char *url,const char *param);
int fufs_curl_range_get(const char *url, char *buf, curl_off_t start_pos, curl_off_t end_pos);
int fufs_curl_upload(const char *url, char *file,char *reply,char *mtoken,char *mdir_id,char *cover);
#endif

