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
* ACJoinState.c
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
#include "wcpss/waw.h"

#include "wcpss/wid/WID.h" 
#include "CWAC.h"
#include "ACDbus_handler.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif
int wtp_upgrade_init_hash(unsigned int wtpid, char *ImageName, char *version);

CWBool CWAssembleJoinResponse
(
	CWProtocolMessage **messagesPtr,
	int *fragmentsNumPtr,
	int PMTU,
	int seqNum,
	CWList msgElemList,
	char *strVersion,
	CWProtocolJoinResponseValues *joinResponseVal,
	enum wtp_upgrade_mode upgrade_mode;
);
CWBool CWParseJoinRequestMessage(char *msg, int len, int *seqNumPtr, CWProtocolJoinRequestValues *valuesPtr, int WTPIndex);
CWBool CWSaveJoinRequestMessage(CWProtocolJoinRequestValues *joinRequest, CWWTPProtocolManager *WTPProtocolManager,unsigned int WTPIndex);
void CWDestroyJoinRequestValues(CWProtocolJoinRequestValues *valPtr);
CWBool CWAssembleMsgElemImageIdentifier(CWProtocolMessage *msgPtr, 
	CWImageIdentifier_cw *ImageIdentifier);
int wid_md5_hash(char *path, unsigned char *md5);


