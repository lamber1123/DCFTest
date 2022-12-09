/*
 * Copyright (c) 2021 Huawei Technologies Co.,Ltd.
 *
 * openGauss is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 *
 * test_main.c
 *    DCF test main
 *
 * IDENTIFICATION
 *    test/test_main/test_main.c
 *
 * -------------------------------------------------------------------------
 */
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "dcf_interface.h"
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

typedef struct
{
    char *buffer;
    size_t bufferLength;
    ssize_t inputLength;

} inputBuffer;

inputBuffer *newInputBuffer()
{
    inputBuffer *buf = (inputBuffer *)malloc(sizeof(inputBuffer));
    buf->buffer = NULL;
    buf->bufferLength = 0;
    buf->inputLength = 0;

    return buf;
}

void readCommand(inputBuffer *buf)
{
    ssize_t byteRead = getline(&(buf->buffer), &(buf->bufferLength), stdin);
    if (byteRead <= 0)
    {
        printf("FAIL TO READ COMMAND\n.");
    }

    buf->inputLength = byteRead - 1;
    buf->buffer[byteRead - 1] = 0;
}

void Print_REPL()
{
    //printf("\033[32m[ PASSED ]\033[0m DCFTest started\n");
    //printf("\033[31m[ FAILED ]\033[0m DCFTest started\n");
    printf("welcome to use DCFTest.\n");
    printf(" _   _      _ _       _ \n| | | | ___| | | ___ | |\n| |_| |/ _ \\ | |/ _ \\| |\n|  _  |  __/ | | (_) |_|\n|_| |_|\\___|_|_|\\___/(_)\n                        \n");
}

void Print_Prompt()
{
    printf("DcfTest > ");
}

int usr_cb_after_writer(unsigned int stream_id, unsigned long long index,
                        const char *buf, unsigned int size, unsigned long long key, int error_no)
{
    return dcf_set_applied_index(stream_id, index);
}

int usr_cb_consensus_notify(unsigned int stream_id, unsigned long long index,
                            const char *buf, unsigned int size, unsigned long long key)
{
    return dcf_set_applied_index(stream_id, index);
}

int DCFTest_exit()
{
    if (dcf_stop() != 0)
    {
        return FAILED;
    }
    
    exit(EXIT_SUCCESS);
    return PASSED;
}

int usr_cb_status_changed_notify(unsigned int stream_id, dcf_role_t new_role)
{
    isleader = true;
    return 0;
}

void Read_Dcf_start_Config(char *config)
{
    int fd = open("../DCFTestConfig.json", O_RDONLY);
    if (fd == -1)
    {
        printf("can not open the DCFTestConfig.json file.\n");
        exit(EXIT_FAILURE);
    }
    int len = read(fd, config, 1024);
    if (len < 0)
    {
        printf("The start config is NULL.\n");
    }

    close(fd);
}

int DCFTest_set_param(int node_id)
{
    int ret = 0;
    if (node_id == 1)
    {
        ret = dcf_set_param("DATA_PATH", "./node1");
    }
    else if (node_id == 2)
    {
        ret = dcf_set_param("DATA_PATH", "./node2");
    }
    else if (node_id == 3)
    {
        ret = dcf_set_param("DATA_PATH", "./node3");
    }
    else if (node_id == 4)
    {
        ret = dcf_set_param("DATA_PATH", "./node4");
    }
    else if (node_id == 5)
    {
        ret = dcf_set_param("DATA_PATH", "./node5");
    }

    if (ret != 0)
    {
        printf("set param DATA_PATH fail\n");
    }
    ret = dcf_set_param("LOG_FILE_PERMISSION", "640");
    if (ret != 0)
    {
        printf("set permission data fail.\n");
    }
    ret = dcf_set_param("LOG_PATH_PERMISSION", "750");
    if (ret != 0)
    {
        printf("set permission log fail.\n");
    }

    ret = dcf_set_param("FLOW_CONTROL_CPU_THRESHOLD", "80");
    if (ret != 0)
    {
        printf("set param FLOW_CONTROL_CPU_THRESHOLD fail\n");
    }
    ret = dcf_set_param("FLOW_CONTROL_NET_QUEUE_MESSAGE_NUM_THRESHOLD", "100");
    if (ret != 0)
    {
        printf("set param FLOW_CONTROL_NET_QUEUE_MESSAGE_NUM_THRESHOLD fail\n");
    }
    ret = dcf_set_param("FLOW_CONTROL_DISK_RAWAIT_THRESHOLD", "12000");
    if (ret != 0)
    {
        printf("set param FLOW_CONTROL_DISK_RAWAIT_THRESHOLD fail\n");
    }
    ret = dcf_set_param("LOG_LEVEL", "RUN_ERR|RUN_WAR|RUN_INF|DEBUG_ERR|DEBUG_WAR|DEBUG_INF|MEC|OPER|TRACE|PROFILE");

    return ret;
}

