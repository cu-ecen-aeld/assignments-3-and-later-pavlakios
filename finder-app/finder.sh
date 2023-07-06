#!/bin/bash

if [ $# -lt 2 ]; then
  exit 1
fi

filesdir="$1"
searchstr="$2"

if [ ! -d "$filesdir" ]; then
  exit 1
fi

file_count=0
line_count=0

while IFS= read -r -d '' file; do
  file_count=$((file_count + 1))
  count=$(grep -c "$searchstr" "$file")
  line_count=$((line_count + count))
done < <(find "$filesdir" -type f -print0)

echo "The number of files are $file_count and the number of matching lines are $line_count"
