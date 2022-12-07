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
 * dcf_interface.h
 *    DCF API
 *
 * IDENTIFICATION
 *    src/interface/dcf_interface.h
 *
 * -------------------------------------------------------------------------
 */

#ifndef __DCF_INTERFACE_H__
#define __DCF_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __attribute__ ((visibility ("default")))
#endif

// don't change the order
typedef enum en_dcf_role {
    DCF_ROLE_UNKNOWN = 0,
    DCF_ROLE_LEADER,
    DCF_ROLE_FOLLOWER,
    DCF_ROLE_LOGGER,
    DCF_ROLE_PASSIVE,
    DCF_ROLE_PRE_CANDIDATE,
    DCF_ROLE_CANDIDATE,
    DCF_ROLE_CEIL,
} dcf_role_t;

typedef enum en_dcf_exception {
    DCF_RUNNING_NORMAL = 0,
    DCF_EXCEPTION_MISSING_LOG,
    DCF_EXCEPTION_CEIL,
} dcf_exception_t;

typedef enum en_dcf_commit_index_type {
    DCF_INDEX_UNKNOWN = 0,
    DCF_LOCAL_COMMIT_INDEX,
    DCF_LEADER_COMMIT_INDEX,
    DCF_CONSENSUS_COMMIT_INDEX,
    DCF_INDEX_LEVEL_CEIL,
} dcf_commit_index_type_t;

/*
    param_name: The parameter types are as follows
    "ELECTION_TIMEOUT"   --unit:ms
    "HEARTBEAT_INTERVAL" --unit:ms
    "RUN_MODE"

    "INSTANCE_NAME"
    "DATA_PATH"
    "LOG_PATH"

    "LOG_LEVEL"  --1:RUN_ERR, 2:RUN_WARN, 3:RUN_INFO, 4:DEBUG_ERR, 5:DEBUG_WARN, 6:DEBUG_INFO
    "LOG_BACKUP_FILE_COUNT"
    "AUDIT_BACKUP_FILE_COUNT"
    "MAX_LOG_FILE_SIZE"
    "MAX_AUDIT_FILE_SIZE"
    "LOG_FILE_PERMISSION"
    "LOG_PATH_PERMISSION"

    "MES_WORK_THREAD_NUM"
    "MES_CHANNEL_NUM"
    "MES_PROC_POOL_SIZE"
    "MEM_POOL_INIT_SIZE"
    "MEM_POOL_MAX_SIZE"
    "STG_POOL_INIT_SIZE"
    "STG_POOL_MAX_SIZE"
    "MEC_POOL_MAX_SIZE"
    "DN_FLOW_CONTROL_RTO"
    "DN_FLOW_CONTROL_RPO"
*/
EXPORT_API int dcf_set_param(const char *param_name, const char *param_value);

/*
    param_name: same as the interface dcf_set_param
    param_value: please allocate memory. if is num, you should convert
    size: the size of param_value
*/
EXPORT_API int dcf_get_param(const char *param_name, char *param_value, unsigned int size);

/**
* Callback function when leader changed
* @param [in]  stream_id
* @param [in]  new leader_id
* @return 0    success
* @return !=0  fail
*/
typedef int (*usr_cb_election_notify_t)(unsigned int stream_id, unsigned int new_leader);

/**
* Callback function for dealing msg
* @param [in]  stream_id
* @param [in]  msg
* @param [in]  msg size
* @return 0    success
* @return !=0  fail
*/
typedef int (*usr_cb_msg_proc_t)(unsigned int stream_id, unsigned int src_node_id, const char* msg,
    unsigned int msg_size);

typedef int (*usr_cb_after_writer_t)(unsigned int stream_id, unsigned long long index,
    const char *buf, unsigned int size, unsigned long long key, int error_no);
typedef int (*usr_cb_consensus_notify_t)(unsigned int stream_id, unsigned long long index,
    const char *buf, unsigned int size, unsigned long long key);
typedef int (*usr_cb_status_notify_t)(unsigned int stream_id, dcf_role_t new_role);
typedef void (*usr_cb_log_output_t)(int log_type, int log_level,
    const char *code_file_name, unsigned int code_line_num,
    const char *module_name, const char *format, ...);
typedef int(*usr_cb_decrypt_pwd_t)(const char *cipher, unsigned int len, char *plain, unsigned int size);
typedef int(*usr_cb_exception_notify_t)(unsigned int stream_id, dcf_exception_t exception);
typedef void (*usr_cb_thread_memctx_init_t)();

