#!/bin/bash

#库函数的.c文件编译成.o文件
gcc -c string.c 

#.o文件打包成libstr.a静态库
ar -rcs libstr.a string.o

# 使用静态库编译main
# 指定库路径： -L <PATH>，环境变量已包含的路径可省略
# 指定库名：写库全名，或用-lxxx两种方式都可以, -lxxx表示库名是libxxx.a或libxxx.so
#gcc -o test main.c libstr.a
gcc -o test main.c -L ./ -lstr

./test
