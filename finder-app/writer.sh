#!/bin/bash

if [ $# -lt 2 ]; then
  exit 1
fi

writefile="$1"
writestr="$2"

dir_path=$(dirname "$writefile")
mkdir -p "$dir_path"

echo "$writestr" > "$writefile"

if [ $? -ne 0 ]; then
  exit 1
fi

echo "File '$writefile' created successfully with the content:"
echo "$writestr"
