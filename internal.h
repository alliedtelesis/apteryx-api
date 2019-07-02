/**
 * @file internal.h
 *
 * Copyright 2019, Allied Telesis Labs New Zealand, Ltd
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>
 */
#ifndef _INTERNAL_H_
#define _INTERNAL_H_
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <syslog.h>

/* Debug */
extern bool apteryx_schema_debug;
#define DEBUG(fmt, args...) \
    if (apteryx_schema_debug) \
    { \
        syslog (LOG_DEBUG, fmt, ## args); \
        printf (fmt, ## args); \
    }
#define ERROR(fmt, args...) \
    { \
        syslog (LOG_CRIT, fmt, ## args); \
        fprintf (stderr, fmt, ## args); \
    }

/* Instance */
struct apteryx_schema_instance
{
    /* Hash table of root nodes */
    GHashTable *modules;
};

/* Node */
#define NODE_FLAGS_LEAF       (1 << 0)
#define NODE_FLAGS_READ       (1 << 1)
#define NODE_FLAGS_WRITE      (1 << 2)
#define NODE_FLAGS_ENUM       (1 << 3)
struct apteryx_schema_node
{
    char name[256];
    int flags;
    char *description;
    char *defvalue;
    char *value;
    char *pattern;
    GList *children;
};
struct apteryx_schema_node * node_create (const char *name);
void node_destroy (struct apteryx_schema_node *node);

/* XML schema support */
#ifdef HAVE_LIBXML
struct apteryx_schema_node * xml_schema_load (const char *filename);
#endif

#endif /* _INTERNAL_H_ */
