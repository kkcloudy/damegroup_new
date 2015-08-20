#ifndef CW_IMAGE_DATA_H
#define CW_IMAGE_DATA_H

typedef struct {
	int VerNum;
	int VerType;
	int VerLen;
	char *Ver;
	int modelLEN;
	char *model;

	unsigned int vendorid;
	int version_len;
	char version[WTP_VERSION_LEN];
	int version_filename_len;
	char version_filename[WTP_IMAGENAME_LEN];
	int   version_model_len;
	char version_model[WTP_MODE_LEN];
}CWImageIdentifier;

typedef struct {
	unsigned char encrypt_type;
	/*add for requirement*/
}CWEncryptionType;

typedef struct {
	CWImageIdentifier *ImageRequest;
	CWEncryptionType *EncryptType;
}CWImageDataRequest;


typedef struct 
{
	int filesize;
	unsigned char hash[WID_MD5_LEN];
}CWImageInformationHash;


typedef struct 
{
	/* CW_MSG_ELEMENT_IMAGE_DATA_CW_TYPE 24 */
	int ImageDateTypeElem;
	unsigned char ImageDateType;
	int ImageDataLen;
	char *ImageData;	
}CWImageDateRequestDate;

typedef struct {
	CWImageInformationHash ImageInfoHash;
} CWProtocolImageDataResponseValues;


typedef enum {
	IMGDATA_DATA = 1,
	IMGDATA_EOF = 2,
	IMGDATA_ERR = 5
}IMG_UPDATE;

#define WTP_MODEL_LEN	(64)
typedef struct {
	CWBool NeedUpgrade;
	char version[WTP_VERSION_LEN];
	char ImageName[WTP_IMAGENAME_LEN];
	char Model[WTP_MODEL_LEN];

}WIDUpgradeInfo;


CWBool CWAssembleMsgElemImageIdentifierAC(CWProtocolMessage *msgPtr, CWImageIdentifier *resPtr);
CWBool CWAssembleResetRequestMessage(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, CWImageIdentifier *resPtr);
CWBool CWAssembleImageDataResponseMessage
(
	CWProtocolMessage **messagesPtr,
	int *fragmentsNumPtr,
	int PMTU,
	int seqNum, 
	CWProtocolImageDataResponseValues *ImageDateRespVal
);
CWBool CWParseImageDataRequestMessage_cw(CWProtocolMessage* msgPtr, int len, CWImageDataRequest *valuesPtr);
int wid_wtp_upgrade(unsigned int wtpid);
CWBool CWAssembleImageDateRequest(CWProtocolMessage **messagesPtr,
					    int *fragmentsNumPtr,
					    int PMTU,
						int seqNum,
						CWImageDateRequestDate *ImageData) ;
CWBool CWParseMsgElemImageIdentifier_cw
(
	CWProtocolMessage *msgPtr,
	int len, 
	CWImageIdentifier *valPtr
);

#endif
