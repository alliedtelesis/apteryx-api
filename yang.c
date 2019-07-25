/**
 * @file yang.c
 * Yang based database schema support for Apteryx.
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
#include "internal.h"
#include <libyang/libyang.h>

/* Convert an YANG node to apteryx_schema_node */
static struct apteryx_schema_node *
yang_to_node (const struct lys_node *yang, int depth)
{
    struct apteryx_schema_node *node;
    struct apteryx_schema_node *rnode;

    /* NULL */
    if (!yang)
    {
        ERROR ("YANG: Node NULL\n");
        return NULL;
    }

    /* Create a new node */
    node = node_create (yang->name);

    /* Description */
    if (yang->dsc)
    {
        node->description = g_strdup (yang->dsc);
    }

    /* Parse node specific values */
    switch (yang->nodetype)
    {
        case LYS_LEAF:
        {
            struct lys_node_leaf *leaf = (struct lys_node_leaf *) yang;
            node->flags |= NODE_FLAGS_LEAF;
            if (leaf->dflt)
                node->defvalue = g_strdup (leaf->dflt);
            switch (leaf->type.base)
            {
                case LY_TYPE_STRING:
                    break;
                case LY_TYPE_ENUM:
                {
                    for (int i = 0; i < leaf->type.info.enums.count; i++)
                    {
                        struct lys_type_enum *enm = &leaf->type.info.enums.enm[i];
                        struct apteryx_schema_node *child = node_create (enm->name);
                        child->flags |= NODE_FLAGS_ENUM;
                        if (enm->dsc)
                        {
                            child->description = g_strdup (enm->dsc);
                        }
                        child->value = g_strdup_printf ("%d", (enm->value));
                        node->children = g_list_append (node->children, child);
                    }
                    break;
                }
                default:
                    ERROR ("YANG: Node \"%s\" data type \"%d\" not supported\n",
                           yang->name, leaf->type.base);
                    node_destroy (node);
                    return NULL;
            }
            break;
        }
        case LYS_CONTAINER:
        case LYS_LEAFLIST:
        case LYS_LIST:
            break;
        default:
            ERROR ("YANG: Node \"%s\" type \"%d\" not supported\n",
                   yang->name, yang->nodetype);
            node_destroy (node);
            return NULL;
    }

    /* Pattern */
    //TODO node->pattern

    /* Flags */
    if (yang->nodetype == LYS_LEAF)
    {
        if (yang->flags & LYS_CONFIG_W)
            node->flags |= (NODE_FLAGS_READ | NODE_FLAGS_WRITE);
        if (yang->flags & LYS_CONFIG_R)
            node->flags |= NODE_FLAGS_READ;
        //TODO write-only
        //TODO hidden
        //TODO config
    }

    /* Check for lists */
    rnode = node;
    if (yang->nodetype == LYS_LIST || yang->nodetype == LYS_LEAFLIST)
    {
        node = node_create ("*");
        node->description = g_strdup ("List entry");
        if (yang->nodetype == LYS_LEAFLIST)
        {
            node->flags |= NODE_FLAGS_LEAF;
            if (yang->flags & LYS_CONFIG_W)
                node->flags |= (NODE_FLAGS_READ | NODE_FLAGS_WRITE);
            if (yang->flags & LYS_CONFIG_R)
                node->flags |= NODE_FLAGS_READ;
        }
        rnode->children = g_list_append (rnode->children, node);
        depth += 1;
    }

    /* Process children */
    for (struct lys_node *child = yang->child; child; child = child->next)
    {
        struct apteryx_schema_node *cn = yang_to_node (child, depth + 1);
        if (cn)
            node->children = g_list_append (node->children, cn);
    }

    return rnode;
}

/* Load an Apteryx schema in XML format */
struct apteryx_schema_node *
yang_schema_load (const char *filename, char **name, char **organization, char **version)
{
    struct apteryx_schema_node *root = NULL;
    struct ly_ctx *ctx;
    const struct lys_module *mod;
    const struct lys_node *node;

    /* Create a new context to load this module with */
    ctx = ly_ctx_new (NULL, 0);
    if (!ctx)
    {
        ERROR ("YANG: Failed to create context.\n");
        return NULL;
    }
    /* Parse */
    mod = lys_parse_path (ctx, (const char *) filename, LYS_IN_YANG);
    if (!mod)
    {
        ERROR ("YANG: Failed to parse file\n");
        ly_ctx_destroy (ctx, NULL);
        return NULL;
    }

    /* Model attributes */
    *name = g_strdup (mod->name);
    *organization = g_strdup (mod->org);
    if (mod->rev_size)
        *version = g_strdup (mod->rev[0].date);

    /* Get root node */
    node = lys_getnext (NULL, NULL, mod, LYS_GETNEXT_NOSTATECHECK);
    if (!node || !node->name)
    {
        ERROR ("YANG: No root node\n");
        ly_ctx_destroy (ctx, NULL);
        return NULL;
    }

    /* Convert to apteryx_schema_node */
    root = yang_to_node (node, 0);
    ly_ctx_destroy (ctx, NULL);
    return root;
}
