.PHONY:clean check_obj_dir

CC := gcc
OBJ_DIR := obj
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS) )
INC := -I.
CFLAGS := -g3 -std=gnu11 -O3
LIBS := -lpthread

test.app:check_obj_dir $(OBJS)
	$(CC) $(INC) $(OBJS) -o $@ $(CFLAGS) $(LIBS)


check_obj_dir:
	@if test ! -d $(OBJ_DIR);\
		then\
		mkdir $(OBJ_DIR);\
	fi


$(OBJ_DIR)/%.o:%.c
	$(CC) $(INC) -c $< -o $@ $(CFLAGS)


test:test.app
	$(EXEC) ./test.app


clean:
	rm -r $(OBJ_DIR)/*.o *.app
