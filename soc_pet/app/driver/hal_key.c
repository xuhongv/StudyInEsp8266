/**
************************************************************
* @file         hal_key.c
* @brief        Push the key
* 
* Key module with timer + GPIO state read mechanism, GPIO configuration according to the relevant manual to configure the ESP8266
*
* This driver supports 0 ~ 12 GPIO key expansion, support cross-platform migration.
* @author       Gizwits
* @date         2017-07-19
* @version      V03030000
* @copyright    Gizwits
*
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值ֵ|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/
#include "driver/hal_key.h"
#include "mem.h"

uint32 keyCountTime = 0; 
static uint8_t keyTotolNum = 0;

/**
* @brief Read the GPIO state
* @param [in] keys Key Function Global structure pointer
* @return uint16_t type GPIO status value
*/
static ICACHE_FLASH_ATTR uint16_t keyValueRead(keys_typedef_t * keys)
{
    uint8_t i = 0; 
    uint16_t read_key = 0;

    //GPIO Cyclic scan
    for(i = 0; i < keys->keyTotolNum; i++)
    {
        if(!GPIO_INPUT_GET(keys->singleKey[i]->gpio_id))
        {
            G_SET_BIT(read_key, keys->singleKey[i]->gpio_number);
        }
    }
    
    return read_key;
}

/**
* @brief Read the KEY value
* @param [in] keys Key Function Global structure pointer
* @return uint16_t type key state value
*/
static uint16_t ICACHE_FLASH_ATTR keyStateRead(keys_typedef_t * keys)
{
    static uint8_t Key_Check = 0;
    static uint8_t Key_State = 0;
    static uint16_t Key_LongCheck = 0;
    uint16_t Key_press = 0; 
    uint16_t Key_return = 0;
    static uint16_t Key_Prev = 0;
    
    //Accumulate key time
    keyCountTime++;
        
    //Press to shake 30MS
    if(keyCountTime >= (DEBOUNCE_TIME / keys->key_timer_ms)) 
    {
        keyCountTime = 0; 
        Key_Check = 1;
    }
    
    if(Key_Check == 1)
    {
        Key_Check = 0;
        
        //Gets the current key trigger value
        Key_press = keyValueRead(keys); 
        
        switch (Key_State)
        {
            //"First capture key" state
            case 0:
                if(Key_press != 0)
                {
                    Key_Prev = Key_press;
                    Key_State = 1;
                }
    
                break;
                
                //"Capture valid key" status
            case 1:
                if(Key_press == Key_Prev)
                {
                    Key_State = 2;
                    Key_return= Key_Prev | KEY_DOWN;
                }
                else
                {
                    //Button lift, jitter, no response button
                    Key_State = 0;
                }
                break;
                
                //"Capture long press" status
            case 2:
    
                if(Key_press != Key_Prev)
                {
                    Key_State = 0;
                    Key_LongCheck = 0;
                    Key_return = Key_Prev | KEY_UP;
                    return Key_return;
                }
    
                if(Key_press == Key_Prev)
                {
                    Key_LongCheck++;
                    if(Key_LongCheck >= (PRESS_LONG_TIME / DEBOUNCE_TIME))    //长按3S (消抖30MS * 100)
                    {
                        Key_LongCheck = 0;
                        Key_State = 3;
                        Key_return= Key_press |  KEY_LONG;
                        return Key_return;
                    }
                }
                break;
                
                //"Restore the initial" state
            case 3:
                if(Key_press != Key_Prev)
                {
                    Key_State = 0;
                }
                break;
        }
    }

    return  NO_KEY;
}

/**
* @brief button callback function

* Call the corresponding callback function after completing the key state monitoring in the function
* @param [in] keys Key Function Global structure pointer
* @return none
*/
void ICACHE_FLASH_ATTR gokitKeyHandle(keys_typedef_t * keys)
{
    uint8_t i = 0;
    uint16_t key_value = 0;

    key_value = keyStateRead(keys); 
    
    if(!key_value) return;
    
    //Check short press button
    if(key_value & KEY_UP)
    {
        //Valid key is detected
        for(i = 0; i < keys->keyTotolNum; i++)
        {
            if(G_IS_BIT_SET(key_value, keys->singleKey[i]->gpio_number)) 
            {
                //key callback function of short press
                if(keys->singleKey[i]->short_press) 
                {
                    keys->singleKey[i]->short_press(); 
                    
                    os_printf("[zs] callback key: [%d][%d] \r\n", keys->singleKey[i]->gpio_id, keys->singleKey[i]->gpio_number); 
                }
            }
        }
    }

    //Check short long button
    if(key_value & KEY_LONG)
    {
        //Valid key is detected
        for(i = 0; i < keys->keyTotolNum; i++)
        {
            if(G_IS_BIT_SET(key_value, keys->singleKey[i]->gpio_number))
            {
                //key callback function of long press
                if(keys->singleKey[i]->long_press) 
                {
                    keys->singleKey[i]->long_press(); 
                    
                    os_printf("[zs] callback long key: [%d][%d] \r\n", keys->singleKey[i]->gpio_id, keys->singleKey[i]->gpio_number); 
                }
            }
        }
    }
}

