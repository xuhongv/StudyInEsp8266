/*
 * Copyright (c) 2014-2015 Alibaba Group. All rights reserved.
 *
 * Alibaba Group retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have
 * obtained a separate written license from Alibaba Group., you are not
 * authorized to utilize all or a part of this computer program for any
 * purpose (including reproduction, distribution, modification, and
 * compilation into object code), and you must immediately destroy or
 * return to Alibaba Group all copies of this computer program.  If you
 * are licensed by Alibaba Group, your rights to utilize this computer
 * program are limited by the terms of that license.  To obtain a license,
 * please contact Alibaba Group.
 *
 * This computer program contains trade secrets owned by Alibaba Group.
 * and, unless unauthorized by Alibaba Group in writing, you agree to
 * maintain the confidentiality of this computer program and related
 * information and to not disclose this computer program and related
 * information to any other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND
 * Alibaba Group EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING THE WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NONINFRINGEMENT.
 */

#ifndef _ALINK_EXPORT_H_
#define _ALINK_EXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*alink_func) (void *, void *);
typedef int (*alink_func_rw)(unsigned char *buffer, unsigned int len);

#define MAX_SYS_CALLBACK 1
enum ALINK_SYSTEM_CALLBACK {
    ALINK_FUNC_SERVER_STATUS = 0
};

/******************************************/
#define ALINK_TRUE 		(1)
#define ALINK_FALSE 		(0)

#define ALINK_OK 		(0)
#define ALINK_ERR 		(-1)
#define ALINK_SOCKET_ERR 	(-2)
#define ALINK_ERR_NO_MEMORY 	(-3)
#define ALINK_WAIT_FOREVER	(-1)

#define MAX_NAME_LEN	(80 + 1)
#define STR_TOKEN_LEN	(64 + 1)
#define STR_AUTHTOKEN_LEN	(64 + 1)
#define STR_ADMIN_LEN	(32 + 1)
#define STR_UUID_LEN	(32 + 1)
#define STR_NAME_LEN	(32 + 1)
#define STR_SN_LEN	(64 + 1)
#define STR_CID_LEN	(64 + 1)
#define STR_MODEL_LEN	(80 + 1)
#define STR_MAC_LEN	(17 + 1)/* include : */
#define STR_KEY_LEN	(20 + 1)
#define STR_SEC_LEN	(40 + 1)
#define mem_check // os_printf
#define LW_ALINK 1
#define STR_UTC_LEN	(18)
#define STR_MSG_LEN	(128)
#define STR_SHORT_LEN (32)
#define STR_LONG_LEN (128)
#define PLATFORM_OS_VERSION_LEN     STR_SHORT_LEN
#define OS_VERSION_LEN               PLATFORM_OS_VERSION_LEN
#define PLATFORM_MODULE_NAME_LEN   STR_SHORT_LEN
#define OS_MODULE_NAME_LEN           PLATFORM_MODULE_NAME_LEN

enum ALINK_CALLBACK {
    ACB_GET_DEVICE_STATUS = 0,
    ACB_SET_DEVICE_STATUS = 1,
    ACB_SET_DEVICE_STATUS_ARRAY = 2,
    ACB_REQUEST_REMOTE_SERVICE = 3,
    ACB_GET_DEVICE_STATUS_BY_RAWDATA = 4,
    ACB_SET_DEVICE_STATUS_BY_RAWDATA = 5,
    ACB_REQUEST_DEVICE_UPGRADE = 6,
    ACB_REQUEST_DEVICE_UPGRADE_CANCEL = 7,
    ACB_REQUEST_DEVICE_UNUPGRADE = ACB_REQUEST_DEVICE_UPGRADE_CANCEL,   //alias for CANCEL
    MAX_CALLBACK_NUM
};

