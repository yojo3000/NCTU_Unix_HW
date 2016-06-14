make clean
make
kill `pidof Webserver`
./Webserver 5500 htdocs
sleep 0.5
ps aux | grep Webserver --color
