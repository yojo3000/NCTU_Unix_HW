###NCTU Unix programming Homework 2

####It's a pretty simple LD_PRELOAD hijack of gnu c library function, it contents fopen64, fwrite and connect function. And I just print some function info and customized output string to file

ltraceResult is the process record of ltrace
```
ltrace -o ltraceResult wget http://adl.tw
```

build cheat library
```
make
```

clean object
```
make clean
```

test the cheat library
```
make test
```

each function hijack result will save as a file named info_output

#####sample output:
```
==========
I am in connect function
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)

ip address: 140.115.xx.xx
==========
```