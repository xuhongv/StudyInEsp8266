/** USER NOTIFICATION
 *  this sample code is only used for evaluation or test of the iLop project.
 *  Users should modify this sample code freely according to the product/device TSL, like
 *  property/event/service identifiers, and the item value type(type, length, etc...).
 *  Create user's own execution logic for specific products.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include <aos/aos.h>
#include <aos/aos.h>
#include <hal/soc/soc.h>
#include "linkkit_export.h"

#include "iot_import.h"

#include "linkkit_app.h"


/*
 * example for product "灯-Demo"
 */


#define INFO(format, ...) printf("File: "__FILE__              \
                                 ", Line: %05d: " format "\n", \
                                 __LINE__, ##__VA_ARGS__)

/* identifier of property/service/event, users should modify this macros according to your own product TSL. */
#define EVENT_ERROR_IDENTIFIER                 "Error"
#define EVENT_ERROR_OUTPUT_INFO_IDENTIFIER     "ErrorCode"
#define EVENT_CUSTOM_IDENTIFIER                "Custom"

/* specify ota buffer size for ota service, ota service will use this buffer for bin download. */
static void ota_init();

/* PLEASE set RIGHT tsl string according to your product. */
static const char TSL_STRING[] = "{\"schema\":\"https://iotx-tsl.oss-ap-southeast-1.aliyuncs.com/schema.json\",\"profile\":{\"productKey\":\"a19UZz1PLW2\"},\"services\":[{\"outputData\":[],\"identifier\":\"set\",\"inputData\":[{\"identifier\":\"PowerSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"电源开关\"},{\"identifier\":\"CountDownList\",\"dataType\":{\"specs\":[{\"identifier\":\"Target\",\"dataType\":{\"specs\":{\"length\":\"2048\"},\"type\":\"text\"},\"name\":\"操作对象\"},{\"identifier\":\"Contents\",\"dataType\":{\"specs\":{\"length\":\"2048\"},\"type\":\"text\"},\"name\":\"倒计时命令\"}],\"type\":\"struct\"},\"name\":\"倒计时列表\"},{\"identifier\":\"WIFI_Band\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"频段\"},{\"identifier\":\"WiFI_RSSI\",\"dataType\":{\"specs\":{\"min\":\"-127\",\"unitName\":\"无\",\"max\":\"-1\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信号强度\"},{\"identifier\":\"WIFI_AP_BSSID\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"热点BSSID\"},{\"identifier\":\"WIFI_Channel\",\"dataType\":{\"specs\":{\"min\":\"1\",\"unitName\":\"无\",\"max\":\"255\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信道\"},{\"identifier\":\"WiFI_SNR\",\"dataType\":{\"specs\":{\"min\":\"-127\",\"unitName\":\"无\",\"max\":\"127\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信噪比\"}],\"method\":\"thing.service.property.set\",\"name\":\"set\",\"required\":true,\"callType\":\"async\",\"desc\":\"属性设置\"},{\"outputData\":[{\"identifier\":\"PowerSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"电源开关\"},{\"identifier\":\"CountDownList\",\"dataType\":{\"specs\":[{\"identifier\":\"Target\",\"dataType\":{\"specs\":{\"length\":\"2048\"},\"type\":\"text\"},\"name\":\"操作对象\"},{\"identifier\":\"Contents\",\"dataType\":{\"specs\":{\"length\":\"2048\"},\"type\":\"text\"},\"name\":\"倒计时命令\"}],\"type\":\"struct\"},\"name\":\"倒计时列表\"},{\"identifier\":\"WIFI_Band\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"频段\"},{\"identifier\":\"WiFI_RSSI\",\"dataType\":{\"specs\":{\"min\":\"-127\",\"unitName\":\"无\",\"max\":\"-1\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信号强度\"},{\"identifier\":\"WIFI_AP_BSSID\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"热点BSSID\"},{\"identifier\":\"WIFI_Channel\",\"dataType\":{\"specs\":{\"min\":\"1\",\"unitName\":\"无\",\"max\":\"255\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信道\"},{\"identifier\":\"WiFI_SNR\",\"dataType\":{\"specs\":{\"min\":\"-127\",\"unitName\":\"无\",\"max\":\"127\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信噪比\"}],\"identifier\":\"get\",\"inputData\":[\"PowerSwitch\",\"CountDownList\",\"WIFI_Band\",\"WiFI_RSSI\",\"WIFI_AP_BSSID\",\"WIFI_Channel\",\"WiFI_SNR\"],\"method\":\"thing.service.property.get\",\"name\":\"get\",\"required\":true,\"callType\":\"async\",\"desc\":\"属性获取\"}],\"properties\":[{\"identifier\":\"PowerSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"电源开关\",\"accessMode\":\"rw\",\"required\":true},{\"identifier\":\"CountDownList\",\"dataType\":{\"specs\":[{\"identifier\":\"Target\",\"dataType\":{\"specs\":{\"length\":\"2048\"},\"type\":\"text\"},\"name\":\"操作对象\"},{\"identifier\":\"Contents\",\"dataType\":{\"specs\":{\"length\":\"2048\"},\"type\":\"text\"},\"name\":\"倒计时命令\"}],\"type\":\"struct\"},\"name\":\"倒计时列表\",\"accessMode\":\"rw\",\"required\":true},{\"identifier\":\"WIFI_Band\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"频段\",\"accessMode\":\"rw\",\"required\":true},{\"identifier\":\"WiFI_RSSI\",\"dataType\":{\"specs\":{\"min\":\"-127\",\"unitName\":\"无\",\"max\":\"-1\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信号强度\",\"accessMode\":\"rw\",\"required\":true},{\"identifier\":\"WIFI_AP_BSSID\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"热点BSSID\",\"accessMode\":\"rw\",\"required\":true},{\"identifier\":\"WIFI_Channel\",\"dataType\":{\"specs\":{\"min\":\"1\",\"unitName\":\"无\",\"max\":\"255\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信道\",\"accessMode\":\"rw\",\"required\":true},{\"identifier\":\"WiFI_SNR\",\"dataType\":{\"specs\":{\"min\":\"-127\",\"unitName\":\"无\",\"max\":\"127\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信噪比\",\"accessMode\":\"rw\",\"required\":true}],\"events\":[{\"outputData\":[{\"identifier\":\"PowerSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"电源开关\"},{\"identifier\":\"CountDownList\",\"dataType\":{\"specs\":[{\"identifier\":\"Target\",\"dataType\":{\"specs\":{\"length\":\"2048\"},\"type\":\"text\"},\"name\":\"操作对象\"},{\"identifier\":\"Contents\",\"dataType\":{\"specs\":{\"length\":\"2048\"},\"type\":\"text\"},\"name\":\"倒计时命令\"}],\"type\":\"struct\"},\"name\":\"倒计时列表\"},{\"identifier\":\"WIFI_Band\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"频段\"},{\"identifier\":\"WiFI_RSSI\",\"dataType\":{\"specs\":{\"min\":\"-127\",\"unitName\":\"无\",\"max\":\"-1\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信号强度\"},{\"identifier\":\"WIFI_AP_BSSID\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"热点BSSID\"},{\"identifier\":\"WIFI_Channel\",\"dataType\":{\"specs\":{\"min\":\"1\",\"unitName\":\"无\",\"max\":\"255\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信道\"},{\"identifier\":\"WiFI_SNR\",\"dataType\":{\"specs\":{\"min\":\"-127\",\"unitName\":\"无\",\"max\":\"127\",\"step\":\"1\"},\"type\":\"int\"},\"name\":\"信噪比\"}],\"identifier\":\"post\",\"method\":\"thing.event.property.post\",\"name\":\"post\",\"type\":\"info\",\"required\":true,\"desc\":\"属性上报\"},{\"outputData\":[{\"identifier\":\"ErrorCode\",\"dataType\":{\"specs\":{\"0\":\"恢复正常\"},\"type\":\"enum\"},\"name\":\"故障代码\"}],\"identifier\":\"Error\",\"method\":\"thing.event.Error.post\",\"name\":\"故障上报\",\"type\":\"info\",\"required\":true}]}";
typedef struct _sample_context {
    const void* thing;

    int cloud_connected;
    int thing_enabled;

    int service_custom_input_transparency;
    int service_custom_output_contrastratio;
#ifdef SERVICE_OTA_ENABLED
    char ota_buffer[OTA_BUFFER_SIZE];
#endif /* SERVICE_OTA_ENABLED */
} sample_context_t;




