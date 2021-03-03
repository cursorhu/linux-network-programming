#!/bin/bash

# -shared生成动态链接库，其中包含-fpic使库变量位置无关(都是相对地址)
# -Wl,表示后面的参数将传给link程序ld，-soname指定库的别名, 该选项作用是支持ldconfig自动创建别名软链接
# -o 是真实库名
gcc -shared -Wl,-soname,libstr.so -o libstr.so.1 string.c

# 这里手动创建库名和真名的软链接，也可以用ldconfig自动创建
# 创建软链接格式： ln -s 原文件名 链接名
ln -s libstr.so.1 libstr.so

# 使用动态库编译main
# 指定库路径： -L <PATH>，环境变量已包含的路径可省略
# 指定库名：写库全名，或用-lxxx两种方式都可以, -lxxx表示库名是libxxx.a或libxxx.so
gcc -o test main.c -L ./ -lstr

# 运行时程序会加载动态库，需要先把动态库加到LD_LIBRARY_PATH环境变量，否则程序找不到动态库
# ${PWD}可以展开成当前路径的绝对地址
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${PWD}

# 运行程序，加载.so库
./test
