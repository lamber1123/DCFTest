#include "message.h"

int main(int argc , char **argv)
{
    /*判断是否为合法输入*/
    if(argc != 2)
    {
        perror("\033[31m[ FAILED ]\033[0m usage: client <IPaddress>");
        exit(1);
    }//if
	else 
	{
        char msg[MAX_LINE];
		char rec[MAX_LINE];

        printf("\033[32m[ ------ ]\033[0m input msg: ");
        scanf("%s", msg); 
		DCFTest_msg(argv[1], msg, rec);
		printf("\033[32m[ PASSED ]\033[0m %s: %s\n", argv[1], rec);
	}

	return 0;
}
