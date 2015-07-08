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
* portal_packet.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* portal_packet
*
*
*******************************************************************************/

#ifndef _PORTAL_PACKET_H
#define _PORTAL_PACKET_H

#include <stdint.h>
#include "session.h"

#define PORTAL_PACKET_MOBILE_HEADSIZE		16
#define PORTAL_PACKET_TELECOM_HEADSIZE		32
#define PORTAL_PACKET_SIZE			1024

#define PORTAL_AUTHLEN				16

#define MAX_ATTR_VALUE_LEN	253

#define PORTAL_PROTOCOL_MOBILE		0
#define PORTAL_PROTOCOL_TELECOM		1

#define PORTAL_TELECOM_USERNAME_LEN		32
#define PORTAL_MOBILE_ERRID_LEN			5

#define IPV4_VERSION	0x01
#define IPV6_VERSION	0x02

#define ERRID_AC101		"AC101"		/*IP��ַ�Ǳ�AC�����ĵ�ַ���е���Ч��ַ���������ַĿǰ�Ѿ�ͨ����֤����*/
#define ERRID_AC102		"AC102"		/*AC�����û�������֤��IP��ַ���Ǳ�AC�����ĵ�ַ���е���Ч��ַ*/
#define ERRID_AC103		"AC103"		/*AC�����û�������֤��IP��ַ�Ǳ�AC�����ĵ�ַ���е���Ч��ַ���������ַĿǰ��δ������û�*/
#define ERRID_AC999		"AC999"		/*����������Ϣ*/
#define ERRID_AC201		"AC201"		/*AC��radiusͨѶ��ʱ����ͨѶ��������*/

struct portal_packet_t {
	uint8_t version;
	uint8_t type;
	uint8_t auth_type;	/* pap or chap */
	uint8_t rsv;
	uint16_t serial_no;
	uint16_t req_id;
	uint32_t user_ip;
	uint16_t user_port;
	uint8_t err_code;
	uint8_t attr_num;
	union {
		uint8_t m_attr[PORTAL_PACKET_SIZE-PORTAL_PACKET_MOBILE_HEADSIZE];
		struct {
			uint8_t authenticator[PORTAL_AUTHLEN];
			uint8_t attr[PORTAL_PACKET_SIZE-PORTAL_PACKET_TELECOM_HEADSIZE];
		} t;
	} payload;
};
/* } __attribute__((packed)); */

struct portal_packet_attr {
	uint8_t type;
	uint8_t len;
	uint8_t value[0];
};

enum {
	IPV4_USER,
	IPV6_USER,
};

typedef enum {
	REQ_CHALLENGE = 0x01,
	ACK_CHALLENGE,
	REQ_AUTH,
	ACK_AUTH,
	REQ_LOGOUT,
	ACK_LOGOUT,
	AFF_ACK_AUTH,
	NTF_LOGOUT,
	REQ_INFO,
	ACK_INFO,
	REQ_MACBINDING_INFO = 0x30,
	ACK_MACBINDING_INFO = 0x31,
	NTF_USER_LOGON = 0x32,
	NTF_USER_LOGOUT = 0x34,
	REQ_USER_OFFLINE = 0x36,
	CROSS_AREA_ROAM_NOTIFY = 0x40,
	CROSS_AREA_ROAM_ACK = 0x41
} PORTAL_PACKET_TYPE;

typedef enum {
	AUTH_CHAP = 0x00,
	AUTH_PAP = 0x01
} AUTH_TYPE;

typedef enum {
	CHALLENGE_SUCCESS = 0,
	CHALLENGE_REJECT,
	CHALLENGE_CONNECTED,
	CHALLENGE_ONAUTH,
	CHALLENGE_FAILED
} ACK_CHALLENGE_ERRCODE;

typedef enum {
	PORTAL_AUTH_SUCCESS = 0,
	PORTAL_AUTH_REJECT,
	PORTAL_AUTH_CONNECTED,
	PORTAL_AUTH_ONAUTH,
	PORTAL_AUTH_FAILED
} ACK_AUTH_ERRCODE;

