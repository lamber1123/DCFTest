/*
 * message.h 包含该tcp/ip套接字编程所需要的基本头文件
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_LINE 2048
#define PORT 6001
#define BACKLOG 10
#define LISTENQ 6666
#define MAX_CONNECT 20

/* DCFTest客户端发送消息函数 */
void DCFTest_msg(char *ip, char *msg, char *rec);

/* DCFTest服务器处理接收客户端消息函数 */
void DCFTest_back();