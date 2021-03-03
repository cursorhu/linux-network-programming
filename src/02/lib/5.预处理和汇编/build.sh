#!/bin/bash

# -E将源码预处理(头文件和宏的替换)，输出.i文件的内容，要生成.i文件需要gcc -o xxx.i -E xxx.c
gcc -E string.c

# -S将源码编译成汇编，输出.s文件
gcc -S string.c