typedef enum {
	EC_REQ_LOGOUT = 0,
	EC_REQ_TIMEOUT
} REQ_LOGOUT_ERRCODE;

typedef enum {
	EC_ACK_LOGOUT_SUCCESS = 0,
	EC_ACK_LOGOUT_REJECT,
	EC_ACK_LOGOUT_FAILED
} ACK_LOGOUT_ERRCODE;

typedef enum {
	EC_ACK_INFO_SUCCESS = 0,
	EC_ACK_INFO_NOTSUPPORT,
	EC_ACK_INFO_FAILED
} ACK_INFO_ERRCODE;

typedef enum {
	ATTR_USERNAME = 0x01,
	ATTR_PASSWORD,
	ATTR_CHALLENGE,
	ATTR_CHAPPASSWORD,
	ATTR_ERRID = 0x05,
	ATTR_UPLINKFLUX = 0x06,
	ATTR_DOWNLINKFLUX = 0x07,
	ATTR_PORT_ID = 0x08, /* ATTR_PORT for China Telecom */
	ATTR_BASIP = 0x0a,
	ATTR_USERMAC = 0x0b,
	ATTR_NASID = 0x30,
	ATTR_SESS_START = 0x31,
	ATTR_SESS_STOP = 0x32,
	ATTR_SESS_TIME = 0x33,
	ATTR_USER_AGENT = 0x34,
	ATTR_INPUT_OCTETS = 0x35,
	ATTR_OUTPUT_OCTETS = 0x36,
	ATTR_INPUT_PACKETS = 0x37,
	ATTR_OUTPUT_PACKETS = 0x38,
	ATTR_INPUT_GIGAWORDS = 0x39,
	ATTR_OUTPUT_GIGAWORDS, /* Not sure 0x40 or 0x3a */
	ATTR_SESS_ID = 0x41,
	ATTR_AUDIT_IP = 0x42,
	ATTR_ROAM_NEW_AP = 0x51,
	ATTR_ROAM_NEW_ESSID = 0x52,
	ATTR_ROAM_AP = 0x53,
	ATTR_ROAM_ESSID = 0x54,
	ATTR_USER_IPV6 = 0xf1, /* add by houyongtao for ipv6 */
} ATTR_TYPE;

/*
void
portal_packet_ntoh(struct portal_packet_t *packet);

void
portal_packet_hton(struct portal_packet_t *packet);
*/

int
portal_packet_get_length(const struct portal_packet_t *packet);

int
portal_packet_add_attr(struct portal_packet_t *packet,
				ATTR_TYPE type, const void *value, size_t len);

struct portal_packet_attr *
portal_packet_get_attr(const struct portal_packet_t *packet,
				ATTR_TYPE type);

struct portal_packet_attr *
portal_packet_get_next_attr(const struct portal_packet_t *packet,
				struct portal_packet_attr *attr);

void
portal_packet_log_packet(struct portal_packet_t *packet);


int
portal_packet_init(struct portal_packet_t *packet,
				uint8_t type, user_addr_t *user_addr);

int
portal_packet_init_v2(struct portal_packet_t *packet,
				uint8_t type, user_addr_t *user_addr, uint8_t version);

int
portal_set_protocol_type(int protocol_type);

int
portal_get_protocol_type(void);

int
portal_set_private_attribute_switch(int status);

int
portal_get_private_attribute_switch(void);

int
portal_packet_minsize(void);

int
portal_request_check(struct portal_packet_t *pack,
		char *secret, size_t secretlen);

int
portal_response_check(struct portal_packet_t *pack,
		struct portal_packet_t *req_pack,
		char *secret, size_t secretlen);

int
portal_resp_authenticator(struct portal_packet_t *pack,
		struct portal_packet_t *req_pack,
		char *secret,
		size_t secretlen);

int
portal_req_authenticator(struct portal_packet_t *pack,
		char *secret,
		size_t secretlen);

#endif /* _PORTAL_PACKET_H */

