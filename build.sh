export LD_LIBRARY_PATH=/data/toolchain/lib:$LD_LIBRARY_PATH
gcc ./src/dcf_test_main.c  -L ./lib -ldcf  -I ./include -g -o ./build/DcfFrameWorkDemo
cd build&&./DcfFrameWorkDemo