/*
    Callback function after dcf_write successfully in leader node
*/
EXPORT_API int dcf_register_after_writer(usr_cb_after_writer_t cb_func);

/*
    Callback function after dcf_write successfully in follower node
*/
// dcf_set commit?
EXPORT_API int dcf_register_consensus_notify(usr_cb_consensus_notify_t cb_func);

/*
    Callback function after node role changed
*/
EXPORT_API int dcf_register_status_notify(usr_cb_status_notify_t cb_func);

/*
    Callback function for system run log output
*/
EXPORT_API int dcf_register_log_output(usr_cb_log_output_t cb_func);

/*
    Callback function when dcf run to an abnormal state, which is only called by follower
*/
EXPORT_API int dcf_register_exception_report(usr_cb_exception_notify_t cb_func);

/**
* Callback function when dcf leader changed, which is only called by follower
* @param [in]  addr of callback function
* @return 0    success
* @return !=0  fail
*/
EXPORT_API int dcf_register_election_notify(usr_cb_election_notify_t cb_func);

/**
* Callback function for dealing msg
* @param [in]  addr of callback function
* @return 0    success
* @return !=0  fail
*/
EXPORT_API int dcf_register_msg_proc(usr_cb_msg_proc_t cb_func);

/**
* Callback function for thread memory context init
* @param [in]  addr of callback function
* @return 0    success
* @return !=0  fail
*/
EXPORT_API int dcf_register_thread_memctx_init(usr_cb_thread_memctx_init_t cb_func);

/*
    node_id: current node id
    cfg_str: cluster node list
             string format: stream_id $ node_id $ ip $ port $ role, multiple use (,) to separate
*/
EXPORT_API int dcf_start(unsigned int node_id, const char *cfg_str);

/*
    This interface can be used only at leader. (better performance).
    stream_id: Log stream channel id
    buffer: The data to write and replicate
    length: size of data buffer
*/
EXPORT_API int dcf_write(unsigned int stream_id, const char* buffer, unsigned int length,
    unsigned long long key, unsigned long long *index);

/*
This interface can be used at any node (leader, follower, etc.)
    stream_id: Log stream channel id
    buffer: The data to write and replicate
    length: size of data buffer
*/
EXPORT_API int dcf_universal_write(unsigned int stream_id, const char *buffer, unsigned int length,
    unsigned long long key, unsigned long long *index);

/*
    stream_id: Log stream channel id
    index: Log serial number, return in callback function
    buffer: The data read to
    length: size of data buffer
*/
EXPORT_API int dcf_read(unsigned int stream_id, unsigned long long index, char *buffer, unsigned int length);

/*
    stop worker thread
*/
EXPORT_API int dcf_stop();

/*
    Discard logs before index: first_index_kept
*/
EXPORT_API int dcf_truncate(unsigned int stream_id, unsigned long long first_index_kept);

/*
    set last applied index
*/
EXPORT_API int dcf_set_applied_index(unsigned int stream_id, unsigned long long index);

/*
    get cluster min applied index
*/
EXPORT_API int dcf_get_cluster_min_applied_idx(unsigned int stream_id, unsigned long long* index);

/*
    query the leader's last log index
*/
EXPORT_API int dcf_get_leader_last_index(unsigned int stream_id, unsigned long long* index);

/*
    query current node's last log index
*/
EXPORT_API int dcf_get_last_index(unsigned int stream_id, unsigned long long* index);

/*
    Query cluster stream list info
*/
EXPORT_API int dcf_query_cluster_info(char* buffer, unsigned int length);

/*
    Query stream info
*/
EXPORT_API int dcf_query_stream_info(unsigned int stream_id, char *buffer, unsigned int length);

/*
    Query leader info
*/
EXPORT_API int dcf_query_leader_info(unsigned int stream_id, char *ip, unsigned int ip_len, unsigned int *port,
    unsigned int *node_id);

/*
    Get the specific error code of API call
*/
EXPORT_API int dcf_get_errorno();

/*
    Get the specific error info of error no
*/
EXPORT_API const char* dcf_get_error(int code);

/*
    lib version
*/
EXPORT_API const char *dcf_get_version();

/*
    get node's last disk index
*/
EXPORT_API int dcf_get_node_last_disk_index(unsigned int stream_id, unsigned int node_id, unsigned long long* index);

/*
    add new node to cluster
*/
EXPORT_API int dcf_add_member(unsigned int stream_id, unsigned int node_id, const char *ip, unsigned int port,
    dcf_role_t role, unsigned int wait_timeout_ms);
