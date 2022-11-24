#!/bin/sh
read num
out=`openssl prime "$num" | grep -oE 'is prime$'`
if [ ! -z "$out" ]; then
    echo "PRIME"
else
    echo "$num"
    echo "NOT PRIME"
fi
