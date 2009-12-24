#!/usr/bin/env sh
#
# dyld is hard.  lets go shopping.
#

echo Rewriting libruby.dylib link paths for .bundles in: $1
for b in $1/*.bundle ; do
  AFTER='@loader_path/../libruby_bp.dylib'
  BEFORE=`otool -LX $b | grep libruby | sed -E -e 's/^[^/]*\//\//g' -e 's/dylib .*$/dylib/'`
  install_name_tool -change "${BEFORE}" "${AFTER}" $b
done

for b in $1/*/*.bundle ; do
  AFTER='@loader_path/../../libruby_bp.dylib'
  BEFORE=`otool -LX $b | grep libruby | sed -E -e 's/^[^/]*\//\//g' -e 's/dylib .*$/dylib/'`
  install_name_tool -change "${BEFORE}" "${AFTER}" $b
done

for b in $1/*/*/*.bundle ; do
  AFTER='@loader_path/../../libruby_bp.dylib'
  BEFORE=`otool -LX $b | grep libruby | sed -E -e 's/^[^/]*\//\//g' -e 's/dylib .*$/dylib/'`
  install_name_tool -change "${BEFORE}" "${AFTER}" $b
done