CWBool ACEnterJoin(int WTPIndex, CWProtocolMessage *msgPtr)
{	
	int seqNum;
	CWProtocolJoinRequestValues joinRequest;
	memset(&joinRequest,0,sizeof(CWProtocolJoinRequestValues));
	CWList msgElemList = NULL;
	int ret = 0;
	wid_syslog_debug_debug(WID_WTPINFO,"######### WTP %d Enter Join State #########",WTPIndex);	
	if(msgPtr == NULL) {
		wid_syslog_err("wtp %d join msgPtr is NULL\n", WTPIndex);
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	}
	
	if(!(CWParseJoinRequestMessage(msgPtr->msg, msgPtr->offset, &seqNum, &joinRequest, WTPIndex))) {
		wid_syslog_err("parse wtp %d jion request failed\n", WTPIndex);
		return CW_FALSE;
	}

	if(!CWTimerCancel(&(gWTPs[WTPIndex].currentTimer),1))
	{
		return CW_FALSE;
	}
	
	if((find_in_wtp_list(WTPIndex) == CW_TRUE))
	{
		wid_syslog_debug_debug(WID_DEFAULT, "find wtp %d update info in updatelist\n", WTPIndex);
		if(!CWTimerCancel(&(gWTPs[WTPIndex].updateTimer),1))
		{
			wid_syslog_info("CWTimerCancel error\n");
			return CW_FALSE;
		}
	}
	enum wtp_upgrade_mode upgrade_mode = AC_WTP[WTPIndex]->upgrade.mode;
//	wid_syslog_debug_debug(WID_WTPINFO,"%s: upgrade_mode %d\n", __func__, upgrade_mode);

	CWBool ACIpv4List = CW_FALSE;
	CWBool ACIpv6List = CW_FALSE;
	CWBool resultCode = CW_TRUE;
	int resultCodeValue = CW_PROTOCOL_SUCCESS;
	//CWBool sessionID = CW_FALSE;
/*	unsigned char *mac;	
	mac = (unsigned char*)malloc(6);
	memset(mac,0,6);
	if(!(CWCheckWTPBoardData(WTPIndex, &(joinRequest.WTPBoardData), mac))){
		gWTPs[WTPIndex].currentState = CW_QUIT;
		AC_WTP[WTPIndex]->WTPStat = 0;
		memset(AC_WTP[WTPIndex]->WTPIP, 0, 128);
		free(mac);
		mac = NULL;
		return CW_TRUE;
	}	
	memcpy(AC_WTP[WTPIndex]->WTPMAC, mac, 6);
	free(mac);
	mac = NULL;*/
	if(!(CWSaveJoinRequestMessage(&joinRequest,&(gWTPs[WTPIndex].WTPProtocolManager),WTPIndex))) 
	{
		resultCodeValue = CW_PROTOCOL_FAILURE_RES_DEPLETION;
	}
	
	CWMsgElemData *auxData;
	if(ACIpv4List) {
		CW_CREATE_OBJECT_ERR_WID(auxData, CWMsgElemData, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		auxData->type = CW_MSG_ELEMENT_AC_IPV4_LIST_CW_TYPE;
		auxData->value = 0;
		CWAddElementToList(&msgElemList,auxData);
	}
	if(ACIpv6List){
		CW_CREATE_OBJECT_ERR_WID(auxData, CWMsgElemData, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        auxData->type = CW_MSG_ELEMENT_AC_IPV6_LIST_CW_TYPE;
        auxData->value = 0;
        CWAddElementToList(&msgElemList,auxData);
	}
	if(resultCode){
		CW_CREATE_OBJECT_ERR_WID(auxData, CWMsgElemData, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        auxData->type =  CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE;
        auxData->value = resultCodeValue;
        CWAddElementToList(&msgElemList,auxData);
	}
// 	if(sessionID){
// 		CW_CREATE_OBJECT_ERR(auxData, CWMsgElemData, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
//                 auxData->type =  CW_MSG_ELEMENT_SESSION_ID_CW_TYPE;
//                 auxData->value = CWRandomIntInRange(0, INT_MAX);
//                 CWAddElementToList(&msgElemList,auxData);
// 	}

	//added by weiay 20080618


	CWWTPDescriptor *valPtr = &gWTPs[WTPIndex].WTPProtocolManager.descriptor;
//	unsigned char *str_wtp_version = NULL;
	
	CWWTPVendorInfos *vendorPtr = &gWTPs[WTPIndex].WTPProtocolManager.WTPBoardData;
	unsigned char *str_wtp_model = NULL;
		
	CWConfigVersionInfo * pVersionNode = gConfigVersionInfo;
	CWConfigVersionInfo_new *tmpnode = gConfigVerInfo;
	CWBool bMatchVersion = CW_FALSE;
	int i=0,len=0,aclen=0,lenmodel=0,verlen=0,result=0;

	char *str_ac_version = NULL;
	
	for(i = 0; i < vendorPtr->vendorInfosCount; i++)
	{

		if((vendorPtr->vendorInfos)[i].type == 0)//CW_WTP_MODEL_NUMBER = 0
		{

			str_wtp_model = (vendorPtr->vendorInfos)[i].model;
			lenmodel = (vendorPtr->vendorInfos)[i].length;
			
			if((AC_WTP[WTPIndex])&&(AC_WTP[WTPIndex]->APCode)&&(strncmp(AC_WTP[WTPIndex]->APCode,(char *)str_wtp_model,lenmodel) != 0))
			{
				CW_FREE_OBJECT_WID(AC_WTP[WTPIndex]->APCode);
				AC_WTP[WTPIndex]->APCode = (char *)WID_MALLOC(lenmodel+1);
				if(AC_WTP[WTPIndex]->APCode == NULL)
				{
					wid_syslog_err("%s %d malloc fail\n",__FUNCTION__,__LINE__);
					return  CW_FALSE;
				}
				memset(AC_WTP[WTPIndex]->APCode,0,lenmodel+1);
				memcpy(AC_WTP[WTPIndex]->APCode,str_wtp_model,strlen((char *)str_wtp_model));
					
			}
			//break;
		}
		else if((vendorPtr->vendorInfos)[i].type == 6)//CW_WTP_CODE_VERSION = 6
		{
			AC_WTP[WTPIndex]->codever = (char*)(vendorPtr->vendorInfos)[i].codever;
			len = (vendorPtr->vendorInfos)[i].length;
		} else {
			wid_syslog_debug_debug(WID_WTPINFO, "unsupported type %d for wtp %d\n", 
				(vendorPtr->vendorInfos)[i].type, WTPIndex);
		}
	}

/*	//special upgrade 20081007 by weiay
	if((lenmodel == 4)&&(*(vendorPtr->vendorInfos)[0].valuePtr == 1000))
	{
//		printf("##### goto image data ######\n");
		//char *str = "1.0.12";
		str_ac_version = pVersionNode->str_ap_version_name;
		goto inter_image_date;
		
	}
	//printf("##### i= %d######\n",i);
*/	
	wid_syslog_debug_debug(WID_WTPINFO,"** wtp model:%s len:%d **\n",str_wtp_model,lenmodel);


	for(i = 0; i < valPtr->vendorInfos.vendorInfosCount; i++)
	{
		if((valPtr->vendorInfos.vendorInfos)[i].type == 0)//CW_WTP_HARDWARE_VERSION = 0
		{
			char * sysver;
			int m = 0;
			wid_syslog_debug_debug(WID_DEFAULT,"** (valPtr->vendorInfos.vendorInfos)[i].sysver %s **\n",(valPtr->vendorInfos.vendorInfos)[i].sysver);
			sysver = (char *)(valPtr->vendorInfos.vendorInfos)[i].sysver;

			for(m = 0; m < (valPtr->vendorInfos.vendorInfos)[i].length; m++){
				wid_syslog_debug_debug(WID_DEFAULT,"** (valPtr->vendorInfos.vendorInfos)[i].sysver[%d] %d %c**\n",m,sysver[m],sysver[m]);
				if((sysver[m] < 33)||(sysver[m] > 126))
					break;
			}
			if(m == (valPtr->vendorInfos.vendorInfos)[i].length)
				AC_WTP[WTPIndex]->sysver = (char *)(valPtr->vendorInfos.vendorInfos)[i].sysver;
			else{
				AC_WTP[WTPIndex]->sysver = NULL;
				wid_syslog_info("WTP %d HD version %s something wrong\n",WTPIndex,sysver);
			}
		} else if((valPtr->vendorInfos.vendorInfos)[i].type == 1) {
			ret = wid_illegal_character_check((char *)(valPtr->vendorInfos.vendorInfos)[i].ver,(valPtr->vendorInfos.vendorInfos)[i].length, 0);
			if(ret == 1){
				AC_WTP[WTPIndex]->ver = (char *)(valPtr->vendorInfos.vendorInfos)[i].ver;
				verlen = (valPtr->vendorInfos.vendorInfos)[i].length;
			}else{
				AC_WTP[WTPIndex]->ver = NULL;
				str_ac_version = (char *)(valPtr->vendorInfos.vendorInfos)[i].ver;				
				verlen = (valPtr->vendorInfos.vendorInfos)[i].length;
				wid_syslog_info("WTP %d SW version %s something wrong\n",WTPIndex,str_ac_version);
			}
			break;
		} else {
			wid_syslog_debug_debug(WID_WTPINFO, "unsuported (valPtr->vendorInfos.vendorInfos)[%d].type=%d for wtp %d\n", 
				i, (valPtr->vendorInfos.vendorInfos)[i].type, WTPIndex);
		}
	}
	
	wid_syslog_debug_debug(WID_WTPINFO,"** wtp version:%s **\n",AC_WTP[WTPIndex]->ver);
	if(AC_WTP[WTPIndex]->codever != NULL)
	{
		wid_syslog_debug_debug(WID_WTPINFO,"** wtp code version:%s **\n",AC_WTP[WTPIndex]->codever);
	}

	//force upgrade mode
	if((AC_WTP[WTPIndex]->updateversion != NULL)&&(AC_WTP[WTPIndex]->updatepath != NULL))
	{
		wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: updateversion=%s updatepath=%s\n", 
			AC_WTP[WTPIndex]->updateversion, AC_WTP[WTPIndex]->updatepath);
		
		str_ac_version = AC_WTP[WTPIndex]->updateversion;

		if(AC_WTP[WTPIndex]->codever == NULL){
			if((strlen(AC_WTP[WTPIndex]->updateversion) == verlen)&&(strncasecmp(AC_WTP[WTPIndex]->ver,AC_WTP[WTPIndex]->updateversion,verlen) == 0)) {
				bMatchVersion = CW_TRUE;
				wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: force upgrade match\n");
			} else {
				wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: force upgrade not match\n");
			}
		} else if((AC_WTP[WTPIndex]->codever != NULL)&&(strlen(AC_WTP[WTPIndex]->updateversion) == len)&&(strncasecmp(AC_WTP[WTPIndex]->codever,AC_WTP[WTPIndex]->updateversion,len) == 0)) {
			str_ac_version = AC_WTP[WTPIndex]->ver;
			bMatchVersion = CW_TRUE;
			wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: force upgrade match brunch B\n");
		} else {
			wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: force upgrade not match brunch C\n");
		}
	} else {
		//muilty models and versions auto upgrade and batchlly upgrade

		//batchlly upgrade
		if((find_in_wtp_list(WTPIndex) == CW_TRUE))
		{
			//batchlly upgrade brunch
			wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: batchlly upgrade for wtp %d\n", WTPIndex);
			for(i=0;i<BATCH_UPGRADE_AP_NUM;i++){
				CWConfigVersionInfo *update_node = gConfigVersionUpdateInfo[i];
				wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: search model %s from gConfigVersionUpdateInfo[%d]\n", 
					AC_WTP[WTPIndex]->WTPModel, i);
				
				if((update_node != NULL)&&(update_node->str_ap_model != NULL))
					wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: update_node->str_ap_model=%s\n", update_node->str_ap_model);
				else {
					wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: update_node is NULL or update_node->str_ap_model is NULL\n");
					continue;
				}
				
				if(strcmp(update_node->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0) {
					while(update_node != NULL){
						
						if (update_node->str_ap_code != NULL) {
							wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: update_node->str_ap_code=%s AC_WTP[%d]->APCode=%s\n", 
								update_node->str_ap_code, WTPIndex, AC_WTP[WTPIndex]->APCode);
						} else {
							wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: update_node->str_ap_code is NULL\n");
							update_node = update_node->next;
							continue;
						}

						if(strcmp(update_node->str_ap_code,AC_WTP[WTPIndex]->APCode) == 0){

							wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: apcode match update_node->str_ap_version_name=%s AC_WTP[%d]->ver=%s\n", 
								update_node->str_ap_version_name, WTPIndex, AC_WTP[WTPIndex]->ver);
							
							str_ac_version = update_node->str_ap_version_name;
							
							if(AC_WTP[WTPIndex]->codever == NULL){
								if((strlen(update_node->str_ap_version_name) == verlen)&&(strncasecmp(AC_WTP[WTPIndex]->ver,update_node->str_ap_version_name,verlen) == 0))
								{
									wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: wtp %d match! bMatchVersion is TRUE.no need upgrade\n", WTPIndex);
									bMatchVersion = CW_TRUE;
								} else {
									wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: not match for wtp %d, need upgrade\n", WTPIndex);
								}
							}
							else if((strlen(update_node->str_ap_version_name) == len)&&(strncasecmp(AC_WTP[WTPIndex]->codever,update_node->str_ap_version_name,len) == 0))
							{
								wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: str_ac_version=%s AC_WTP[%d]->codever=%s bMatchVersion is TRUE\n", 
									AC_WTP[WTPIndex]->ver, WTPIndex, AC_WTP[WTPIndex]->codever);
								str_ac_version = AC_WTP[WTPIndex]->ver;
								bMatchVersion = CW_TRUE;
							} else {
								wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: update_node->str_ap_code=%s AC_WTP[%d]->APCode=%s, not match\n", 
									update_node->str_ap_code, WTPIndex, AC_WTP[WTPIndex]->APCode);
							}
							result = 1;
							break;
						} else {
							wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: update_node->str_ap_code=%s AC_WTP[%d]->APCode=%s\n", 
								update_node->str_ap_code, WTPIndex, AC_WTP[WTPIndex]->APCode);	
						}
						update_node = update_node->next;
					}

					#if 0
					if (result == 1) {
						wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: gConfigVersionUpdateInfo search done\n");
						break;
					}
					#endif
					
					if(update_node == NULL){
						wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: str_ac_version for wtp %d is %s bMatchVersion is TRUE, no need to upgrade\n",
							WTPIndex, AC_WTP[WTPIndex]->ver);
						str_ac_version = AC_WTP[WTPIndex]->ver;
						bMatchVersion = CW_TRUE;
					} else {
						wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: update_node is not NULL str_ac_version for wtp %d is %s bMatchVersion is %s\n",
							WTPIndex, str_ac_version, bMatchVersion == CW_TRUE ? "CW_TRUE" : "CW_FALSE");
					}
					wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: search gConfigVersionUpdateInfo done i=%d\n", i);
					break;
				} else {
					wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: %d update_node->str_ap_model=%s AC_WTP[%d]->WTPModel=%s\n", 
						i, update_node->str_ap_model, WTPIndex, AC_WTP[WTPIndex]->WTPModel);
				}
			}
		} else {
			//auto upgrade brunch
			int do_check = 0;
			wid_syslog_debug_debug(WID_WTPINFO,"**** ap model is %s ****\n",AC_WTP[WTPIndex]->WTPModel);	//for test
			
//			CWThreadMutexLock(&(gAllThreadMutex));
			pVersionNode = gConfigVersionInfo;
			tmpnode = gConfigVerInfo;
			
			if(img_now == 0) {
				wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: auto upgrade not start str_ac_version=%s bMatchVersion == CW_TRUE\n", 
					AC_WTP[WTPIndex]->ver);
				str_ac_version = AC_WTP[WTPIndex]->ver;
				bMatchVersion = CW_TRUE;
			} else {
				wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: auto upgrade started, search gConfigVersionInfo for wtp %d\n", WTPIndex);

				while(pVersionNode != NULL) {
					str_ac_version = AC_WTP[WTPIndex]->ver;
					if (pVersionNode->str_ap_model != NULL)
						wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: pVersionNode->str_ap_model=%s AC_WTP[%d]->WTPModel=%s\n", 
							pVersionNode->str_ap_model, WTPIndex, AC_WTP[WTPIndex]->WTPModel);
					else {
						wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: pVersionNode->str_ap_model is NULL\n");
						pVersionNode = pVersionNode->next;
						continue;
					}
					
					if(strcmp(pVersionNode->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0){	

						wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: match model for wtp %d \n", WTPIndex);
							
						while(tmpnode != NULL) {
							if (tmpnode->str_ap_model != NULL)
								wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: tmpnode->str_ap_model=%s AC_WTP[%d]->WTPModel=%s\n", 
									tmpnode->str_ap_model, WTPIndex, AC_WTP[WTPIndex]->WTPModel);
							else {
								wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: tmpnode->str_ap_model is NULL\n");
								tmpnode = tmpnode->next;
								continue;
							}

							//model match
							if(strcmp(tmpnode->str_ap_model,AC_WTP[WTPIndex]->WTPModel) == 0){
								wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: match model for wtp %d\n", WTPIndex);
								CWCodeInfo *codenode = tmpnode->code_info;
								
								while(codenode != NULL) {
									
									if (codenode->str_ap_version_code) 
										wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: codenode->str_ap_version_code=%s AC_WTP[%d]->APCode=%s\n", 
											codenode->str_ap_version_code, WTPIndex, AC_WTP[WTPIndex]->APCode);
									else {
										wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: codenode->str_ap_version_code is NULL\n");
										codenode = codenode->next;
										continue;
									}

									//code match
									if(strcmp(codenode->str_ap_version_code,AC_WTP[WTPIndex]->APCode) == 0) {
										do_check = 1;
										str_ac_version = codenode->str_ap_version_name;
										aclen = strlen(codenode->str_ap_version_name);
										
									wid_syslog_debug_debug(WID_WTPINFO,"ac surport version:%s\n",str_ac_version);
									wid_syslog_debug_debug(WID_WTPINFO,"** AC version name len:%d   WTP version name len:%d **\n",aclen,len);
									
									if((AC_WTP[WTPIndex]->codever == NULL)){						
										if((strlen(codenode->str_ap_version_name) == verlen)&&(strncasecmp(AC_WTP[WTPIndex]->ver,codenode->str_ap_version_name,verlen) == 0))
										{
											wid_syslog_debug_debug(WID_WTPINFO,"ap model match 222\n"); 	//for test
											bMatchVersion = CW_TRUE;
											break;
										}
									}
									else if((aclen == len)&&(AC_WTP[WTPIndex]->codever != NULL)&&(strncasecmp(AC_WTP[WTPIndex]->codever,codenode->str_ap_version_name,len) == 0))
									{
										wid_syslog_debug_debug(WID_WTPINFO,"ap model match 333\n"); 	//for test
											bMatchVersion = CW_TRUE;
											str_ac_version = AC_WTP[WTPIndex]->ver;
											wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: B2 wtp %d str_ac_version=%s match\n", 
												WTPIndex, str_ac_version);
											break;
										} else {
											wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: match is %s\n", 
												bMatchVersion == CW_TRUE ? "CW_TRUE" : "CW_FALSE");
										}
										break;
									} else {
										wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: mismatch is %s\n", 
												bMatchVersion == CW_TRUE ? "CW_TRUE" : "CW_FALSE");
									}
									codenode = codenode->next;
								}
							}
							
							if(do_check == 1)
								break;
							tmpnode = tmpnode->next;
						}
						
						if(do_check == 0){
							bMatchVersion = CW_TRUE;
							str_ac_version = AC_WTP[WTPIndex]->ver; 		
							wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: A wtp %d str_ac_version=%s bMatchVersion == CW_TRUE\n", WTPIndex, str_ac_version);
						} else {
							wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: wtp %d bMatchVersion=%s\n", 
								WTPIndex, bMatchVersion == CW_TRUE ? "CW_TRUE" : "CW_FALSE");
						}
						break;
					} else {
						wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: pVersionNode->str_ap_model=%s AC_WTP[%d]->WTPModel=%s not match\n", 
							pVersionNode->str_ap_model, WTPIndex, AC_WTP[WTPIndex]->WTPModel);
					}
					pVersionNode = pVersionNode->next;
				}
				
				if (pVersionNode) {
					wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: search gConfigVersionInfo for wtp %d done and found the match ap model\n", 
						WTPIndex);
				} else {
					wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: search gConfigVersionInfo for wtp %d done and not found the match ap model\n", 
						WTPIndex);
				}
			}
			
			if (do_check == 0){
				bMatchVersion = CW_TRUE;
				str_ac_version = AC_WTP[WTPIndex]->ver;
				wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: B wtp %d str_ac_version=%s bMatchVersion == CW_TRUE\n", WTPIndex, str_ac_version);
			} else {
				wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: B do_check==1 for wtp %d\n", WTPIndex);
			}
		}
	}

	CWProtocolJoinResponseValues joinResponseVal;
	memset(&joinResponseVal, 0, sizeof(joinResponseVal));
	if(bMatchVersion){
		wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: A wtp %d str_ac_version=%s, no need to upgrade\n", 
			WTPIndex, str_ac_version);
		if(!(CWAssembleJoinResponse(&(gWTPs[WTPIndex].messages), &(gWTPs[WTPIndex].messagesCount), gWTPs[WTPIndex].pathMTU, seqNum, msgElemList,str_ac_version))){ // random session ID
			CWDeleteList(&msgElemList, CWProtocolDestroyMsgElemData);
			wid_syslog_err("wtp_upgrade: exception return %s %d\n", __FUNCTION__, __LINE__);
			return CW_FALSE;
		}
	}else{
		char ac_version[DEFAULT_LEN] = {" "};
		memset(ac_version,0,DEFAULT_LEN);
		
		if ((WTP_UPGRADE_MODE_FTP == upgrade_mode)
			|| (WTP_UPGRADE_MODE_TFTP == upgrade_mode))
		{
			sprintf(ac_version,"%s1",str_ac_version);
		}
		else if (WTP_UPGRADE_MODE_CAPWAP == upgrade_mode)
		{
			sprintf(ac_version,"%s1",str_ac_version);
			
			wtp_upgrade_init_hash(WTPIndex, NULL, str_ac_version);
			strncpy(joinResponseVal.ImageIndentifier.version, ac_version, WTP_VERSION_LEN-1);			
			joinResponseVal.ImageIndentifier.VersionLen = strlen(joinResponseVal.ImageIndentifier.version);

			/* TODO */
			snprintf(joinResponseVal.ImageIndentifier.ImageName, WTP_IMAGENAME_LEN, "%s.img", "wtp");			
			joinResponseVal.ImageIndentifier.ImageNameLen = strlen(joinResponseVal.ImageIndentifier.ImageName);

			snprintf(joinResponseVal.ImageIndentifier.Model, WTP_MODE_LEN, "%s", str_wtp_model);			
			joinResponseVal.ImageIndentifier.ModelLen = strlen(joinResponseVal.ImageIndentifier.Model);
			
		}
		else if(WTP_UPGRADE_MODE_NONE == upgrade_mode)
		{
			sprintf(ac_version,"%s1",str_ac_version);
		}
		else
		{
			wid_syslog_err("wtp%d upgrade mode error %d\n",WTPIndex, AC_WTP[WTPIndex]->upgrade.mode);
		}
		
		wid_syslog_info("wtp%d upgrade mode %d version %s model %s\n",WTPIndex, AC_WTP[WTPIndex]->upgrade.mode, ac_version, str_wtp_model);
		if(!(CWAssembleJoinResponse(&(gWTPs[WTPIndex].messages), &(gWTPs[WTPIndex].messagesCount),
				gWTPs[WTPIndex].pathMTU, seqNum, msgElemList,ac_version, &joinResponseVal, upgrade_mode)))
		{ // random session ID
			CWDeleteList(&msgElemList, CWProtocolDestroyMsgElemData);
			wid_syslog_err("wtp_upgrade: exception return %s %d\n", __FUNCTION__, __LINE__);
			return CW_FALSE;
		}
		AC_WTP[WTPIndex]->upgrade.state = WTP_UPGRADE_STATE_JOIN;
	}
	CWDeleteList(&msgElemList, CWProtocolDestroyMsgElemData);
	
	if(!CWACSendFragments(WTPIndex)) {
		wid_syslog_err("wtp_upgrade: exception return %s %d\n", __FUNCTION__, __LINE__);
		return CW_FALSE;
 	}
	for(i = 0; i < gWTPs[WTPIndex].messagesCount; i++) {
		CW_FREE_PROTOCOL_MESSAGE(gWTPs[WTPIndex].messages[i]);
	}
	
	CW_FREE_OBJECT_WID(gWTPs[WTPIndex].messages);
	if(!CWErr(CWTimerRequest(CW_JOIN_INTERVAL_DEFAULT, &(gWTPs[WTPIndex].thread), &(gWTPs[WTPIndex].currentTimer), CW_CRITICAL_TIMER_EXPIRED_SIGNAL,WTPIndex))) { // start Change State Pending timer
		wid_syslog_err("wtp_upgrade: quit wtp %d %s %d\n", WTPIndex, __FUNCTION__, __LINE__);
		_CWCloseThread(WTPIndex);
	}

	if(bMatchVersion)
	{
		wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: no need to upgrade for wtp %d,enter into configure state\n", 
			WTPIndex);
		gWTPs[WTPIndex].currentState = CW_ENTER_CONFIGURE;	
		AC_WTP[WTPIndex]->WTPStat = 3;
	}
	else
	{
	
		if((updatewtplist != NULL)&&(find_in_wtp_list(WTPIndex) == CW_FALSE))
		{
			if(updatewtplist->count >= gupdateCountOneTime)
			{
				wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: wtp %d need to upgrade,but updatewtplist->count=%d is not lower than gupdateCountOneTime=%d,so enter into quite state\n", 
					WTPIndex, updatewtplist->count, gupdateCountOneTime);
				gWTPs[WTPIndex].currentState = CW_QUIT; 
				AC_WTP[WTPIndex]->WTPStat = 7;	
				_CWCloseThread(WTPIndex);
			}
			else
			{
				wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: A wtp %d need to upgrade enter into image data state \n", 
					WTPIndex);
				gWTPs[WTPIndex].currentState = CW_ENTER_IMAGE_DATA; 
				AC_WTP[WTPIndex]->WTPStat = 8;
			}
		}
		else
		{
			wid_syslog_debug_debug(WID_WTPINFO, "wtp_upgrade: B wtp %d need to upgrade enter into image data state \n", 
					WTPIndex);
			gWTPs[WTPIndex].currentState = CW_ENTER_IMAGE_DATA; 
			AC_WTP[WTPIndex]->WTPStat = 8;
		}
	}
	AC_WTP[WTPIndex]->quitreason = WTP_NORMAL;
	wid_syslog_debug_debug(WID_WTPINFO,"** match result is:%d Next state is:%d **\n",bMatchVersion,gWTPs[WTPIndex].currentState);
	//wid_syslog_debug_info("** WTP %d quit reason is %d **",WTPIndex,AC_WTP[WTPIndex]->quitreason);
	return CW_TRUE;
}

