#ifndef MSMARTCONFIG_H_INCLUDED
#define MSMARTCONFIG_H_INCLUDED

typedef enum {
	xSmartConfig_Status_Succeed = 0, //连接成功
	xSmartConfig_Status_Failed = 1,  //连接失败
	xSmartConfig_Status_Get_Pas = 3, //成功得到手机发来的网关的帐号和密码信息
	xSmartConfig_Status_Connectting_GateWay = 4,  //正在连接网关
	xSmartConfig_Status_Connectting_Early = 5,  //未开始连接
} xSmartConfig_Status_Code;

typedef void (*xSmartConfig_CallBack)(xSmartConfig_Status_Code statusCode);

//开始一键配网
void xSmartConfig_Start();
//注册回调函数
void register_xSmartConfigCallBack(xSmartConfig_CallBack callBack);

#endif
