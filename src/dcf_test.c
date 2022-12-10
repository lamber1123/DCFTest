#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "dcf_interface.h"
#include "dcf_demo.h"
#include "message.h"
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef WIN32
#include <windows.h>
#define cm_sleep(ms) Sleep(ms)
#else
#include <signal.h>
static inline void cm_sleep(int ms)
{
    struct timespec tq, tr;
    tq.tv_sec = ms / 1000;
    tq.tv_nsec = (ms % 1000) * 1000000;

    (void)nanosleep(&tq, &tr);
}
#endif

#define PASSED 0
#define FAILED -1

#ifndef _WIN32
static void sig_proc(int signal)
{
    exit(0);
}
#endif

bool isleader = false;

void *start_server(void *fd)
{
    DCFTest_back();
}

int main(int argc, char *argv[])
{
#ifndef _WIN32
    if (signal(SIGUSR1, sig_proc) == SIG_ERR)
    {
        printf("register SIGUSR1 sig_proc failed!\n");
    }
    if (signal(SIGUSR2, sig_proc) == SIG_ERR)
    {
        printf("register SIGUSR2 sig_proc failed!\n");
    }
#endif

    // 输出欢迎
    Print_REPL();

    int node_id = 1;
    char *dcf_start_config = (char *)malloc(1024);

    // 尝试以1-5作为node_id启动DCF
    for (node_id = 1; node_id <= 5; node_id++)
    {
        if (DCFTest_start(node_id, dcf_start_config) == 0) 
        {
            break;
        }

        if (node_id == 5) 
        {
            printf("\033[31m[ FAILED ]\033[0m failed to start with node_id 1 to 5.\n");
        }
    }
    
    printf("\n");

    char *writeContents = (char *)malloc(1024);
    inputBuffer *input_buffer = newInputBuffer();

    long long count = 0;
    unsigned long long int writeIndex = 0;
    char readbuffer[2048];
    unsigned long long readIndex = -1;

    bool isopenback = 0;

    // DCFTest主循环
    do
    {
        // DCFTest >
        Print_Prompt();
        readCommand(input_buffer);
        
        // DCFTest > exit
        if (strcmp(input_buffer->buffer, "exit") == 0)
        {
            free(dcf_start_config);
            free(writeContents);
            free(input_buffer);

            if (DCFTest_exit() == PASSED)
            {
                printf("\033[32m[ PASSED ]\033[0m Dcf Stop succeed.\n");
            }
            else
            {
                printf("\033[31m[ FAILED ]\033[0m Dcf stop failed.\n");
            }
        }

        // DCFTest > start
        else if (strncmp(input_buffer->buffer, "start", 5) == 0)
        {
            dcf_stop() == 0;

            int temp_id = -1;
            int arg_size = sscanf(input_buffer->buffer, "start %d", &temp_id);

            if (arg_size < 1) temp_id = node_id;
            if (DCFTest_start(temp_id, dcf_start_config) == FAILED)
            {
                printf("\033[31m[ FAILED ]\033[0m stop DCF failed with node_id %d.\n", temp_id);
            }
            else
            {
                node_id = temp_id;
            }
        }

        // DCFTest > stop
        else if (strcmp(input_buffer->buffer, "stop") == 0)
        {
            dcf_stop();
        }

        // DCFTest > add node
        else if (strncmp(input_buffer->buffer, "add node", 8) == 0)
        {
            unsigned int AddNode_id = -1;
            char *Addip = (char *)malloc(128);
            unsigned int Addport = -1;

            int arg_size = sscanf(input_buffer->buffer, "add node %d %s %d", &AddNode_id, Addip, &Addport);
            if (arg_size < 3) 
            {
                printf("\033[34m[ REMIND ]\033[0m usage: add node <a_id> <a_ip> <a_port>.\n");
                printf("\033[34m[ REMIND ]\033[0m option:\n");
                printf("\033[34m[ REMIND ]\033[0m     -a_id         add node id.\n");
                printf("\033[34m[ REMIND ]\033[0m     -a_ip         add node ip.\n");
                printf("\033[34m[ REMIND ]\033[0m     -a_port       add node port.\n");
            }
            else
            {
                if (DCFTest_add_node(AddNode_id, Addip, Addport) == FAILED)
                {
                    printf("\033[31m[ FAILED ]\033[0m add node failed.\n");
                }
                else
                {
                    printf("\033[32m[ PASSED ]\033[0m add node succeed, node_id is %d, IP is %s, port is %d.\n", AddNode_id, Addip, Addport);
                }
            }
        }

        // DCFTest > remove node
        else if (strncmp(input_buffer->buffer, "remove node", 11) == 0)
        {
            unsigned int AddNode_id = -1;

            int arg_size = sscanf(input_buffer->buffer, "remove node %d", &AddNode_id);
            if (arg_size < 1) 
            {
                printf("\033[34m[ REMIND ]\033[0m usage: remove node <a_id>.\n");
                printf("\033[34m[ REMIND ]\033[0m option:\n");
                printf("\033[34m[ REMIND ]\033[0m     -a_id         remove node id.\n");
            }
            else
            {
                if (DCFTest_remove_node(AddNode_id) == FAILED)
                {
                    printf("\033[31m[ FAILED ]\033[0m remove node failed.\n");
                }
                else
                {
                    printf("\033[32m[ PASSED ]\033[0m remove node succeed, node_id is %d.\n", AddNode_id);
                }
            }
        }
        
        // DCFTest > index
        else if (strcmp(input_buffer->buffer, "index") == 0)
        {
            DCFTest_index(node_id);
        }

        // DCFTest > query
        else if (strcmp(input_buffer->buffer, "query") == 0)
        {
            DCFTest_query();
        }

        // DCFTest > write <w_data>
        else if (strncmp(input_buffer->buffer, "write", 5) == 0)
        {
            int arg_size = sscanf(input_buffer->buffer, "write %s", writeContents);
            if (arg_size < 1)
            {
                printf("\033[34m[ REMIND ]\033[0m usage: write <w_data>.\n");
                printf("\033[34m[ REMIND ]\033[0m option:\n");
                printf("\033[34m[ REMIND ]\033[0m     -w_data       write data.\n");
            }
            else
            {
                DCFTest_write(isleader, input_buffer, writeContents, &writeIndex);
            }
        }

        // DCFTest > read <r_index>
        else if (strncmp(input_buffer->buffer, "read", 4) == 0)
        {
            int arg_size = sscanf(input_buffer->buffer, "read %ld", &readIndex);
            if (arg_size < 1)
            {
                printf("\033[34m[ REMIND ]\033[0m usage: read <r_index>.\n");
                printf("\033[34m[ REMIND ]\033[0m option:\n");
                printf("\033[34m[ REMIND ]\033[0m     -r_index       read index.\n");
            }
            else
            {
                DCFTest_read(1, readIndex, readbuffer, 1024);
            }
        }

        // DCFTest > change role <n_id> <n_role>
        // else if (strncmp(input_buffer->buffer, "change role", 11) == 0)
        // {   
        //     unsigned int n_id = 0;
        //     unsigned int n_role = 0;
        //     int arg_size = sscanf(input_buffer->buffer, "change role %d %d", &n_id, &n_role);
        //     if (arg_size < 2)
        //     {
        //         printf("\033[34m[ REMIND ]\033[0m usage: change role <n_id> <n_role>.\n");
        //         printf("\033[34m[ REMIND ]\033[0m option:\n");
        //         printf("\033[34m[ REMIND ]\033[0m     -n_id       node id.\n");
        //         printf("\033[34m[ REMIND ]\033[0m     -n_role     node role (1:leader, 2:follower).\n");
        //     }
        //     else
        //     {
        //         if(dcf_change_member_role(1, n_id, n_role, 200) == 0)
        //         {
        //             printf("\033[32m[ PASSED ]\033[0m change member role succeed.\n");
        //         }
        //         else
        //         {
        //             printf("\033[31m[ FAILED ]\033[0m change member role failed.\n");
        //         }
        //     }
        // }

        // DCFTest > promote leader <n_id>
        else if (strncmp(input_buffer->buffer, "promote leader", 14) == 0)
        {   
            // int dcf_promote_leader(unsigned int stream_id, unsigned int node_id, unsigned int wait_timeout_ms);

            unsigned int n_id = 0;
            int arg_size = sscanf(input_buffer->buffer, "promote leader %d", &n_id);
            if (arg_size < 1)
            {
                printf("\033[34m[ REMIND ]\033[0m usage: promote leader <n_id>.\n");
                printf("\033[34m[ REMIND ]\033[0m option:\n");
                printf("\033[34m[ REMIND ]\033[0m     -n_id       node id.\n");
            }
            else
            {
                if(dcf_promote_leader(1, n_id, 200) == 0)
                {
                    printf("\033[32m[ PASSED ]\033[0m promote leader succeed.\n");
                }
                else
                {
                    printf("\033[31m[ FAILED ]\033[0m promote leader failed.\n");
                }
            }
        }

        // DCFTest > demote follower
        else if (strcmp(input_buffer->buffer, "demote follower") == 0)
        {   
            // int dcf_demote_follower(unsigned int stream_id);

            if(dcf_demote_follower(1) == 0)
            {
                printf("\033[32m[ PASSED ]\033[0m demote follower succeed.\n");
            }
            else
            {
                printf("\033[31m[ FAILED ]\033[0m demote follower failed.\n");
            }
        }

        // DCFTest > msg <n_ip> [m_msg]
        else if (strncmp(input_buffer->buffer, "msg", 3) == 0)
        {
            char m_ip[MAX_LINE];
            char m_msg[MAX_LINE];
            char m_rec[MAX_LINE];
            int arg_size = sscanf(input_buffer->buffer, "msg %s %s", m_ip, m_msg);
            if (arg_size < 1)
            {
                printf("\033[34m[ REMIND ]\033[0m usage: msg <m_ip> [m_msg].\n");
                printf("\033[34m[ REMIND ]\033[0m option:\n");
                printf("\033[34m[ REMIND ]\033[0m     -m_ip         target node ip.\n");
                printf("\033[34m[ REMIND ]\033[0m     -m_msg        message to send.\n");
            }
            else
            {
                printf("\033[32m[ RUN    ]\033[0m sending the message to %s...\n", m_ip);
                if(DCFTest_msg(m_ip, m_msg, m_rec) == PASSED)
                {
                    printf("\033[32m[ ------ ]\033[0m %s: %s\n", m_ip, m_rec);
                    printf("\033[32m[ PASSED ]\033[0m send message succed.\n");
                }
            }
        }

        // DCFTest > back
        else if (strcmp(input_buffer->buffer, "back") == 0)
        {
            pthread_t back_tid;
            if(isopenback == 0)
            {
                if (pthread_create(&back_tid , NULL , start_server, NULL) == -1)
                {
                    printf("\033[31m[ FAILED ]\033[0m pthread create failed.\n");
                }
                else 
                {
                    isopenback = 1;
                }
            }
            else
            {
                printf("\033[31m[ FAILED ]\033[0m the background has started.\n");
            }
        }

        // DCFTest > other case
        else
        {
            printf("\033[34m[ REMIND ]\033[0m do you want to input ?\n");
            printf("\033[34m[ REMIND ]\033[0m start             start local DCF node.\n");
            printf("\033[34m[ REMIND ]\033[0m stop              stop local DCF node.\n");
            printf("\033[34m[ REMIND ]\033[0m add node          add DCF node to cluster.\n");
            printf("\033[34m[ REMIND ]\033[0m remove node       remove DCF node from cluster.\n");
            printf("\033[34m[ REMIND ]\033[0m index             get index information.\n");
            printf("\033[34m[ REMIND ]\033[0m query             query cluster information.\n");
            printf("\033[34m[ REMIND ]\033[0m write             write w_data to file.\n");
            printf("\033[34m[ REMIND ]\033[0m read              read data from r_index.\n");
            printf("\033[34m[ REMIND ]\033[0m promote leader    promote follower to leader.\n");
            printf("\033[34m[ REMIND ]\033[0m demote follower   promote leader to follower.\n");
            printf("\033[34m[ REMIND ]\033[0m exit              exit DCFTest.\n");
        }
        
        // log truncate
        if ((++count) % 10000 == 0)
        {
            if (dcf_truncate(1, count / 2) != 0)
            {
                printf("\033[31m[ FAILED ]\033[0m truncate failed.\n");
            }
            else
            {
                printf("\033[31m[ PASSED ]\033[0m truncate succeed index = %lld.\n", count / 2);
            }
        }

        cm_sleep(300);
        printf("\n");

    } while (1);
    
    return 0;
}