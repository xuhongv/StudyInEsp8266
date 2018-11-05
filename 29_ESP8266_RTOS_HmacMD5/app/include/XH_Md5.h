#ifndef _XH_MD5_H_
#define _XH_MD5_H_

#include "user_config.h"
#include "XH_Md5.h"

#define	XH_MD5_16 16
#define XH_MD5_32 32

typedef struct {
	unsigned int count[2];
	unsigned int state[4];
	unsigned char buffer[64];
} XH_MD5_CTX;

#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))

#define FF(a,b,c,d,x,s,ac) do{ \
a += F(b, c, d) + x + ac; \
a = ROTATE_LEFT(a, s); \
a += b; \
} while(0)

#define GG(a,b,c,d,x,s,ac) do { \
a += G(b, c, d) + x + ac; \
a = ROTATE_LEFT(a, s); \
a += b; \
} while(0)

#define HH(a,b,c,d,x,s,ac) do { \
a += H(b, c, d) + x + ac; \
a = ROTATE_LEFT(a, s); \
a += b; \
}while(0)

#define II(a,b,c,d,x,s,ac) do{ \
a += I(b, c, d) + x + ac; \
a = ROTATE_LEFT(a, s); \
a += b; \
}while(0)

void XH_MD5Init(XH_MD5_CTX *context);
void XH_MD5Update(XH_MD5_CTX *context, unsigned char *input,
		unsigned int inputlen);
void XH_MD5Final(XH_MD5_CTX *context, unsigned char digest[16]);
void XH_MD5Transform(unsigned int state[4], unsigned char block[64]);
void XH_MD5Encode(unsigned char *output, unsigned int *input, unsigned int len);
void XH_MD5Decode(unsigned int *output, unsigned char *input, unsigned int len);

void XH_MD5Digest(void const *strContent, u16 iLength, u8 output[16]);
void XH_MD5Digest(void const *strContent, u16 iLength, u8 output[16]);
void HMAC_XH_MD5(u8 *inBuffer, u16 len, u8 *ky, u8 *pUserOut);

#endif	/* _XH_MD5_H_ */
