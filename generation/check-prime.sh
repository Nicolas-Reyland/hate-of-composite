#!/bin/sh
read num
out=`openssl prime "$num" | grep -o 'is prime'`
if [ ! -z "$out" ]; then
    echo "PRIME"
else
    echo "$num"
    echo "NOT PRIME"
fi
