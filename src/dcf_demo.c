#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "dcf_interface.h"
#include "dcf_demo.h"
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define PASSED 0
#define FAILED -1

extern bool isleader;

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

int DCFTest_read(unsigned int streamId, unsigned long long ReadIndex, char *ReadBuffer, unsigned int ReadLength)
{
    if (dcf_read(1, ReadIndex, ReadBuffer, ReadLength) == -1)
    {
        printf("\033[31m[ FAILED ]\033[0m dcf read failed.\n");
        return FAILED;
    }

    return PASSED;
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
        printf("\033[32m[ PASSED ]\033[0m the cluster min applied index is %ld.\n", cluster_min_applied_idx);
    }
    
}

int DCFTest_query(char *query_buffer, unsigned int length)
{
    if (dcf_query_cluster_info(query_buffer, length) < 0)
    {
        return FAILED;
    }
    else
    {
        return PASSED;
    }
}
