CC=gcc
CFLAGS=-Iadd -Isub -O2

# 目标文件必须指定每个文件名，不能直接用通配符写成*.o，

# 写法一：配合VPATH，直接指定文件名，文件较多时很低效
# VPATH是Makefile的搜索路径环境参数,多路径用:隔开
# VPATH=add:sub:.
# OBJS=add_int.o sub_int.o main.o

# 写法二：利用源码文件和目标文件同名的特性，用源码展开和模式替换，变相使用通配符
# OBJS=$(patsubst %.c, %.o, $(wildcard *.c */*.c))

# 写法三：foreach函数，优化写法二
# foreach VAR, LIST, TEXT, 表示对以空格分隔的每一个VAR in LIST, 做TEXT内的操作
DIRS=add sub .
FILES=$(foreach dir, ${DIR}, $(wildcard ${dir}/*.c))
OBJS=$(patsubst %.c, %.o, ${FILES})

#目标程序
TARGET=cacu


$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)
# 等同于如下写法，$@表示目标文件，$^表示所有依赖文件，好处是TARGET和OBJS修改时无需修改命令的内容
# $(CC) -o $(TARGET) $(OBJS) $(CFLAGS) 

clean:
	rm -rf $(TARGET) $(OBJS)