/**
* @brief single button initialization

* In this function to complete a single key initialization, here need to combine the ESP8266 GPIO register description document to set the parameters
* @param [in] gpio_id ESP8266 GPIO number
* @param [in] gpio_name ESP8266 GPIO name
* @param [in] gpio_func ESP8266 GPIO function
* @param [in] long_press Long press the callback function address
* @param [in] short_press Short press state callback function address
* @return single-button structure pointer
*/
key_typedef_t * ICACHE_FLASH_ATTR keyInitOne(uint8 gpio_id, uint32 gpio_name, uint8 gpio_func, gokit_key_function long_press, gokit_key_function short_press)
{
    static int8_t key_total = -1;

    key_typedef_t * singleKey = (key_typedef_t *)os_zalloc(sizeof(key_typedef_t));

    singleKey->gpio_number = ++key_total;
    
    //Platform-defined GPIO
    singleKey->gpio_id = gpio_id;
    singleKey->gpio_name = gpio_name;
    singleKey->gpio_func = gpio_func;
    
    //Button trigger callback type
    singleKey->long_press = long_press;
    singleKey->short_press = short_press;
    
    keyTotolNum++;    

    return singleKey;
}

/**
* @brief button driver initialization

* In the function to complete all the keys GPIO initialization, and open a timer to start the key state monitoring
* @param [in] keys Key Function Global structure pointer
* @return none
*/
void ICACHE_FLASH_ATTR keyParaInit(keys_typedef_t * keys)
{
    uint8 tem_i = 0; 
    
    if(NULL == keys)
    {
        return ;
    }
    
    //init key timer 
    keys->key_timer_ms = KEY_TIMER_MS; 
    os_timer_disarm(&keys->key_timer); 
    os_timer_setfn(&keys->key_timer, (os_timer_func_t *)gokitKeyHandle, keys); 
    
    keys->keyTotolNum = keyTotolNum;

    //Limit on the number keys (Allowable number: 0~12)
    if(KEY_MAX_NUMBER < keys->keyTotolNum) 
    {
        keys->keyTotolNum = KEY_MAX_NUMBER; 
    }
    
    //GPIO configured as a high level input mode
    for(tem_i = 0; tem_i < keys->keyTotolNum; tem_i++) 
    {
        PIN_FUNC_SELECT(keys->singleKey[tem_i]->gpio_name, keys->singleKey[tem_i]->gpio_func); 
        GPIO_OUTPUT_SET(GPIO_ID_PIN(keys->singleKey[tem_i]->gpio_id), 1); 
        PIN_PULLUP_EN(keys->singleKey[tem_i]->gpio_name); 
        GPIO_DIS_OUTPUT(GPIO_ID_PIN(keys->singleKey[tem_i]->gpio_id)); 
        
        os_printf("gpio_name %d \r\n", keys->singleKey[tem_i]->gpio_id); 
    }
    
    //key timer start
    os_timer_arm(&keys->key_timer, keys->key_timer_ms, 1); 
}

/**
* @brief button to drive the test

* This function simulates the external initialization of the key module

* Note: The user needs to define the corresponding key callback function (such as key1LongPress ...)
* @param none
* @return none
*/
void ICACHE_FLASH_ATTR keyTest(void)
{
#ifdef KEY_TEST
    //Press the GPIO parameter macro to define
    #define GPIO_KEY_NUM                            2                           ///< Defines the total number of key members
    #define KEY_0_IO_MUX                            PERIPHS_IO_MUX_GPIO0_U      ///< ESP8266 GPIO function
    #define KEY_0_IO_NUM                            0                           ///< ESP8266 GPIO number
    #define KEY_0_IO_FUNC                           FUNC_GPIO0                  ///< ESP8266 GPIO name
    #define KEY_1_IO_MUX                            PERIPHS_IO_MUX_MTMS_U       ///< ESP8266 GPIO function
    #define KEY_1_IO_NUM                            14                          ///< ESP8266 GPIO number
    #define KEY_1_IO_FUNC                           FUNC_GPIO14                 ///< ESP8266 GPIO name
    LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];                              ///< Defines a single key member array pointer
    LOCAL keys_typedef_t keys;                                                  ///< Defines the overall key module structure pointer    
    
    //Each time you initialize a key call once keyInitOne, singleKey order plus one
    singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
                                key1LongPress, key1ShortPress);
                                
    singleKey[1] = keyInitOne(KEY_1_IO_NUM, KEY_1_IO_MUX, KEY_1_IO_FUNC,
                                key2LongPress, key2ShortPress);
                                
    keys.key_timer_ms = KEY_TIMER_MS; //Set the key timer cycle recommended by 10ms
    keys.singleKey = singleKey; //Complete the key member assignment
    keyParaInit(&keys); //Key driver initialization
#endif
}