// assemble Join Response
CWBool CWAssembleJoinResponse
(
	CWProtocolMessage **messagesPtr,
	int *fragmentsNumPtr,
	int PMTU,
	int seqNum,
	CWList msgElemList,
	char *strVersion,
	CWProtocolJoinResponseValues *joinResponseVal,
	enum wtp_upgrade_mode upgrade_mode
)
{	
	CWProtocolMessage *msgElems= NULL;
	int msgElemCount = 0;
	 int mandatoryMsgElemCount=4; 	//Result code is not included because it's already in msgElemList. Control IPv6 to be added
	CWProtocolMessage *msgElemsBinding= NULL;
	const int msgElemBindingCount=0;
	int i;
	CWListElement *current;
	int k = -1;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL || msgElemList == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	msgElemCount = CWCountElementInList(msgElemList);
	
//	wid_syslog_debug_debug(WID_WTPINFO,"upgrade_mode %d\n", upgrade_mode);
	if (WTP_UPGRADE_MODE_CAPWAP == upgrade_mode)
	{
		mandatoryMsgElemCount++;
		
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount+mandatoryMsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		wid_syslog_debug_debug(WID_WTPINFO,"Assembling Join Response elem cunt %d\n", mandatoryMsgElemCount);
		
		
		if((!(CWAssembleMsgElemACDescriptor(&(msgElems[++k]))))
			|| (!(CWAssembleMsgElemACName(&(msgElems[++k]))))
			|| (!(CWAssembleMsgElemWTPUpgradeMode(&(msgElems[++k]), upgrade_mode)))
			|| (!(CWAssembleMsgElemImageIdentifier(&(msgElems[++k]), &(joinResponseVal->ImageIndentifier))))
			|| (!(CWAssembleMsgElemCWControlIPv4Addresses(&(msgElems[++k]),0)))	//zhanglei set wtpid 0 in join	  
			)
		{
			CWErrorHandleLast();
			int i;
			for(i = 0; i <= k; i++) {CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT(msgElems);
			return CW_FALSE; // error will be handled by the caller
		}		
	}
	else if(WTP_UPGRADE_MODE_FTP == upgrade_mode
		   ||WTP_UPGRADE_MODE_TFTP == upgrade_mode)
	{
		mandatoryMsgElemCount++;
		
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount+mandatoryMsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		wid_syslog_debug_debug(WID_WTPINFO,"Assembling Join Response...");
		
		if((!(CWAssembleMsgElemACDescriptor(&(msgElems[++k])))) ||
		   (!(CWAssembleMsgElemACName(&(msgElems[++k])))) ||
		   (!(CWAssembleMsgElemWTPUpgradeMode(&(msgElems[++k]), upgrade_mode))) ||
		   (!(CWAssembleMsgElemWTPVersion(&(msgElems[++k]),strVersion))) || //added by weiay 20080618
		   (!(CWAssembleMsgElemCWControlIPv4Addresses(&(msgElems[++k]),0)))//zhanglei set wtpid 0 in join	  
			)
		{
			CWErrorHandleLast();
			int i;
			for(i = 0; i <= k; i++) {CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT(msgElems);
			return CW_FALSE; // error will be handled by the caller
		} 
	}
	else if(WTP_UPGRADE_MODE_NONE == upgrade_mode)
	{
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount+mandatoryMsgElemCount, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		wid_syslog_debug_debug(WID_WTPINFO,"Assembling Join Response...");
		
		if((!(CWAssembleMsgElemACDescriptor(&(msgElems[++k])))) ||
		   (!(CWAssembleMsgElemACName(&(msgElems[++k])))) ||
		   (!(CWAssembleMsgElemWTPVersion(&(msgElems[++k]),strVersion))) ||
		   (!(CWAssembleMsgElemCWControlIPv4Addresses(&(msgElems[++k]),0)))	  
		){
			CWErrorHandleLast();
			int i;
			for(i = 0; i <= k; i++) {CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
			CW_FREE_OBJECT(msgElems);
			return CW_FALSE; // error will be handled by the caller
		} 
	}

	current=msgElemList;
	for (i=0; i<msgElemCount; i++)
	{
                switch (((CWMsgElemData *) (current->data))->type)
		{
			case CW_MSG_ELEMENT_AC_IPV4_LIST_CW_TYPE:
				if (!(CWAssembleMsgElemACIPv4List(&(msgElems[++k]))))
					goto cw_assemble_error;	
				break;			
			case CW_MSG_ELEMENT_AC_IPV6_LIST_CW_TYPE:
				if (!(CWAssembleMsgElemACIPv6List(&(msgElems[++k]))))
					goto cw_assemble_error;
				break;
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
				if (!(CWAssembleMsgElemResultCode(&(msgElems[++k]), ((CWMsgElemData *) current->data)->value)))
					goto cw_assemble_error;
				break;
/*			case CW_MSG_ELEMENT_SESSION_ID_CW_TYPE:
				if (!(CWAssembleMsgElemSessionID(&(msgElems[++k]), ((CWMsgElemData *) current->data)->value)))
					goto cw_assemble_error;
				break;*/
                        default: {
                                int j;
                                for(j = 0; j <= k; j++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[j]);}
                                CW_FREE_OBJECT_WID(msgElems);
                                return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element for Join Response Message");
				break;
		        }
                }

		current = current->next;
	}

	if (!(CWAssembleMessage(messagesPtr, fragmentsNumPtr, PMTU, seqNum, CW_MSG_TYPE_VALUE_JOIN_RESPONSE, msgElems, msgElemCount+mandatoryMsgElemCount, msgElemsBinding, msgElemBindingCount, CW_PACKET_CRYPT)))
		return CW_FALSE;

	wid_syslog_debug_debug(WID_WTPINFO,"Join Response Assembled");
	
	return CW_TRUE;

cw_assemble_error:
	{
		int i;
		for(i = 0; i <= k; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		CW_FREE_OBJECT_WID(msgElems);
		return CW_FALSE; // error will be handled by the caller
	}

	return CW_TRUE;
}

// Parse Join Request
CWBool CWParseJoinRequestMessage(char *msg, int len, int *seqNumPtr, CWProtocolJoinRequestValues *valuesPtr, int WTPIndex) {
	CWControlHeaderValues controlVal;
	
	int offsetTillMessages;
	//int i=0;
	CWProtocolMessage completeMsg;
	
	if(msg == NULL || seqNumPtr == NULL || valuesPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	wid_syslog_debug_debug(WID_WTPINFO,"Parse Join Request");
	
	completeMsg.msg = msg;
	completeMsg.offset = 0;
		
	if(!(CWParseControlHeader(&completeMsg, &controlVal))) return CW_FALSE; // will be handled by the caller

	// different type
	if(controlVal.messageTypeValue != CW_MSG_TYPE_VALUE_JOIN_REQUEST)
		return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message is not Join Request as Expected");
	
	*seqNumPtr = controlVal.seqNum;
	controlVal.msgElemsLen -= CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS; // skip timestamp
	
	offsetTillMessages = completeMsg.offset;
	
	// parse message elements
	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) {
		unsigned short int elemType=0;// = CWProtocolRetrieve32(&completeMsg);
		unsigned short int elemLen=0;// = CWProtocolRetrieve16(&completeMsg);
		
		CWParseFormatMsgElem(&completeMsg,&elemType,&elemLen);
		
		//wid_syslog_debug_debug("Parsing Message Element: %u, elemLen: %u", elemType, elemLen);
									
		switch(elemType) {
			case CW_MSG_ELEMENT_LOCATION_DATA_CW_TYPE:
				if(!(CWParseLocationData(&completeMsg, elemLen, &(valuesPtr->location)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_BOARD_DATA_CW_TYPE:{
				if(!(CWParseWTPBoardData(&completeMsg, elemLen, &(valuesPtr->WTPBoardData)))) return CW_FALSE; // will be handled by the caller
				break; 

			}
			case CW_MSG_ELEMENT_SESSION_ID_CW_TYPE:
				if(!(CWParseSessionID(&completeMsg, elemLen, valuesPtr))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_DESCRIPTOR_CW_TYPE:
				if(!(CWParseWTPDescriptor(&completeMsg, elemLen, &(valuesPtr->WTPDescriptor)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_IPV4_ADDRESS_CW_TYPE:
				if(!(CWParseWTPIPv4Address(&completeMsg, elemLen, valuesPtr))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_NAME_CW_TYPE:
				if(!(CWParseWTPName(&completeMsg, elemLen, &(valuesPtr->name)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_FRAME_TUNNEL_MODE_CW_TYPE:
				if(!(CWParseWTPFrameTunnelMode(&completeMsg, elemLen, &(valuesPtr->frameTunnelMode)))) return CW_FALSE; // will be handled by the caller
				break;
			case CW_MSG_ELEMENT_WTP_MAC_TYPE_CW_TYPE:
				if(!(CWParseWTPMACType(&completeMsg, elemLen, &(valuesPtr->MACType)))) return CW_FALSE; // will be handled by the caller
				break;
			default:
				return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Unrecognized Message Element");
		}
		
		//wid_syslog_debug_debug("bytes: %d/%d", (completeMsg.offset-offsetTillMessages), controlVal.msgElemsLen);
	}
	
	if(completeMsg.offset != len) return CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Garbage at the End of the Message");
		
	return CW_TRUE;
}

CWBool CWSaveJoinRequestMessage (CWProtocolJoinRequestValues *joinRequest, CWWTPProtocolManager *WTPProtocolManager, unsigned int WTPIndex)
{  
	wid_syslog_debug_debug(WID_WTPINFO,"Saving Join Request...");
	int i;
	if(joinRequest == NULL || WTPProtocolManager == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if ((joinRequest->location)!= NULL)
	{
		CW_FREE_OBJECT_WID(WTPProtocolManager->locationData);
		WTPProtocolManager->locationData= joinRequest->location;
	}
	else return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if ((joinRequest->name)!= NULL)
	{
		CW_FREE_OBJECT_WID(WTPProtocolManager->name);
		WTPProtocolManager->name= joinRequest->name;
	}
	else return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	for(i = 0; i < WTPProtocolManager->WTPBoardData.vendorInfosCount; i++) {

		if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_MODEL_NUMBER)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].model);
		}else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].SN);
		}else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].mac);
		}else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER)
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].Rmodel);
		}
		else if((WTPProtocolManager->WTPBoardData.vendorInfos)[i].type == CW_WTP_CODE_VERSION)
		{
			AC_WTP[WTPIndex]->codever = NULL;
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].codever);
		}
		else
		{
			CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData.vendorInfos)[i].valuePtr);
		}
	}
	CW_FREE_OBJECT_WID((WTPProtocolManager->WTPBoardData).vendorInfos);
	WTPProtocolManager->WTPBoardData = joinRequest->WTPBoardData;

	WTPProtocolManager->sessionID= joinRequest->sessionID;
	WTPProtocolManager->ipv4Address= joinRequest->addr;
	
	for(i = 0; i < (WTPProtocolManager->descriptor.vendorInfos).vendorInfosCount; i++) {

		if(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_HARDWARE_VERSION)
		{
			CW_FREE_OBJECT_WID(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].sysver);
		}else if(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_SOFTWARE_VERSION)
		{
			AC_WTP[WTPIndex]->ver = NULL;
			CW_FREE_OBJECT_WID(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].ver);
		}
		else if(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].type == CW_BOOT_VERSION)
		{
			CW_FREE_OBJECT(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].valuePtr);
		}
		else
		{
			CW_FREE_OBJECT_WID(((WTPProtocolManager->descriptor.vendorInfos).vendorInfos)[i].valuePtr);
		}
	}
	CW_FREE_OBJECT_WID((WTPProtocolManager->descriptor.vendorInfos).vendorInfos);
	WTPProtocolManager->descriptor= joinRequest->WTPDescriptor;
	WTPProtocolManager->radiosInfo.radioCount = (joinRequest->WTPDescriptor).radiosInUse;
	CW_FREE_OBJECT_WID(WTPProtocolManager->radiosInfo.radiosInfo);
	CW_CREATE_ARRAY_ERR(WTPProtocolManager->radiosInfo.radiosInfo, WTPProtocolManager->radiosInfo.radioCount, CWWTPRadioInfoValues,return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	

	for(i=0; i< WTPProtocolManager->radiosInfo.radioCount; i++)
	{
		WTPProtocolManager->radiosInfo.radiosInfo[i].radioID = i;
                //WTPProtocolManager->radiosInfo.radiosInfo[i].stationCount = 0;
                WTPProtocolManager->radiosInfo.radiosInfo[i].adminState = ENABLED; //default value for CAPWAP
                WTPProtocolManager->radiosInfo.radiosInfo[i].adminCause = AD_NORMAL;
                WTPProtocolManager->radiosInfo.radiosInfo[i].operationalState = DISABLED;
                WTPProtocolManager->radiosInfo.radiosInfo[i].operationalCause = OP_NORMAL;
                WTPProtocolManager->radiosInfo.radiosInfo[i].TxQueueLevel = 0;
                WTPProtocolManager->radiosInfo.radiosInfo[i].wirelessLinkFramesPerSec = 0; 
	}
	wid_syslog_debug_debug(WID_WTPINFO,"Join Request Saved");
	return CW_TRUE;
}
void CWDestroyJoinRequestValues(CWProtocolJoinRequestValues *valPtr) 
{
	int i = 0;
	
	if(NULL == valPtr) 
	{
		return;
	}
	
	CW_FREE_OBJECT(valPtr->location);
	CW_FREE_OBJECT(valPtr->name);
	if(NULL != (valPtr->WTPDescriptor.vendorInfos).vendorInfos)
	{
		for(i = 0; i < (valPtr->WTPDescriptor.vendorInfos).vendorInfosCount; i++)
		{
			if(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_HARDWARE_VERSION)
			{
				CW_FREE_OBJECT(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].sysver);
			}else if(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].type == CW_WTP_SOFTWARE_VERSION)
			{
				CW_FREE_OBJECT(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].ver);
			}
			else if(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].type == CW_BOOT_VERSION)
			{
				CW_FREE_OBJECT(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].valuePtr);
			}
			else
			{
				CW_FREE_OBJECT(((valPtr->WTPDescriptor.vendorInfos).vendorInfos)[i].valuePtr);
			}
		}		
		CW_FREE_OBJECT((valPtr->WTPDescriptor.vendorInfos).vendorInfos);
	}

	if(NULL != valPtr->WTPBoardData.vendorInfos)
	{
		for(i = 0; i < valPtr->WTPBoardData.vendorInfosCount; i++) 
		{
			if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_MODEL_NUMBER)
			{
				CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].model);
			}else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER)
			{
				CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].SN);
			}else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS)
			{
				CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].mac);
			}else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER)
			{
				CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].Rmodel);
			}else if((valPtr->WTPBoardData.vendorInfos)[i].type == CW_WTP_CODE_VERSION)
			{
				CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].codever);
			}
			else
			{
				CW_FREE_OBJECT((valPtr->WTPBoardData.vendorInfos)[i].valuePtr);
			}
		}		
		CW_FREE_OBJECT(valPtr->WTPBoardData.vendorInfos);
	}

}

