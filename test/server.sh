gcc ./server.c ../src/message.c -I ../include -g -o ./build/server -pthread
cd build&&./server
