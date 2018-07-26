<p align="center">
  <img src="http://h5xuhong.oss-cn-hongkong.aliyuncs.com/8266%E5%9B%BE%E7%89%87/8266popuuo.png" width="550px" height="360px" alt="Banner" />
</p>
 
 
***************************************************
## 《StudyIn8266》 小徐寄言：
 
《StudyIn8266》 的设计目的是用于帮助新手学习8266原生SDK的各种项目汇总，同时也有集成的项目，让开发者能在这基础代码的设计。不管是新项目的创建，或是已有项目的维护，均可使开发效率和项目质量得到大幅度提升。


CSDN博客博文：[http://blog.csdn.net/xh870189248/article/details/77985541](http://blog.csdn.net/xh870189248/article/details/77985541)

## 目录说明：

|      **文件夹名字（代表一个工程）**   | 说明|文档说明链接|
|-------|------|-----------|
| 《**2_ButtonLight**》 | 一个按键点亮一个LED灯。|http://blog.csdn.net/xh870189248/article/details/78126689|
| 《**3_TimerLED**》| 软件定时器实现0.5s闪烁一盏LED。 |http://blog.csdn.net/xh870189248/article/details/78155357|
| 《**5_PWM_LED**》| PWM控制一盏LED的亮度变化。 |http://blog.csdn.net/xh870189248/article/details/78202224|
| 《**6_PostAndGet**》|post和get请求天气预报信息封装例子。 |http://blog.csdn.net/xh870189248/article/details/78656563|
| 《**7_SmartConfig**》|基于乐鑫的SDK的按键触发一键配网smartConfig模式上云端代码。 |http://blog.csdn.net/xh870189248/article/details/78677952|
| 《**8_softAPToNet**》|基于乐鑫的SDK的按键触发softAP模式上云端代码。 |http://blog.csdn.net/xh870189248/article/details/78703905|
| 《**9_TcpAndUdp**》|8266的TCP、UDP的客户端服务端代码工程。 |http://blog.csdn.net/xh870189248/article/details/78739311|
| 《**10_ESP8266_MQTT_CSDN_1**》|【小实战上篇】Windows系统搭建8266的本地Mqtt服务器，局域网点亮一盏LED灯。|http://blog.csdn.net/xh870189248/article/details/78761948|
| 《**11_ESP8266_CSDN_MQTT_2**》|【小实战下篇】Windows系统搭建8266的本地Mqtt服务器，局域网点亮一盏LED灯。|http://blog.csdn.net/xh870189248/article/details/78777371|
| 《**12_Esp8266-Alink_OneLED**》|接入阿里智能控制LED。|http://blog.csdn.net/xh870189248/article/details/78807018|
| 《**13_ESP8266_smartConfi_MQTT**》|Mqtt协议+smartConfig控制一盏LED（基于2.1.0版本）|http://blog.csdn.net/xh870189248/article/details/79052347|
| 《**14_ESP8266_smartConfig_Mqtt**》|Mqtt协议+smartConfig控制一盏LED（基于2.2.0版本）|http://blog.csdn.net/xh870189248/article/details/79052347|
| 《**14_ESP8266_smartConfig_Mqtt**》|Mqtt协议+smartConfig控制一盏LED（基于2.2.0版本）|http://blog.csdn.net/xh870189248/article/details/79052347|
| 《**15_ESP8266_Timer2**》|2018-1-17 （基于NONOS 2.2.0版本）硬件中断定时器控制闪烁一盏LED|http://blog.csdn.net/xh870189248/article/details/78155357|
| 《**16_ESP8266_RTOS_SDK_V1.5.0**》|（基于FreeRtos 1.5.0版本）的 hello world 串口打印工程模板|http://blog.csdn.net/xh870189248/article/details/79103373|
| 《**17_ESP8266_RedInfra1883**》|乐鑫8266的红外线遥控控制1883固件|http://blog.csdn.net/xh870189248/article/details/79486075|
| 《**18_Esp8266_quickNet_5**》|乐鑫8266的自研的快速开关五次，开-关-开-关-开，之后灯光闪烁，进去配网模式|http://blog.csdn.net/xh870189248/article/details/79486075|
| 《**19_Esp8266_ds18b20_dht11**》|乐鑫esp8266驱动 ds18b20、dht11 温湿度传感器，采集温湿度传感器到服务器。|https://blog.csdn.net/xh870189248/article/details/80284827|
| 《**20_SmartConfigInterface**》|使用C语言对`smartConfig`的二次封装。|https://blog.csdn.net/xh870189248/article/details/80375188|
| 《**21_Esp8266_NOW**》|利用乐鑫的`snow`进行网关组网。|https://blog.csdn.net/xh870189248/article/details/80631739|
| 《**Gizkit_soc_pet**》| 机智云Gokit2、3扩展板的soc方案宠物屋代码。 |...|
| 《**GokitTimerLight**》| 基于机智云平台soc方案定时开启一个LED的源码。 |http://club.gizwits.com/thread-7787-1-1.html|
 

**************************************************************************************************

### 小徐精华总结：
**************************************************************************************************
 #### 一 、 安信可的型号；
 <img src="http://img.blog.csdn.net/20180310172639139" width="1000px" height="360px" alt="Banner" />
 
 #### 二 、 烧录问题；
 >所有自己编译出来的工程，都是有四个文件要烧录的，其中2个是必须烧录的（防止初始化失败），这2个文件分别是 esp_init_data_default.bin 和blank.bin;
 
 >烧录地址根据您的8266型号区别，我们最平常用的是安信可8266-12系列的，这个是32Mbit，也就是4M flash ,所以他们的地址分别是 0x3FC000 和 0x3FE000 , 最后2个的烧录文件地址看编译之后结果而决定！也即是不一定是0x10000，有可能是0x2000 , 这个根据SDK的版本有关系。所以大家还是留心点！
 
 #### 三 、 8266其他好玩的项目收集汇总；
 
 - [x] 8266的5路PWM,允许占空比从0％到100％，步长为200ns。1kHz PWM是5000步，19kHz是256步（8位分辨率）。
    https://github.com/StefanBruens/ESP8266_new_pwm
    
 #### 四 、 如何判断上次掉电重启的原因；
  >这个代码可以判断是否软件复位或者硬件复位，或者判断是否正常重启（包括看门狗复位、非法指针）；


```
    struct rst_info *rtc_info = system_get_rst_info();
   
    printf( "reset reason: %x\n", rtc_info->reason);
  
    if (rtc_info->reason == REASON_WDT_RST ||
        rtc_info->reason == REASON_EXCEPTION_RST ||
        rtc_info->reason == REASON_SOFT_WDT_RST){
        if (rtc_info->reason == REASON_EXCEPTION_RST)
        {
            GIZWITS_LOG("Fatal exception (%d):\n", rtc_info->exccause);
        }
        printf( "epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",
                rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);
    }


```
 
 
 
 