void DCFTest_write(bool isleader, inputBuffer *input_buffer, char *writeContents, unsigned long long *writeIndex)
{
    if (isleader)
    {
        if (dcf_write(1, writeContents, input_buffer->inputLength, 0, writeIndex) != 0)
        {
            printf("\033[31m[ FAILED ]\033[0m write fail.\n");
        }
        else
        {
            printf("\033[32m[ PASSED ]\033[0m dcf write succeed,size=%d, index =%d.\n", input_buffer->inputLength, *writeIndex);
        }
    }
    else
    {
        if (dcf_universal_write(1, writeContents, input_buffer->inputLength, 0, writeIndex) != 0)
        {
            printf("\033[31m[ FAILED ]\033[0m universal write fail.\n");
        }
        else
        {
            printf("\033[32m[ PASSED ]\033[0m dcf universal write succeed,size=%d,index = %d.\n", input_buffer->inputLength, *writeIndex);
        }
    }
}

void DCFTest_read(unsigned int streamId, unsigned long long ReadIndex, char *ReadBuffer, unsigned int ReadLength)
{
    if (dcf_read(1, ReadIndex, ReadBuffer, ReadLength) == -1)
    {
        printf("\033[31m[ FAILED ]\033[0m dcf read failed.\n");
    }
    else
    {
        printf("\033[32m[ PASSED ]\033[0m dcf read succeed, the index %ld read contents is %s.\n", ReadIndex, ReadBuffer);
    }
}

int DCFTest_start(int node_id, char *dcf_start_config)
{
    char printlog[1024] = "";
    int ret = 0;

    if (dcf_start_config == NULL)
    {
        // printf("\033[31m[ FAILED ]\033[0m allocate memory failed.\n");
        strcat(printlog, "\033[31m[ FAILED ]\033[0m allocate memory failed.\n");
        printf("%s", printlog);
        return FAILED;
    }

    Read_Dcf_start_Config(dcf_start_config);

    // 设置DCF参数
    if (DCFTest_set_param(node_id) != 0)
    {
        // printf("\033[31m[ FAILED ]\033[0m set param LOG_LEVEL failed.\n");
        strcat(printlog, "\033[31m[ FAILED ]\033[0m set param LOG_LEVEL failed.\n");
        printf("%s", printlog);
        return FAILED;
    }

    // 注册回调函数
    if (dcf_register_after_writer(usr_cb_after_writer) != 0)
    {
        // printf("\033[31m[ FAILED ]\033[0m dcf_register_after_writer failed.\n");
        strcat(printlog, "\033[31m[ FAILED ]\033[0m dcf_register_after_writer failed.\n");
        printf("%s", printlog);
        return FAILED;
    }
    if (dcf_register_consensus_notify(usr_cb_consensus_notify) != 0)
    {
        // printf("\033[31m[ FAILED ]\033[0m dcf_register_consensus_notify failed.\n");
        strcat(printlog, "\033[31m[ FAILED ]\033[0m dcf_register_consensus_notify failed.\n");
        printf("%s", printlog);
        return FAILED;
    }
    if (dcf_register_status_notify(usr_cb_status_changed_notify) != 0)
    {
        // printf("\033[31m[ FAILED ]\033[0m dcf_register_status_notify failed.\n");
        strcat(printlog, "\033[31m[ FAILED ]\033[0m dcf_register_status_notify failed.\n");
        printf("%s", printlog);
        return FAILED;
    }

    // 启动DCF
    if (dcf_start(node_id, dcf_start_config) == 0)
    {
        // printf("\033[32m[ PASSED ]\033[0m dcf start succeed, node_id = %d.\n", node_id);
        char templog[128];
        sprintf(templog, "\033[32m[ PASSED ]\033[0m dcf start succeed, node_id = %d.\n", node_id);
        strcat(printlog, templog);
    }
    else
    {
        // printf("\033[31m[ FAILED ]\033[0m dcf start failed,node_id = %d.\n", node_id);
        char templog[128];
        sprintf(templog, "\033[31m[ FAILED ]\033[0m dcf start failed, node_id = %d.\n", node_id);
        strcat(printlog, templog);
        return FAILED;
    }

    printf("%s", printlog);
    return PASSED;
}

