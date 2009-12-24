#!/usr/bin/env sh

echo working on $1
AFTER='@loader_path/libruby.dylib'
BEFORE=`otool -LX $1 | grep libruby | sed -E -e 's/^[^/]*\//\//g' -e 's/dylib .*$/dylib/'`
install_name_tool -change "${BEFORE}" "${AFTER}" $1
