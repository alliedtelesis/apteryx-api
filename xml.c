/**
 * @file xml.c
 * XML based database schema support for Apteryx.
 *
 * Copyright 2016, Allied Telesis Labs New Zealand, Ltd
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
#include <libxml/parser.h>
#include <libxml/tree.h>

/* Convert an XML node to apteryx_schema_node */
static struct apteryx_schema_node *
xml_to_node (xmlNode *xml, int depth)
{
    struct apteryx_schema_node *node;
    char *field;

    /* NULL */
    if (!xml)
    {
        ERROR ("XML: Node NULL\n");
        return NULL;
    }

    /* Create a new node */
    field = (char *) xmlGetProp (xml, (xmlChar *) "name");
    if (!field)
    {
        ERROR ("XML: Node has no name\n");
        return NULL;
    }
    node = node_create (field);
    xmlFree (field);

    DEBUG ("%*s%s", depth * 2, " ", node->name);

    /* Default */
    field = (char *) xmlGetProp (xml, (xmlChar *) "default");
    if (field)
    {
        node->defvalue = g_strdup (field);
        xmlFree (field);
    }

    /* Pattern */
    field = (char *) xmlGetProp (xml, (xmlChar *) "pattern");
    if (field)
    {
        node->pattern = g_strdup (field);
        xmlFree (field);
    }

    /* Mode */
    field = (char *) xmlGetProp (xml, (xmlChar *) "mode");
    if (field)
    {
        if (strchr (field, 'r') != NULL)
        {
            node->flags |= NODE_FLAGS_READ;
        }
        if (strchr (field, 'w') != NULL)
        {
            node->flags |= NODE_FLAGS_WRITE;
        }
        //TODO 'c' and 'p'
        DEBUG ("[%s]", field);
        xmlFree (field);
    }

    /* Handle ENUMs */
    if (g_strcmp0 (xml->name, "VALUE") == 0)
    {
        /* Value */
        field = (char *) xmlGetProp (xml, (xmlChar *) "value");
        if (field)
        {
            node->value = g_strdup (field);
            node->flags |= NODE_FLAGS_ENUM;
            DEBUG ("[=%s]", field);
            xmlFree (field);
        }
    }

    /* Description */
    field = (char *) xmlGetProp (xml, (xmlChar *) "help");
    if (field)
    {
        node->description = g_strdup (field);
        DEBUG ("\t\t\"%s\"", field);
        xmlFree (field);
    }

    DEBUG ("\n");

    /* Leaf */
    if (!xml->children || g_strcmp0 (xml->children->name, "VALUE") == 0)
    {
        node->flags |= NODE_FLAGS_LEAF;
    }

    /* Process children */
    for (xmlNode *child = xml->children; child; child = child->next)
    {
        struct apteryx_schema_node *cn = xml_to_node (child, depth + 1);
        if (cn)
            node->children = g_list_prepend (node->children, cn);
    }

    return node;
}

/* Remove unwanted nodes and attributes from a parsed tree */
static void
cleanup_nodes (xmlNode *node)
{
    xmlNode *n, *next;

    n = node;
    while (n)
    {
        next = n->next;
        if (n->type == XML_ELEMENT_NODE)
        {
            cleanup_nodes (n->children);
            xmlSetNs (n, NULL);
        }
        else
        {
            xmlUnlinkNode (n);
            xmlFreeNode (n);
        }
        n = next;
    }
}

/* Load an Apteryx schema in XML format */
struct apteryx_schema_node *
xml_schema_load (const char *filename)
{
    struct apteryx_schema_node *schema;
    xmlDoc *doc;
    xmlNode *root;

    /* Parse document */
    doc = xmlParseFile (filename);
    if (!doc)
    {
        ERROR ("XML: Invalid XML\n");
        return NULL;
    }

    /* Find root node */
    root = xmlDocGetRootElement (doc);
    if (!root || g_strcmp0 (root->name, "MODULE") != 0)
    {
        ERROR ("XML: No root MODULE element\n");
        xmlFreeDoc (doc);
        return NULL;
    }

    /* Remove TEXT etc */
    cleanup_nodes (root->children);

    /* Convert to apteryx_schema_node */
    schema = xml_to_node (root->children, 0);
    xmlFreeDoc (doc);
    return schema;
}
