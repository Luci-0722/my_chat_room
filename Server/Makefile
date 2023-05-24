#!Makefile

PROGRAM = chat_room_srv

C_SOURCES = $(shell find . -name "*.cpp")

CC = g++

C_FLAGS = -DDEBUG -c -Wall -Wextra -ggdb3 -I. --std=c++11
chat_room_srv: $(C_SOURCES) 
	@echo link...
	$(CC) $(C_SOURCES) -g -lm -lpthread -L/usr/lib64/mysql -lmysqlclient -o $(PROGRAM)


.PHONY:clean
clean:
	$(RM) $(C_OBJECTS) chat_room_srv

