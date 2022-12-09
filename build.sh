export LD_LIBRARY_PATH=/data/toolchain/lib:$LD_LIBRARY_PATH
gcc ./src/dcf_test.c ./src/dcf_demo.c -L ./lib -ldcf  -I ./include -g -o ./build/DCFTestClient
cd build&&./DCFTestClient
