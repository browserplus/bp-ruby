#!/usr/bin/env sh

echo working on $1
AFTER='@loader_path/libruby_bp.dylib'
BEFORE=`otool -LX $1 | grep libruby | sed -E -e 's/^[^/]*\//\//g' -e 's/dylib .*$/dylib/'`
install_name_tool -id libruby_bp.dylib -change "${BEFORE}" "${AFTER}" $1
