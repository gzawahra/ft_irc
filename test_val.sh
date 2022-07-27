#!/bin/bash
make re
#if [ $USER = "gz" ] ; then
terminator -e "valgrind --leak-check=yes --track-origins=yes ./ircserv 6667 hello"
#else
#xterm -e "valgrind --leak-check=yes --track-origins=yes ./ircserv 6667 hello"
#fi
sleep 3s
nc 127.0.0.1 6667 < ./files/test_cmds
echo "Press any key to continue"
while [ true ] ; do
read -t 3 -n 1
if [ $? = 0 ] ; then
exit ;
else
echo "waiting for the keypress"
fi
done