/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* ACImageData.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/
#include "CWAC.h"
#include "ACImageData.h"
#include "ACDbus.h"
#include "wcpss/waw.h"

#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "ACDbus.h"
#include "ACDbus_handler.h"
#include "ACUpdateManage.h"
#include "AC.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

int gCWImageDataPendingTimer = CW_IMAGE_INTERVAL_DEFAULT;
extern struct ap_update_config  g_ap_udapte_config;
extern CWBool CWAssembleResetRequestMessage_cw
(
	CWProtocolMessage **messagesPtr, 
	int *fragmentsNumPtr, 
	int PMTU, 
	unsigned char  seqNum,
	CWImageIdentifier_cw *resPtr
);

char *mac2str(unsigned char *haddr)
{
	static int count = 0;
	static unsigned char buffer[16][32];
	memset(buffer,0,sizeof(buffer));
	int len = sizeof("00:11:22:33:44:55\0");	
	unsigned char *tmp = NULL;

	if (NULL == haddr) 
	{
		return NULL;
	}

	/* Coverity: CID: 16462 Error-Type: Out-of-bounds read */
	count++;

	if (count >= 16)
	{
		count = 0;
	}
	
	tmp = (unsigned char *)&(buffer[count][0]);
	
	memset(tmp, 0, len);
	if (NULL != haddr)
	{
		snprintf((char *)tmp, 32, "%02X:%02X:%02X:%02X:%02X:%02X",
				 haddr[0], haddr[1], haddr[2],
				 haddr[3], haddr[4], haddr[5]);
	}
	
	return (char *)tmp;
}	

