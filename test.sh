#!/bin/bash
# TEST_WRAPPER="G_SLICE=always-malloc valgrind --leak-check=full" make test
# TEST_WRAPPER="gdb" make test
if test -e /tmp/apteryxd.pid; then
    kill -TERM `cat /tmp/apteryxd.pid` && sleep 0.1;
fi;
rm -f /tmp/apteryxd.pid;
rm -f /tmp/apteryxd.run;
ln -sf libapteryx_schema.so .libs/apteryx_schema.so;
apteryxd -b -p /tmp/apteryxd.pid -r /tmp/apteryxd.run && sleep 0.1;
eval LUA_CPATH=.libs/?.so $TEST_WRAPPER ./unittest;
kill -TERM `cat /tmp/apteryxd.pid`;