CWBool CWAssembleMsgElemImageIdentifier
(
	CWProtocolMessage *msgPtr, 
	CWImageIdentifier_cw *ImageIdentifier
)
{
	short length = 0;  

	if ((NULL == msgPtr) || (NULL == ImageIdentifier))
	{
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	} 

	length = 4 + ((4 + ImageIdentifier->ImageNameLen)
			   + (4 + ImageIdentifier->VersionLen)
			   + (4 + ImageIdentifier->ModelLen));
	
	wid_syslog_debug_debug(WID_WTPINFO,"%s image %s len %d version %s %d model %s len %d\n",
						__func__, 
						ImageIdentifier->ImageName, ImageIdentifier->ImageNameLen,
						ImageIdentifier->version, ImageIdentifier->VersionLen, 
						ImageIdentifier->Model, ImageIdentifier->ModelLen);
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

//	wid_syslog_debug_debug(WID_WTPINFO,"%s:%d\n", __func__, __LINE__);

	
	CWProtocolStore32(msgPtr, CW_MSG_ELEMENT_VENDOR_IDENTIFIER);
 
	CWProtocolStore16(msgPtr, CW_IMAGEIDENTIFIER_IMAGENAME);
	CWProtocolStore16(msgPtr, ImageIdentifier->ImageNameLen);
	CWProtocolStoreStr(msgPtr, ImageIdentifier->ImageName);
 
	CWProtocolStore16(msgPtr, CW_IMAGEIDENTIFIER_VERSION);
	CWProtocolStore16(msgPtr, ImageIdentifier->VersionLen);
	CWProtocolStoreStr(msgPtr, ImageIdentifier->version);

	CWProtocolStore16(msgPtr, CW_IMAGEIDENTIFIER_MODE);
	CWProtocolStore16(msgPtr, ImageIdentifier->ModelLen);
	CWProtocolStoreStr(msgPtr, ImageIdentifier->Model);

//	wid_syslog_debug_debug(WID_WTPINFO,"%s:%d\n", __func__, __LINE__);

	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE);
}

