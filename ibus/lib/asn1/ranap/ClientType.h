/*
 * Generated by asn1c-0.9.23 (http://lionet.info/asn1c)
 * From ASN.1 module "RANAP-IEs"
 * 	found in "RANAP-IEs.asn"
 */

#ifndef	_ClientType_H_
#define	_ClientType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ENUMERATED.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ClientType {
	ClientType_emergency_Services	= 0,
	ClientType_value_Added_Services	= 1,
	ClientType_pLMN_Operator_Services	= 2,
	ClientType_lawful_Intercept_Services	= 3,
	ClientType_pLMN_Operator_Broadcast_Services	= 4,
	ClientType_pLMN_Operator_O_et_M	= 5,
	ClientType_pLMN_Operator_Anonymous_Statistics	= 6,
	ClientType_pLMN_Operator_Target_MS_Service_Support	= 7
	/*
	 * Enumeration is extensible
	 */
} e_ClientType;

/* ClientType */
typedef ENUMERATED_t	 ClientType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ClientType;
asn_struct_free_f ClientType_free;
asn_struct_print_f ClientType_print;
asn_constr_check_f ClientType_constraint;
ber_type_decoder_f ClientType_decode_ber;
der_type_encoder_f ClientType_encode_der;
xer_type_decoder_f ClientType_decode_xer;
xer_type_encoder_f ClientType_encode_xer;
per_type_decoder_f ClientType_decode_uper;
per_type_encoder_f ClientType_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _ClientType_H_ */
#include <asn_internal.h>