enum ALINK_SYSTEM_FUNC {
    ALINK_FUNC_AVAILABLE_MEMORY = 0,
    ALINK_FUNC_PREPARE_NETWORK,
    ALINK_FUNC_WAITING_NETWORK,
    ALINK_FUNC_SET_UTC_TIME,
    ALINK_FUNC_READ_CONFIG,
    ALINK_FUNC_WRITE_CONFIG,
    ALINK_FUNC_GET_STATUS,
    ALINK_FUNC_OTA_FIRMWARE_SAVE,
    ALINK_FUNC_OTA_UPGRADE,
    ALINK_FUNC_MAX
};

enum ALINK_STATUS {
    ALINK_STATUS_LINK_DOWN = 0,
    ALINK_STATUS_INITED = 1,
    ALINK_STATUS_REGISTERED,
    ALINK_STATUS_LOGGED,
    ALINK_STATUS_LOGOUT
};

enum ALINK_NETWORK {
    ALINK_NW_UP = 0,
    ALINK_NW_DOWN
};

enum ALINK_LOGLEVEL_BIT {
    ALINK_LL_NONE_BIT = -1,
    ALINK_LL_FATAL_BIT,
    ALINK_LL_ERROR_BIT,
    ALINK_LL_WARN_BIT,
    ALINK_LL_INFO_BIT,
    ALINK_LL_DEBUG_BIT,
    ALINK_LL_TRACE_BIT,
    ALINK_LL_MAX_BIT
};

#define ALINK_LL_NONE  0
#define ALINK_LL_FATAL (1 << ALINK_LL_FATAL_BIT) | ALINK_LL_NONE
#define ALINK_LL_ERROR (1 << ALINK_LL_ERROR_BIT) | ALINK_LL_FATAL
#define ALINK_LL_WARN  (1 << ALINK_LL_WARN_BIT)  | ALINK_LL_ERROR
#define ALINK_LL_INFO  (1 << ALINK_LL_INFO_BIT)  | ALINK_LL_WARN
//#define ALINK_LL_DUMP  (1 << ALINK_LL_DUMP_BIT)  | ALINK_LL_INFO
#define ALINK_LL_DEBUG (1 << ALINK_LL_DEBUG_BIT) | ALINK_LL_INFO
#define ALINK_LL_TRACE (1 << ALINK_LL_TRACE_BIT) | ALINK_LL_DEBUG

typedef enum {
    MEMUSED = 0,
    DISCONNEDCOUNTER,
    DISAPCOUNTER,
    MALFORMEDPACKETS,
    LATESTRTT,
    AVERAGERTT,
    WIFISTRENGTH,
    MAX_INFO
} info_type;

#define ALINK_CONFIG_LEN 2048

/******************************************/
typedef struct alink_down_cmd {
    int id;
    int time;
    char *account;
    char *param;
    int method;
    char uuid[STR_UUID_LEN];
} alink_down_cmd, *alink_down_cmd_ptr;

typedef struct alink_up_cmd {
    /*If the command is not a response of any down-cmd, resp_id MUST be set to -1 */
    int resp_id;
    int emergency;
    const char *target;
    const char *param;

} alink_up_cmd, *alink_up_cmd_ptr;

/******************************************/
typedef int (*alink_callback) (alink_down_cmd_ptr);

/******************************************/
typedef struct device_info {
    /* optional */
    char sn[STR_SN_LEN];
    char name[STR_NAME_LEN];
    char brand[STR_NAME_LEN];
    char type[STR_NAME_LEN];
    char category[STR_NAME_LEN];
    char manufacturer[STR_NAME_LEN];
    char version[STR_NAME_LEN];

    /* mandatory */
    /* must follow the format xx:xx:xx:xx:xx:xx */
    char mac[STR_MAC_LEN];

    /* manufacturer_category_type_name */
    char model[STR_MODEL_LEN];

    /* mandatory for gateway, optional for child dev */
    char cid[STR_CID_LEN];
    char key[STR_KEY_LEN];
    char secret[STR_SEC_LEN];

    alink_callback dev_callback[MAX_CALLBACK_NUM];
    alink_func sys_callback[MAX_SYS_CALLBACK];
    char ip[20];
    char key_sandbox[STR_KEY_LEN];
    char secret_sandbox[STR_SEC_LEN];
} alink_product_t;

