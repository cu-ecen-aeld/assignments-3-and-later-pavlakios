#!/bin/sh

if [ $# -ne 2 ]; then
	echo "wrong number of input argumets. exiting.."
	exit 1
fi

if [ ! -d "$1" ]; then
	echo ""$1" is not a directory. exiting.."
	exit 1
fi

echo	"The number of files are" \
		"$(find "$1" -type f | wc -l)" \
		"and the number of matching lines are" \
		"$(grep -r "$2" "$1" | wc -l)"