sample_context_t g_sample_context;

#ifdef LOCAL_CONN_ENABLE
static int on_connect(void* ctx, int cloud)
#else
static int on_connect(void* ctx)
#endif
{
    sample_context_t* sample = ctx;

    sample->cloud_connected = 1;

    ota_init();

#ifdef LOCAL_CONN_ENABLE
    INFO("%s is connected\n", cloud ? "cloud" : "local");
#else
    INFO("%s is connected\n", "cloud");
#endif

    aos_post_event(EV_YUNIO, CODE_YUNIO_ON_CONNECTED, 0);

    return 0;
}

#ifdef LOCAL_CONN_ENABLE
static int on_disconnect(void* ctx, int cloud)
#else
static int on_disconnect(void* ctx)
#endif
{
    sample_context_t* sample = ctx;

    sample->cloud_connected = 0;

#ifdef LOCAL_CONN_ENABLE
    INFO("%s is disconnect\n", cloud ? "cloud" : "local");
#else
    INFO("%s is disconnect\n", "cloud");
#endif

    aos_post_event(EV_YUNIO, CODE_YUNIO_ON_DISCONNECTED, 0);

    return 0;
}

static int raw_data_arrived(const void* thing_id, const void* data, int len, void* ctx)
{
    char raw_data[128] = {0};

    INFO("raw data arrived,len:%d\n", len);

    /* do user's raw data process logical here. */

    /* ............................... */

    /* user's raw data process logical complete */

    snprintf(raw_data, sizeof(raw_data), "test down raw reply data %lld", (long long)HAL_UptimeMs());

    linkkit_invoke_raw_service(thing_id, 0, raw_data, strlen(raw_data));

    return 0;
}