int DCFTest_add_node(unsigned int AddNode_id, char* Addip, unsigned int Addport)
{
    dcf_role_t Addrole = 2; // Addrole is follower
    unsigned int wait_time = 200;
    
    if (dcf_add_member(1, AddNode_id, Addip, Addport, Addrole, wait_time) == FAILED)
    {
        return FAILED;
    }
    
    return PASSED;
}

int DCFTest_remove_node(int AddNode_id)
{
    unsigned int wait_time = 200;
            
    if (dcf_remove_member(1, AddNode_id, wait_time) == FAILED)
    {
        return FAILED;
    }
    
    return PASSED;
}

void DCFTest_index(int node_id)
{
    unsigned long long leadlastindex = -1;
    unsigned long long last_disk_index = -1;
    unsigned long long data_last_commit_index = -1;
    unsigned long long cluster_min_applied_idx = -1;

    if (dcf_get_leader_last_index(1, &leadlastindex) == -1)
    {
        printf("\033[31m[ FAILED ]\033[0m get leader last index failed.\n");
    }
    else
    {
        printf("\033[32m[ ------ ]\033[0m the leader last index is %ld.\n", leadlastindex);
    }

    if (dcf_get_node_last_disk_index(1, node_id, &last_disk_index) == -1)
    {
        printf("\033[31m[ FAILED ]\033[0m get node last disk index failed.\n");
    }
    else
    {
        printf("\033[32m[ ------ ]\033[0m the node last disk index is %ld.\n", last_disk_index);
    }

    if (dcf_get_data_commit_index(1, node_id, &data_last_commit_index) == -1)
    {
        printf("\033[31m[ FAILED ]\033[0m get dcf data last commit index failed.\n");
    }
    else
    {
        printf("\033[32m[ ------ ]\033[0m the data last commit index is %ld.\n", data_last_commit_index);
    }

    if (dcf_get_cluster_min_applied_idx(1, &cluster_min_applied_idx) == -1)
    {
        printf("\033[31m[ FAILED ]\033[0m get cluster min applied index failed.\n");
    }
    else
    {
        printf("\033[32m[ PASSED ]\033[0m the cluster min applid index is %ld.\n", cluster_min_applied_idx);
    }
    
}

void DCFTest_query()
{
    char query_buffer[1024];
    unsigned int length = 1024;
    if (dcf_query_cluster_info(query_buffer, length) == -1)
    {
        printf("\033[33m[ FAILED ]\033[0m query cluster info failed.\n");
    }
    else
    {
        printf("\033[32m[ PASSED ]\033[0m cluster info: %s.\n", query_buffer);
    }
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

        // // DCFTest > change role <n_id> <n_role>
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

        printf("\n");
        cm_sleep(200);

    } while (1);
    
    return 0;
}