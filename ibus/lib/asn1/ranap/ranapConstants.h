#ifndef __RANAP_CONSTANTS_H__
#define __RANAP_CONSTANTS_H__
#define id_RAB_Assignment	0
#define id_Iu_Release	1                   //disconnect
#define id_RelocationPreparation	2
#define id_RelocationResourceAllocation	3
#define id_RelocationCancel	4
#define id_SRNS_ContextTransfer	5
#define id_SecurityModeControl	6
#define id_DataVolumeReport	7
#define id_Reset	9                       //connectionless
#define id_RAB_ReleaseRequest	10
#define id_Iu_ReleaseRequest	11
#define id_RelocationDetect	12
#define id_RelocationComplete	13
#define id_Paging	14                      //connectionless
#define id_CommonID	15
#define id_CN_InvokeTrace	16
#define id_LocationReportingControl	17
#define id_LocationReport	18
#define id_InitialUE_Message	19          //connect
#define id_DirectTransfer	20
#define id_OverloadControl	21              //connectionless
#define id_ErrorIndication	22              //connection | connectionless
#define id_SRNS_DataForward	23
#define id_ForwardSRNS_Context	24
#define id_privateMessage	25
#define id_CN_DeactivateTrace	26
#define id_ResetResource	27              //connectionless
#define id_RANAP_Relocation	28
#define id_RAB_ModifyRequest	29
#define id_LocationRelatedData	30
#define id_InformationTransfer	31          //connectionless
#define id_UESpecificInformation	32
#define id_UplinkInformationExchange	33  //connectionless
#define id_DirectInformationTransfer	34  //connectionless
#define id_MBMSSessionStart	35
#define id_MBMSSessionUpdate	36
#define id_MBMSSessionStop	37
#define id_MBMSUELinking	38
#define id_MBMSRegistration	39              //connection | connectionless
#define id_MBMSCNDe_Registration_Procedure	40  //connectionless
#define id_MBMSRABEstablishmentIndication	41
#define id_MBMSRABRelease	42
#define id_enhancedRelocationComplete	43
#define id_enhancedRelocationCompleteConfirm	44
#define id_RANAPenhancedRelocation	45
#define id_SRVCCPreparation	46
#define maxPrivateIEs	65535
#define maxProtocolExtensions	65535
#define maxProtocolIEs	65535
#define maxNrOfDTs	15
#define maxNrOfErrors	256
#define maxNrOfIuSigConIds	250
#define maxNrOfPDPDirections	2
#define maxNrOfPoints	15
#define maxNrOfRABs	256
#define maxNrOfSeparateTrafficDirections	2
#define maxNrOfSRBs	8
#define maxNrOfVol	2
#define maxNrOfLevels	256
#define maxNrOfAltValues	16
#define maxNrOfPLMNsSN	32
#define maxNrOfLAs	65536
#define maxNrOfSNAs	65536
#define maxNrOfUEsToBeTraced	64
#define maxNrOfInterfaces	16
#define maxRAB_Subflows	7
#define maxRAB_SubflowCombination	64
#define maxSet	9
#define maxNrOfHSDSCHMACdFlows_1	7
#define maxnoofMulticastServicesPerUE	128
#define maxnoofMulticastServicesPerRNC	512
#define maxMBMSSA	256
#define maxMBMSRA	65536
#define maxNrOfEDCHMACdFlows_1	7
#define maxGANSSSet	9
#define maxNrOfCSGs	256
#define id_AreaIdentity	0
#define id_CN_DomainIndicator	3
#define id_Cause	4
#define id_ChosenEncryptionAlgorithm	5
#define id_ChosenIntegrityProtectionAlgorithm	6
#define id_ClassmarkInformation2	7
#define id_ClassmarkInformation3	8
#define id_CriticalityDiagnostics	9
#define id_DL_GTP_PDU_SequenceNumber	10
#define id_EncryptionInformation	11
#define id_IntegrityProtectionInformation	12
#define id_IuTransportAssociation	13
#define id_L3_Information	14
#define id_LAI	15
#define id_NAS_PDU	16
#define id_NonSearchingIndication	17
#define id_NumberOfSteps	18
#define id_OMC_ID	19
#define id_OldBSS_ToNewBSS_Information	20
#define id_PagingAreaID	21
#define id_PagingCause	22
#define id_PermanentNAS_UE_ID	23
#define id_RAB_ContextItem	24
#define id_RAB_ContextList	25
#define id_RAB_DataForwardingItem	26
#define id_RAB_DataForwardingItem_SRNS_CtxReq	27
#define id_RAB_DataForwardingList	28
#define id_RAB_DataForwardingList_SRNS_CtxReq	29
#define id_RAB_DataVolumeReportItem	30
#define id_RAB_DataVolumeReportList	31
#define id_RAB_DataVolumeReportRequestItem	32
#define id_RAB_DataVolumeReportRequestList	33
#define id_RAB_FailedItem	34
#define id_RAB_FailedList	35
#define id_RAB_ID	36
#define id_RAB_QueuedItem	37
#define id_RAB_QueuedList	38
#define id_RAB_ReleaseFailedList	39
#define id_RAB_ReleaseItem	40
#define id_RAB_ReleaseList	41
#define id_RAB_ReleasedItem	42
#define id_RAB_ReleasedList	43
#define id_RAB_ReleasedList_IuRelComp	44
#define id_RAB_RelocationReleaseItem	45
#define id_RAB_RelocationReleaseList	46
#define id_RAB_SetupItem_RelocReq	47
#define id_RAB_SetupItem_RelocReqAck	48
#define id_RAB_SetupList_RelocReq	49
#define id_RAB_SetupList_RelocReqAck	50
#define id_RAB_SetupOrModifiedItem	51
#define id_RAB_SetupOrModifiedList	52
#define id_RAB_SetupOrModifyItem	53
#define id_RAB_SetupOrModifyList	54
#define id_RAC	55
#define id_RelocationType	56
#define id_RequestType	57
#define id_SAI	58
#define id_SAPI	59
#define id_SourceID	60
#define id_Source_ToTarget_TransparentContainer	61
#define id_TargetID	62
#define id_Target_ToSource_TransparentContainer	63
#define id_TemporaryUE_ID	64
#define id_TraceReference	65
#define id_TraceType	66
#define id_TransportLayerAddress	67
#define id_TriggerID	68
#define id_UE_ID	69
#define id_UL_GTP_PDU_SequenceNumber	70
#define id_RAB_FailedtoReportItem	71
#define id_RAB_FailedtoReportList	72
#define id_KeyStatus	75
#define id_DRX_CycleLengthCoefficient	76
#define id_IuSigConIdList	77
#define id_IuSigConIdItem	78
#define id_IuSigConId	79
#define id_DirectTransferInformationItem_RANAP_RelocInf	80
#define id_DirectTransferInformationList_RANAP_RelocInf	81
#define id_RAB_ContextItem_RANAP_RelocInf	82
#define id_RAB_ContextList_RANAP_RelocInf	83
#define id_RAB_ContextFailedtoTransferItem	84
#define id_RAB_ContextFailedtoTransferList	85
#define id_GlobalRNC_ID	86
#define id_RAB_ReleasedItem_IuRelComp	87
#define id_MessageStructure	88
#define id_Alt_RAB_Parameters	89
#define id_Ass_RAB_Parameters	90
#define id_RAB_ModifyList	91
#define id_RAB_ModifyItem	92
#define id_TypeOfError	93
#define id_BroadcastAssistanceDataDecipheringKeys	94
#define id_LocationRelatedDataRequestType	95
#define id_GlobalCN_ID	96
#define id_LastKnownServiceArea	97
#define id_SRB_TrCH_Mapping	98
#define id_InterSystemInformation_TransparentContainer	99
#define id_NewBSS_To_OldBSS_Information	100
#define id_SourceRNC_PDCP_context_info	103
#define id_InformationTransferID	104
#define id_SNA_Access_Information	105
#define id_ProvidedData	106
#define id_GERAN_BSC_Container	107
#define id_GERAN_Classmark	108
#define id_GERAN_Iumode_RAB_Failed_RABAssgntResponse_Item	109
#define id_GERAN_Iumode_RAB_FailedList_RABAssgntResponse	110
#define id_VerticalAccuracyCode	111
#define id_ResponseTime	112
#define id_PositioningPriority	113
#define id_ClientType	114
#define id_LocationRelatedDataRequestTypeSpecificToGERANIuMode	115
#define id_SignallingIndication	116
#define id_hS_DSCH_MAC_d_Flow_ID	117
#define id_UESBI_Iu	118
#define id_PositionData	119
#define id_PositionDataSpecificToGERANIuMode	120
#define id_CellLoadInformationGroup	121
#define id_AccuracyFulfilmentIndicator	122
#define id_InformationTransferType	123
#define id_TraceRecordingSessionInformation	124
#define id_TracePropagationParameters	125
#define id_InterSystemInformationTransferType	126
#define id_SelectedPLMN_ID	127
#define id_RedirectionCompleted	128
#define id_RedirectionIndication	129
#define id_NAS_SequenceNumber	130
#define id_RejectCauseValue	131
#define id_APN	132
#define id_CNMBMSLinkingInformation	133
#define id_DeltaRAListofIdleModeUEs	134
#define id_FrequenceLayerConvergenceFlag	135
#define id_InformationExchangeID	136
#define id_InformationExchangeType	137
#define id_InformationRequested	138
#define id_InformationRequestType	139
#define id_IPMulticastAddress	140
#define id_JoinedMBMSBearerServicesList	141
#define id_LeftMBMSBearerServicesList	142
#define id_MBMSBearerServiceType	143
#define id_MBMSCNDe_Registration	144
#define id_MBMSServiceArea	145
#define id_MBMSSessionDuration	146
#define id_MBMSSessionIdentity	147
#define id_PDP_TypeInformation	148
#define id_RAB_Parameters	149
#define id_RAListofIdleModeUEs	150
#define id_MBMSRegistrationRequestType	151
#define id_SessionUpdateID	152
#define id_TMGI	153
#define id_TransportLayerInformation	154
#define id_UnsuccessfulLinkingList	155
#define id_MBMSLinkingInformation	156
#define id_MBMSSessionRepetitionNumber	157
#define id_AlternativeRABConfiguration	158
#define id_AlternativeRABConfigurationRequest	159
#define id_E_DCH_MAC_d_Flow_ID	160
#define id_SourceBSS_ToTargetBSS_TransparentContainer	161
#define id_TargetBSS_ToSourceBSS_TransparentContainer	162
#define id_TimeToMBMSDataTransfer	163
#define id_IncludeVelocity	164
#define id_VelocityEstimate	165
#define id_RedirectAttemptFlag	166
#define id_RAT_Type	167
#define id_PeriodicLocationInfo	168
#define id_MBMSCountingInformation	169
#define id_170_not_to_be_used_for_IE_ids	170
#define id_ExtendedRNC_ID	171
#define id_Alt_RAB_Parameter_ExtendedGuaranteedBitrateInf	172
#define id_Alt_RAB_Parameter_ExtendedMaxBitrateInf	173
#define id_Ass_RAB_Parameter_ExtendedGuaranteedBitrateList	174
#define id_Ass_RAB_Parameter_ExtendedMaxBitrateList	175
#define id_RAB_Parameter_ExtendedGuaranteedBitrateList	176
#define id_RAB_Parameter_ExtendedMaxBitrateList	177
#define id_Requested_RAB_Parameter_ExtendedMaxBitrateList	178
#define id_Requested_RAB_Parameter_ExtendedGuaranteedBitrateList	179
#define id_LAofIdleModeUEs	180
#define id_newLAListofIdleModeUEs	181
#define id_LAListwithNoIdleModeUEsAnyMore	182
#define id_183_not_to_be_used_for_IE_ids	183
#define id_GANSS_PositioningDataSet	184
#define id_RequestedGANSSAssistanceData	185
#define id_BroadcastGANSSAssistanceDataDecipheringKeys	186
#define id_d_RNTI_for_NoIuCSUP	187
#define id_RAB_SetupList_EnhancedRelocCompleteReq	188
#define id_RAB_SetupItem_EnhancedRelocCompleteReq	189
#define id_RAB_SetupList_EnhancedRelocCompleteRes	190
#define id_RAB_SetupItem_EnhancedRelocCompleteRes	191
#define id_RAB_SetupList_EnhRelocInfoReq	192
#define id_RAB_SetupItem_EnhRelocInfoReq	193
#define id_RAB_SetupList_EnhRelocInfoRes	194
#define id_RAB_SetupItem_EnhRelocInfoRes	195
#define id_OldIuSigConId	196
#define id_RAB_FailedList_EnhRelocInfoRes	197
#define id_RAB_FailedItem_EnhRelocInfoRes	198
#define id_Global_ENB_ID	199
#define id_UE_History_Information	200
#define id_MBMSSynchronisationInformation	201
#define id_SubscriberProfileIDforRFP	202
#define id_CSG_Id	203
#define id_OldIuSigConIdCS	204
#define id_OldIuSigConIdPS	205
#define id_GlobalCN_IDCS	206
#define id_GlobalCN_IDPS	207
#define id_SourceExtendedRNC_ID	208
#define id_RAB_ToBeReleasedItem_EnhancedRelocCompleteRes	209
#define id_RAB_ToBeReleasedList_EnhancedRelocCompleteRes	210
#define id_SourceRNC_ID	211
#define id_Relocation_TargetRNC_ID	212
#define id_Relocation_TargetExtendedRNC_ID	213
#define id_Alt_RAB_Parameter_SupportedGuaranteedBitrateInf	214
#define id_Alt_RAB_Parameter_SupportedMaxBitrateInf	215
#define id_Ass_RAB_Parameter_SupportedGuaranteedBitrateList	216
#define id_Ass_RAB_Parameter_SupportedMaxBitrateList	217
#define id_RAB_Parameter_SupportedGuaranteedBitrateList	218
#define id_RAB_Parameter_SupportedMaxBitrateList	219
#define id_Requested_RAB_Parameter_SupportedMaxBitrateList	220
#define id_Requested_RAB_Parameter_SupportedGuaranteedBitrateList	221
#define id_Relocation_SourceRNC_ID	222
#define id_Relocation_SourceExtendedRNC_ID	223
#define id_EncryptionKey	224
#define id_IntegrityProtectionKey	225
#define id_SRVCC_HO_Indication	226
#define id_SRVCC_Information	227
#define id_SRVCC_Operation_Possible	228
#define id_CSG_Id_List	229
#define id_PSRABtobeReplaced	230
#define id_E_UTRAN_Service_Handover	231
#define id_UE_AggregateMaximumBitRate	233
#define id_CSG_Membership_Status	234
#define id_Cell_Access_Mode	235
#define id_IP_Source_Address	236
#define id_CSFB_Information	237
#endif
