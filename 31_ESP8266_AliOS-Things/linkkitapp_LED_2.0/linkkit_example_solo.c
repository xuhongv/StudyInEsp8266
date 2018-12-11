/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <hal/soc/soc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iot_export.h"
#include "linkkit_export.h"
#include "app_entry.h"
#if defined(OTA_ENABLED)
#include "ota_service.h"
#endif
/*
 * please modify this string follow as product's TSL.
 */
#include "data/solo_tsl.data"

#define EVENT_ERROR_IDENTIFIER "Error"
#define EVENT_ERROR_OUTPUT_INFO_IDENTIFIER "ErrorCode"

// for demo only
#define PRODUCT_KEY "a19UZz1PLW2"
#define PRODUCT_SECRET "f5366JonuJvfDR7T"
#define DEVICE_NAME "smartSocket"
#define DEVICE_SECRET "8sQ88XWdfZGkLMe9Stdv7Fw0AuPrkazx"

#define INFO(format, ...) printf("File: "__FILE__              \
                                 ", Line: %05d: " format "\n", \
                                 __LINE__, ##__VA_ARGS__)

gpio_dev_t trigger;
#define GPIO_TRIGGER_IO 14

typedef struct _sample_context
{
    const void *thing;
    int cloud_connected;
    int local_connected;
    int thing_enabled;
} sample_context_t;

/*
 * the callback of linkkit_post_property.
 * response_id is compare with the result of linkkit_post_property.
 *
 */
void post_property_cb(const void *thing_id, int response_id, int code,
                      const char *response_message, void *ctx)
{
    INFO("thing@%p: response arrived:\nid:%d\tcode:%d\tmessage:%s\n",
         thing_id, response_id, code,
         response_message == NULL ? "NULL" : response_message);

    /* do user's post property callback process logical here. */

    /* ............................... */

    /* user's post property callback process logical complete */
}

/* connect handle
 * cloud and local
 */
#ifdef LOCAL_CONN_ENABLE
static int on_connect(void *ctx, int cloud)
#else
static int on_connect(void *ctx)
#endif
{
    sample_context_t *sample_ctx = ctx;
#ifdef LOCAL_CONN_ENABLE
    if (cloud)
    {
        sample_ctx->cloud_connected = 1;
    }
    else
    {
        sample_ctx->local_connected = 1;
    }
    INFO("%s is connected\n", cloud ? "cloud" : "local");
#else
    sample_ctx->cloud_connected = 1;
    INFO("%s is connected\n", "cloud");
#endif
#if defined(OTA_ENABLED)
    ota_service_init(NULL);
#endif
    /* do user's connect process logical here. */

    /* ............................... */

    /* user's connect process logical complete */

    return 0;
}

#ifdef LOCAL_CONN_ENABLE
static int on_disconnect(void *ctx, int cloud)
#else
static int on_disconnect(void *ctx)
#endif
{
    sample_context_t *sample_ctx = ctx;

#ifdef LOCAL_CONN_ENABLE
    if (cloud)
    {
        sample_ctx->cloud_connected = 0;
    }
    else
    {
        sample_ctx->local_connected = 0;
    }
    INFO("%s is disconnect\n", cloud ? "cloud" : "local");
#else
    sample_ctx->cloud_connected = 0;
    INFO("%s is disconnect\n", "cloud");
#endif

    /* do user's disconnect process logical here. */

    /* ............................... */

    /* user's disconnect process logical complete */
    return 0;
}

static int raw_data_arrived(const void *thing_id, const void *data, int len,
                            void *ctx)
{
    char raw_data[128] = {0};

    INFO("raw data arrived,len:%d\n", len);

    /* do user's raw data process logical here. */

    /* ............................... */

    /* user's raw data process logical complete */

    /* send result to cloud
     * please send your data via raw_data
     * example rule: just reply a string to check
     */
    snprintf(raw_data, sizeof(raw_data), "test down raw reply data %lld",
             (long long)HAL_UptimeMs());
    /* answer raw data handle result */
    linkkit_invoke_raw_service(thing_id, 0, raw_data, strlen(raw_data));

    return 0;
}

/* thing create succuss */
static int thing_create(const void *thing_id, void *ctx)
{
    sample_context_t *sample_ctx = ctx;

    INFO("new thing@%p created.\n", thing_id);
    sample_ctx->thing = thing_id;

    /* do user's thing create process logical here. */

    /* ............................... */

    /* user's thing create process logical complete */

    return 0;
}

/* thing enable
 * thing is enabled, than it can be communicated
 */
static int thing_enable(const void *thing_id, void *ctx)
{
    sample_context_t *sample_ctx = ctx;

    sample_ctx->thing_enabled = 1;

    /* do user's thing enable process logical here. */

    /* ............................... */

    /* user's thing enable process logical complete */

    return 0;
}