CWBool CWCheckImageIdentifier(CWImageIdentifier *valPtr, CWImageIdentifier *resPtr){
	CWConfigVersionInfo *info = gConfigVersionInfo;
	if(info == NULL){
		wid_syslog_err("gConfigVersionInfo is NULL\n");
		return CW_FALSE;
	}
	while(info != NULL){
		//printf("info->str_ap_version_name %s\n",info->str_ap_version_name);
		wid_syslog_err("info->str_ap_version_name %s\n",info->str_ap_version_name);
		unsigned int model_len = 0;
		
		model_len = strlen(info->str_ap_model);
		if(model_len == valPtr->modelLEN)
		{
			if((memcmp(info->str_ap_version_name, valPtr->Ver, valPtr->VerLen) == 0)
				||((memcmp(info->str_ap_model, valPtr->model, valPtr->modelLEN) == 0)&&(memcmp(info->str_ap_version_name, valPtr->Ver, valPtr->VerLen) == 0)))
			{
				resPtr->Ver = info->str_ap_version_path;
				resPtr->VerLen = strlen(info->str_ap_version_path);
				//printf("give ap version is:%s\n",resPtr->Ver);
				wid_syslog_err("give ap version is:%s\n",resPtr->Ver);
				resPtr->VerType = 0;
				resPtr->VerNum = 1;
				return CW_TRUE;
			}
		}
		info = info->next;
	}
	wid_syslog_err("info == NULL\n");
	//printf("info == NULL\n");
	return CW_FALSE;
}
CWBool CWParseMsgElemImageIdentifier(CWProtocolMessage *msgPtr, int len, CWImageIdentifier *valPtr) 
{
	wid_syslog_debug_debug(WID_WTPINFO,"CWParseMsgElemImageIdentifier\n");
	CWParseMessageElementStart();
	int a=0, b=0;
	valPtr->VerNum = CWProtocolRetrieve8(msgPtr);
	a = CWProtocolRetrieve8(msgPtr);
	b = CWProtocolRetrieve16(msgPtr);
	wid_syslog_debug_debug(WID_WTPINFO,"a %d b %d\n",a,b);
	valPtr->VerType = CWProtocolRetrieve16(msgPtr);
	valPtr->VerLen = CWProtocolRetrieve16(msgPtr);
	if((valPtr->VerLen<0)||(valPtr->VerLen>64)){
		wid_syslog_err("%s VerLen==%d\n",__func__,valPtr->VerLen);
		valPtr->VerLen = 64;
	}else{
	
	}

	switch(valPtr->VerType)
	{
		case 1000:
			
			CW_CREATE_STRING_ERR_WID(valPtr->model, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			memset(valPtr->model,0, 5);
			if(valPtr->model != NULL){
				memcpy(valPtr->model,"2010",4);
				}
			else
				{
				wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
				}
			valPtr->modelLEN = 4;
			wid_syslog_debug_debug(WID_WTPINFO,"2010\n");
			valPtr->Ver = CWProtocolRetrieveRawBytes(msgPtr, valPtr->VerLen);
			break;
		case 2000:
			CW_CREATE_STRING_ERR_WID(valPtr->model, 5, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			memset(valPtr->model,0, 6);
			if(valPtr->model != NULL){
				memcpy(valPtr->model,"1110T",5);
				}
			else
				{
				wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
				}
			valPtr->modelLEN = 5;
			wid_syslog_debug_debug(WID_WTPINFO,"1110T\n");
			valPtr->Ver = CWProtocolRetrieveRawBytes(msgPtr, valPtr->VerLen);
			break;
		case 1:
			valPtr->modelLEN = valPtr->VerLen;
			valPtr->model = CWProtocolRetrieveRawBytes(msgPtr, valPtr->VerLen);
			valPtr->VerLen = CWProtocolRetrieve16(msgPtr);
			valPtr->Ver = CWProtocolRetrieveRawBytes(msgPtr, valPtr->VerLen);
			//printf("valPtr->model:%s,valPtr->VerLen:%d,valPtr->Ver:%s",valPtr->model,valPtr->VerLen,valPtr->Ver);
			break;
		default:
			CW_CREATE_STRING_ERR_WID(valPtr->model, 4, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
			memset(valPtr->model,0, 5);
			if(valPtr->model != NULL){
				memcpy(valPtr->model,"1000",4);
				}
			else
				{
				wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
				}
			valPtr->modelLEN = 4;
			wid_syslog_debug_debug(WID_WTPINFO,"1000\n");
			valPtr->Ver = CWProtocolRetrieveRawBytes(msgPtr, valPtr->VerLen);
			break;			

	}
	
	wid_syslog_debug_debug(WID_WTPINFO,"valPtr->VerLen %d  valPtr->Ver %s valPtr->model %s \n",valPtr->VerLen,valPtr->Ver,valPtr->model);
	//printf("valPtr->VerLen %d  valPtr->Ver %s valPtr->model %s \n",valPtr->VerLen,valPtr->Ver,valPtr->model);
	CWParseMessageElementEnd();
}


CWBool CWParseMsgElemEncryptType(CWProtocolMessage *msgPtr, int len, CWEncryptionType *valPtr) 
{
	wid_syslog_debug_debug(WID_WTPINFO,"%s\n",__func__);
	CWParseMessageElementStart();
	valPtr->encrypt_type= CWProtocolRetrieve8(msgPtr);
	wid_syslog_debug_debug(WID_WTPINFO,"%s encrypt_type %d\n",__func__, valPtr->encrypt_type);
	CWParseMessageElementEnd();
}

CWBool CWParseImageDataRequestMessage(CWProtocolMessage* msgPtr, int len, CWImageDataRequest *valuesPtr){
	int offsetTillMessages;
	//unsigned char total_phase = 0;
	//unsigned char current_phase = 0;
	//unsigned char state = 0;//0 means start ; 1 means end
	//unsigned char result = 0; //0 for success; other for failure
				
	if(msgPtr == NULL || valuesPtr==NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	offsetTillMessages = msgPtr->offset;
	//printf("Parsing Image data request...\n");
	wid_syslog_debug_debug(WID_WTPINFO,"Parsing Image data request...");
	valuesPtr->ImageRequest = NULL;
	valuesPtr->EncryptType = NULL;
	
	while((msgPtr->offset - offsetTillMessages) < len) {
		unsigned short int elemType=0;
		unsigned short int elemLen=0;
		unsigned short int subelemid=0;
		unsigned short int subelemlen=0;
		int venderid = 0;
		
		CWParseFormatMsgElem(msgPtr, &elemType, &elemLen);
		
		switch(elemType) {
			case CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE:
				if(valuesPtr->ImageRequest)
				{
					CW_FREE_OBJECT(valuesPtr->ImageRequest->Ver);
					CW_FREE_OBJECT(valuesPtr->ImageRequest->model);
					CW_FREE_OBJECT(valuesPtr->ImageRequest);
				}
				CW_CREATE_OBJECT_ERR(valuesPtr->ImageRequest, CWImageIdentifier, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
				memset(valuesPtr->ImageRequest, 0, sizeof(CWImageIdentifier));
				if (!(CWParseMsgElemImageIdentifier(msgPtr, elemLen, valuesPtr->ImageRequest))){
					wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWParseMsgElemImageIdentifier\n");
					/*free valuesPtr->ImageRequest*/
					CW_FREE_OBJECT(valuesPtr->ImageRequest->Ver);
					CW_FREE_OBJECT(valuesPtr->ImageRequest->model);
					CW_FREE_OBJECT(valuesPtr->ImageRequest);
					return CW_FALSE;
				}
				break;	
		      case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				venderid = CWProtocolRetrieve32(msgPtr);
				CWParseFormatMsgElem(msgPtr, &subelemid, &subelemlen);
				if(subelemid == CW_VENDOR_EVENT_ELEMENT_ENCRYPT_TYPE )
				{
					if(valuesPtr->EncryptType)
					{
						CW_FREE_OBJECT(valuesPtr->EncryptType);
					}
					
					CW_CREATE_OBJECT_ERR(valuesPtr->EncryptType, CWEncryptionType, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
					memset(valuesPtr->EncryptType, 0, sizeof(CWEncryptionType));
					
					if (!(CWParseMsgElemEncryptType(msgPtr, subelemlen, valuesPtr->EncryptType)))
					{
						wid_syslog_debug_debug(WID_WTPINFO,"wrong in %s\n",__func__);
						CW_FREE_OBJECT(valuesPtr->EncryptType);
						return CW_FALSE;
					}	
				}
				break;
			default:				
				wid_syslog_debug_debug(WID_WTPINFO,"wrong in default\n");
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Image data request");				
				// no need break;	
		}
	}
	
	if((msgPtr->offset - offsetTillMessages) != len){ 
		//printf("((msgPtr->offset - offsetTillMessages) != len)\n");
		wid_syslog_debug_debug(WID_WTPINFO,"((msgPtr->offset - offsetTillMessages) != len)");
		/*free valuesPtr->ImageRequest and its elements */
		CW_FREE_OBJECT(valuesPtr->ImageRequest->Ver);
		CW_FREE_OBJECT(valuesPtr->ImageRequest->model);
		CW_FREE_OBJECT(valuesPtr->ImageRequest);
		return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
	}
	//printf("Image data request Parsed\n");
	wid_syslog_debug_debug(WID_WTPINFO,"Image data request Parsed");
	return CW_TRUE;	
}

CWBool CWAssembleMsgElemImageIdentifierAC(CWProtocolMessage *msgPtr, CWImageIdentifier *resPtr){
	int size = 8;
	size += resPtr->VerLen;	
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	CWProtocolStore8(msgPtr, resPtr->VerNum);
	CWProtocolStore8(msgPtr, 0);	
	CWProtocolStore16(msgPtr, 0);
	CWProtocolStore16(msgPtr, resPtr->VerType);	
	CWProtocolStore16(msgPtr, resPtr->VerLen);
	CWProtocolStoreRawBytes(msgPtr, resPtr->Ver, resPtr->VerLen);
	//wid_syslog_debug_debug("resPtr->VerLen %d resPtr->Ver %s\n",resPtr->VerLen, resPtr->Ver);
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE);
}

CWBool CWAssembleMsgElemServerInformation
(
	CWProtocolMessage *msgPtr,
	CWEncryptionType *resPtr,
	int WTPIndex
)
{
/*
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
|		  element				|		   len					|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
|	                          TFTP/FTP server ip                                             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
|	           user-len                       |                  user                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
|	  . . .                         |     passwd-len    |    passwd ....        
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
|	  . . .                                |     encryption type    |         
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
*/	
	int user_len = 0;
	int passwd_len = 0;
	
	//char *str = " ";
	//char __str[128];	
	unsigned int ipaddr = 0;
	struct sockaddr_in  *sin = NULL;
	unsigned char encryption_type_list = 0;
	int i=0;
	short int elementid = CW_VENDOR_SPEC_ELEMENT_SUPPORT_ENCRYPTION; 
	short int length = 4;
	
	for(i=0;g_ap_udapte_config.user[i] != 0;i++)
	{
		user_len++;
	}

	for(i=0;g_ap_udapte_config.passwd[i] != 0;i++)
	{
		passwd_len++;
	}
	wid_syslog_debug_debug(WID_DEFAULT,"%s: user_len  %d passwd_len %d\n",__func__, user_len, passwd_len);

	/*ip + user_len + user + passed_len + passwd + encryption_type_list*/
	length +=4 +  2 + user_len + 2 + passwd_len + 1;

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	

	CWProtocolStore16(msgPtr, elementid); 
	CWProtocolStore16(msgPtr, (length-4));
	
	if(0 != g_ap_udapte_config.ipaddr)
	{
		CWProtocolStore32(msgPtr, g_ap_udapte_config.ipaddr);
	}
	else
	{
		if(gWTPs != NULL && gInterfaces != NULL)
		{
			sin = (struct sockaddr_in*)&(gInterfaces[gWTPs[WTPIndex].interfaceIndex].addr);
			ipaddr = (unsigned int)(sin->sin_addr.s_addr);
		}
		CWProtocolStore32(msgPtr, ipaddr);
	}
	wid_syslog_debug_debug(WID_DEFAULT,"%s: 111\n",__func__);
	CWProtocolStore16(msgPtr, (unsigned short)user_len);
	CWProtocolStoreRawBytes(msgPtr, (char*)(g_ap_udapte_config.user), user_len);
	
	wid_syslog_debug_debug(WID_DEFAULT,"%s: 222\n",__func__);
	CWProtocolStore16(msgPtr, (unsigned short)passwd_len);
	CWProtocolStoreRawBytes(msgPtr, (char*)(g_ap_udapte_config.passwd), passwd_len);
	
	wid_syslog_debug_debug(WID_DEFAULT,"%s: 333\n",__func__);
	encryption_type_list = (resPtr->encrypt_type)&(g_ap_udapte_config.encrypt_type);
	CWProtocolStore8(msgPtr, encryption_type_list);	

	//str = sock_ntop_r(((struct sockaddr*)&(gInterfaces[gWTPs[WTPIndex].interfaceIndex].addr)), __str);
	/*
	wid_syslog_debug_debug(WID_DEFAULT, "%s: user %s passwd %s encrypt %d",
					__func__, 
					g_ap_udapte_config.user,
					g_ap_udapte_config.passwd,
					encryption_type_list);
	*/	
	wid_syslog_debug_debug(WID_DEFAULT,"%s: before CWAssembleMsgElem\n",__func__);
	return CWAssembleMsgElemVendor(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);


	/*
	short int elementid = CW_VENDOR_SPEC_ELEMENT_UPGRADE_MODE; 
	short int length = 5;
	
	if (msgPtr == NULL) 
	{
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);	
	}
	
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore16(msgPtr, elementid); 
	CWProtocolStore16(msgPtr, (length-4));	

	CWProtocolStore8(msgPtr, mode);

	wid_syslog_debug_debug(WID_DEFAULT,"%s: elemid %d elemlen %d mode %d\n", 
						__func__,elementid,length,mode);
		
	return CWAssembleMsgElemVendor(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
	*/
}

CWBool CWAssembleImageDataRequestMessage
(
	CWProtocolMessage **messagesPtr, 
	int *fragmentsNumPtr,
	int PMTU,
	int seqNum,
	CWImageIdentifier *resPtr
)
{
	
		CWProtocolMessage *msgElems = NULL;
		const int MsgElemCount=1;
		CWProtocolMessage *msgElemsBinding = NULL;
		int msgElemBindingCount=0;
		int k = -1;
		
		if(messagesPtr == NULL || fragmentsNumPtr == NULL)
			return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
		
		//printf("Assembling Image Data...\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Assembling Image Data...");
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
		// Assemble Message Elements
		if ((!(CWAssembleMsgElemImageIdentifierAC(&(msgElems[++k]), resPtr)))){
			int i;
			for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT(msgElems);
			return CW_FALSE; // error will be handled by the caller
		}
				
	//wid_syslog_debug_debug("~~~~~ msg count: %d ", msgElemBindingCount);
		
		if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST, msgElems, MsgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
			{return CW_FALSE;}
		
		//printf("Image Data request Assembled\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Image Data request Assembled");
		return CW_TRUE;
}

CWBool CWAssembleImageDataRequestMessage_new
(
	CWProtocolMessage **messagesPtr,
	int *fragmentsNumPtr, int PMTU, 
	int seqNum,
	CWImageIdentifier *resPtr,
	CWEncryptionType *encryptypePtr,
	int WTPIndex
)
{
	
		CWProtocolMessage *msgElems = NULL;
		const int MsgElemCount=2;
		CWProtocolMessage *msgElemsBinding = NULL;
		int msgElemBindingCount=0;
		int k = -1;
		if(messagesPtr == NULL || fragmentsNumPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
		
		//printf("Assembling Image Data...\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Assembling Image Data...");
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
		// Assemble Message Elements
		if ((!(CWAssembleMsgElemImageIdentifierAC(&(msgElems[++k]), resPtr)))
			||(!(CWAssembleMsgElemServerInformation(&(msgElems[++k]), 
														encryptypePtr,
														WTPIndex))))
		{
			wid_syslog_debug_debug(WID_WTPINFO,"2222222\n");
			int i;
			for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT_WID(msgElems);
			return CW_FALSE; // error will be handled by the caller
		}
				
		//wid_syslog_debug_debug("~~~~~ msg count: %d ", msgElemBindingCount);
		wid_syslog_debug_debug(WID_WTPINFO,"1111111111111111111111111\n");
		if(!(CWAssembleMessage(messagesPtr, 
								fragmentsNumPtr,
								PMTU, 
								seqNum,
								CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST, 
								msgElems, 
								MsgElemCount,
								msgElemsBinding,
								msgElemBindingCount,
								CW_PACKET_CRYPT))) 
			{return CW_FALSE;}
		
		//printf("Image Data request Assembled\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Image Data request Assembled");
		return CW_TRUE;
}

CWBool CWParseImageDataResponseMessage(CWProtocolMessage* msgPtr, int len, CWProtocolResultCode* resultCode)
{
	int offsetTillMessages;
	if(msgPtr == NULL || resultCode==NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if((msgPtr->msg) == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	offsetTillMessages = msgPtr->offset;
	
	//printf("Parsing Image Data Response...\n");
	wid_syslog_debug_debug(WID_WTPINFO,"Parsing Image Data Response...");
	// parse message elements
	while((msgPtr->offset - offsetTillMessages) < len) {
		unsigned short int elemType=0;
		unsigned short int elemLen=0;
		
		CWParseFormatMsgElem(msgPtr, &elemType, &elemLen);
		
		switch(elemType) {
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
				*resultCode=CWProtocolRetrieve32(msgPtr);
				break;	
			default:
				//printf("default\n");
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Configuration Update Response");
				break;	
		}
	}
	//printf("*resultCode %d\n",*resultCode);
	wid_syslog_debug_debug(WID_WTPINFO,"*resultCode %d.\n",*resultCode);
	if((msgPtr->offset - offsetTillMessages) != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");

	//printf("Image Data Response Parsed\n");
	wid_syslog_debug_debug(WID_WTPINFO,"Image Data Response Parsed.\n");
	return CW_TRUE;	
}



CWBool ACEnterImageData(int WTPIndex, CWProtocolMessage *msgPtr)
{
	int i=0,result=0;
	//printf("\n");
	//printf("######### Image Data State #########\n");	
	wid_syslog_debug_debug(WID_WTPINFO,"######### WTP %d Enter Image Data State #########\n",WTPIndex);
	CWControlHeaderValues controlVal;
	enum wtp_upgrade_mode upgrade_mode = WTP_UPGRADE_MODE_NONE;
	//CWProtocolMessage* messages =NULL;
	//int messagesCount=0;
	if(!msgPtr)
	{
		wid_syslog_err("%s line %d msgPtr NULL pointer!\n", __func__, __LINE__);
		return CW_FALSE;
	}
	
	memset(&controlVal, 0, sizeof(CWControlHeaderValues));
	msgPtr->offset = 0;
	
	if(!(CWACParseGenericRunMessage(WTPIndex, msgPtr, &controlVal))) {
		//## Two possible errors: WRONG_ARG and INVALID_FORMAT
		//## In the second case we have an unexpected response: ignore the
		//## message and log the event.
		return CW_FALSE;
	}
	
	/* Coverity: CID: 10064  Error-Type: Free of address-of expression  */
	if(!check_wtpid_func(WTPIndex))
	{
		wid_syslog_err("%s line %d WTPIndex %d out of bounds!\n", __func__,__LINE__, WTPIndex);
		return CW_FALSE;
	}
	if(!CWErr(CWTimerCancel(&(gWTPs[WTPIndex].currentTimer),1)))
	{
		_CWCloseThread(WTPIndex);
	}

	if (NULL == AC_WTP[WTPIndex])
	{
		return CW_FALSE;
	}
	
	upgrade_mode = AC_WTP[WTPIndex]->upgrade.mode;

	if ((WTP_UPGRADE_MODE_TFTP == upgrade_mode)
		|| (WTP_UPGRADE_MODE_FTP == upgrade_mode))
	{
		switch(controlVal.messageTypeValue) {
			case CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST:{
				CWImageDataRequest valuesPtr;
				CWImageIdentifier resPtr;
				CWEncryptionType encryptypePtr;
				/* Coverity: CID: 14375  Error-Type: Uninitialized scalar variable	*/
				memset(&valuesPtr, 0, sizeof(CWImageDataRequest));
				memset(&resPtr, 0, sizeof(CWImageIdentifier));
				memset(&encryptypePtr, 0, sizeof(encryptypePtr));
				if(!(CWParseImageDataRequestMessage(msgPtr, controlVal.msgElemsLen, &valuesPtr)))
				{
					wid_syslog_warning("wrong in CWParseImageDataRequestMessage");
					return CW_FALSE;		
				}
				if (NULL != valuesPtr.EncryptType)
				{
					encryptypePtr.encrypt_type = valuesPtr.EncryptType->encrypt_type;
				}
				
				if(gtrapflag >= 1)
				{
					wid_dbus_trap_wtp_enter_imagedata_state(WTPIndex);//wu:ap升级告警
				}
							
				if((AC_WTP[WTPIndex]->updateversion != NULL)&&(AC_WTP[WTPIndex]->updatepath != NULL))
				{
					resPtr.Ver = AC_WTP[WTPIndex]->updatepath;
					resPtr.VerLen = strlen(AC_WTP[WTPIndex]->updatepath);
					resPtr.VerType = 0;
					resPtr.VerNum = 1;				
				}
				else
				{
					/*mahz modify code here to match ap upgrade batchlly*/
					CWConfigVersionInfo_new *tmpnode = gConfigVerInfo;
					CWBool bMatchVersion = CW_FALSE;
					
					if(find_in_wtp_list(WTPIndex) == CW_TRUE)
					{
						for(i=0;i<BATCH_UPGRADE_AP_NUM;i++)
						{
							CWConfigVersionInfo *update_node = gConfigVersionUpdateInfo[i];
							if((update_node != NULL)
								&&(update_node->str_ap_model != NULL)
								&&(0 == strcmp(update_node->str_ap_model,AC_WTP[WTPIndex]->WTPModel) ))
							{
								while(update_node != NULL)
								{
									wid_syslog_debug_debug(WID_WTPINFO,"*** enter image data upgrade ***\n");	//for test
									if(strcmp(update_node->str_ap_code,AC_WTP[WTPIndex]->APCode) == 0)
									{
										wid_syslog_debug_debug(WID_WTPINFO,
															"***111 upgrade node is not null***\n"); //for test
										wid_syslog_debug_debug(WID_WTPINFO,
															"**111 ap code: %s **\n",AC_WTP[WTPIndex]->APCode); //for test
										wid_syslog_debug_debug(WID_WTPINFO,
															"**111 upgrade node code: %s **\n",update_node->str_ap_code);	//for test
										wid_syslog_debug_debug(WID_WTPINFO,
															"**111 upgrade node name: %s **\n",update_node->str_ap_version_name);	//for test
										wid_syslog_debug_debug(WID_WTPINFO,
															"**111 upgrade node path: %s **\n",update_node->str_ap_version_path);	//for test

									resPtr.Ver = update_node->str_ap_version_path;
									resPtr.VerLen = strlen(update_node->str_ap_version_path);
									resPtr.VerType = 0;
									resPtr.VerNum = 1;
									bMatchVersion = CW_TRUE;

									if(AC_WTP[WTPIndex]->updateStat >= 1)
									{
										AC_WTP[WTPIndex]->updatefailcount++;	
									}

									if(AC_WTP[WTPIndex]->updatefailcount >= updatemaxfailcount)
									{
										delete_wtp_list(WTPIndex);
										insert_uptfail_wtp_list(WTPIndex);
										
										update_complete_check();						

										AC_WTP[WTPIndex]->updatefailstate = 1;

										_CWCloseThread(WTPIndex);
									}
									//printf("ACEnterImageData update\n");

									result = 1;
									break;
								}
								update_node = update_node->next;
							}
							if(bMatchVersion == CW_FALSE)	//it will not goto this branch
							{
								resPtr.Ver = valuesPtr.ImageRequest->Ver;
								resPtr.VerLen = valuesPtr.ImageRequest->VerLen;
								resPtr.VerType = 0;
								resPtr.VerNum = 1;
								wid_syslog_debug_debug(WID_WTPINFO,"check image data ver response\n");	//for test
								/*if(!(CWCheckImageIdentifier(valuesPtr.ImageRequest, &resPtr))){
									//printf("wrong in CWCheckImageIdentifier\n");
									wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWCheckImageIdentifier\n");
									return CW_FALSE;
								}*/
							}
							break;
						}
					}
				}
				/*ap upgrade automatically here,the gloable variable gConfigVersionUpdateInfo may be NULL here,so use gConfigVerInfo instead*/
				/*the upgrade automatically can be used now,but after the upgrade is over,the wtpcompatible.xml is not updated,so what the cmd
				show model list shows is still old information, this situation should be modified sooner or later*/
				else if(img_now == 1){
					while(tmpnode != NULL){
						if(strcmp(tmpnode->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0){
							wid_syslog_debug_debug(WID_WTPINFO,"**** find model in apimg.xml ****\n");	//for test
							CWCodeInfo *codenode = tmpnode->code_info;
							while(codenode != NULL){
								wid_syslog_debug_debug(WID_WTPINFO,"**** match code operation ****\n"); //for test
								if(strcmp(codenode->str_ap_version_code,AC_WTP[WTPIndex]->APCode) == 0){
									resPtr.Ver = codenode->str_ap_version_path;
									resPtr.VerLen = strlen(codenode->str_ap_version_path);
									resPtr.VerType = 0;
									resPtr.VerNum = 1;
									
									wid_syslog_debug_debug(WID_WTPINFO,"ac surport version name:%s\n",codenode->str_ap_version_name);
									break;
								}
								codenode = codenode->next;
							}
							break;
						}
						tmpnode = tmpnode->next;
					}
				}
			}
			gWTPs[WTPIndex].responseSeqNum = controlVal.seqNum;
				if(!CWAssembleImageDataRequestMessage_new(&(gWTPs[WTPIndex].messages), 
													     &(gWTPs[WTPIndex].messagesCount),
													      gWTPs[WTPIndex].pathMTU, controlVal.seqNum, 
													      &resPtr,
													      &encryptypePtr,
													      WTPIndex))
				{
					
					wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWAssembleImageDateRequestMessage\n");
					/*valuesPtr.ImageRequest and its elements leak */
					CW_FREE_OBJECT(valuesPtr.ImageRequest->model);
					CW_FREE_OBJECT(valuesPtr.ImageRequest->Ver);
					CW_FREE_OBJECT(valuesPtr.ImageRequest);
					CW_FREE_OBJECT(valuesPtr.EncryptType);
					return CW_FALSE;
				}
				gWTPs[WTPIndex].responseType = CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE;
				AC_WTP[WTPIndex]->updateStat = 1;
		
				if(gtrapflag >= 4)
				{
						wid_dbus_trap_wtp_tranfer_file(WTPIndex);
				}
				
				CW_FREE_OBJECT(valuesPtr.ImageRequest->model);
				CW_FREE_OBJECT(valuesPtr.ImageRequest->Ver);
				CW_FREE_OBJECT(valuesPtr.ImageRequest);
				CW_FREE_OBJECT(valuesPtr.EncryptType);
				break;
			}
			case CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE:
			{
//				wid_syslog_debug_debug(WID_WTPINFO,"CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE");
				CWProtocolResultCode resultCode;			
				CWImageIdentifier resPtr;
				/* Coverity: CID: 14375  Error-Type: Uninitialized scalar variable	*/
				memset(&resultCode, 0, sizeof(CWProtocolResultCode));
				memset(&resPtr, 0, sizeof(CWImageIdentifier));
	//			CWThreadMutexLock(&(gAllThreadMutex));		
				//ap doesn't use the value of resPtr in Reset state
				if(AC_WTP[WTPIndex]->ver != NULL)
				{
					resPtr.Ver = AC_WTP[WTPIndex]->ver;
				}
				else
				{
					resPtr.Ver = " ";
				}
				//resPtr.Ver = gConfigVersionInfo->str_ap_version_path;
				resPtr.VerNum = 1;
				resPtr.VerType = 0;
				resPtr.VerLen = strlen(resPtr.Ver);

				if(!(CWParseImageDataResponseMessage(msgPtr, controlVal.msgElemsLen, &resultCode)))
				{
					//CWThreadMutexUnlock(&(gAllThreadMutex));		
					return CW_FALSE;
				}
				gWTPs[WTPIndex].responseSeqNum = controlVal.seqNum;
				if(!CWAssembleResetRequestMessage(&(gWTPs[WTPIndex].messages), 
												     &(gWTPs[WTPIndex].messagesCount),
												     gWTPs[WTPIndex].pathMTU, controlVal.seqNum,
												     &resPtr))
				{
					
					wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWAssembleResetRequestMessage");
					//CWThreadMutexUnlock(&(gAllThreadMutex));		
					return CW_FALSE;
				}
				
				//CWThreadMutexUnlock(&(gAllThreadMutex));		
				if(resultCode == CW_PROTOCOL_FAILURE_FIRM_WRT_ERROR)
				{	
					if(gtrapflag>=4)
					{
						wid_dbus_trap_wtp_ap_flash_write_failed(WTPIndex);
					}
			}
			time(&AC_WTP[WTPIndex]->imagedata_time);
			gWTPs[WTPIndex].responseType = CW_MSG_TYPE_VALUE_RESET_RESPONSE;
			gWTPs[WTPIndex].currentState = CW_ENTER_RESET;
			AC_WTP[WTPIndex]->WTPStat = 6;
			AC_WTP[WTPIndex]->updateStat = 2;
			break;			
		}
		default:
			//printf("controlVal.messageTypeValue %d\n",controlVal.messageTypeValue);
			result = 0;
			for(i=0;i<BATCH_UPGRADE_AP_NUM;i++)
			{
				if(gConfigVersionUpdateInfo[i] != NULL)
				{
					result = 1;
					break;
				}
			}
			if((result != 0)&&(find_in_wtp_list(WTPIndex) == CW_TRUE))
			{
				delete_wtp_list(WTPIndex);
				insert_uptfail_wtp_list(WTPIndex);	
				update_complete_check();
				
				AC_WTP[WTPIndex]->updatefailstate = 2;

				_CWCloseThread(WTPIndex);		

				}			
				wid_syslog_debug_debug(WID_WTPINFO,"controlVal.messageTypeValue %d",controlVal.messageTypeValue);
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Configuration Update Response");
				// no need break;
		}
		
		if(!CWACSendFragments(WTPIndex))
		{
			/*gWTPs[WTPIndex].messages leak*/
			for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++)
			{
				CW_FREE_PROTOCOL_MESSAGE(gWTPs[WTPIndex].messages[i]);
			}
			
			CW_FREE_OBJECT(gWTPs[WTPIndex].messages);
			return CW_FALSE;
		}
		
		//printf("Image Data Sent\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Image Data Sent");
		if(!CWErr(CWTimerRequest(gCWImageDataPendingTimer, &(gWTPs[WTPIndex].thread), &(gWTPs[WTPIndex].currentTimer), CW_CRITICAL_TIMER_EXPIRED_SIGNAL,WTPIndex))) 
		{
			// start Change State Pending timer
			_CWCloseThread(WTPIndex);
		}
		
		for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++) 
		{
			CW_FREE_PROTOCOL_MESSAGE(gWTPs[WTPIndex].messages[i]);
		}
		
		CW_FREE_OBJECT(gWTPs[WTPIndex].messages);

	}
	else if (WTP_UPGRADE_MODE_CAPWAP == upgrade_mode)
	{
		/* CAPWAP */
		switch(controlVal.messageTypeValue) 
		{
			case CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST:
			{
				CWImageDataRequest valuesPtr;
				CWImageIdentifier resPtr;
				memset(&valuesPtr, 0, sizeof(CWImageDataRequest));
				memset(&resPtr, 0, sizeof(CWImageIdentifier));
				
				if(!(CWParseImageDataRequestMessage_cw(msgPtr, controlVal.msgElemsLen, &valuesPtr)))
				{
					//printf("wrong in CWParseImageDataRequestMessage\n");
					wid_syslog_err("wtp%d parse imagedata request error\n", WTPIndex);
					return CW_FALSE;		
				}

				if(gtrapflag>=1)
				{
					wid_dbus_trap_wtp_enter_imagedata_state(WTPIndex);
				}
							
				//printf("ACEnterImageData\n");
				if((AC_WTP[WTPIndex]->updateversion != NULL)&&(AC_WTP[WTPIndex]->updatepath != NULL))
				{
					resPtr.Ver = AC_WTP[WTPIndex]->updatepath;
					resPtr.VerLen = strlen(AC_WTP[WTPIndex]->updatepath);
					resPtr.VerType = 0;
					resPtr.VerNum = 1;				
				}
				else
				{
					/*mahz modify code here to match ap upgrade batchlly*/
					CWConfigVersionInfo_new *tmpnode = gConfigVerInfo;
					CWBool bMatchVersion = CW_FALSE;
					
					if(find_in_wtp_list(WTPIndex) == CW_TRUE){
						for(i=0;i<BATCH_UPGRADE_AP_NUM;i++){
							CWConfigVersionInfo *update_node = gConfigVersionUpdateInfo[i];
							if((update_node != NULL)&&(update_node->str_ap_model != NULL)&&(strcmp(update_node->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0)){
								while(update_node != NULL){
									wid_syslog_debug_debug(WID_WTPINFO,"*** enter image data upgrade ***\n");	//for test
									if(strcmp(update_node->str_ap_code,AC_WTP[WTPIndex]->APCode) == 0)
									{
										wid_syslog_debug_debug(WID_WTPINFO,"***111 upgrade node is not null***\n"); //for test
										wid_syslog_debug_debug(WID_WTPINFO,"**111 ap code: %s **\n",AC_WTP[WTPIndex]->APCode); //for test
										wid_syslog_debug_debug(WID_WTPINFO,"**111 upgrade node code: %s **\n",update_node->str_ap_code);	//for test
										wid_syslog_debug_debug(WID_WTPINFO,"**111 upgrade node name: %s **\n",update_node->str_ap_version_name);	//for test
										wid_syslog_debug_debug(WID_WTPINFO,"**111 upgrade node path: %s **\n",update_node->str_ap_version_path);	//for test
				
										resPtr.Ver = update_node->str_ap_version_path;
										resPtr.VerLen = strlen(update_node->str_ap_version_path);
										resPtr.VerType = 0;
										resPtr.VerNum = 1;
										bMatchVersion = CW_TRUE;
				
										if(AC_WTP[WTPIndex]->updateStat >= 1)
										{
											AC_WTP[WTPIndex]->updatefailcount++;	
										}
				
										if(AC_WTP[WTPIndex]->updatefailcount >= updatemaxfailcount)
										{
											delete_wtp_list(WTPIndex);
											insert_uptfail_wtp_list(WTPIndex);
											
											update_complete_check();						
				
											AC_WTP[WTPIndex]->updatefailstate = 1;
				
											_CWCloseThread(WTPIndex);
										}
										//printf("ACEnterImageData update\n");
				
										result = 1;
										break;
									}
									update_node = update_node->next;
								}
								if(bMatchVersion == CW_FALSE)	//it will not goto this branch
								{
									resPtr.Ver = valuesPtr.ImageRequest->Ver;
									resPtr.VerLen = valuesPtr.ImageRequest->VerLen;
									resPtr.VerType = 0;
									resPtr.VerNum = 1;
									wid_syslog_debug_debug(WID_WTPINFO,"check image data ver response\n");	//for test
									/*if(!(CWCheckImageIdentifier(valuesPtr.ImageRequest, &resPtr))){
										//printf("wrong in CWCheckImageIdentifier\n");
										wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWCheckImageIdentifier\n");
										return CW_FALSE;
									}*/
								}
								break;
							}
						}
					}
					/*ap upgrade automatically here,the gloable variable gConfigVersionUpdateInfo may be NULL here,so use gConfigVerInfo instead*/
					/*the upgrade automatically can be used now,but after the upgrade is over,the wtpcompatible.xml is not updated,so what the cmd
					show model list shows is still old information, this situation should be modified sooner or later*/
					else if(img_now == 1){
						while(tmpnode != NULL){
							if(strcmp(tmpnode->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0){
								wid_syslog_debug_debug(WID_WTPINFO,"**** find model in apimg.xml ****\n");	//for test
								CWCodeInfo *codenode = tmpnode->code_info;
								while(codenode != NULL){
									wid_syslog_debug_debug(WID_WTPINFO,"**** match code operation ****\n"); //for test
									if(strcmp(codenode->str_ap_version_code,AC_WTP[WTPIndex]->APCode) == 0){
										resPtr.Ver = codenode->str_ap_version_path;
										resPtr.VerLen = strlen(codenode->str_ap_version_path);
										resPtr.VerType = 0;
										resPtr.VerNum = 1;
										
										wid_syslog_debug_debug(WID_WTPINFO,"ac surport version name:%s\n",codenode->str_ap_version_name);
										break;
									}
									codenode = codenode->next;
								}
								break;
							}
							tmpnode = tmpnode->next;
						}
					}
				}

				
				/* init md5 hash  */
				if ((WTP_UPGRADE_MODE_CAPWAP == upgrade_mode)
					&& (WTP_UPGRADE_STATE_JOIN == AC_WTP[WTPIndex]->upgrade.state))
				{
					wtp_upgrade_init_hash(WTPIndex, resPtr.Ver, NULL);
				}

				CWProtocolImageDataResponseValues ImageDateRespVal;
				
				memset(&ImageDateRespVal, 0, sizeof(ImageDateRespVal));
				ImageDateRespVal.ImageInfoHash.filesize = AC_WTP[WTPIndex]->upgrade.filesize;
				memcpy(ImageDateRespVal.ImageInfoHash.hash, AC_WTP[WTPIndex]->upgrade.hash, WID_MD5_LEN);
				
			//	gWTPs[WTPIndex].responseSeqNum = controlVal.seqNum;
				if (!CWAssembleImageDataResponseMessage(&(gWTPs[WTPIndex].messages), 
					&(gWTPs[WTPIndex].messagesCount),
					gWTPs[WTPIndex].pathMTU,
					controlVal.seqNum,
					&ImageDateRespVal))
				{
					//printf("wrong in CWAssembleImageDateRequestMessage\n");
					wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWAssembleImageDateRequestMessage\n");
					/*valuesPtr.ImageRequest and its elements leak */
					CW_FREE_OBJECT(valuesPtr.ImageRequest->model);
					CW_FREE_OBJECT(valuesPtr.ImageRequest->Ver);
					CW_FREE_OBJECT(valuesPtr.ImageRequest);
					return CW_FALSE;
				}
			//	gWTPs[WTPIndex].responseType = 0;
				AC_WTP[WTPIndex]->updateStat = 1;


				/* SEND ImageData response */
				if(!CWACSendFragments(WTPIndex)) 
				{
					/*gWTPs[WTPIndex].messages leak*/
					for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++) {
						CW_FREE_PROTOCOL_MESSAGE(gWTPs[WTPIndex].messages[i]);
					}
					
					CW_FREE_OBJECT(gWTPs[WTPIndex].messages);
					return CW_FALSE;
				}
				
				//printf("Image Data Sent\n");
				wid_syslog_debug("wtp%d send imagedata response\n", WTPIndex);
				
						
				for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++) {
					CW_FREE_PROTOCOL_MESSAGE(gWTPs[WTPIndex].messages[i]);
				}
				
				CW_FREE_OBJECT(gWTPs[WTPIndex].messages);

				
				if(gtrapflag>=4)
				{
					wid_dbus_trap_wtp_tranfer_file(WTPIndex);
				}
				
				
				CW_FREE_OBJECT(valuesPtr.ImageRequest->model);
				CW_FREE_OBJECT(valuesPtr.ImageRequest->Ver);
				CW_FREE_OBJECT(valuesPtr.ImageRequest);


				/* send image file */
				wid_wtp_upgrade(WTPIndex);
				
				break;
			}
			case CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE:
			{
		//		wid_syslog_debug_debug(WID_WTPINFO,"CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE");
				
				CWProtocolResultCode resultCode;			
				memset(&resultCode, 0, sizeof(CWProtocolResultCode));

				if(!(CWParseImageDataResponseMessage(msgPtr, controlVal.msgElemsLen, &resultCode)))
				{

					return CW_FALSE;
				}

				/* TODO: */
				if(CW_PROTOCOL_SUCCESS != resultCode)
				{
					/* 
					delete_wtp_list(WTPIndex);
					insert_uptfail_wtp_list(WTPIndex);	
					update_complete_check();
					*/
					AC_WTP[WTPIndex]->updatefailstate = 2;				
					_CWCloseThread(WTPIndex);		
				}
				
				CWACStopRetransmission(WTPIndex);

				/* upgradeing ... */
				if (WTP_UPGRADE_STATE_IMAGEING == AC_WTP[WTPIndex]->upgrade.state)
				{
					wid_wtp_upgrade(WTPIndex);
				}				
				else if (WTP_UPGRADE_STATE_IMAGEEND == AC_WTP[WTPIndex]->upgrade.state)
				{
					CWImageIdentifier_cw resPtr;
					memset(&resPtr, 0 , sizeof(resPtr));

					if('\0' != AC_WTP[WTPIndex]->upgrade.ImageName[0])
					{
						strncpy(resPtr.ImageName, AC_WTP[WTPIndex]->upgrade.ImageName, WTP_IMAGENAME_LEN-1);
						resPtr.ImageNameLen = strlen(resPtr.ImageName);		
					}
					else
					{
						strncpy(resPtr.ImageName, "wtp.img", WTP_IMAGENAME_LEN-1);
						resPtr.ImageNameLen = strlen(resPtr.ImageName);				
					}
					
					if('\0' != AC_WTP[WTPIndex]->upgrade.version)
					{
						strncpy(resPtr.version, AC_WTP[WTPIndex]->upgrade.version, WTP_VERSION_LEN-1);
						resPtr.VersionLen = strlen(resPtr.version);		
					}
					else
					{
						strncpy(resPtr.version, "version", WTP_VERSION_LEN-1);
						resPtr.VersionLen = strlen(resPtr.version);		
					}
					
					if(AC_WTP[WTPIndex]->APCode != NULL)
					{
						strncpy(resPtr.Model, AC_WTP[WTPIndex]->APCode, WTP_MODE_LEN-1);
						resPtr.ModelLen = strlen(resPtr.Model);	
					}
					else
					{
						strncpy(resPtr.Model, "Model", WTP_MODE_LEN-1);
						resPtr.ModelLen = strlen(resPtr.Model);	
					}
					
					/* upgrade end enter reset state */
					
					int seqNum = CWGetSeqNum();
					if(!CWAssembleResetRequestMessage_cw(&(gWTPs[WTPIndex].messages),
								&(gWTPs[WTPIndex].messagesCount),
								gWTPs[WTPIndex].pathMTU, 
								seqNum,
								&resPtr))
					{
						wid_syslog_err("wtp%d assemble reset request msg failed\n", WTPIndex);
						return CW_FALSE;
					}
				
					AC_WTP[WTPIndex]->upgrade.state = WTP_UPGRADE_STATE_DONE;


					if(CWACSendAcknowledgedPacket(WTPIndex, CW_MSG_TYPE_VALUE_RESET_RESPONSE, seqNum)) 
					{
						//bResult = CW_TRUE;
					}
					else
					{
						CWACStopRetransmission(WTPIndex);
					}

					time(&AC_WTP[WTPIndex]->imagedata_time);
					gWTPs[WTPIndex].currentState = CW_ENTER_RESET;
					AC_WTP[WTPIndex]->WTPStat = WID_RESET;
					AC_WTP[WTPIndex]->updateStat = 2;
				
					wid_syslog_debug("wtp%d imagedata finished enter reset\n", WTPIndex);

				}
				else
				{
						wid_syslog_err("wtp%d %s upgrade state err %d\n",
							WTPIndex,  mac2str(AC_WTP[WTPIndex]->WTPMAC), AC_WTP[WTPIndex]->upgrade.state);
				}
					
				if(resultCode == CW_PROTOCOL_FAILURE_FIRM_WRT_ERROR)
				{	
					if(gtrapflag>=4)
					{
						wid_dbus_trap_wtp_ap_flash_write_failed(WTPIndex);
					}
				}
				break;			
			}

 			default:
			
				result = 0;
				for(i=0;i<BATCH_UPGRADE_AP_NUM;i++)
				{
					if(gConfigVersionUpdateInfo[i] != NULL)
					{
						result = 1;
						break;
					}
				}
				if((result != 0)&&(find_in_wtp_list(WTPIndex) == CW_TRUE))
				{
					delete_wtp_list(WTPIndex);
					insert_uptfail_wtp_list(WTPIndex);	
					update_complete_check();
					
					AC_WTP[WTPIndex]->updatefailstate = 2;

					_CWCloseThread(WTPIndex);		

				}			
				wid_syslog_err("wtp%d rcv messageType %d in imagedata state\n",
					WTPIndex, controlVal.messageTypeValue);
				return CW_FALSE;
				/*no need break*/


 		}
	}
	else if (WTP_UPGRADE_MODE_NONE == upgrade_mode)
	{
		switch(controlVal.messageTypeValue) {
			case CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST:{
				CWImageDataRequest valuesPtr;
				CWImageIdentifier resPtr;
				
				/* Coverity: CID: 14375  Error-Type: Uninitialized scalar variable	*/
				memset(&valuesPtr, 0, sizeof(CWImageDataRequest));
				memset(&resPtr, 0, sizeof(CWImageIdentifier));
				
				if(!(CWParseImageDataRequestMessage(msgPtr, controlVal.msgElemsLen, &valuesPtr)))
				{
					wid_syslog_warning("wrong in CWParseImageDataRequestMessage");
					return CW_FALSE;		
				}
				
				if(gtrapflag>=1){
					wid_dbus_trap_wtp_enter_imagedata_state(WTPIndex);//wu:ap升级告警
				}
							
				if((AC_WTP[WTPIndex]->updateversion != NULL)&&(AC_WTP[WTPIndex]->updatepath != NULL))
				{
					resPtr.Ver = AC_WTP[WTPIndex]->updatepath;
					resPtr.VerLen = strlen(AC_WTP[WTPIndex]->updatepath);
					resPtr.VerType = 0;
					resPtr.VerNum = 1;				
				}
				else
				{
					/*mahz modify code here to match ap upgrade batchlly*/
					CWConfigVersionInfo_new *tmpnode = gConfigVerInfo;
					CWBool bMatchVersion = CW_FALSE;
					
					if(find_in_wtp_list(WTPIndex) == CW_TRUE)
					{
						for(i=0;i<BATCH_UPGRADE_AP_NUM;i++)
						{
							CWConfigVersionInfo *update_node = gConfigVersionUpdateInfo[i];
							if((update_node != NULL)
								&&(update_node->str_ap_model != NULL)
								&&(0 == strcmp(update_node->str_ap_model,AC_WTP[WTPIndex]->WTPModel) ))
							{
								while(update_node != NULL)
								{
									wid_syslog_debug_debug(WID_WTPINFO,"*** enter image data upgrade ***\n");	//for test
									if(strcmp(update_node->str_ap_code,AC_WTP[WTPIndex]->APCode) == 0)
									{
										wid_syslog_debug_debug(WID_WTPINFO,
															"***111 upgrade node is not null***\n"); //for test
										wid_syslog_debug_debug(WID_WTPINFO,
															"**111 ap code: %s **\n",AC_WTP[WTPIndex]->APCode); //for test
										wid_syslog_debug_debug(WID_WTPINFO,
															"**111 upgrade node code: %s **\n",update_node->str_ap_code);	//for test
										wid_syslog_debug_debug(WID_WTPINFO,
															"**111 upgrade node name: %s **\n",update_node->str_ap_version_name);	//for test
										wid_syslog_debug_debug(WID_WTPINFO,
															"**111 upgrade node path: %s **\n",update_node->str_ap_version_path);	//for test

										resPtr.Ver = update_node->str_ap_version_path;
										resPtr.VerLen = strlen(update_node->str_ap_version_path);
										resPtr.VerType = 0;
										resPtr.VerNum = 1;
										bMatchVersion = CW_TRUE;

										if(AC_WTP[WTPIndex]->updateStat >= 1)
										{
											AC_WTP[WTPIndex]->updatefailcount++;	
										}

										if(AC_WTP[WTPIndex]->updatefailcount >= updatemaxfailcount)
										{
											delete_wtp_list(WTPIndex);
											insert_uptfail_wtp_list(WTPIndex);
											
											update_complete_check();						

											AC_WTP[WTPIndex]->updatefailstate = 1;

											_CWCloseThread(WTPIndex);
										}
										//printf("ACEnterImageData update\n");

										result = 1;
										break;
									}
									update_node = update_node->next;
								}
								if(bMatchVersion == CW_FALSE)	//it will not goto this branch
								{
									resPtr.Ver = valuesPtr.ImageRequest->Ver;
									resPtr.VerLen = valuesPtr.ImageRequest->VerLen;
									resPtr.VerType = 0;
									resPtr.VerNum = 1;
									wid_syslog_debug_debug(WID_WTPINFO,"check image data ver response\n");	//for test
									/*if(!(CWCheckImageIdentifier(valuesPtr.ImageRequest, &resPtr))){
										//printf("wrong in CWCheckImageIdentifier\n");
										wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWCheckImageIdentifier\n");
										return CW_FALSE;
									}*/
								}
								break;
							}
						}
					}
					/*ap upgrade automatically here,the gloable variable gConfigVersionUpdateInfo may be NULL here,so use gConfigVerInfo instead*/
					/*the upgrade automatically can be used now,but after the upgrade is over,the wtpcompatible.xml is not updated,so what the cmd
					show model list shows is still old information, this situation should be modified sooner or later*/
					else if(img_now == 1)
					{
						while(tmpnode != NULL)
						{
							if(strcmp(tmpnode->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0)
							{
								wid_syslog_debug_debug(WID_WTPINFO,"**** find model in apimg.xml ****\n");	//for test
								CWCodeInfo *codenode = tmpnode->code_info;
								while(codenode != NULL)
								{
									wid_syslog_debug_debug(WID_WTPINFO,"**** match code operation ****\n"); //for test
									if(strcmp(codenode->str_ap_version_code,AC_WTP[WTPIndex]->APCode) == 0)
									{
										resPtr.Ver = codenode->str_ap_version_path;
										resPtr.VerLen = strlen(codenode->str_ap_version_path);
										resPtr.VerType = 0;
										resPtr.VerNum = 1;
										
										wid_syslog_debug_debug(WID_WTPINFO,"ac surport version name:%s\n",codenode->str_ap_version_name);
										break;
									}
									codenode = codenode->next;
								}
								break;
							}
							tmpnode = tmpnode->next;
						}
					}
				}
				gWTPs[WTPIndex].responseSeqNum = controlVal.seqNum;
				if(!CWAssembleImageDataRequestMessage(&(gWTPs[WTPIndex].messages), 
													     &(gWTPs[WTPIndex].messagesCount),
													      gWTPs[WTPIndex].pathMTU, 
													      controlVal.seqNum, 
													      &resPtr))
				{
					
					wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWAssembleImageDateRequestMessage\n");
					/*valuesPtr.ImageRequest and its elements leak */
					CW_FREE_OBJECT(valuesPtr.ImageRequest->model);
					CW_FREE_OBJECT(valuesPtr.ImageRequest->Ver);
					CW_FREE_OBJECT(valuesPtr.ImageRequest);
					return CW_FALSE;
				}
				gWTPs[WTPIndex].responseType = CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE;
				AC_WTP[WTPIndex]->updateStat = 1;
		
				if(gtrapflag>=4){
						wid_dbus_trap_wtp_tranfer_file(WTPIndex);
				}
				
				CW_FREE_OBJECT(valuesPtr.ImageRequest->model);
				CW_FREE_OBJECT(valuesPtr.ImageRequest->Ver);
				CW_FREE_OBJECT(valuesPtr.ImageRequest);
				CW_FREE_OBJECT(valuesPtr.EncryptType);
				break;
			}
			case CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE:
			{
				wid_syslog_debug_debug(WID_WTPINFO,"CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE");
				CWProtocolResultCode resultCode;			
				CWImageIdentifier resPtr;
				/* Coverity: CID: 14375  Error-Type: Uninitialized scalar variable	*/
				memset(&resultCode, 0, sizeof(CWProtocolResultCode));
				memset(&resPtr, 0, sizeof(CWImageIdentifier));
	//			CWThreadMutexLock(&(gAllThreadMutex));		
				//ap doesn't use the value of resPtr in Reset state
				if(AC_WTP[WTPIndex]->ver != NULL)
				{
					resPtr.Ver = AC_WTP[WTPIndex]->ver;
				}
				else
				{
					resPtr.Ver = " ";
				}
				//resPtr.Ver = gConfigVersionInfo->str_ap_version_path;
				resPtr.VerNum = 1;
				resPtr.VerType = 0;
				resPtr.VerLen = strlen(resPtr.Ver);

				if(!(CWParseImageDataResponseMessage(msgPtr, controlVal.msgElemsLen, &resultCode)))
				{
					//CWThreadMutexUnlock(&(gAllThreadMutex));		
					return CW_FALSE;
				}
				gWTPs[WTPIndex].responseSeqNum = controlVal.seqNum;
				if(!CWAssembleResetRequestMessage(&(gWTPs[WTPIndex].messages), 
												     &(gWTPs[WTPIndex].messagesCount),
												     gWTPs[WTPIndex].pathMTU, controlVal.seqNum,
												     &resPtr))
				{
					
					wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWAssembleResetRequestMessage");
					//CWThreadMutexUnlock(&(gAllThreadMutex));		
					return CW_FALSE;
				}
				
				//CWThreadMutexUnlock(&(gAllThreadMutex));		
				if(resultCode == CW_PROTOCOL_FAILURE_FIRM_WRT_ERROR)
				{	
					if(gtrapflag>=4)
					{
						wid_dbus_trap_wtp_ap_flash_write_failed(WTPIndex);
					}
				}
				
				time(&AC_WTP[WTPIndex]->imagedata_time);
				gWTPs[WTPIndex].responseType = CW_MSG_TYPE_VALUE_RESET_RESPONSE;
				gWTPs[WTPIndex].currentState = CW_ENTER_RESET;
				AC_WTP[WTPIndex]->WTPStat = WID_RESET;
				AC_WTP[WTPIndex]->updateStat = 2;
				break;			
			}
			default:
				//printf("controlVal.messageTypeValue %d\n",controlVal.messageTypeValue);
				result = 0;
				for(i=0;i<BATCH_UPGRADE_AP_NUM;i++)
				{
					if(gConfigVersionUpdateInfo[i] != NULL)
					{
						result = 1;
						break;
					}
				}
				if((result != 0)&&(find_in_wtp_list(WTPIndex) == CW_TRUE))
				{
					delete_wtp_list(WTPIndex);
					insert_uptfail_wtp_list(WTPIndex);	
					update_complete_check();
					
					AC_WTP[WTPIndex]->updatefailstate = 2;

					_CWCloseThread(WTPIndex);		

				}			
				wid_syslog_debug_debug(WID_WTPINFO,"controlVal.messageTypeValue %d",controlVal.messageTypeValue);
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Configuration Update Response");
				// no need break;
		}
		
		if(!CWACSendFragments(WTPIndex))
		{
			/*gWTPs[WTPIndex].messages leak*/
			for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++)
			{
				CW_FREE_PROTOCOL_MESSAGE(gWTPs[WTPIndex].messages[i]);
			}
			
			CW_FREE_OBJECT(gWTPs[WTPIndex].messages);
			return CW_FALSE;
		}
		
		//printf("Image Data Sent\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Image Data Sent");
		if(!CWErr(CWTimerRequest(gCWImageDataPendingTimer, &(gWTPs[WTPIndex].thread), &(gWTPs[WTPIndex].currentTimer), CW_CRITICAL_TIMER_EXPIRED_SIGNAL,WTPIndex))) 
		{
			// start Change State Pending timer
			_CWCloseThread(WTPIndex);
		}
		
		for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++) 
		{
			CW_FREE_PROTOCOL_MESSAGE(gWTPs[WTPIndex].messages[i]);
		}
		
		CW_FREE_OBJECT(gWTPs[WTPIndex].messages);
	}
	
	return CW_TRUE;
}



int wid_wtp_upgrade(unsigned int wtpid)
{

	int wtpindex = wtpid;
	IMG_UPDATE type = IMGDATA_DATA;
	int datasize = MAX_DATE_SIZE;
	CWBool bResult = CW_FALSE;
//	int DATA_SIZE = MAX_DATE_SIZE;
	char buf[MAX_DATE_SIZE] = {0};
	char path[256] = {0};
	int ssize = 0;
	CWImageDateRequestDate ImageData;
	FILE *fp = NULL;


	if ((wtpid >= WTP_NUM) || (NULL == AC_WTP[wtpid]))
	{
		wid_syslog_err("%s:%d max wtp%d not exist\n", __func__, wtpid);
		return -1;
	}

	snprintf(path, sizeof(path), "/mnt/wtp/%s", AC_WTP[wtpid]->upgrade.ImageName);

	if (NULL == AC_WTP[wtpid]->upgrade.fp)
	{
		fp = fopen(path, "r");
		if (fp == NULL)
		{		
			wid_syslog_err("%s fopen %s failed %s\n",__func__,path, strerror(errno));
			return -1;
		}
		AC_WTP[wtpid]->upgrade.fp = fp;
	}
	else
	{
		fp = AC_WTP[wtpid]->upgrade.fp;
	}

	memset(buf, 0, sizeof(buf));
	ssize = fread(buf, 1, datasize, fp);
	/* TODO */
	if (0 != ssize)
	{
	//	gWTPs[wtpindex].IMG.type = IMGDATA_DATA;
		type = IMGDATA_DATA;

		AC_WTP[wtpid]->upgrade.pktcnt++;

		wid_syslog_debug("wtp%d send imagedata request %d times\n",
				wtpindex, AC_WTP[wtpid]->upgrade.pktcnt);			
		
		if ((datasize != ssize) && (feof(fp)))
		{
			type = IMGDATA_EOF;
			fclose(fp);
			AC_WTP[wtpid]->upgrade.fp = NULL;	
			wid_syslog_debug("wtp%d image upgrade end\n", wtpindex);
		}	
	}
	
	memset(&ImageData, 0, sizeof(ImageData));
	ImageData.ImageData = buf;
	ImageData.ImageDataLen = ssize;
	ImageData.ImageDateType = type;

	int seqNum = CWGetSeqNum();
	//int seqNum = CWGetSeqNumWtp(wtpindex);

//	wid_syslog_debug_debug(WID_WTPINFO,"%s,%d,wtp%d seqNum=%d\n",__func__,__LINE__,wtpindex,seqNum);
	if(CWAssembleImageDateRequest(&(gWTPs[wtpindex].messages), 
			&(gWTPs[wtpindex].messagesCount), 
			gWTPs[wtpindex].pathMTU, 
			seqNum, &ImageData))
	{
		if(CWACSendAcknowledgedPacket(wtpindex, CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE, seqNum)) 
		{
			bResult = CW_TRUE;
		}
		else
		{
			CWACStopRetransmission(wtpindex);
		}
	}

	if (IMGDATA_EOF == type)
	{
		AC_WTP[wtpindex]->upgrade.state = WTP_UPGRADE_STATE_IMAGEEND;
	}
	else
	{
		AC_WTP[wtpindex]->upgrade.state = WTP_UPGRADE_STATE_IMAGEING;
	}

	AC_WTP[wtpindex]->updateStat = 1;

	return 0;
}


CWBool CWAssembleImageDataElem(CWProtocolMessage *msgPtr, CWImageDateRequestDate *ImageData) 
{
	 int headerLength = 2;
	 int messageBodyLength = 0;
	 int totalMessageLength = 0;

	if(msgPtr == NULL)
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	messageBodyLength = ImageData->ImageDataLen;
	totalMessageLength = headerLength + messageBodyLength;
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, totalMessageLength, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, ImageData->ImageDateType);
	CWProtocolStoreRawBytes(msgPtr, ImageData->ImageData, ImageData->ImageDataLen);

	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_DATA_CW_TYPE);
}




CWBool CWProtocolAssembleImageDataRequest(CWProtocolMessage **msgElems, 
				int *msgElemCountPtr, CWImageDateRequestDate *ImageData)
{
	int k = -1;
	
	if(msgElems == NULL || msgElemCountPtr == NULL) 
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	*msgElemCountPtr = 1;

//	wid_syslog_debug_debug(WID_DEFAULT,"Assembling Protocol Configuration Update Request...\n");

	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(*msgElems, *msgElemCountPtr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	// Assemble Message Elements
	if (!(CWAssembleImageDataElem(&(*msgElems[++k]), ImageData)))
	{
		int i;
		for(i = 0; i <= k; i++) {CW_FREE_PROTOCOL_MESSAGE(*msgElems[i]);}
		CW_FREE_OBJECT(*msgElems);
		return CW_FALSE; // error will be handled by the caller
	}

//	wid_syslog_debug_debug(WID_DEFAULT,"Protocol Configuration Update Request Assembled\n");
	
	return CW_TRUE;
}


CWBool CWAssembleImageDateRequest(CWProtocolMessage **messagesPtr,
					    int *fragmentsNumPtr,
					    int PMTU,
						int seqNum,
						CWImageDateRequestDate *ImageData) 
{

	CWProtocolMessage *msgElemsBinding = NULL;
	int msgElemBindingCount=0;
	CWProtocolMessage *msgElems = NULL;
	int msgElemCount=0;
	
	if (messagesPtr == NULL || fragmentsNumPtr == NULL)
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	CWLog("Assembling Configuration Update Request...");

	if (!CWProtocolAssembleImageDataRequest(&msgElems, &msgElemCount, ImageData)) 
	{
		return CW_FALSE;
	}
	
	if(!(CWAssembleMessage(messagesPtr,
			       fragmentsNumPtr,
			       PMTU,
			       seqNum,
			       CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST,
			       msgElems,
			       msgElemCount,
			       msgElemsBinding,
			       msgElemBindingCount,
			       CW_PACKET_CRYPT)))
		return CW_FALSE;

	CWLog("Configuration Update Request Assembled");
	
	return CW_TRUE;
}

CWBool CWAssembleMsgElemImageInfor(CWProtocolMessage *msgPtr, CWImageInformationHash *resPtr){
	int size = 0;
	size += 4*5;	

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	CWProtocolStore32(msgPtr, resPtr->filesize);	//sizeof imagefile
	CWProtocolStoreRawBytes(msgPtr, (char*)resPtr->hash, WID_MD5_LEN);	//hash
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_INFO_CW_TYPE);
}




CWBool CWParseImageDataRequestMessage_cw
(
	CWProtocolMessage* msgPtr,
	int len,
	CWImageDataRequest *valuesPtr
)
{
	int offsetTillMessages;
	if(msgPtr == NULL || valuesPtr==NULL) 
	{
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	}
	if((msgPtr->msg) == NULL) 
	{
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	}
	
	offsetTillMessages = msgPtr->offset;
	
//	wid_syslog_debug_debug(WID_WTPINFO,"Parsing Image data request...");
	valuesPtr->ImageRequest = NULL;
	
	/* parse message elements*/
	while((msgPtr->offset - offsetTillMessages) < len) 
	{
		unsigned short int elemType=0;
		unsigned short int elemLen=0;
		
		CWParseFormatMsgElem(msgPtr, &elemType, &elemLen);
		
		switch(elemType) 
		{
			case CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE:

				/* TODO packet prase */
				if(valuesPtr->ImageRequest)
				{
					CW_FREE_OBJECT(valuesPtr->ImageRequest->Ver);
					CW_FREE_OBJECT(valuesPtr->ImageRequest->model);
					CW_FREE_OBJECT(valuesPtr->ImageRequest);
				}
				
				CW_CREATE_OBJECT_ERR(valuesPtr->ImageRequest, 
										      CWImageIdentifier,
										      return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
				memset(valuesPtr->ImageRequest, 0, sizeof(CWImageIdentifier));
				
				if (!(CWParseMsgElemImageIdentifier_cw(msgPtr, elemLen, valuesPtr->ImageRequest)))
				{
					wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWParseMsgElemImageIdentifier\n");
					/*free valuesPtr->ImageRequest*/
					CW_FREE_OBJECT(valuesPtr->ImageRequest->Ver);
					CW_FREE_OBJECT(valuesPtr->ImageRequest->model);
					CW_FREE_OBJECT(valuesPtr->ImageRequest);
					return CW_FALSE;
				}
				
				break;	

			case CW_MSG_ELEMENT_INITIATED_DOWNLOAD_CW_TYPE:
				msgPtr->offset += elemLen;
				wid_syslog_info("%s,%d,elemType=%d,elemLen=%d.",__func__,__LINE__,elemType,elemLen);
				break;	
				
			default:				
				wid_syslog_debug_debug(WID_WTPINFO,"wrong in default\n");
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element in Image data request");				
				/*no need break;*/	
		}
	}
	
	if((msgPtr->offset - offsetTillMessages) != len)
	{ 
		wid_syslog_debug_debug(WID_WTPINFO,"((msgPtr->offset - offsetTillMessages) != len)");
		/*free valuesPtr->ImageRequest and its elements */
		if(valuesPtr->ImageRequest)
		{ /* CID:10468: null pointer dereference */
			CW_FREE_OBJECT(valuesPtr->ImageRequest->Ver);
			CW_FREE_OBJECT(valuesPtr->ImageRequest->model);
			CW_FREE_OBJECT(valuesPtr->ImageRequest);
		}
		return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
	}
//	wid_syslog_debug_debug(WID_WTPINFO,"Image data request Parsed");
	return CW_TRUE;	
}


CWBool CWParseMsgElemImageIdentifier_cw
(
	CWProtocolMessage *msgPtr,
	int len, 
	CWImageIdentifier *valPtr
) 
{
	wid_syslog_debug_debug(WID_WTPINFO,"CWParseMsgElemImageIdentifier, len : %d\n", len);
	CWParseMessageElementStart();
	int initOffSet = msgPtr->offset;
	unsigned short type = 0;

	valPtr->vendorid = CWProtocolRetrieve32(msgPtr);
	
	if (CW_MSG_ELEMENT_VENDOR_IDENTIFIER != valPtr->vendorid)
	{
		return CW_FALSE;
	}
	
	while((msgPtr->offset - initOffSet )<(len-4))
	{
		type = CWProtocolRetrieve16(msgPtr);

		if (CW_IMAGEIDENTIFIER_VERSION == type)
		{
			valPtr->version_len = CWProtocolRetrieve16(msgPtr);
			if (valPtr->version_len >= WTP_VERSION_LEN)
			{
				return CW_FALSE;
			}
			CWProtocolGetRawBytes(msgPtr, (char *)valPtr->version, valPtr->version_len);
			wid_syslog_debug_debug(WID_WTPINFO,"%s: version is %s len is %d\n", __func__, valPtr->version, valPtr->version_len);
		}
		else if (CW_IMAGEIDENTIFIER_IMAGENAME == type)
		{
			valPtr->version_filename_len = CWProtocolRetrieve16(msgPtr);
			if (valPtr->version_filename_len >= WTP_IMAGENAME_LEN)
			{
				return CW_FALSE;
			}
			CWProtocolGetRawBytes(msgPtr, (char *)valPtr->version_filename, valPtr->version_filename_len);
			wid_syslog_debug_debug(WID_WTPINFO,"%s: version_filename is %s len is %d\n", __func__, valPtr->version_filename, valPtr->version_filename_len);
		}
		else if (CW_IMAGEIDENTIFIER_MODE == type)
		{
			valPtr->version_model_len = CWProtocolRetrieve16(msgPtr);
			if (valPtr->version_model_len >= WTP_MODE_LEN)
			{
				return CW_FALSE;
			}
			CWProtocolGetRawBytes(msgPtr, (char *)valPtr->version_model, valPtr->version_model_len);
			wid_syslog_debug_debug(WID_WTPINFO,"%s: model is %s len is %d\n", __func__, valPtr->version_model, valPtr->version_model_len);
		}
		else
		{
			wid_syslog_err("<err> %s,%d,error type %d\n",__func__,__LINE__, type);
			return CW_FALSE;
		}
	}

	CWParseMessageElementEnd();
}

CWBool CWAssembleImageDataRequestMessage_new2(CWProtocolMessage **messagesPtr, 
	int *fragmentsNumPtr, int PMTU, int seqNum, CWImageIdentifier *resPtr)
{
	
		CWProtocolMessage *msgElems = NULL;
		const int MsgElemCount=1;
		CWProtocolMessage *msgElemsBinding = NULL;
		int msgElemBindingCount=0;
		int k = -1;
		
		if(messagesPtr == NULL || fragmentsNumPtr == NULL)
			return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
		
		//printf("Assembling Image Data...\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Assembling Image Data...");
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
		// Assemble Message Elements
		if ((!(CWAssembleMsgElemImageIdentifierAC(&(msgElems[++k]), resPtr)))){
			int i;
			for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT(msgElems);
			return CW_FALSE; // error will be handled by the caller
		}
				
	//wid_syslog_debug_debug("~~~~~ msg count: %d ", msgElemBindingCount);
		
		if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST, msgElems, MsgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
			{return CW_FALSE;}
		
		//printf("Image Data request Assembled\n");
		wid_syslog_debug_debug(WID_WTPINFO,"Image Data request Assembled");
		return CW_TRUE;
}


CWBool CWAssembleMsgElemImageInformationAC(CWProtocolMessage *msgPtr, 
	CWImageInformationHash *resPtr)
{
	int size = 0;
	size += 4*5;	
	//wid_syslog_info("%s,%d.\n",__func__,__LINE__);
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	CWProtocolStore32(msgPtr, resPtr->filesize);	//sizeof imagefile
	CWProtocolStoreRawBytes(msgPtr, (char *)resPtr->hash, WID_MD5_LEN);	//hash
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_INFO_CW_TYPE);
}




CWBool CWAssembleImageDataResponseMessage
(
	CWProtocolMessage **messagesPtr,
	int *fragmentsNumPtr,
	int PMTU,
	int seqNum, 
	CWProtocolImageDataResponseValues *ImageDateRespVal
)
{

	CWProtocolMessage *msgElems = NULL;
	const int MsgElemCount=2;
	CWProtocolMessage *msgElemsBinding = NULL;
	int msgElemBindingCount=0;
	int k = -1;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL) 
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	//printf("Assembling Image Data...\n");
//	wid_syslog_debug_debug(WID_WTPINFO,"%s,%d,Assembling Image Data...",__func__,__LINE__);
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	if ((!(CWAssembleMsgElemResultCode(&(msgElems[++k]), CW_PROTOCOL_SUCCESS)))||
		(!(CWAssembleMsgElemImageInfor(&(msgElems[++k]), &(ImageDateRespVal->ImageInfoHash)))))
	{
		int i;
		//wid_syslog_info("%s %d.\n",__func__,__LINE__);
		for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		CW_FREE_OBJECT(msgElems);
		return CW_FALSE; // error will be handled by the caller
	}
			
	
	if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE, msgElems, MsgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
		{return CW_FALSE;}
	
//	wid_syslog_debug_debug(WID_WTPINFO,"%s,%d,Image Data request Assembled",__func__,__LINE__);
	return CW_TRUE;
}


int wid_get_upgrade_info(unsigned int wtpid, WIDUpgradeInfo *UpgradeInfo)
{
	int i=0,result=0;

	WID_CHECK_POINTER_RET(UpgradeInfo, -1);
	WID_CHECK_WTP_STANDARD_RET(wtpid,-1);
	
	if((AC_WTP[wtpid]->updateversion != NULL)&&(AC_WTP[wtpid]->updatepath != NULL))
	{
		UpgradeInfo->NeedUpgrade = CW_TRUE;
		strncpy(UpgradeInfo->ImageName, AC_WTP[wtpid]->updatepath, WTP_IMAGENAME_LEN-1);
	}
	else
	{
		/*mahz modify code here to match ap upgrade batchlly*/
		CWConfigVersionInfo_new *tmpnode = gConfigVerInfo;
		CWBool bMatchVersion = CW_FALSE;
		
		if(find_in_wtp_list(wtpid) == CW_TRUE)
		{
			for(i=0;i<BATCH_UPGRADE_AP_NUM;i++)
			{
				CWConfigVersionInfo *update_node = gConfigVersionUpdateInfo[i];
				if((update_node != NULL)
					&&(update_node->str_ap_model != NULL)
					&&(strcmp(update_node->str_ap_model,AC_WTP[wtpid]->WTPModel) == 0))
				{
					while(update_node != NULL)
					{
						wid_syslog_debug_debug(WID_WTPINFO,"*** enter image data upgrade ***\n");	//for test
						if(strcmp(update_node->str_ap_code, AC_WTP[wtpid]->APCode) == 0)
						{
							wid_syslog_debug_debug(WID_WTPINFO,"***111 upgrade node is not null***\n"); //for test
							wid_syslog_debug_debug(WID_WTPINFO,"**111 ap code: %s **\n",AC_WTP[wtpid]->APCode); //for test
							wid_syslog_debug_debug(WID_WTPINFO,"**111 upgrade node code: %s **\n",update_node->str_ap_code);	//for test
							wid_syslog_debug_debug(WID_WTPINFO,"**111 upgrade node name: %s **\n",update_node->str_ap_version_name);	//for test
							wid_syslog_debug_debug(WID_WTPINFO,"**111 upgrade node path: %s **\n",update_node->str_ap_version_path);	//for test

	
							UpgradeInfo->NeedUpgrade = CW_TRUE;
							strncpy(UpgradeInfo->ImageName, update_node->str_ap_version_path, WTP_IMAGENAME_LEN-1);
							
							bMatchVersion = CW_TRUE;
	
							if(AC_WTP[wtpid]->updateStat >= 1)
							{
								AC_WTP[wtpid]->updatefailcount++;	
							}
	
							if(AC_WTP[wtpid]->updatefailcount >= updatemaxfailcount)
							{
								delete_wtp_list(wtpid);
								insert_uptfail_wtp_list(wtpid);
								
								update_complete_check();						
	
								AC_WTP[wtpid]->updatefailstate = 1;
	
								_CWCloseThread(wtpid);
							}
							//printf("ACEnterImageData update\n");
	
							result = 1;
							break;
						}
						update_node = update_node->next;
					}
					#if 0
					if(bMatchVersion == CW_FALSE)	//it will not goto this branch
					{
						resPtr.Ver = valuesPtr.ImageRequest->Ver;
						resPtr.VerLen = valuesPtr.ImageRequest->VerLen;
						resPtr.VerType = 0;
						resPtr.VerNum = 1;
						wid_syslog_debug_debug(WID_WTPINFO,"check image data ver response\n");	//for test
						/*if(!(CWCheckImageIdentifier(valuesPtr.ImageRequest, &resPtr))){
							//printf("wrong in CWCheckImageIdentifier\n");
							wid_syslog_debug_debug(WID_WTPINFO,"wrong in CWCheckImageIdentifier\n");
							return CW_FALSE;
						}*/
					}
					#endif
					break;
				}
			}
		}
		/*ap upgrade automatically here,the gloable variable gConfigVersionUpdateInfo may be NULL here,so use gConfigVerInfo instead*/
		/*the upgrade automatically can be used now,but after the upgrade is over,the wtpcompatible.xml is not updated,so what the cmd
		show model list shows is still old information, this situation should be modified sooner or later*/
		else if (img_now == 1)
		{
			while(tmpnode != NULL)
			{
				if(strcmp(tmpnode->str_ap_model,AC_WTP[wtpid]->WTPModel) == 0)
				{
					wid_syslog_debug_debug(WID_WTPINFO,"**** find model in apimg.xml ****\n");	//for test
					CWCodeInfo *codenode = tmpnode->code_info;
					while(codenode != NULL)
					{
						wid_syslog_debug_debug(WID_WTPINFO,"**** match code operation ****\n"); //for test
						if (strcmp(codenode->str_ap_version_code,AC_WTP[wtpid]->APCode) == 0)
						{
						
							UpgradeInfo->NeedUpgrade = CW_TRUE;
							strncpy(UpgradeInfo->ImageName, codenode->str_ap_version_path, WTP_IMAGENAME_LEN-1);
							
							
							wid_syslog_debug_debug(WID_WTPINFO,"ac surport version name:%s\n",codenode->str_ap_version_name);
							break;
						}
						codenode = codenode->next;
					}
					break;
				}
				tmpnode = tmpnode->next;
			}
		}
	}

return 0;
}

#if 0
CWBool CWAssembleImageDataRequestMessageData(CWProtocolMessage **messagesPtr,
	int *fragmentsNumPtr, int PMTU, int seqNum, CWImageInformationHash *resPtr,
	IMG_UPDATE type,char *data_str,int ssize)
{
	
		CWProtocolMessage *msgElems = NULL;
		int MsgElemCount=2;
		if(type == IMGDATA_EOF)
			MsgElemCount=2;
		else
			MsgElemCount=1;
		CWProtocolMessage *msgElemsBinding = NULL;
		int msgElemBindingCount=0;
		int k = -1;
		if(messagesPtr == NULL || fragmentsNumPtr == NULL) 
			return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
		
		//printf("Assembling Image Data...\n");
		wid_syslog_debug_debug(WID_WTPINFO,"%s,%d,Assembling Image Data...",__func__,__LINE__);
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, MsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

		
		//wid_syslog_info("%s,%d,ssize=%d.\n",__func__,__LINE__,ssize);
		if(!(CWAssembleMsgElemImageData(&(msgElems[++k]), resPtr,type,data_str,ssize)))
		{
			int i;
			//wid_syslog_info("%s,%d.\n",__func__,__LINE__);
			for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT(msgElems);
			return CW_FALSE; // error will be handled by the caller
		}
		//wid_syslog_info("%s,%d.\n",__func__,__LINE__);
		if(!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST, msgElems, MsgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT))) 
			{return CW_FALSE;}
		
		wid_syslog_debug_debug(WID_WTPINFO,"%s,%d,Image Data request Assembled",__func__,__LINE__);
		return CW_TRUE;
}

CWBool CWAssembleMsgElemImageData(CWProtocolMessage *msgPtr, 
	CWImageIdentifier *resPtr,IMG_UPDATE type,char *data_str,int ssize)
{
	int size = 1;
	int resultcode =0;
	size += ssize;	
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	
	CWProtocolStore8(msgPtr, type);/*data--1;eof--2;err--5*/
	if((type != IMGDATA_ERR)&&(ssize != 0))
	{
		//wid_syslog_info("%s,%d,messagesize=%d,datassize=%d.\n",__func__,__LINE__,size,ssize);
		CWProtocolStoreRawBytes(msgPtr, data_str,ssize);
	}
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_DATA_CW_TYPE);
}

#endif
