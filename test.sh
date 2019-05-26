#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$expected exected, but got $actual"
        exit 1
    fi
}

try 47 "5+6*7"
try 77 "(5+6)*7"
try 86 "5*6+7*8"

echo OK
