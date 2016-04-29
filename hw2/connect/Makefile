# fopen Makefile
# write by Mars Huang

input_c_file 	= cheat.c
outputLib 		= cheat.so

CC 				= gcc
CFLAGS 			= -Wall -g -shared -fPIC -ldl

# target: dependencies
# 	action

all:	# default make target
	$(CC) -o $(outputLib) $(CFLAGS) $(input_c_file)
clean:
	rm -rf *.o $(outputLib)
del:
	rm ind* info_output
test:
	LD_PRELOAD=./cheat.so wget http://adl.tw
