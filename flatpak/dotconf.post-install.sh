#!/usr/bin/env sh
set -e

if [ -f /app/share/pkgconfig/dotconf.pc ] && [ ! -f /app/lib/pkgconfig/dotconf.pc ]; then
  mkdir -p /app/lib/pkgconfig
  cp /app/share/pkgconfig/dotconf.pc /app/lib/pkgconfig/dotconf.pc
fi
