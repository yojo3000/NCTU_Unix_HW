# Simple C Shell Makefile

CC = gcc
CFLAGS  = -Wall -g
OBJ = ysh.o

all: ysh

ysh: $(OBJ)
	$(CC) $(CFLAGS) -o ysh $(OBJ) 

%.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	rm *.o ysh
