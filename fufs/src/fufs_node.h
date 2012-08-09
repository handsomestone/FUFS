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

#ifndef __FUFS_NODE_H__
#define __FUFS_NODE_H__

#include <glib.h>
#include <sys/types.h>
#include "fufs_conf.h"
typedef enum{
	FUFS_NODE_TYPE_FILE,
	FUFS_NODE_TYPE_FOLDER
}fufs_node_type;

typedef struct fufs_node_t fufs_node;

struct fufs_node_t{
	char *fullpath;
	char *id;
	char *dir_id;
	char *name;
	fufs_node_type type;
	char *sha1; /*used to upload file with sha1*/
	struct stat st;
	pthread_mutex_t mutex;
	GHashTable *sub_nodes;
};

#define FUFS_NODE_LOCK(node) pthread_mutex_lock(&(node->mutex));
#define FUFS_NODE_UNLOCK(node) pthread_mutex_unlock(&(node->mutex));


fufs_node *fufs_node_root_get();
char *fufs_node_root_get_id();
fufs_node *fufs_node_root_create(char *id,char *name,off_t size);

off_t strToLong(char *str);
off_t fufs_node_str2size(char *str);
int fufs_node_get_root_path();

fufs_node *fufs_node_get_by_path(fufs_node *node, const char *path);
fufs_ret fufs_node_parse_dir(fufs_node * parent_node, const char *path,char *parent_node_id);

void fufs_node_free(gpointer p);
void fufs_node_free_sub_nodes(GHashTable * sub_nodes);
fufs_ret fufs_node_rebuild(fufs_node * node);
#endif
