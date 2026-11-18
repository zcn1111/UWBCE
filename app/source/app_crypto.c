#include "bsp.h"
#include "main.h"


cmox_ecb_handle_t ecb_Ctx;
cmox_ccm_handle_t Ccm_ctx;
size_t computed_size;

const uint8_t Key[] =
{
	0x2b, 0x7e, 0x15, 0x16, 0x1e, 0x03, 0xac, 0x9c, 0x93, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51
};
const uint8_t AddData[]=
{//sxshmj07335252
	0x73, 0x78, 0x73, 0x68, 0x6D, 0x6A, 0x30, 0x37, 0x33, 0x33, 0x35, 0x32, 0x35, 0x32
};

const uint8_t sxsh[4]=
{//SXSH
	0x53, 0x58, 0x53, 0x48
};
uint8_t Expected_Tag[4];
uint8_t CCMIV[12];

uint8_t g_ecbTime[6];
uint8_t g_ecbDataBuf[16];
uint8_t g_ecbComputedBuf[16];
uint8_t g_ccmDataBuf[32];
uint8_t g_ccmComputedBuf[36];


/******************************************************************
函数名称: uint8_t  ecb_encrypte(void)
功能描述: 
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
uint8_t  ecb_encrypte(uint8_t *src,uint16_t len)
{
	cmox_cipher_retval_t retval;
	
	/* Compute directly the ciphertext passing all the needed parameters */
	retval = cmox_cipher_encrypt(CMOX_AES_ECB_ENC_ALGO,                  /* Use AES CBC algorithm */
							   g_ecbDataBuf, sizeof(g_ecbDataBuf),           /* Plaintext to encrypt */
							   Key, sizeof(Key),                       /* AES key to use */
							   NULL, 0,                         /* Initialization vector */
							   g_ecbComputedBuf, &computed_size);   /* Data buffer to receive generated ciphertext */
							   
	if (retval == CMOX_CIPHER_SUCCESS && computed_size == sizeof(g_ecbDataBuf))
		return 1;
	else
		return 0;
}

/******************************************************************
函数名称: uint8_t ecb_decrypte(void)
功能描述: 
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
uint8_t ecb_decrypte(uint8_t *src,uint16_t len)
{	

	cmox_cipher_retval_t retval;
	
	retval = cmox_cipher_decrypt(CMOX_AES_ECB_DEC_ALGO,                 /* Use AES CBC algorithm */
							   g_ecbDataBuf, sizeof(g_ecbDataBuf), /* Ciphertext to decrypt */
							   Key, sizeof(Key),                      /* AES key to use */
							   NULL, 0,                        /* Initialization vector */
							   g_ecbComputedBuf, &computed_size);   /* Data buffer to receive generated plaintext */

	/* Verify API returned value */
	if (retval == CMOX_CIPHER_SUCCESS && computed_size == sizeof(g_ecbDataBuf))
		return 1;
	else
		return 0;
}

/******************************************************************
函数名称: uint8_t ccm_encrypt(void)
功能描述: 
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
uint8_t ccm_encrypt(uint8_t *src,uint16_t len)
{
	cmox_cipher_retval_t retval;
	
	retval = cmox_aead_encrypt(CMOX_AES_CCM_ENC_ALGO,            /* Use AES GCM algorithm */
						 src, len,           /* Plaintext to encrypt */
						 sizeof(Expected_Tag),                  /* Authentication tag size */
						 Key, sizeof(Key),                       /* AES key to use */
						 CCMIV, sizeof(CCMIV),                       /* Initialization vector */
						 AddData, sizeof(AddData),               /* Additional authenticated data */
						 g_ccmComputedBuf, &computed_size);   /* Data buffer to receive generated ciphertext
																	and authentication tag */
	/* Verify API returned value */
	if (retval == CMOX_CIPHER_SUCCESS && computed_size == sizeof(g_ccmComputedBuf) + sizeof(Expected_Tag))
		return 1;
	else
		return 0; 
}

/******************************************************************
函数名称: uint8_t ccm_decrypt(void)
功能描述: 
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
uint8_t ccm_decrypt(uint8_t *src,uint16_t len)
{	
	cmox_cipher_retval_t retval;

	 retval = cmox_aead_decrypt(CMOX_AES_CCM_DEC_ALGO,               /* Use AES GCM algorithm */
                             src, len,  		/* Ciphertext + tag to decrypt and verify */
                             sizeof(Expected_Tag),                  /* Authentication tag size */
                             Key, sizeof(Key),                       /* AES key to use */
                             CCMIV, sizeof(CCMIV),                       /* Initialization vector */
                             AddData, sizeof(AddData),               /* Additional authenticated data */
                             g_ccmDataBuf, &computed_size);    /* Data buffer to receive generated plaintext */

	/* Verify API returned value */
	if (retval != CMOX_CIPHER_AUTH_SUCCESS && computed_size == sizeof(g_ccmDataBuf))
		return 1;
	else
		return 0; 
}

/******************************************************************
函数名称: void encrypt(void)
功能描述: 
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
void GetCCMCCMIV(uint8_t *time)
{
	memset(g_ecbDataBuf,0,sizeof(g_ecbDataBuf));

	memcpy(g_ecbDataBuf,g_BTMac,6);	
	memcpy(g_ecbDataBuf+6,time,6);
	memcpy(g_ecbDataBuf+12,sxsh,4);
	if(ecb_encrypte(g_ecbDataBuf,16) == 1)
	{
		CCMIV[0] =  g_ecbComputedBuf[7];
		CCMIV[1] =  g_ecbComputedBuf[5];
		CCMIV[2] =  g_ecbComputedBuf[13];
		CCMIV[3] =  g_ecbComputedBuf[2];
		CCMIV[4] =  g_ecbComputedBuf[3];
		CCMIV[5] =  g_ecbComputedBuf[10];
		CCMIV[6] =  g_ecbComputedBuf[9];
		CCMIV[7] =  g_ecbComputedBuf[8];
		CCMIV[8] =  g_ecbComputedBuf[1];
		CCMIV[9] =  g_ecbComputedBuf[12];
		CCMIV[10] =  g_ecbComputedBuf[14];
		CCMIV[11] =  g_ecbComputedBuf[11];
	}
}

/******************************************************************
函数名称: void encrypt(void)
功能描述: 
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
void encrypt(uint8_t *src,uint16_t len)
{
	GetCCMCCMIV(g_ecbTime);
	ccm_encrypt(src,len);
	memcpy(g_BTCmdData,g_ccmComputedBuf,len+4);
}

/******************************************************************
函数名称: void decrypt(void)
功能描述: 
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
void decrypt(uint8_t *time,uint8_t *src,uint16_t len)
{
	GetCCMCCMIV(time);
	ccm_decrypt(src,len);
	memcpy(g_BTCmdData,g_ccmDataBuf,len-4);
}