static int thing_create(const void* thing_id, void* ctx)
{
    sample_context_t* sample = ctx;

    INFO("new thing@%p created.\n", thing_id);

    sample->thing = thing_id;
    return 0;
}

static int thing_enable(const void* thing_id, void* ctx)
{
    sample_context_t* sample = ctx;

    sample->thing_enabled = 1;

    return 0;
}

static int thing_disable(const void* thing, void* ctx)
{
    sample_context_t* sample = ctx;

    sample->thing_enabled = 0;

    return 0;
}
#ifdef RRPC_ENABLED
static int handle_service_custom(sample_context_t* sample, const void* thing, const char* service_identifier, int request_id, int rrpc)
#else
static int handle_service_custom(sample_context_t* sample, const void* thing, const char* service_identifier, int request_id)
#endif /* RRPC_ENABLED */
{
    char identifier[128] = {0};

    /*
     * get iutput value.
     */
    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "transparency");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &sample->service_custom_input_transparency, NULL);

    /*
     * set output value according to user's process result.
     */

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "Contrastratio");

    sample->service_custom_output_contrastratio = sample->service_custom_input_transparency >= 0 ? sample->service_custom_input_transparency : sample->service_custom_input_transparency * -1;

    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, &sample->service_custom_output_contrastratio, NULL);
#ifdef RRPC_ENABLED
    linkkit_answer_service(thing, service_identifier, request_id, 200, rrpc);
#else
    linkkit_answer_service(thing, service_identifier, request_id, 200);
#endif /* RRPC_ENABLED */

    return 0;
}
#ifdef RRPC_ENABLED
static int thing_call_service(const void* thing_id, const char* service, int request_id, int rrpc, void* ctx)
#else
static int thing_call_service(const void* thing_id, const char* service, int request_id, void* ctx)
#endif /* RRPC_ENABLED */
{
    sample_context_t* sample = ctx;

    INFO("service(%s) requested, id: thing@%p, request id:%d\n",
                   service, thing_id, request_id);

    if (strcmp(service, "Custom") == 0) {
#ifdef RRPC_ENABLED
        handle_service_custom(sample, thing_id, service, request_id, rrpc);
#else
        handle_service_custom(sample, thing_id, service, request_id);
#endif /* RRPC_ENABLED */
    }

    return 0;
}

gpio_dev_t trigger;
#define GPIO_TRIGGER_IO 14

