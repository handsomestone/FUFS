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

#ifndef __FUFS_UTIL_H__
#define __FUFS_UTIL_H__
#include <sys/types.h>

typedef struct fufs_quota_info_t fufs_quota_info;

struct fufs_quota_info_t {
	off_t quota_total;
	off_t quota_used;
};
int fufs_file_log(const char *fmt, ...);
char *fufs_util_get_parent_path(char *path);
int fufs_util_quota_info_store(off_t quota_total,off_t quota_used);
off_t fufs_util_quota_total_get();
off_t fufs_util_quota_used_get();


#endif
