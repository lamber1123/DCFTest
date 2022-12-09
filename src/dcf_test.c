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
void DCFTest_exit(inputBuffer *buf)
{
    free(buf);

    if (dcf_stop() == 0)
    {
        printf("\033[32m[ PASSED ]\033[0m Dcf Stop succeed.\n\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("\033[31m[ FAILED ]\033[0m Dcf stop failed.\n\n");
    }
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
        printf("can not open the DcfFrameworkConfig.json file.\n");
        exit(EXIT_FAILURE);
    }
    int len = read(fd, config, 1024);
    if (len < 0)
    {
        printf("The start config is NULL.\n");
    }

    close(fd);
}

int Dcf_Set_Param(int node_id)
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
            printf("\033[32m[ PASSED ]\033[0m dcf write succeed,size=%d,index =%d.\n", input_buffer->inputLength, *writeIndex);
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

int DCFTest_start(int node_id, inputBuffer *input_buffer, char *dcf_start_config, char *writeContents)
{
    char printlog[1024] = "";
    int ret = 0;

    // 设置DCF参数
    if (Dcf_Set_Param(node_id) != 0)
    {
        // printf("\033[31m[ FAILED ]\033[0m set param LOG_LEVEL failed.\n");
        strcat(printlog, "\033[31m[ FAILED ]\033[0m set param LOG_LEVEL failed.\n");
    }
    else
    {
        // printf("\033[32m[ ------ ]\033[0m set param LOG_LEVEL succeeded.\n");
        strcat(printlog, "\033[32m[ ------ ]\033[0m set param LOG_LEVEL succeeded.\n");
    }

    // 注册回调函数
    if (dcf_register_after_writer(usr_cb_after_writer) != 0)
    {
        // printf("\033[31m[ FAILED ]\033[0m dcf_register_after_writer failed.\n");
        strcat(printlog, "\033[31m[ FAILED ]\033[0m dcf_register_after_writer failed.\n");
    }
    else if (dcf_register_consensus_notify(usr_cb_consensus_notify) != 0)
    {
        // printf("\033[31m[ FAILED ]\033[0m dcf_register_consensus_notify failed.\n");
        strcat(printlog, "\033[31m[ FAILED ]\033[0m dcf_register_consensus_notify failed.\n");
    }
    else if (dcf_register_status_notify(usr_cb_status_changed_notify) != 0)
    {
        // printf("\033[31m[ FAILED ]\033[0m dcf_register_status_notify failed.\n");
        strcat(printlog, "\033[31m[ FAILED ]\033[0m dcf_register_status_notify failed.\n");
    }
    else
    {
        // printf("\033[32m[ ------ ]\033[0m dcf_register_status_notify succesed.\n");
        strcat(printlog, "\033[32m[ ------ ]\033[0m dcf_register_status_notify succesed.\n");
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
        sprintf(templog, "\033[31m[ FAILED ]\033[0m dcf start failed,node_id = %d.\n", node_id);
        strcat(printlog, templog);
        ret = -1;
    }

    // 根据启动是否成功决定是否打印日志
    if (ret == 0) printf("%s", printlog);

    return ret;

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
    // inputBuffer *input_buffer;
    char *dcf_start_config = (char *)malloc(1024);
    char *writeContents = (char *)malloc(1024);

    // 启动DCF
    printf("\033[32m[ RUN--- ]\033[0m start DCF...\n");
    // printf("\033[32m[ ------ ]\033[0m dcf lib version: %s\r\n", dcf_get_version());

    if (dcf_start_config == NULL || writeContents == NULL)
    {
        printf("\033[31m[ FAILED ]\033[0m allocate memory failed.\n");
    }
    else
    {
        printf("\033[32m[ ------ ]\033[0m allocate memory succeed.\n");
    }

    Read_Dcf_start_Config(dcf_start_config);
    inputBuffer *input_buffer = newInputBuffer();
    for (node_id = 1; node_id <= 5; node_id++)
    {
        if (DCFTest_start(node_id, input_buffer, dcf_start_config, writeContents) == 0) 
        {
            break;
        }

        if (node_id == 5) 
        {
            printf("\033[31m[ FAILED ]\033[0m start DCF failed, check node id.\n");
        }
    }
    printf("\n");

    long long count = 0;
    unsigned long long int writeIndex = 0;
    char readbuffer[2048];
    unsigned long long lastindex = 10;
    unsigned long long appliedindex = 10;
    unsigned long long leadlastindex = 10;
    unsigned long long last_disk_index = 0;
    unsigned long long readIndex = -1;
    unsigned long long data_last_commit_index;
    unsigned long long cluster_min_applied_idx;

    // DCFTest主循环
    do
    {
        Print_Prompt();
        readCommand(input_buffer);
        
        // DCFTest > exit
        if (strcmp(input_buffer->buffer, "exit") == 0)
        {
            free(dcf_start_config);
            free(writeContents);
            DCFTest_exit(input_buffer);
        }

        // DCFTest > start
        else if (strcmp(input_buffer->buffer, "start") == 0)
        {
            printf("\033[32m[ RUN--- ]\033[0m start DCF...\n");
            // printf("\033[32m[ ------ ]\033[0m dcf lib version: %s\r\n", dcf_get_version());

            if (dcf_start_config == NULL || writeContents == NULL)
            {
                printf("\033[31m[ FAILED ]\033[0m allocate memory failed.\n");
            }
            else
            {
                printf("\033[32m[ ------ ]\033[0m allocate memory succeed.\n");
            }

            Read_Dcf_start_Config(dcf_start_config);
            
            for (node_id = 1; node_id <= 5; node_id++)
            {
                if (DCFTest_start(node_id, input_buffer, dcf_start_config, writeContents) == 0) 
                {
                    break;
                }

                if (node_id == 5) 
                {
                    printf("\033[31m[ FAILED ]\033[0m start DCF failed, check node id.\n");
                }
            }
        }

        // DCFTest > stop
        else if (strcmp(input_buffer->buffer, "stop") == 0)
        {
            int ret_stop = dcf_stop();
            if (ret_stop == 0)
            {
                printf("\033[32m[ PASSED ]\033[0m stop DCF succeed.\n");
            }
            else
            {
                printf("\033[31m[ FAILED ]\033[0m stop DCF failed.\n");
            }
        }

        // DCFTest > add node
        else if (strcmp(input_buffer->buffer, "add node") == 0)
        {
            unsigned int AddNode_id = 4;
            const char *Addip = "172.19.0.201";
            unsigned int Addport = 26222;
            dcf_role_t Addrole = 2; // Addrole is follower
            unsigned int wait_time = 200;
            
            // int dcf_add_member(unsigned int stream_id, unsigned int node_id, const char *ip, unsigned int port, dcf_role_t role, unsigned int wait_timeout_ms);

            int ret_dcf_add_member = dcf_add_member(1, AddNode_id, Addip, Addport, Addrole, wait_time);
            if (ret_dcf_add_member == -1)
            {
                printf("\033[31m[ FAILED ]\033[0m add node failed.\n");
            }
            else
            {
                printf("\033[32m[ PASSED ]\033[0m add node succeed, node_id is %d, IP is %s, port is %d.\n", AddNode_id, Addip, Addport);
            }
        }

        // DCFTest > remove node
        else if (strcmp(input_buffer->buffer, "remove node") == 0)
        {
            unsigned int AddNode_id = 4;
            const char *Addip = "172.19.0.201";
            unsigned int Addport = 20229;
            dcf_role_t Addrole = 2; // Addrole is follower
            unsigned int wait_time = 200;

            int ret_dcf_remove_member = dcf_remove_member(1, AddNode_id,wait_time);
            if (ret_dcf_remove_member == -1)
            {
                printf("\033[31m[ FAILED ]\033[0m remove node failed.\n");
            }
            else
            {
                printf("\033[32m[ PASSED ]\033[0m remove node failed, node_id is %d, IP is %s, port is %d.\n", AddNode_id, Addip, Addport);
            }
        }
        
        // DCFTest > index
        else if (strcmp(input_buffer->buffer, "index") == 0)
        {
            if (dcf_get_leader_last_index(1, &leadlastindex) == -1)
            {

                printf("\033[31m[ FAILED ]\033[0m get leader last index failed.\n");
            }
            else
            {
                printf("\033[32m[ PASSED ]\033[0m the leader last index is %ld.\n", leadlastindex);
            }

            if (dcf_get_node_last_disk_index(1, node_id, &last_disk_index) == -1)
            {

                printf("\033[31m[ FAILED ]\033[0m get node last disk index failed.\n");
            }
            else
            {
                printf("\033[32m[ PASSED ]\033[0m the node last disk index is %ld.\n", last_disk_index);
            }

            if (dcf_get_data_commit_index(1, node_id, &data_last_commit_index) == -1)
            {

                printf("\033[31m[ FAILED ]\033[0m get dcf data last commit index failed.\n");
            }
            else
            {
                printf("\033[32m[ PASSED ]\033[0m the data last commit index is %ld.\n", data_last_commit_index);
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

        // DCFTest > query
        else if (strcmp(input_buffer->buffer, "query") == 0)
        {
            char query_buffer[2048];
            unsigned int length = 2048;
            if (dcf_query_cluster_info(query_buffer, length) == -1)
            {
                printf("\033[33m[ FAILED ]\033[0m query cluster info failed.\n");
            }
            else
            {
                printf("\033[32m[ PASSED ]\033[0m query cluster info succeed, cluster info is %s.\n", query_buffer);
            }

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
        
        // DCFTest > get error
        else if (strcmp(input_buffer->buffer, "get error") == 0)
        {
            int errorno = dcf_get_errorno();
            printf("\033[32m[ RUN--- ]\033[0m error code is %d.\n", errorno);
            char errorinfo[1024] = "";
            printf("\033[32m[ PASSED ]\033[0m %s.\n", dcf_get_error(errorno));
        }    

        // DCFTest > get version
        else if (strcmp(input_buffer->buffer, "get version") == 0)
        {
            printf("\033[32m[ PASSED ]\033[0m %s.\n", dcf_get_version());
        }    

        // DCFTest > change role <n_id> <n_role>
        else if (strncmp(input_buffer->buffer, "change role", 11) == 0)
        {   
            // int dcf_change_member_role(unsigned int stream_id, unsigned int node_id, dcf_role_t new_role, unsigned int wait_timeout_ms);

            unsigned int n_id = 0;
            unsigned int n_role = 0;
            int arg_size = sscanf(input_buffer->buffer, "change role %d %d", &n_id, &n_role);
            if (arg_size < 2)
            {
                printf("\033[34m[ REMIND ]\033[0m usage: change role <n_id> <n_role>.\n");
                printf("\033[34m[ REMIND ]\033[0m option:\n");
                printf("\033[34m[ REMIND ]\033[0m     -n_id       node id.\n");
                printf("\033[34m[ REMIND ]\033[0m     -n_role     node role (1:leader, 2:follower).\n");
            }
            else
            {
                if(dcf_change_member_role(1, n_id, n_role, 200) == 0)
                {
                    printf("\033[32m[ PASSED ]\033[0m change member role succeed.\n");
                }
                else
                {
                    printf("\033[31m[ FAILED ]\033[0m change member role failed.\n");
                }
            }
        }

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
            printf("\033[34m[ REMIND ]\033[0m get version       get dcf version information.\n");
            printf("\033[34m[ REMIND ]\033[0m change role       change node role.\n");
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