void GPIO_Init()
{

    /* gpio port config */
    trigger.port = GPIO_TRIGGER_IO;
    /* set as output mode */
    trigger.config = OUTPUT_PUSH_PULL;
    /* configure GPIO with the given settings */
    hal_gpio_init(&trigger);
}


static int thing_prop_changed(const void* thing_id, const char* property, void* ctx)
{
   
 
    char *value_str = NULL;
    char property_buf[64] = {0};
    int response_id = -1;

    INFO("post property(%s) response id: %d \n\n\n\n\n\n\n\n\n\n", property, response_id);

    if (strstr(property, "PowerSwitch") != 0)
    {

        int PowerSwitch;

        /* generate property identifier HSVColor.Hue */
        snprintf(property_buf, sizeof(property_buf), "%s", property, "PowerSwitch");
        /* get value by linkkit_get_value */
        linkkit_get_value(linkkit_method_get_property_value, thing_id,
                          property_buf, &PowerSwitch, &value_str);

        if (PowerSwitch == 1)
        {
            hal_gpio_output_low(&trigger);
        }
        else if (PowerSwitch == 0)
        {
            hal_gpio_output_high(&trigger);
        }

        if (value_str)
        {
            free(value_str);
            value_str = NULL;
        }

        INFO("property(%s), PowerSwitch:%d\n", property, PowerSwitch);
    }

    /* do user's process logical here. */
    linkkit_post_property(thing_id, property);

    return 0;
}

int on_response_arrived(const void* thing_id, int respons_id, int code, char* message, void* ctx)
{
    INFO("response arrived:\nid:%d\tcode:%d\tmessage:%s\n", respons_id, code, message == NULL ? "NULL" : message);

    return 0;
}

int masterdev_linkit_data_arrived(const void* thing_id, const void* data, int len, void* ctx)
{
    INFO("#### masterdev_linkkit_data(%d byte): %s####\n", len, (char*)data);
    return 0;
}

static linkkit_ops_t alinkops = {
    .on_connect         = on_connect,
    .on_disconnect      = on_disconnect,
    .raw_data_arrived   = raw_data_arrived,
    .thing_create       = thing_create,
    .thing_enable       = thing_enable,
    .thing_disable      = thing_disable,
    .thing_call_service = thing_call_service,
    .thing_prop_changed = thing_prop_changed,
    .response_arrived   = on_response_arrived,
    .linkit_data_arrived = masterdev_linkit_data_arrived,
};

#ifdef SUBDEV_ENABLE
int subdev_topo_add_success(const void* sub_thing_id, void* ctx)
{
    INFO("#### subthing(@%p) topo add success####\n", sub_thing_id);
    return 0;
}
int subdev_topo_add_fail(const void* sub_thing_id, int code, const char* message, void* ctx)
{
    INFO("#### subthing(@%p) code(%d) message(%s) topo add fail####\n", sub_thing_id, code, message ? message: "NULL");
    return 0;
}

int subdev_login_success(const void* sub_thing_id, void* ctx)
{
    INFO("#### subthing(@%p) login success####\n", sub_thing_id);
    return 0;
}

int subdev_login_fail(const void* sub_thing_id, int code, const char* message, void* ctx)
{
    INFO("#### subthing(@%p) code(%d) message(%s) topo add fail####\n", sub_thing_id, code, message ? message : "NULL");
    return 0;
}

int subdev_raw_data_arrived(const void* sub_thing_id, const void* data, int len, void* ctx)
{

    INFO("subdev(@%p) raw data arrived,len:%d\n", sub_thing_id, len);

    /* do user's raw data process logical here. */

    /* ............................... */

    /* user's raw data process logical complete */
    return 0;
}

int subdev_thing_create(const void* sub_thing_id, void* ctx)
{
    INFO("new thing@%p created.\n", sub_thing_id);
    return 0;
}

int subdev_thing_enable(const void* sub_thing_id, void* ctx)
{
    INFO("thing@%p has been enabled.\n", sub_thing_id);
    return 0;
}

int subdev_thing_disable(const void* sub_thing_id, void* ctx)
{
    INFO("thing@%p has been disabled.\n", sub_thing_id);
    return 0;
}

