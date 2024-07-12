TAG=./main # 指定最终生成的可执行文件的路径+名字
SRC=$(wildcard ./src/*.c) # 指定所有的.c源文件的路径+名字 （wildcard 函数用来匹配指定路径./src/ 下面的所有的.c  ） 
OBJ=$(SRC:%.c=%.o) # 指定生成的.o 文件的路径+名字 （把SRC 中的xx.c 匹配生成 xxx.o）
CC=gcc # 指定编译器  arm-linux-gcc
override CONFIG+=   -I./inc  -pthread #配置细信息（指定头文件、库文件等特殊选项）
# override 用来防止CONFIG 被覆盖 += 则可以让 CONFIG 被追加

# 该规则为最终目标
# 该规则中的依赖文件为OBJ 变量 实际上是一堆 .o 文件的单词列表
# Make 工具会根据所需的.o文件自动创建一个生成.o文件的简单规则
#（不需要指定头文件路径以及库文件连接的配置信息）
$(TAG):$(OBJ) 
	$(CC) $(^) -o $(@) $(CONFIG)


#	静态规则
#	用来匹配当前Makefile 中所有关的.o 文件的生成规则
#	优于隐士规则的地方在于该规则可以给编译命令添加必要的选项 $(CONFIG)
%.o:%.c  # 如何把.c生成．ｏ　的静态规则
	$(CC) $(^) -o $(@) -c $(CONFIG)

# 清除编译过程中产生过程文件 .o 以及 可执行文件
clean:
	$(RM) ./src/*.o  ./main

send:
	scp $(TAG) root@192.168.1.210:/demo

# 不要对 clean 运用任何隐式规则
# 不能运用隐式规则的目标被称为伪目标
.PHONY:clean
.PHONY:send