/* thing disable
 * thing is disable, than it can not be communicated
 */
static int thing_disable(const void *thing_id, void *ctx)
{
    sample_context_t *sample_ctx = ctx;

    sample_ctx->thing_enabled = 0;

    /* do user's thing disable process logical here. */

    /* ............................... */

    /* user's thing disable process logical complete */

    return 0;
}

/*
 * this is the "custom" service handler
 * alink method: thing.service.Custom
 * please follow TSL modify the idendifier
 */
#ifdef RRPC_ENABLED
static int handle_service_custom(sample_context_t *_sample_ctx,
                                 const void *thing,
                                 const char *service_identifier, int request_id,
                                 int rrpc)
#else
static int handle_service_custom(sample_context_t *_sample_ctx,
                                 const void *thing,
                                 const char *service_identifier, int request_id)
#endif /* RRPC_ENABLED */
{
    char identifier[128] = {0};
    /*
     * please follow TSL modify the value type
     */
    int transparency_value;
    int contrastratio_value;

    /*
     * get iutput value.
     * compare the service identifier
     * please follow user's TSL modify the "transparency".
     */
    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier,
             "transparency");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier,
                      &transparency_value, NULL);

    INFO("identifier: %s value is %d.\n", identifier,
         transparency_value);

    /*
     * set output value according to user's process result.
     * example rule: Contrastratio will changed by transparency.
     */

    /* do user's service process logical here. */

    /* ............................... */

    /* user's service process logical complete */

    /*
     * please follow user's TSL modify the "transparency".
     */
    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier,
             "Contrastratio");
    contrastratio_value = transparency_value + 1;
    linkkit_set_value(linkkit_method_set_service_output_value, thing,
                      identifier, &contrastratio_value, NULL);
#ifdef RRPC_ENABLED
    linkkit_answer_service(thing, service_identifier, request_id, 200, rrpc);
#else
    linkkit_answer_service(thing, service_identifier, request_id, 200);
#endif /* RRPC_ENABLED */

    return 0;
}

/*
 * the handler of service which is defined by identifier, not property
 * alink method: thing.service.{tsl.service.identifier}
 */
#ifdef RRPC_ENABLED
static int thing_call_service(const void *thing_id, const char *service,
                              int request_id, int rrpc, void *ctx)
#else
static int thing_call_service(const void *thing_id, const char *service,
                              int request_id, void *ctx)
#endif /* RRPC_ENABLED */
{
    sample_context_t *sample_ctx = ctx;

    INFO("service(%s) requested, id: thing@%p, request id:%d\n",
         service, thing_id, request_id);

    /* please follow TSL modify the idendifier --- Custom */
    if (strcmp(service, "Custom") == 0)
    {
#ifdef RRPC_ENABLED
        handle_service_custom(sample_ctx, thing_id, service, request_id, rrpc);
#else
        handle_service_custom(sample_ctx, thing_id, service, request_id);
#endif /* RRPC_ENABLED */
    }

    return 0;
}

/*
 * the handler of property changed
 * alink method: thing.service.property.set
 */
static int thing_prop_changed(const void *thing_id, const char *property,
                              void *ctx)
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

    response_id = linkkit_post_property(thing_id, property, post_property_cb);

    INFO("post property(%s) response id: %d\n", property, response_id);

    return 0;
}

/* there is some data transparent transmission by linkkit */
static int linkit_data_arrived(const void *thing_id, const void *params,
                               int len, void *ctx)
{
    INFO("thing@%p: masterdev_linkkit_data(%d byte): %s\n\n\n\n\n\n\n\n", thing_id,
         len, (const char *)params);

    /* do user's data arrived process logical here. */

    /* ............................... */

    /* user's data arrived process logical complete */
    return 0;
}

static int is_active(sample_context_t *sample_ctx)
{
#ifdef LOCAL_CONN_ENABLE
    return (sample_ctx->cloud_connected /* && sample_ctx->thing_enabled*/) ||
           (sample_ctx->local_connected /* && sample_ctx->thing_enabled*/);
#else
    return sample_ctx->cloud_connected /* && sample_ctx->thing_enabled*/;
#endif
}

static unsigned long long uptime_sec(void)
{
    static unsigned long long start_time = 0;

    if (start_time == 0)
    {
        start_time = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - start_time) / 1000;
}

int post_all_prop(sample_context_t *sample)
{
    /* demo for post all property */
    return linkkit_post_property(sample->thing, NULL, post_property_cb);
}

