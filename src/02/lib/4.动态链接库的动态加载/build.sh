#!/bin/bash

gcc -shared -o libstr.so.1 string.c

ln -s libstr.so.1 libstr.so

# -ldl动态加载库是dlopen,dlsym,dlclose等函数所在的库
gcc -o test main.c -L ./ -lstr -ldl

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${PWD}

./test