unsigned long int HexToDec(unsigned char *hex)
{
	char *tmp=(char *)hex;
	unsigned int dec=0;
	//int m = 0;
	//printf("tmp:%p\n");
	while (*tmp)
	{
		//m++;
		//printf("$m:%d$",m);
		dec<<=4;
		//printf("$dec:%d$",dec);
		if (*tmp&16) dec+=*tmp++&15;
		else dec+=(*tmp++&15)+9;
	}
	//printf("dec:%d\n",dec);
	return dec;
}

unsigned int get_file_size(const char *path)
{
	unsigned int size = 0;
	FILE *fp = NULL;

	fp = fopen(path, "rb");
	if(NULL == fp)
	{
		wid_syslog_err("%s Open file %s failed.", path, strerror(errno));
		return 0;
	}

	fseek(fp, SEEK_SET, SEEK_END);
	size = ftell(fp);
	fclose(fp);
	
	wid_syslog_debug_debug(WID_DEFAULT, "Size of file %s size %d bytes\n", path, size);
	
	return size;
}

int md5_hash(char *path, unsigned char *md5)
{
	char cmd[256] = {0};
	char buf[256] = {0};
	FILE *fp = 	NULL;

	snprintf(cmd, sizeof(cmd), "sor.sh imgmd5 %s 120", path);
	wid_syslog_debug("%s %s\n", __func__, cmd);

	fp = popen(cmd, "r");
	if (NULL == fp)
	{
		wid_syslog_err("%s popen %s failed %s\n", __func__, path, strerror(errno));
		return -1;
	}	
	fgets(buf, sizeof(buf)-1, fp);	
	pclose(fp);
	fp = NULL;

	char *p = NULL;
	p = strstr(buf, "= ");
	if (NULL == p)
	{
		wid_syslog_err("%s get md5 string error\n", __func__);
		return -1;
	}
	p +=2;	/* stip string "= " */

	unsigned char a[3];
	int i = 0;
	unsigned long ul = 0;
	
	memset(md5, 0, WID_MD5_LEN);
	
	for(i = 0; i < 32; i = i + 2)
	{
		memset(a, 0, sizeof(a));
		strncpy((char *)a, p+i, 2);
		ul = HexToDec(a);
		md5[i/2] = ul;
	}
	
	wid_syslog_debug("%02x %02x %02x %02x %02x %02x %02x %02x\n",
		md5[0], md5[1], md5[2], md5[3], md5[4], md5[5], md5[6], md5[7]);

	wid_syslog_debug("%02x %02x %02x %02x %02x %02x %02x %02x\n",
		md5[8], md5[9], md5[10], md5[11], md5[12], md5[13], md5[14], md5[15]);

	#if 0
	ret = pclose(fp);
	
	switch (WEXITSTATUS(ret)) 
	{
		case 0:
			wid_syslog_info("%s,%s\n",__func__, md5);
			return 0;
		case 1:
			wid_syslog_info("Sysetm internal error (1).\n");
			break;
		case 2:
			wid_syslog_info("Sysetm internal error (2).\n");
			break;
		case 3:
			wid_syslog_info("Storage media is busy.\n");
			break;
		case 4:
			wid_syslog_info("Storage operation time out.\n");
			break;
		case 5:
			wid_syslog_info("No left space on storage media.\n");
			break;
		default:
			wid_syslog_info("Sysetm internal error (3).\n");
			break;
		}
	#endif
	
	return 0;
}

