# fopen Makefile
# write by Mars Huang

input_c_file 	= cheat.c
outputLib 		= cheat.so

CC 				= gcc
CFLAGS 			= -Wall -shared -fPIC -ldl

# target: dependencies
# 	action

all:	# default make target
	$(CC) -o $(outputLib) $(CFLAGS) $(input_c_file)

del:
	rm ind* info_output

clean:
	rm -rf *.o $(outputLib)

test:
	LD_PRELOAD=./cheat.so wget http://adl.tw