int trigger_event(sample_context_t *sample)
{
    char event_output_identifier[64];
    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s",
             EVENT_ERROR_IDENTIFIER, EVENT_ERROR_OUTPUT_INFO_IDENTIFIER);

    int errorCode = 0;
    linkkit_set_value(linkkit_method_set_event_output_value, sample->thing,
                      event_output_identifier, &errorCode, NULL);

    return linkkit_trigger_event(sample->thing, EVENT_ERROR_IDENTIFIER,
                                 post_property_cb);

    /* please modify the event_id by TSL */
    return linkkit_trigger_event(sample->thing, "TemperatureAlarm",
                                 post_property_cb);
}

#ifdef EXTENDED_INFO_ENABLED
int trigger_deviceinfo(sample_context_t *sample)
{
    /* please modify the parameter */
    return linkkit_trigger_extended_info_operate(
        sample->thing, "[{device_info : 21}]",
        linkkit_extended_info_operate_update);
}
#endif

static linkkit_ops_t linkkit_ops = {
    .on_connect = on_connect,                   /* connect handler */
    .on_disconnect = on_disconnect,             /* disconnect handler */
    .raw_data_arrived = raw_data_arrived,       /* receive raw data handler */
    .thing_create = thing_create,               /* thing created handler */
    .thing_enable = thing_enable,               /* thing enabled handler */
    .thing_disable = thing_disable,             /* thing disabled handler */
    .thing_call_service = thing_call_service,   /* self-defined service handler */
    .thing_prop_changed = thing_prop_changed,   /* property set handler */
    .linkit_data_arrived = linkit_data_arrived, /* transparent transmission data handler */
};

int linkkit_example()
{

    sample_context_t sample_ctx = {0};
    // int execution_time = 20;
    int exit = 0;
    unsigned long long now = 0;
    unsigned long long prev_sec = 0;
    int get_tsl_from_cloud = 0; /* the param of whether it is get tsl from cloud */


    /*
     * linkkit start
     * max_buffered_msg = 16, set the handle msg max numbers.
     *     if it is enough memory, this number can be set bigger.
     * if get_tsl_from_cloud = 0, it will use the default tsl [TSL_STRING]; if
     * get_tsl_from_cloud =1, it will get tsl from cloud.
     */
    if (-1 == linkkit_start(16, get_tsl_from_cloud, linkkit_loglevel_debug,
                            &linkkit_ops, linkkit_cloud_domain_shanghai,
                            &sample_ctx))
    {
        INFO("linkkit start fail");
        return -1;
    }
    else
    {
        INFO("linkkit start success");
    }

    if (!get_tsl_from_cloud)
    {

        linkkit_set_tsl(TSL_STRING, strlen(TSL_STRING));
    }

    INFO("linkkit enter loop");

    while (!linkkit_is_try_leave())
    {

#if (CONFIG_SDK_THREAD_COST == 0)
        linkkit_yield(100);
        linkkit_dispatch();
#else
        HAL_SleepMs(100);
#endif /* CONFIG_SDK_THREAD_COST */
        now = uptime_sec();
        if (prev_sec == now)
        {
            continue;
        }

#ifdef POST_WIFI_STATUS
        if (now % 10 == 0)
        {
            post_property_wifi_status_once(&sample_ctx);
        }
#endif
        if (now % 30 == 0 && is_active(&sample_ctx))
        {
            post_all_prop(&sample_ctx);
        }

        if (now % 45 == 0 && is_active(&sample_ctx))
        {
            trigger_event(&sample_ctx);
        }

#ifdef EXTENDED_INFO_ENABLED
        if (now % 50 == 0 && is_active(&sample_ctx))
        {
            trigger_deviceinfo(&sample_ctx);
        }
#endif

        if (exit)
        {
            break;
        }
        prev_sec = now;
    }

    linkkit_end();
    return 0;
}

void set_iotx_info()
{
    HAL_SetProductKey(PRODUCT_KEY);
    HAL_SetProductSecret(PRODUCT_SECRET);
    HAL_SetDeviceName(DEVICE_NAME);
    HAL_SetDeviceSecret(DEVICE_SECRET);
}

void GPIO_Init()
{

    /* gpio port config */
    trigger.port = GPIO_TRIGGER_IO;
    /* set as output mode */
    trigger.config = OUTPUT_PUSH_PULL;
    /* configure GPIO with the given settings */
    hal_gpio_init(&trigger);
}

int linkkit_main(void *paras)
{
#ifndef WIFI_AWSS_ENABLED
    set_iotx_info();
#endif

    GPIO_Init();

    INFO("IOT_CloseLog!\n");

    IOT_CloseLog();

    INFO("start!");
    /*
     * linkkit dome
     * please check document: https://help.aliyun.com/document_detail/73708.html
     *         API introduce: https://help.aliyun.com/document_detail/68687.html
     */
    linkkit_example();

    //IOT_DumpMemoryStats(IOT_LOG_DEBUG);

    return 0;
}
