#!/bin/sh
# Compatibility wrapper for older makefiles expecting leblibc-gcc.specs.sh
exec "$(dirname "$0")/musl-gcc.specs.sh" "$@"
