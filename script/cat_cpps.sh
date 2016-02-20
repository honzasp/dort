#!/bin/sh
for file in "$@"
do
  printf '#line 1 "%s"\n' "$file"
  cat "$file"
done
