/**
 * @file apteryx-schema.h
 *
 * Database schema support for Apteryx.
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
#ifndef _APTERYX_SCHEMA_H_
#define _APTERYX_SCHEMA_H_

typedef struct apteryx_schema_instance apteryx_schema_instance;
typedef struct apteryx_schema_node apteryx_schema_node;
apteryx_schema_instance* apteryx_schema_load (const char *folders);
void apteryx_schema_free (apteryx_schema_instance *schema);
void apteryx_schema_dump (FILE *fp, apteryx_schema_instance *schema);
apteryx_schema_node* apteryx_schema_lookup (apteryx_schema_instance *schema, const char *path);
bool apteryx_schema_is_leaf (apteryx_schema_node *node);
bool apteryx_schema_is_readable (apteryx_schema_node *node);
bool apteryx_schema_is_writable (apteryx_schema_node *node);
char* apteryx_schema_name (apteryx_schema_node *node);
char* apteryx_schema_translate_to (apteryx_schema_node *node, char *value);
char* apteryx_schema_translate_from (apteryx_schema_node *node, char *value);

#endif /* _APTERYX_SCHEMA_H_ */