/*
    remove node from cluster
*/
EXPORT_API int dcf_remove_member(unsigned int stream_id, unsigned int node_id, unsigned int wait_timeout_ms);

/*
    change node role
*/
EXPORT_API int dcf_change_member_role(unsigned int stream_id, unsigned int node_id,
    dcf_role_t new_role, unsigned int wait_timeout_ms);

/*
    change node role/group/priority etc.
    change_str: change string list
             string format example: '[{"stream_id":1,"node_id":1,"group":1,"priority":5,"role":"FOLLOWER"}]'
*/
EXPORT_API int dcf_change_member(const char *change_str, unsigned int wait_timeout_ms);

/*
    promote the specified node to leader
*/
EXPORT_API int dcf_promote_leader(unsigned int stream_id, unsigned int node_id, unsigned int wait_timeout_ms);

/*
    External trigger node timeout notification
    if stream_id== 0, means all stream the node in
*/
EXPORT_API int dcf_timeout_notify(unsigned int stream_id);

/*
    decrypt password
*/
EXPORT_API int dcf_register_decrypt_pwd(usr_cb_decrypt_pwd_t cb_func);

/*
    judging current node is healthy or not.
*/
EXPORT_API int dcf_node_is_healthy(unsigned int stream_id, dcf_role_t* node_role, unsigned int* is_healthy);

typedef enum e_dcf_work_mode {
    WM_NORMAL = 0,
    WM_MINORITY = 1,
    WM_CEIL
}dcf_work_mode_t;

/*
External trigger node timeout notification
if stream_id== 0, means all stream the node in
*/
EXPORT_API int dcf_set_work_mode(unsigned int stream_id, dcf_work_mode_t work_mode, unsigned int vote_num);

/*
    Query statistics info
*/
EXPORT_API int dcf_query_statistics_info(char *buffer, unsigned int length);

/*
    Check whether all dcf logs of the current node are applied.
*/
EXPORT_API int dcf_check_if_all_logs_applied(unsigned int stream_id, unsigned int *all_applied);

/*
    Set trace key to print trace log
*/
EXPORT_API int dcf_set_trace_key(unsigned long long trace_key);

/*
    Internal Invocation
*/
void dcf_set_exception(int stream_id, dcf_exception_t exception);

/**
* send msg
* @param [in]  stream_id
* @param [in]  dest_node_id
* @param [in]  msg
* @param [in]  msg_size. MAXSIZE = 512K
* @return 0    success
* @return !=0  fail
*/
EXPORT_API int dcf_send_msg(unsigned int stream_id, unsigned int dest_node_id, const char* msg,
    unsigned int msg_size);

/**
* send msg
* @param [in]  stream_id
* @param [in]  msg
* @param [in]  msg_size. MAXSIZE = 512K
* @return 0    success
* @return !=0  fail
*/
EXPORT_API int dcf_broadcast_msg(unsigned int stream_id, const char* msg, unsigned int msg_size);

/**
* Suspend replication log for this node
* @param [in]  stream_id
* @param [in]  node_id
* @param [in]  time_us, range[0, 1000000]
* @return 0    success
* @return !=0  fail
*/
EXPORT_API int dcf_pause_rep(unsigned int stream_id, unsigned int node_id, unsigned int time_us);

/**
* demote follower
* @param [in]  stream_id
* @return 0    success
* @return !=0  fail
*/
EXPORT_API int dcf_demote_follower(unsigned int stream_id);

/**
* Get last log data commit index
* @param [in]  stream_id
* @param [in]  is_consensus, range[0, 1]
* @param [out] index
* @return 0    success
* @return !=0  fail
*/
EXPORT_API int dcf_get_data_commit_index(unsigned int stream_id, dcf_commit_index_type_t index_type,
    unsigned long long* index);

/**
* Get cur node's current_term and role
* @param [in]  stream_id
* @param [out]  term
* @param [out] role
* @return 0    success
* @return !=0  fail
*/
EXPORT_API int dcf_get_current_term_and_role(unsigned int stream_id, unsigned long long* term, dcf_role_t* role);

/**
* Set cur node's election_priority
* @param [in]  stream_id
* @param [in]  priority
* @return 0    success
* @return !=0  fail
*/
EXPORT_API int dcf_set_election_priority(unsigned int stream_id, unsigned long long priority);

/**
* set global timer to dcf, use by dcc.
* @param [in]  timer ptr of struct:gs_timer_t
* @return void
*/
EXPORT_API void dcf_set_timer(void *timer);

#ifdef __cplusplus
}
#endif

#endif
