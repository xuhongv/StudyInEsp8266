#include "xLight_pwm.h"

struct light_saved_param light_param;

// pwm pin number
const uint32_t pinNum[PWM_CHANNEL] = {
    PWM_0_OUT_IO_NUM,
    PWM_1_OUT_IO_NUM,
    PWM_2_OUT_IO_NUM,
    PWM_3_OUT_IO_NUM,
    PWM_4_OUT_IO_NUM};
// don't alter it !!! dutys table, (duty/PERIOD)*depth , init
uint32_t setDuties[PWM_CHANNEL] = {0, 0, 0, 0, 0};

//此处设置相位角，可以做互补driver
int16_t phase[PWM_CHANNEL] = {
    0,
    0,
    0,
    0,
    0,
};

/******************************************************************************
 * FunctionName : user_light_get_duty
 * Description  : get duty of each channel
 * Parameters   : uint8 channel : LIGHT_RED/LIGHT_GREEN/LIGHT_BLUE
 * Returns      : NONE
*******************************************************************************/
uint32_t user_light_get_duty(uint8_t channel)
{

    uint32_t duty_p = 0;
    if (pwm_get_duty(channel, &duty_p) != ESP_OK)
    {
        printf("Error in getting period...\n\n");
    }

    return duty_p;
}

/******************************************************************************
 * FunctionName : user_light_set_duty
 * Description  : set each channel's duty params
 * Parameters   : uint8 duty    : 0 ~ PWM_DEPTH
 *                uint8 channel : LIGHT_RED/LIGHT_GREEN/LIGHT_BLUE
 * Returns      : NONE
*******************************************************************************/
void user_light_set_duty(uint32_t duty, uint8_t channel)
{
    // if (duty != light_param.pwm_duty[channel])
    // {
    //     //printf("channel :%d , duty : %d \n", channel, duty);
    //     pwm_set_duty(channel, duty);
    //     light_param.pwm_duty[channel] = duty;
    //     vTaskDelay(7 / portTICK_RATE_MS);
    // }


     if (duty != light_param.pwm_duty[channel])
    {
        //printf("channel :%d , duty : %d \n", channel, duty);
        pwm_set_duty(channel, duty);
        light_param.pwm_duty[channel] = duty;
        if (duty < 20)
        {
            vTaskDelay(15 / portTICK_RATE_MS);
        }
        else
        {
            vTaskDelay(20 / portTICK_RATE_MS);
        }
    }


}

/******************************************************************************
 * FunctionName : user_light_get_period
 * Description  : get pwm period
 * Parameters   : NONE
 * Returns      : uint32_t : pwm period
*******************************************************************************/
uint32_t user_light_get_period(void)
{
    return light_param.pwm_period;
}

/******************************************************************************
 * FunctionName : 设置周期
 * Description  : set pwm frequency
 * Parameters   : uint16 freq 
 * Returns      : NONE
*******************************************************************************/
void user_light_set_period(uint32_t period)
{
    if (period != light_param.pwm_period)
    {
        pwm_set_period(period);
        if (pwm_get_period(&light_param.pwm_period) != ESP_OK)
        {
            printf("Error in getting period...\n\n");
        }
    }
}
/**
 * @description: 重启
 * @param {type} 
 * @return: 
 */
void user_light_restart(void)
{
}

/******************************************************************************
 * FunctionName : user_light_init
 * Description  : light demo init, mainy init pwm
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_light_init(void)
{
    pwm_init(PWM_PERIOD, setDuties, PWM_CHANNEL, pinNum);
    pwm_set_phases(phase);
    uint8_t i = 0;
    for (i = 0; i < PWM_CHANNEL; i++)
    {
        light_param.pwm_duty[i] = 0;
    }
}
