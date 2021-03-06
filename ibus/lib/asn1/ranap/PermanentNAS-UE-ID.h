/*
 * Generated by asn1c-0.9.23 (http://lionet.info/asn1c)
 * From ASN.1 module "RANAP-IEs"
 * 	found in "RANAP-IEs.asn"
 */

#ifndef	_PermanentNAS_UE_ID_H_
#define	_PermanentNAS_UE_ID_H_


#include <asn_application.h>

/* Including external dependencies */
#include "IMSI.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum PermanentNAS_UE_ID_PR {
	PermanentNAS_UE_ID_PR_NOTHING,	/* No components present */
	PermanentNAS_UE_ID_PR_iMSI,
	/* Extensions may appear below */
	
} PermanentNAS_UE_ID_PR;

/* PermanentNAS-UE-ID */
typedef struct PermanentNAS_UE_ID {
	PermanentNAS_UE_ID_PR present;
	union PermanentNAS_UE_ID_u {
		IMSI_t	 iMSI;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PermanentNAS_UE_ID_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PermanentNAS_UE_ID;

#ifdef __cplusplus
}
#endif

#endif	/* _PermanentNAS_UE_ID_H_ */
#include <asn_internal.h>
