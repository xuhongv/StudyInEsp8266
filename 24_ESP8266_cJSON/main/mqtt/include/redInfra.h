#ifndef __COMHWX_H__
#define __COMHWX__
void comhwx_init(void);

typedef void (*redInfra_CallBack)(char *pdata);

void funRedInfra_CallBack(redInfra_CallBack callBack);

#endif
