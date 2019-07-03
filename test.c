/**
 * @file test.c
 * Unit tests for the Apteryx XML Schema
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
#include <inttypes.h>
#include <unistd.h>
#include <sys/time.h>
#ifdef HAVE_LUA
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#endif
#include <apteryx.h>

#define TEST_PATH           "/test"
#define TEST_ITERATIONS     1000
#define TEST_SCHEMA_PATH    "."
#define TEST_SCHEMA_FILE    "test1.xml"
#define TEST_SCHEMA_FILE2   "test2.xml"

static void
generate_test_schemas (void)
{
    FILE *xml;

    xml = fopen (TEST_SCHEMA_FILE, "w");
    if (xml)
    {
        fprintf (xml,
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<MODULE xmlns=\"https://github.com/alliedtelesis/apteryx\"\n"
"    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
"    xsi:schemaLocation=\"https://github.com/alliedtelesis/apteryx\n"
"    https://github.com/alliedtelesis/apteryx/releases/download/v2.10/apteryx.xsd\">\n"
"    <NODE name=\"test\" help=\"this is a test node\">\n"
"        <NODE name=\"debug\" mode=\"rw\" default=\"0\" help=\"Debug configuration\" pattern=\"^(0|1)$\">\n"
"            <VALUE name=\"disable\" value=\"0\" help=\"Debugging is disabled\" />\n"
"            <VALUE name=\"enable\" value=\"1\" help=\"Debugging is enabled\" />\n"
"        </NODE>\n"
"        <NODE name=\"list\" help=\"this is a list of stuff\">\n"
"            <NODE name=\"*\" help=\"the list item\">\n"
"                <NODE name=\"name\" mode=\"rw\" help=\"this is the list key\"/>\n"
"                <NODE name=\"type\" mode=\"rw\" default=\"1\" help=\"this is the list type\">\n"
"                    <VALUE name=\"big\" value=\"1\"/>\n"
"                    <VALUE name=\"little\" value=\"2\"/>\n"
"                </NODE>\n"
"                <NODE name=\"sub-list\" help=\"this is a list of stuff attached to a list\">\n"
"                    <NODE name=\"*\" help=\"the sublist item\">\n"
"                        <NODE name=\"i-d\" mode=\"rw\" help=\"this is the sublist key\"/>\n"
"                    </NODE>\n"
"                </NODE>\n"
"            </NODE>\n"
"        </NODE>\n"
"        <NODE name=\"trivial-list\" help=\"this is a simple list of stuff\">\n"
"            <NODE name=\"*\" help=\"the list item\" />\n"
"        </NODE>\n"
"    </NODE>\n"
"</MODULE>\n");
        fclose (xml);
    }
    xml = fopen (TEST_SCHEMA_FILE2, "w");
    if (xml)
    {
        fprintf (xml,
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<MODULE xmlns=\"https://github.com/alliedtelesis/apteryx\"\n"
"	xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
"	xsi:schemaLocation=\"https://github.com/alliedtelesis/apteryx\n"
"	https://github.com/alliedtelesis/apteryx/releases/download/v2.10/apteryx.xsd\">\n"
"	<NODE name=\"test\" help=\"this is a test node that will be merged\">\n"
"		<NODE name=\"state\" mode=\"r\" default=\"0\" help=\"Read only field\" >\n"
"			<VALUE name=\"up\" value=\"0\" help=\"State is up\" />\n"
"			<VALUE name=\"down\" value=\"1\" help=\"State is down\" />\n"
"		</NODE>\n"
"		<NODE name=\"kick\" mode=\"w\" help=\"Write only field\" pattern=\"^(0|1)$\" />\n"
"		<NODE name=\"secret\" mode=\"h\" help=\"Hidden field\" />\n"
"	</NODE>\n"
"</MODULE>\n");
        fclose (xml);
    }
}

static void
destroy_test_schemas (void)
{
    unlink (TEST_SCHEMA_FILE);
    unlink (TEST_SCHEMA_FILE2);
}

static inline uint64_t
get_time_us (void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec);
}

static bool
assert_apteryx_empty (void)
{
    GList *paths = apteryx_search ("/");
    GList *iter;
    bool ret = true;
    for (iter = paths; iter; iter = g_list_next (iter))
    {
        char *path = (char *) (iter->data);
        if (strncmp (TEST_PATH, path, strlen (TEST_PATH)) == 0)
        {
            if (ret) fprintf (stderr, "\n");
            fprintf (stderr, "ERROR: Node still set: %s\n", path);
            ret = false;
        }
    }
    g_list_free_full (paths, free);
    return ret;
}

static inline unsigned long
_memory_usage (void)
{
    unsigned long memory;
    FILE *f = fopen ("/proc/self/statm","r");
    g_assert (1 == fscanf (f, "%*d %ld %*d %*d %*d %*d %*d", &memory));
    fclose (f);
    return memory * getpagesize () / 1024;
}

#ifdef HAVE_LUA
static bool
_run_lua (char *script)
{
    char *buffer = strdup (script);
    lua_State *L;
    char *line;
    int res = -1;

    L = luaL_newstate ();
    luaL_openlibs (L);
    line = strtok (buffer, "\n");
    while (line != NULL)
    {
        res = luaL_loadstring (L, line);
        if (res == 0)
            res = lua_pcall (L, 0, 0, 0);
        if (res != 0)
            fprintf (stderr, "%s\n", lua_tostring(L, -1));
        g_assert (res == 0);
        line = strtok (NULL,"\n");
    }
    lua_close (L);
    free (buffer);
    return res == 0;
}

void
test_lua_api_load (void)
{
    g_assert (_run_lua ("xml = require('apteryx_schema')"));
    g_assert (assert_apteryx_empty ());
}

void
test_lua_api_set_get (void)
{
    g_assert (_run_lua (
        "api = require('apteryx_schema').api('"TEST_SCHEMA_PATH"')        \n"
        "api.test.debug = 'enable'                                        \n"
        "assert(api.test.debug == 'enable')                               \n"
        "api.test.debug = nil                                             \n"
        "assert(api.test.debug == 'disable')                              \n"
    ));
    g_assert (assert_apteryx_empty ());
}

void
test_lua_api_list (void)
{
    g_assert (_run_lua (
        "api = require('apteryx_schema').api('"TEST_SCHEMA_PATH"')        \n"
        "api.test.list('cat-nip').sub_list('dog').i_d = '1'               \n"
        "assert(api.test.list('cat-nip').sub_list('dog').i_d == '1')      \n"
        "api.test.list('cat-nip').sub_list('dog').i_d = nil               \n"
        "assert(api.test.list('cat-nip').sub_list('dog').i_d == nil)      \n"
    ));
    g_assert (assert_apteryx_empty ());
}

void
test_lua_api_trivial_list (void)
{
    g_assert (_run_lua (
        "api = require('apteryx_schema').api('"TEST_SCHEMA_PATH"')        \n"
        "api.test.trivial_list('cat-nip', 'cat-nip')                      \n"
        "assert(api.test.trivial_list('cat-nip') == 'cat-nip')            \n"
        "api.test.trivial_list('cat-nip', nil)                            \n"
        "assert(api.test.trivial_list('cat-nip') == nil)                  \n"
    ));
    g_assert (assert_apteryx_empty ());
}

void
test_lua_api_search (void)
{
    g_assert (_run_lua (
        "api = require('apteryx_schema').api('"TEST_SCHEMA_PATH"')        \n"
        "api.test.list('cat-nip').sub_list('dog').i_d = '1'               \n"
        "api.test.list('cat-nip').sub_list('cat').i_d = '2'               \n"
        "api.test.list('cat-nip').sub_list('mouse').i_d = '3'             \n"
        "api.test.list('cat_nip').sub_list('bat').i_d = '4'               \n"
        "api.test.list('cat_nip').sub_list('frog').i_d = '5'              \n"
        "api.test.list('cat_nip').sub_list('horse').i_d = '6'             \n"
        "cats1 = api.test.list('cat-nip').sub_list()                      \n"
        "assert(#cats1 == 3)                                              \n"
        "cats2 = api.test.list('cat_nip').sub_list()                      \n"
        "assert(#cats2 == 3)                                              \n"
        "api.test.list('cat-nip').sub_list('dog').i_d = nil               \n"
        "api.test.list('cat-nip').sub_list('cat').i_d = nil               \n"
        "api.test.list('cat-nip').sub_list('mouse').i_d = nil             \n"
        "api.test.list('cat_nip').sub_list('bat').i_d = nil               \n"
        "api.test.list('cat_nip').sub_list('frog').i_d = nil              \n"
        "api.test.list('cat_nip').sub_list('horse').i_d = nil             \n"
    ));
    g_assert (assert_apteryx_empty ());
}

void
test_lua_load_api_memory (void)
{
    lua_State *L;
    unsigned long before;
    unsigned long after;
    int res = -1;

    before = _memory_usage ();
    L = luaL_newstate ();
    luaL_openlibs (L);
    res = luaL_loadstring (L, "apteryx = require('apteryx_schema').api('"TEST_SCHEMA_PATH"')");
    if (res == 0)
        res = lua_pcall (L, 0, 0, 0);
    if (res != 0)
        fprintf (stderr, "%s\n", lua_tostring(L, -1));
    after = _memory_usage ();
    lua_close (L);
    printf ("%ldkb ... ", (after - before));
    g_assert (res == 0);
}

void
test_lua_load_api_performance (void)
{
    uint64_t start;
    int i;

    start = get_time_us ();
    for (i = 0; i < 10; i++)
    {
        g_assert (_run_lua ("apteryx = require('apteryx_schema').api('"TEST_SCHEMA_PATH"')"));
    }
    printf ("%"PRIu64"us ... ", (get_time_us () - start) / 10);
    g_assert (assert_apteryx_empty ());
}

void
test_lua_api_perf_get ()
{
    lua_State *L;
    uint64_t start;
    int i;

    for (i = 0; i < TEST_ITERATIONS; i++)
    {
        char *path = NULL;
        g_assert (asprintf(&path, TEST_PATH"/list/%d/name", i) > 0);
        apteryx_set (path, "private");
        free (path);
    }
    L = luaL_newstate ();
    luaL_openlibs (L);
    g_assert (luaL_loadstring (L, "apteryx = require('apteryx_schema').api('"TEST_SCHEMA_PATH"')") == 0);
    g_assert (lua_pcall (L, 0, 0, 0) == 0);
    start = get_time_us ();
    for (i = 0; i < TEST_ITERATIONS; i++)
    {
        char *cmd = NULL;
        int res;
        g_assert (asprintf(&cmd, "assert(apteryx.test.list('%d').name == 'private')", i) > 0);
        res = luaL_loadstring (L, cmd);
        if (res == 0)
            res = lua_pcall (L, 0, 0, 0);
        if (res != 0)
            fprintf (stderr, "%s\n", lua_tostring(L, -1));
        if (res != 0)
            goto exit;
        free (cmd);
    }
    printf ("%"PRIu64"us ... ", (get_time_us () - start) / TEST_ITERATIONS);
exit:
    lua_close (L);
    for (i = 0; i < TEST_ITERATIONS; i++)
    {
        char *path = NULL;
        g_assert (asprintf(&path, TEST_PATH"/list/%d/name", i) > 0);
        g_assert (apteryx_set (path, NULL));
        free (path);
    }
    g_assert (assert_apteryx_empty ());
}

void
test_lua_api_perf_set ()
{
    lua_State *L;
    uint64_t start;
    int i;

    L = luaL_newstate ();
    luaL_openlibs (L);
    g_assert (luaL_loadstring (L, "apteryx = require('apteryx_schema').api('"TEST_SCHEMA_PATH"')") == 0);
    g_assert (lua_pcall (L, 0, 0, 0) == 0);
    start = get_time_us ();
    for (i = 0; i < TEST_ITERATIONS; i++)
    {
        char *cmd = NULL;
        int res;
        g_assert (asprintf(&cmd, "apteryx.test.list('%d').name = 'private'", i) > 0);
        res = luaL_loadstring (L, cmd);
        if (res == 0)
            res = lua_pcall (L, 0, 0, 0);
        if (res != 0)
            fprintf (stderr, "%s\n", lua_tostring(L, -1));
        if (res != 0)
            goto exit;
        free (cmd);
    }
    printf ("%"PRIu64"us ... ", (get_time_us () - start) / TEST_ITERATIONS);
exit:
    lua_close (L);
    for (i = 0; i < TEST_ITERATIONS; i++)
    {
        char *path = NULL;
        g_assert (asprintf(&path, TEST_PATH"/list/%d/name", i) > 0);
        g_assert (apteryx_set (path, NULL));
        free (path);
    }
    g_assert (assert_apteryx_empty ());
}
#endif /* HAVE_LUA */

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);
    apteryx_schema_debug = g_test_verbose ();
    generate_test_schemas ();
#ifdef HAVE_LUA
    g_test_add_func ("/lua/load", test_lua_api_load);
    g_test_add_func ("/lua/setget", test_lua_api_set_get);
    g_test_add_func ("/lua/list", test_lua_api_list);
    g_test_add_func ("/lua/list/trivial", test_lua_api_trivial_list);
    g_test_add_func ("/lua/search", test_lua_api_search);
    g_test_add_func ("/lua/perf/memory", test_lua_load_api_memory);
    g_test_add_func ("/lua/perf/load", test_lua_load_api_performance);
    g_test_add_func ("/lua/perf/get", test_lua_api_perf_get);
    g_test_add_func ("/lua/perf/set", test_lua_api_perf_set);
#endif
    int rc = g_test_run();
    destroy_test_schemas ();
    return rc;
}