int subdev_linkit_data_arrived(const void* sub_thing_id, const void* data, int len, void* ctx)
{
    INFO("#### subdev_linkkit_data(%d byte): %s####\n", len, (char*)data);
    return 0;
}

static linkkit_subdev_ops_t subdev_ops = {
    .topo_add_success = subdev_topo_add_success,
    .topo_add_fail = subdev_topo_add_fail,
    .login_success = subdev_login_success,
    .login_fail = subdev_login_fail,
    .raw_data_arrived = subdev_raw_data_arrived,
    .thing_create = subdev_thing_create,
    .thing_enable = subdev_thing_enable,
    .thing_disable = subdev_thing_disable,
    .linkit_data_arrived = subdev_linkit_data_arrived,
};
#endif

static unsigned long long uptime_sec(void)
{
    static unsigned long long start_time = 0;

    if (start_time == 0) {
        start_time = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - start_time) / 1000;
}

static int post_event_error(sample_context_t* sample)
{
    char event_output_identifier[64];
    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s", EVENT_ERROR_IDENTIFIER, EVENT_ERROR_OUTPUT_INFO_IDENTIFIER);

    int errorCode = 0;
    linkkit_set_value(linkkit_method_set_event_output_value,
                      sample->thing,
                      event_output_identifier,
                      &errorCode, NULL);

    return linkkit_trigger_event(sample->thing, EVENT_ERROR_IDENTIFIER);
}

static int is_active(sample_context_t* sample)
{
    return sample->cloud_connected && sample->thing_enabled;
}

void linkkit_action(void *params)
{
    static unsigned long long now = 0;

    static int now_size = 0;
    static int last_size = 0;

    sample_context_t* sample_ctx = params;

    linkkit_dispatch();

    now += 1;

#if 1
	/* about 60 seconds, assume trigger event about every 60s. */
    if (now % 600 == 0 && is_active(sample_ctx)) {
        int id_send = 0;
        int ret;
        ret = post_event_error(sample_ctx);
#if 0
        INFO("####%s, ret=%d####\n", "post_event_error", ret);
		ret = linkkit_post_property(sample_ctx->thing, NULL);
        INFO("####%s, ret=%d####\n", "linkkit_post_property", ret);
#endif
        if (ret > 0) {
            id_send = ret;
            INFO("send id:%d\n", id_send);
        }
    }
#endif

#if 0
    now_size = system_get_free_heap_size();
    if (now_size != last_size) {
        last_size = now_size;
        if ((now_size - last_size) > 256 ||  (last_size - now_size) > 256) {
            INFO("[heap check task] free heap size:%d\n", now_size);
        }
    } else if (now % 600 == 0) {
        INFO("[heap check task] free heap size:%d Bytes(now time:%d)\n", now_size, now);
	}
#endif

    aos_post_delayed_action(100, linkkit_action, sample_ctx);
}


int linkkit_main()
{

    GPIO_Init();
    sample_context_t* sample_ctx = &g_sample_context;
    int execution_time = 0;
    int get_tsl_from_cloud = 0;

    execution_time = execution_time < 1 ? 1 : execution_time;
    INFO("sample execution time: %d minutes\n", execution_time);
    INFO("%s tsl from cloud\n", get_tsl_from_cloud == 0 ? "Not get" : "get");

    memset(sample_ctx, 0, sizeof(sample_context_t));
    sample_ctx->thing_enabled = 1;

    linkkit_start(16, get_tsl_from_cloud, linkkit_loglevel_debug, &alinkops, linkkit_cloud_domain_sh, sample_ctx);
    if (!get_tsl_from_cloud) {
        linkkit_set_tsl(TSL_STRING, strlen(TSL_STRING));
    }
#ifdef SUBDEV_ENABLE
    linkkit_subdev_init(&subdev_ops, &g_subdev_sample_context);
#endif
    aos_post_delayed_action(100, linkkit_action, sample_ctx);

    return 0;
}

int linkkit_app(void)
{
    linkkit_main();

    return 0;
}

static void ota_init()
{
    static int init = 0;
    if (init) {
        return;
    }
    init = 1;

    aos_post_event(EV_SYS, CODE_SYS_ON_START_FOTA, 0);
}

