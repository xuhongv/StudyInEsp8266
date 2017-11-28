/*
 * my_http.h
 *
 *  Created on: 2015Äê7ÔÂ24ÈÕ
 *      Author: Administrator
 */

#ifndef APP_INCLUDE_MY_HTTP_H_
#define APP_INCLUDE_MY_HTTP_H_

void startHttpQuestByPOST(char *URL,char *method,char *postdata);
void startHttpQuestByGET(char *URL);
char host[32];
char filename[208];
unsigned short port;



#endif /* APP_INCLUDE_MY_HTTP_H_ */