int wid_md5_hash(char *path, unsigned char *md5)
{
	unsigned char buf[WID_MD5_LEN] = {0};

	if (md5_hash(path, buf) < 0)
	{
		return -1;
	}

	memcpy(md5, buf, WID_MD5_LEN);
	
	return 0;
}


int wtp_upgrade_init_hash(unsigned int wtpid, char *ImageName, char *version)
{
	char path[128] = {0};
	
	/* init upgrade image name */
	if (NULL != version)
	{
		strncpy(AC_WTP[wtpid]->upgrade.version, version, WTP_VERSION_LEN-1);
	}

	if (NULL != ImageName)
	{
		strncpy(AC_WTP[wtpid]->upgrade.ImageName, ImageName, WTP_IMAGENAME_LEN-1);
		
		snprintf(path, sizeof(path), "/mnt/wtp/%s", ImageName);
		
		AC_WTP[wtpid]->upgrade.filesize = get_file_size(path);
		wid_md5_hash(path, AC_WTP[wtpid]->upgrade.hash);
		AC_WTP[wtpid]->upgrade.state = WTP_UPGRADE_STATE_INITMD5;
		
		wid_syslog_debug("wtp%d init md5 hash image %s size %d\n", 
			wtpid, ImageName, AC_WTP[wtpid]->upgrade.filesize);
	}

	return 0;	
}


