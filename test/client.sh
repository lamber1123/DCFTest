gcc ./client.c ../src/message.c -I ../include -g -o ./build/client -pthread
cd build&&./client 172.19.0.202
