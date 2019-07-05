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
#include "apteryx-schema.h"

#define TEST_APTERYX_PATH   "/test"
#define TEST_ITERATIONS     1000
#define TEST_SCHEMA_PATH    "."

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
        if (strncmp (TEST_APTERYX_PATH, path, strlen (TEST_APTERYX_PATH)) == 0)
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
    g_assert_true (1 == fscanf (f, "%*d %ld %*d %*d %*d %*d %*d", &memory));
    fclose (f);
    return memory * getpagesize () / 1024;
}

#ifdef HAVE_LIBXML
static void
generate_xml_schemas (gpointer fixture, gconstpointer data)
{
    FILE *schema;

    schema = fopen ("./test.xml", "w");
    if (schema)
    {
        fprintf (schema,
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
        fclose (schema);
    }
    schema = fopen ("./test1.xml", "w");
    if (schema)
    {
        fprintf (schema,
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
        fclose (schema);
    }
}

static void
destroy_xml_schemas (gpointer fixture, gconstpointer data)
{
    unlink ("./test.xml");
    unlink ("./test1.xml");
}
#endif

#ifdef HAVE_LIBYANG
static void
generate_yang_schemas (gpointer fixture, gconstpointer data)
{
    FILE *schema;

    schema = fopen ("./test.yang", "w");
    if (schema)
    {
        fprintf (schema,
        "module test {"
            "namespace \"https://github.com/alliedtelesis/apteryx\";"
            "prefix test;"
            "container test {"
                "description \"This is a test node\";"
                "leaf debug {"
                    "description \"Debug configuration\";"
                    "default \"disable\";"
                    "type enumeration {"
                        "enum \"disable\" {"
                            "value 0;"
                            "description \"Debugging is disabled\";"
                        "}"
                        "enum \"enable\" {"
                            "value 1;"
                            "description \"Debugging is enabled\";"
                        "}"
                    "}"
                "}"
                "list list {"
                    "key \"name\";"
                    "description \"This is a list of stuff\";"
                    "leaf name {"
                        "description \"This is the list key\";"
                        "type string;"
                    "}"
                    "leaf type {"
                        "description \"This is the list type\";"
                        "default \"big\";"
                        "type enumeration {"
                            "enum \"big\" {"
                                "value 1;"
                            "}"
                            "enum \"little\" {"
                                "value 2;"
                            "}"
                        "}"
                    "}"
                    "list sub-list {"
                        "key \"i-d\";"
                        "description \"This is a list of stuff attached to a list\";"
                        "leaf i-d {"
                            "description \"This is the sublist key\";"
                            "type string;"
                        "}"
                    "}"
                "}"
                "leaf-list trivial-list {"
                    "description \"This is a simple list of stuff\";"
                    "type string;"
                "}"
            "}"
        "}");
        fclose (schema);
    }
    schema = fopen ("./test1.yang", "w");
    if (schema)
    {
        fprintf (schema,
        "module test1 {"
            "namespace \"https://github.com/alliedtelesis/apteryx\";"
            "prefix test1;"
            "description \"This is a test node that will be merged\";"
            "container test {"
                "leaf state {"
                    "description \"Debug configuration\";"
                    "default \"up\";"
                    "config false;"
                    "type enumeration {"
                        "enum \"up\" {"
                            "value 0;"
                            "description \"State is up\";"
                        "}"
                        "enum \"down\" {"
                            "value 1;"
                            "description \"State is down\";"
                        "}"
                    "}"
                "}"
                "leaf kick {"
                    "description \"Write only field\";"
                    "type string;"
                "}"
                "leaf secret {"
                    "description \"Hidden field\";"
                    "type string;"
                "}"
            "}"
        "}");
        fclose (schema);
    }
}

static void
destroy_yang_schemas (gpointer fixture, gconstpointer data)
{
    unlink ("./test.yang");
    unlink ("./test1.yang");
}
#endif

static void
test_api_parse (gpointer fixture, gconstpointer data)
{
    apteryx_schema_instance *schema = apteryx_schema_load (TEST_SCHEMA_PATH);
    g_assert_nonnull (schema);
    if (apteryx_schema_debug)
    {
        apteryx_schema_dump (stdout, schema);
    }
    apteryx_schema_free (schema);
    g_assert_true (assert_apteryx_empty ());
}

#ifdef HAVE_LUA
int luaopen_libapteryx_schema (lua_State *L);
static bool
_run_lua (char *script)
{
    char *buffer = strdup (script);
    lua_State *L;
    char *line;
    int res = -1;

    L = luaL_newstate ();
    luaL_openlibs (L);
    luaopen_libapteryx_schema (L);
    lua_setglobal (L, "apteryx");
    line = strtok (buffer, "\n");
    while (line != NULL)
    {
        res = luaL_loadstring (L, line);
        if (res == 0)
            res = lua_pcall (L, 0, 0, 0);
        if (res != 0)
            fprintf (stderr, "%s\n", lua_tostring(L, -1));
        g_assert_true (res == 0);
        line = strtok (NULL,"\n");
    }
    lua_close (L);
    free (buffer);
    return res == 0;
}

void
test_lua_api_parse (gpointer fixture, gconstpointer data)
{
    g_assert_true (_run_lua ("api = apteryx.api('"TEST_SCHEMA_PATH"')"));
    g_assert_true (assert_apteryx_empty ());
}

void
test_lua_api_set_get (gpointer fixture, gconstpointer data)
{
    g_assert_true (_run_lua (
        "api = apteryx.api('"TEST_SCHEMA_PATH"')                          \n"
        "api.test.debug = 'enable'                                        \n"
        "assert(api.test.debug == 'enable')                               \n"
        "api.test.debug = nil                                             \n"
        "assert(api.test.debug == 'disable')                              \n"
    ));
    g_assert_true (assert_apteryx_empty ());
}

void
test_lua_api_list (gpointer fixture, gconstpointer data)
{
    g_assert_true (_run_lua (
        "api = apteryx.api('"TEST_SCHEMA_PATH"')                          \n"
        "api.test.list('cat-nip').sub_list('dog').i_d = '1'               \n"
        "assert(api.test.list('cat-nip').sub_list('dog').i_d == '1')      \n"
        "api.test.list('cat-nip').sub_list('dog').i_d = nil               \n"
        "assert(api.test.list('cat-nip').sub_list('dog').i_d == nil)      \n"
    ));
    g_assert_true (assert_apteryx_empty ());
}

void
test_lua_api_trivial_list (gpointer fixture, gconstpointer data)
{
    g_assert_true (_run_lua (
        "api = apteryx.api('"TEST_SCHEMA_PATH"')                          \n"
        "api.test.trivial_list('cat-nip', 'cat-nip')                      \n"
        "assert(api.test.trivial_list('cat-nip') == 'cat-nip')            \n"
        "api.test.trivial_list('cat-nip', nil)                            \n"
        "assert(api.test.trivial_list('cat-nip') == nil)                  \n"
    ));
    g_assert_true (assert_apteryx_empty ());
}

void
test_lua_api_search (gpointer fixture, gconstpointer data)
{
    g_assert_true (_run_lua (
        "api = apteryx.api('"TEST_SCHEMA_PATH"')                          \n"
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
    g_assert_true (assert_apteryx_empty ());
}

void
test_lua_load_api_memory (gpointer fixture, gconstpointer data)
{
    lua_State *L;
    unsigned long before;
    unsigned long after;
    int res = -1;

    before = _memory_usage ();
    L = luaL_newstate ();
    luaL_openlibs (L);
    luaopen_libapteryx_schema (L);
    lua_setglobal (L, "apteryx");
    res = luaL_loadstring (L, "apteryx.api('"TEST_SCHEMA_PATH"')");
    if (res == 0)
        res = lua_pcall (L, 0, 0, 0);
    if (res != 0)
        fprintf (stderr, "%s\n", lua_tostring(L, -1));
    after = _memory_usage ();
    lua_close (L);
    printf ("%ldkb ... ", (after - before));
    g_assert_true (res == 0);
}

void
test_lua_load_api_performance (gpointer fixture, gconstpointer data)
{
    uint64_t start;
    int i;

    start = get_time_us ();
    for (i = 0; i < 10; i++)
    {
        g_assert_true (_run_lua ("apteryx.api('"TEST_SCHEMA_PATH"')"));
    }
    printf ("%"PRIu64"us ... ", (get_time_us () - start) / 10);
    g_assert_true (assert_apteryx_empty ());
}

void
test_lua_api_perf_get (gpointer fixture, gconstpointer data)
{
    lua_State *L;
    uint64_t start;
    int i;

    for (i = 0; i < TEST_ITERATIONS; i++)
    {
        char *path = NULL;
        g_assert_true (asprintf(&path, TEST_APTERYX_PATH"/list/%d/name", i) > 0);
        apteryx_set (path, "private");
        free (path);
    }
    L = luaL_newstate ();
    luaL_openlibs (L);
    luaopen_libapteryx_schema (L);
    lua_setglobal (L, "apteryx");
    g_assert_true (luaL_loadstring (L, "api = apteryx.api('"TEST_SCHEMA_PATH"')") == 0);
    g_assert_true (lua_pcall (L, 0, 0, 0) == 0);
    start = get_time_us ();
    for (i = 0; i < TEST_ITERATIONS; i++)
    {
        char *cmd = NULL;
        int res;
        g_assert_true (asprintf(&cmd, "assert(api.test.list('%d').name == 'private')", i) > 0);
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
        g_assert_true (asprintf(&path, TEST_APTERYX_PATH"/list/%d/name", i) > 0);
        g_assert_true (apteryx_set (path, NULL));
        free (path);
    }
    g_assert_true (assert_apteryx_empty ());
}

void
test_lua_api_perf_set (gpointer fixture, gconstpointer data)
{
    lua_State *L;
    uint64_t start;
    int i;

    L = luaL_newstate ();
    luaL_openlibs (L);
    luaopen_libapteryx_schema (L);
    lua_setglobal (L, "apteryx");
    g_assert_true (luaL_loadstring (L, "api = apteryx.api('"TEST_SCHEMA_PATH"')") == 0);
    g_assert_true (lua_pcall (L, 0, 0, 0) == 0);
    start = get_time_us ();
    for (i = 0; i < TEST_ITERATIONS; i++)
    {
        char *cmd = NULL;
        int res;
        g_assert_true (asprintf(&cmd, "api.test.list('%d').name = 'private'", i) > 0);
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
        g_assert_true (asprintf(&path, TEST_APTERYX_PATH"/list/%d/name", i) > 0);
        g_assert_true (apteryx_set (path, NULL));
        free (path);
    }
    g_assert_true (assert_apteryx_empty ());
}
#endif /* HAVE_LUA */

static void
add_tests (GTestSuite *suite, GTestFixtureFunc setup, GTestFixtureFunc teardown)
{
    GTestSuite *api = g_test_create_suite ("api");
    g_test_suite_add_suite (suite, api);
    g_test_suite_add (api, g_test_create_case ("parse", 0, NULL, setup, test_api_parse, teardown));
#ifdef HAVE_LUA
    GTestSuite *lua = g_test_create_suite ("lua");
    g_test_suite_add_suite (suite, lua);
    g_test_suite_add (lua, g_test_create_case ("parse", 0, NULL, setup, test_lua_api_parse, teardown));
    g_test_suite_add (lua, g_test_create_case ("setget", 0, NULL, setup, test_lua_api_set_get, teardown));
    g_test_suite_add (lua, g_test_create_case ("list", 0, NULL, setup, test_lua_api_list, teardown));
    g_test_suite_add (lua, g_test_create_case ("trivial_list", 0, NULL, setup, test_lua_api_trivial_list, teardown));
    g_test_suite_add (lua, g_test_create_case ("search", 0, NULL, setup, test_lua_api_search, teardown));
    g_test_suite_add (lua, g_test_create_case ("memory", 0, NULL, setup, test_lua_load_api_memory, teardown));
    GTestSuite *lua_perf = g_test_create_suite ("perf");
    g_test_suite_add_suite (lua, lua_perf);
    g_test_suite_add (lua_perf, g_test_create_case ("load", 0, NULL, setup, test_lua_load_api_performance, teardown));
    g_test_suite_add (lua_perf, g_test_create_case ("get", 0, NULL, setup, test_lua_api_perf_get, teardown));
    g_test_suite_add (lua_perf, g_test_create_case ("set", 0, NULL, setup, test_lua_api_perf_set, teardown));
#endif
}

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);
    apteryx_schema_debug = g_test_verbose ();
    g_test_set_nonfatal_assertions ();
    GTestSuite *root = g_test_create_suite ("schema");
#ifdef HAVE_LIBXML
    GTestSuite *xml = g_test_create_suite ("xml");
    g_test_suite_add_suite (root, xml);
    add_tests (xml, generate_xml_schemas, destroy_xml_schemas);
#endif
#ifdef HAVE_LIBYANG
    GTestSuite *yang = g_test_create_suite ("yang");
    g_test_suite_add_suite (root, yang);
    add_tests (yang, generate_yang_schemas, destroy_yang_schemas);
#endif
    return g_test_run_suite (root);
}
