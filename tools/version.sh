#!/bin/sh

if type git >/dev/null 2>&1 && git rev-parse --git-dir >/dev/null 2>&1; then
version=$(git describe --tags --match 'v[0-9]*' 2>/dev/null || true)
if test -n "$version"; then
printf '%s\n' "$version" | sed -e 's/^v//' -e 's/-/-git-/'
else
git rev-parse --short HEAD
fi
elif test -f VERSION; then
cat VERSION
else
echo unknown
fi