struct thread_stacksize {
    unsigned int alink_main_thread_size;
    unsigned int wsf_thread_size;
    unsigned int send_work_thread_size;
	unsigned int func_thread_size;
};
extern struct thread_stacksize g_thread_stacksize;
/******************************************/
/******************************************/

/***
    default:
    unsigned int alink_main_thread_size 0xc00
    unsigned int wsf_thread_size        0x1000
    unsigned int send_work_thread_size  0x800
	unsigned int func_thread_size       0x800
*/
void set_thread_stack_size(struct thread_stacksize *p_thread_stacksize);

/***
 * @desc:    Configure sdk features, such as default thread number of thread pool
 * @para:    config: configure that need to be set
 * @retc:    None/Always Success.
 */
    //void alink_set_config(struct alink_config *config);

/***
 * @desc:    Start ALINK-SDK. Note alink_config should be called first if there's any special settings.
 * @para:    dev: Device info. If ALINK-SDK is running on gateway, the 'dev' MUST implies the gateway device.
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_start(struct device_info *dev);

/***
 * @desc:    End ALINK-SDK. Note this function returns when all thread has already been shutdown.
 * @para:    None
 * @retc:    Always return ALINK_OK in normal mode. Returns ALINK_ERR if DEBUG_MEM is set and MEMORY_CHECK is failed.
 */
int alink_end();

/***
 * @desc:    Attach a new sub-device to ALINK SDK and server. This API is supposed to return immediatly.
 * @para:    dev: Device info of sub-device.
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_attach_sub_device(struct device_info *dev);

/***
 * @desc:    Detach an existed sub-device to ALINK SDK and server. This API is supposed to return immediatly.
 * @para:    dev: Device info of sub-device.
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_detach_sub_device(struct device_info *dev);

/***
 * @desc:    Remove all device information (including sub-device) from both server and local.
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_factory_reset(void);

/***
 * @desc:    Set special callbacks to ALINK-SDK.
 * @para:    method:
 *               func:
 * @retc:    None/Always Success.
 */
int alink_set_callback(int method, alink_func func);

/***
 * @desc:    Get alink time. The SDK sync with server every 30 minutes
 *           or anytime if SDK is notified by the server if the timestamp is out order.
 * @para:    None
 * @retc:    utc time, the number of seconds since Epoth.
 */
unsigned int alink_get_time(void);

/***
 * @desc:    Post device data. Refer to ALINK protocol.
 * @para:    cmd: Command for the API. Memory referred by cmd is ready to release right after the function returns.
 * @retc:    Refer to ALINK protocol API returns
 */
int alink_post_device_data(alink_up_cmd_ptr cmd);

/***
 * @desc:    Post device data array. Refer to ALINK protocol.
 * @para:    cmd: Command for the API. Memory referred by cmd is ready to release right after the function returns.
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_post_device_data_array(alink_up_cmd_ptr cmd);

/***
 * @desc:    Register remote service. Refer to ALINK protocol.
 * @para:    cmd: Command for the API. Memory referred by cmd is ready to release right after the function returns.
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_register_remote_service(alink_up_cmd_ptr cmd);

/***
 * @desc:    Unregister remote service. Refer to ALINK protocol.
 * @para:    cmd: Command for the API. Memory referred by cmd is ready to release right after the function returns.
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_unregister_remote_service(alink_up_cmd_ptr cmd);

/***
 * @desc:    Post remote service response. Refer to ALINK protocol.
 * @para:    cmd: Command for the API. Memory referred by cmd is ready to release right after the function returns.
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_post_remote_service_rsp(alink_up_cmd_ptr cmd);

/***
 * @desc:    Normally, the ALINK-SDK is responsible to decide when to send the message to server
             if alink_up_cmd_ptr.emergency is set to ALINK_FALSE. However, one could demand ALINK-SDK to send
             all reserved messages (if any) immediatly by calling this method.
 * @para:    None
 * @retc:    ALINK_OK/ALINK_ERR (Only if ALINK is not started)
 */
int alink_flush();

/***
 * @desc:    Get uuid by device mac or sn. Set mac or sn to NULL if one wish to get the main device's uuid (also by main device's mac or sn).
 * @para:    id: Mac address. ("xx:xx:xx:xx:xx:xx") or SN (max len refs STR_SN_LEN)
 * @retc:    NULL if mac or sn does NOT match any device. Blank string ("") if the uuid is not set yet.
 */
const char *alink_get_uuid(const char *id);

/***
 * @desc:    Get device mac by uuid. Set uuid to NULL if one wish to get the main device's mac (also by main device's uuid).
 * @para:    uuid(max len refs STR_UUID_LEN)
 * @retc:    NULL if uuid does NOT match any device. Mac address. ("xx:xx:xx:xx:xx:xx").
 */
const char *get_mac_by_uuid(const char *uuid);

/***
 * @desc:    Wait until alink is connected to server and successfully logged in, or simple timeout.
 *               The function can be interrupted in other thread by calling alink_wakeup_connect_waiting.
 * @para:    id: Device mac or sn of which the function is waiting. Set to NULL implies the main device.
                  timeout: seconds before the function timeout, set to -1 if wait forever.
 * @retc:    ALINK_OK if alink is connected to server and successfully logged in. ALINK_ERR if timeout or being wakeup.
 */
int alink_wait_connect(const char *id, int timeout /*second */ );

/***
 * @desc:    Wake up the device waiting function.
 * @para:    id: Device mac or sn of which the function was waiting. Set to NULL implies the main device.
 * @retc:    None. Return immediately.
 */
void alink_wakeup_connect_waiting(const char *id);

/***
 * @desc:    Set alink loglevel, default is ALINK_LL_ERROR.
 * @retc:    None.
 */
void alink_set_loglevel(int loglevel);

/***
 * @desc:    backup device data to alink server
 * @para:    id: Device mac or sn. Set to NULL implies the main device.
 *           key/val: key pair that will be backup.
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_backup_data(char *id, char *key, char *val);

/***
 * @desc:    retrieve device data from alink server
 * @para:    id: Device mac or sn. Set to NULL implies the main device.
 *           key: name of key to be retrieved
 *           val: buffer to hold the return string
 *           val_len: input/output params.
 *                    input: size of val buffer;
 *                    output: len of data retrieved from server,
 *                    if ouput_size > input_size, copy at most input_size to val buffer
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_retrieve_data(char *id, char *key, char *val, int *val_len);

/***
 * @desc:    retrieve device status from alink server
 * @para:    id: Device mac or sn. Set to NULL implies the main device.
 *           buf: buffer to hold the return string
 *           buf_len: input/output params.
 *                    input: size of buffer;
 *                    output: len of data retrieved from server,
 *                    if ouput_size > input_size, copy at most input_size to buffer
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_sync_device_status(char *id, char *buf, int *buf_len);

/***
 * @desc:    generate password for device to connect alink router
 * @para:    ie: alink router IE
 *           model: product model
 *           secret: product secret
 *           passwd: output: buffer to hold the generated password
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_get_passwd(unsigned char *ie, char *model, char *secret, char passwd[64]);

/***
 * @desc:    enable/disable sandbox mode
 * @para:    bEnabled: sandbox switch
 * @retc:    ALINK_OK/ALINK_ERR
 */
int alink_enable_sandbox_mode();

/***
 * @desc:    set networking status
 * @para:    state: The current network connection status
 * @retc:    None. Return immediately.
 */
void alink_update_network_status(int state);

/***
 * @desc:	 unbind device 
 * @para:	 void
 * @retc:	 ALINK_OK/ALINK_ERR
 */
/***
 * @desc:	 read config from flash, write config to flash 
 * @para:	 call back func
 * @retc:	 ALINK_OK/ALINK_ERR
 */
int alink_register_cb(int method , void  *func);

#ifdef __cplusplus
}
#endif
/******************************************/
#endif

