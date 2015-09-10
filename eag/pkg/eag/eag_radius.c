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
* eag_radius.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* eag radius
*
*
*******************************************************************************/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
	
#include "nm_list.h"
#include "hashtable.h"
#include "eag_mem.h"
#include "eag_log.h"
#include "eag_blkmem.h"
#include "eag_thread.h"
#include "eag_conf.h"  //not need
#include "eag_interface.h"
#include "eag_dbus.h"
#include "eag_hansi.h"
#include "eag_time.h"
#include "eag_util.h"

#include "radius_packet.h"
#include "portal_packet.h"
#include "rdc_handle.h"
#include "eag_radius.h"
#include "eag_portal.h"
#include "appconn.h"
#include "eag_ins.h"
#include "eag_statistics.h"
#include "session.h"

#define SOCKRADIUS_BLKMEM_NAME 			"sockradius_blkmem"
#define SOCKRADIUS_BLKMEM_ITEMNUM		4
#define SOCKRADIUS_BLKMEM_MAXNUM		16

#define RADIUS_ID_NUM		256

#define RADIUS_REQ_NOWAIT		0
#define RADIUS_REQ_WAIT			1

#define RADIUS_ERR_CODE_PROFILE_CMCC	"/opt/eag/radius_errorcode_profile_CMCC"
#define RADIUS_SPECIFIC_ATTR_PATH	"/opt/eag/vendor_id"

#define REPLY_MESSAGE_LEN			256

#define MAX_RADIUS_ERR_CODE_LEN		64
#define RADIUS_ERR_CODE_DIV			'#'
#define ERR_CODE_DETECT_LEN			2	/* cmp ERR_CODE_DETECT_LEN char! */

int idletime_valuecheck = 0;	/* for telecom */

typedef struct sock_radius sock_radius_t;

typedef struct radius_specific_attr {
	uint8_t audit_ip;
	uint8_t user_agent;
	uint8_t user_type;
	uint8_t framed_ipv6_address; /* user ipv6 address */
	uint8_t acct_ipv6_input_octets;
	uint8_t acct_ipv6_output_octets;
	uint8_t acct_ipv6_input_packets;
	uint8_t acct_ipv6_output_packets;
	uint8_t acct_ipv6_input_gigawords;
	uint8_t acct_ipv6_output_gigawords;
} radius_specific_attr_t;

struct eag_radius {
	uint32_t local_ip;
	int is_distributed;
	
	int retry_interval;
	int retry_times;
	int vice_retry_times;
	int acct_interval;
	int swap_octets;

	int hansi_type;
	int hansi_id;

	/* add by houyt for radius Attribute-Specific */
	uint32_t vendor_id;
	radius_specific_attr_t radius_specific_attr;

	struct list_head sockradius_list;
	eag_blk_mem_t *sockradius_blkmem;
	eag_thread_master_t *master;
	
	struct list_head errcode_head;
	struct radius_conf *radiusconf;
	
	eag_ins_t *eagins;
	eag_portal_t *portal;
	appconn_db_t *appdb;
	eag_statistics_t *eagstat;
};

typedef enum {
	RADIUS_ID_FREE = 0,
	RADIUS_ID_USED,
	RADIUS_ID_WAIT,
} radius_id_status_t;

struct radius_id {
	uint8_t index;
	radius_id_status_t status;
	struct radius_packet_t req_packet;

	sock_radius_t *sockradius;
	eag_thread_master_t *master;
	eag_thread_t *t_timeout;
	int timeout;
	
	int retry_count;
	int vice_retry_count;
	unsigned long total_time;
	time_t start_time;
	struct radius_srv_t radius_srv;
	user_addr_t user_addr;
};

struct sock_radius {
	struct list_head node;
	int sockfd;
	uint32_t local_ip;
	
	uint8_t roll_id;
	struct radius_id id[RADIUS_ID_NUM];

	eag_radius_t *radius;
	eag_thread_master_t *master;
	eag_thread_t *t_read;
};

struct radius_err_code_list {
	struct list_head node;
	int portal_server_errcode;
	char radius_err_code[MAX_RADIUS_ERR_CODE_LEN];
};

typedef enum {
	SOCK_RADIUS_READ,
} sock_radius_event_t;

typedef enum {
	RADIUS_ID_STATUS_TIMEOUT,
	RADIUS_ID_RETRY_TIMEOUT,
} radius_id_event_t;
 extern int radius_nas_port_type;
static void
sock_radius_event(sock_radius_event_t event,
		sock_radius_t *sockradius);

static void
radius_id_event(radius_id_event_t event,
		struct radius_id *id);

static struct radius_err_code_list *
create_radius_err_code_node(char *radius_err, int portal_code)
{
	struct radius_err_code_list *err_code = NULL;

	err_code = (struct radius_err_code_list *)
	    eag_malloc(sizeof (struct radius_err_code_list));
	if (NULL == err_code) {
		return NULL;
	}

	memset(err_code, 0, sizeof (struct radius_err_code_list));
	strncpy(err_code->radius_err_code, radius_err,
		sizeof (err_code->radius_err_code) - 1);
	err_code->portal_server_errcode = portal_code;

	return err_code;
}

static int
init_radius_err_code(struct list_head *head, const char *file_path)
{
	FILE *fp = NULL;
	char line[1024];
	int iret = -1;
	struct radius_err_code_list *errcodenode = NULL;

	if (NULL == file_path || strlen(file_path) == 0) {
		eag_log_err("init_radius_err_code  file path error!");
		return iret;
	}

	fp = fopen(file_path, "r");
	if (NULL != fp) {
		while (!feof(fp)) {
			char *div;
			int portal_code;

			if (NULL == fgets(line, sizeof (line), fp)) {
				break;
			}
			eag_log_debug("eag_radius",
				      "init_radius_err_code  get line=%s\n",
				      line);

			div = strchr(line, RADIUS_ERR_CODE_DIV);
			if (NULL == div) {
				continue;
			}
			*div = '\0';
			div++;

			portal_code = strtol(line, NULL, 10);
			if (0 != portal_code) {
				errcodenode =
				    create_radius_err_code_node(div,
								portal_code);
				if (NULL == errcodenode) {
					eag_log_err
					    ("create radius err code err!  %s",
					     div);
					continue;
				}

				list_add_tail(&(errcodenode->node), head);
				eag_log_debug("eag_radius",
					      "get radius err code succes: %d   %s",
					      portal_code, div);
			} else {
				eag_log_err
				    ("get radius err code err!  portal_code = %d    %s",
				     portal_code, div);
			}
		}

		fclose(fp);
		iret = 0;
	} else {
		eag_log_err("file %s open error!\n", file_path);
	}

	eag_log_debug("eag_radius",
		      "init_radius_err_code  load radius error code success!\n");
	return iret;
}

static int
release_radius_err_code(struct list_head *head)
{
	struct radius_err_code_list *temp = NULL, *n;

	list_for_each_entry_safe(temp, n, head, node) {
		eag_free(temp);
	}

	return 0;
}

static int
get_portal_err_code_by_radius_err(eag_radius_t *radius,
		const char *radius_err)
{
	int ret_code = 1;	//1=PORTAL_AUTH_REJECT
	struct radius_err_code_list *temp = NULL;

	if (NULL == radius || NULL == radius_err) {
		eag_log_err("get_portal_err_code_by_radius_err input error");
		return ret_code;
	}
	//if (strlen(radius_err) != 0) {
	//	ret_code = 4;  //PORTAL_AUTH_FAILED
	//}
	list_for_each_entry(temp, &(radius->errcode_head), node) {
		if (strncmp(radius_err, temp->radius_err_code,
		     		ERR_CODE_DETECT_LEN) == 0) {
			ret_code = temp->portal_server_errcode;
			break;
		}
	}

	eag_log_info("get portal errcode %d, radius replymessage=%s", ret_code,
		      radius_err);
	return ret_code;
}

int 
eag_radius_log_all_sockradius(eag_radius_t *radius)
{
	sock_radius_t *sockradius = NULL;
	int num = 0;
	int free = 0;
	int used = 0;
	int wait = 0;
	int i = 0;
	
	if (NULL == radius) {
		eag_log_err("eag_radius_log_all_sockradius input error");
		return -1;
	}
	eag_log_info("log all sockradius begin");
	list_for_each_entry(sockradius, &(radius->sockradius_list), node) {
		num++;
		free = 0;
		used = 0;
		wait = 0;
		for (i = 0; i < RADIUS_ID_NUM; i++) {
			if (RADIUS_ID_FREE == sockradius->id[i].status) {
				free++;
			}
			else if (RADIUS_ID_USED == sockradius->id[i].status) {
				used++;
			}
			else {
				wait++;
			}
		}
		eag_log_info("%-5d sockradius:%d free:%d used:%d wait:%d",
				num,
				sockradius->sockfd,
				free,
				used,
				wait);
	}

	eag_log_info("log all sockradius end, num: %d", num);
	return 0;
}

static sock_radius_t *
sock_radius_new(eag_radius_t *radius)
{
	sock_radius_t *sockradius = NULL;
	int i = 0;

	if (NULL == radius) {
		eag_log_err("sock_radius_new input error");
		return NULL;
	}

	sockradius = eag_blkmem_malloc_item(radius->sockradius_blkmem);
	if (NULL == sockradius) {
		eag_log_err("sock_radius_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(sockradius, 0, sizeof(*sockradius));
	list_add_tail(&(sockradius->node), &(radius->sockradius_list));
	sockradius->sockfd = -1;
	sockradius->local_ip = radius->local_ip;
	sockradius->radius = radius;
	sockradius->master = radius->master;

	for (i = 0; i < RADIUS_ID_NUM; i++) {
		sockradius->id[i].index = i;
		sockradius->id[i].status = RADIUS_ID_FREE;
		sockradius->id[i].sockradius = sockradius;
		sockradius->id[i].master = sockradius->master;
	}

	eag_log_debug("eag_radius", "sock_radius new ok");
	return sockradius;
}

static int
sock_radius_free(sock_radius_t *sockradius)
{
	eag_radius_t *radius = NULL;

	if (NULL == sockradius) {
		eag_log_err("sock_radius_free input error");
		return -1;
	}
	radius = sockradius->radius;
	if (NULL == radius) {
		eag_log_err("sock_radius_free radius null");
		return -1;
	}

	list_del(&(sockradius->node));
	if (sockradius->sockfd >= 0) {
		close(sockradius->sockfd);
		sockradius->sockfd = -1;
	}
	eag_blkmem_free_item(radius->sockradius_blkmem, sockradius);

	eag_log_info("sock_radius free ok");
	
	return 0;
}

static int
sock_radius_start(sock_radius_t *sockradius)
{
	int ret = 0;
	struct sockaddr_in addr = {0};
	char ipstr[32] = "";
	eag_radius_t *radius = NULL;
	uint32_t nasip = 0;
	//int is_distributed = 0;
	int rdc_distributed = 0;
	
	if (NULL == sockradius) {
		eag_log_err("sock_radius_start input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	radius = sockradius->radius;
	
	rdc_distributed = eag_ins_get_rdc_distributed(radius->eagins);
	nasip = eag_ins_get_nasip(radius->eagins);
	if (sockradius->sockfd >= 0) {
		eag_log_err("sock_radius_start already start fd(%d)", 
			sockradius->sockfd);
		return EAG_RETURN_OK;
	}

	sockradius->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockradius->sockfd < 0) {
		eag_log_err("Can't create sockradius dgram socket: %s",
			safe_strerror(errno));
		sockradius->sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	if (rdc_distributed) {
		addr.sin_addr.s_addr = htonl(sockradius->local_ip);
	} else {
		addr.sin_addr.s_addr = htonl(nasip);
	}
	addr.sin_port = htons(0);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	#if 0
	ret = setsockopt(sockradius->sockfd, SOL_SOCKET, SO_REUSEADDR, 
					&opt, sizeof(opt));
	if (0 != ret) {
		eag_log_err("Can't set REUSEADDR to sockradius socket(%d): %s",
			sockradius->sockfd, safe_strerror(errno));
	}
	#endif
	ret  = bind(sockradius->sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		eag_log_err("Can't bind to sockradius socket(%d): %s",
			sockradius->sockfd, safe_strerror(errno));
		close(sockradius->sockfd);
		sockradius->sockfd = -1;
		return EAG_ERR_SOCKET_BIND_FAILED;
	}

	sock_radius_event(SOCK_RADIUS_READ, sockradius);
	
	if (rdc_distributed) {
		eag_log_debug("eag_radius", "sockradius(%s) fd(%d) start ok", 
			ip2str(sockradius->local_ip, ipstr, sizeof(ipstr)),
			sockradius->sockfd);
	} else {
		eag_log_debug("eag_radius", "sockradius(%s) fd(%d) start ok", 
			ip2str(nasip, ipstr, sizeof(ipstr)),
			sockradius->sockfd);
	}
	
	return EAG_RETURN_OK;
}

static int
sock_radius_stop(sock_radius_t *sockradius)
{
	int i = 0;
	char ipstr[32] = "";
	eag_radius_t *radius = NULL;
	uint32_t nasip = 0;
	//int is_distributed = 0;
	int rdc_distributed = 0;

	if (NULL == sockradius) {
		eag_log_err("sock_radius_stop input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	radius = sockradius->radius;
	
	rdc_distributed = eag_ins_get_rdc_distributed(radius->eagins);
	nasip = eag_ins_get_nasip(radius->eagins);
	if (NULL != sockradius->t_read) {
		eag_thread_cancel(sockradius->t_read);
		sockradius->t_read = NULL;
	}

	if (rdc_distributed) {
		eag_log_info("sockradius(%s) fd(%d) stop ok",
			ip2str(sockradius->local_ip, ipstr, sizeof(ipstr)),
			sockradius->sockfd);
	} else {
		eag_log_info("sockradius(%s) fd(%d) stop ok",
			ip2str(nasip, ipstr, sizeof(ipstr)),
			sockradius->sockfd);
	}
	
	if (sockradius->sockfd >= 0) {
		close(sockradius->sockfd);
		sockradius->sockfd = -1;
	}
	
	for (i = 0; i < RADIUS_ID_NUM; i++) {
		if (NULL != sockradius->id[i].t_timeout) {
			eag_thread_cancel(sockradius->id[i].t_timeout);
			sockradius->id[i].t_timeout = NULL;
		}
		sockradius->id[i].status = RADIUS_ID_FREE;
		memset(&(sockradius->id[i].req_packet), 0,
					sizeof(sockradius->id[i].req_packet));
		sockradius->id[i].timeout = 0;
		sockradius->id[i].retry_count = 0;
		sockradius->id[i].vice_retry_count = 0;
		sockradius->id[i].total_time = 0;
		sockradius->id[i].start_time = 0;
		memset(&(sockradius->id[i].radius_srv), 0,
					sizeof(sockradius->id[i].radius_srv));
		memset(&(sockradius->id[i].user_addr), 0, sizeof(user_addr_t));
	}
	
	return 0;
}

static int
radius_add_public_attr(struct radius_packet_t *packet,
		struct app_conn_t *appconn)
{
	char buff[256] = "";
	
	if (NULL == packet || NULL == appconn) {
		eag_log_err("radius_add_public_attr input error");
		return -1;
	}
	radius_addattr(packet, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0,
	       appconn->session.user_addr.user_ip, NULL, 0);

	radius_addattr(packet, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0,
		       (uint8_t *)appconn->session.nasid,
		       strlen(appconn->session.nasid));

	mac2str(appconn->session.usermac, buff, sizeof(buff)-1, '-');
	radius_addattr(packet, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0,
		       (uint8_t *)buff, strlen(buff));

	mac2str(appconn->session.apmac, buff, sizeof(buff)-1, '-');
	strncat(buff, ":", sizeof(buff)-1-strlen(buff));
	strncat(buff, appconn->session.essid, sizeof(buff)-1-strlen(buff));
	radius_addattr(packet, RADIUS_ATTR_CALLED_STATION_ID, 0, 0, 0,
		       (uint8_t *)buff, strlen(buff));

	radius_addattr(packet, RADIUS_ATTR_NAS_PORT, 0, 0, 2000, NULL, 0);

	radius_addattr(packet, RADIUS_ATTR_NAS_PORT_ID, 0, 0, 0,
		       (uint8_t *)appconn->session.nas_port_id,
		       strlen(appconn->session.nas_port_id));
        if(radius_nas_port_type == 0)
	    radius_addattr(packet, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
                	        		      0x13, NULL, 0);
        else
	       radius_addattr(packet, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
	        		      0xf, NULL, 0);
	return 0;
}

static int
sock_radius_detect_free_id(sock_radius_t *sockradius)
{
	uint8_t i = 0;
	
	if (NULL == sockradius) {
		eag_log_err("sock_radius_detect_free_id input error");
		return -1;
	}

	eag_log_debug("eag_radius","sock_radius_detect_free_id old roll_id is %u",
			sockradius->roll_id);
	for (i = sockradius->roll_id+1; i != sockradius->roll_id; i++) {
		if (RADIUS_ID_FREE == sockradius->id[i].status) {
			sockradius->roll_id = i;
			eag_log_debug("eag_radius","sock_radius_detect_free_id new roll_id is %u",
				sockradius->roll_id);
			return 0;
		}
	}

	eag_log_debug("eag_radius","sock_radius_detect_free_id have no free id now");
	return -1;
}

static int
sock_radius_send_packet(sock_radius_t *sockradius,
		uint32_t radius_ip,
		uint16_t radius_port,
		struct radius_packet_t *packet)
{
	size_t length = 0;
	struct sockaddr_in addr = {0};
	ssize_t nbyte = 0;
	char ipstr[32] = "";
	eag_radius_t *radius = NULL;
	//int is_distributed = 0;
	int rdc_distributed = 0;
	
	if (NULL == sockradius || NULL == packet) {
		eag_log_err("sock_radius_send_packet input error");
		return -1;
	}
	radius = sockradius->radius;
	
	rdc_distributed = eag_ins_get_rdc_distributed(radius->eagins);
	length = ntohs(packet->length);
	
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(radius_ip);
	addr.sin_port = htons(radius_port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif

	eag_log_debug("eag_radius",
		"sock_radius send packet %d bytes, fd %d, radius server %s:%u, code %u",
		length, sockradius->sockfd,
		ip2str(radius_ip, ipstr, sizeof(ipstr)),
		radius_port,
		packet->code);
	
	if (rdc_distributed) {
		nbyte = rdc_sendto(sockradius->sockfd, packet, length, 0,
					(struct sockaddr *)&addr, sizeof(addr));
		if (nbyte < 0) {
			eag_log_err("sock_radius_send_packet failed fd(%d)",
				sockradius->sockfd);
			return -1;
		}
	} else {
		nbyte = sendto(sockradius->sockfd, packet, length, 0,
					(struct sockaddr *)&addr, sizeof(addr));
		if (nbyte < 0) {
			eag_log_err("sock_radius_send_packet failed fd(%d): %s",
				sockradius->sockfd, safe_strerror(errno));
			return -1;
		}
	}

	return EAG_RETURN_OK;
}

static int
sock_radius_req(sock_radius_t *sockradius,
		user_addr_t *user_addr,
		struct radius_packet_t *packet,
		struct radius_srv_t *radius_srv,
		int wait)
{
	struct timeval tv = {0};
	time_t timenow = 0;
	struct radius_id *radid = NULL;
	eag_radius_t *radius = NULL;
	uint32_t radius_ip = 0;
	uint16_t radius_port = 0;
	char user_ipstr[IPX_LEN] = "";
	if (NULL == sockradius || NULL == packet 
		|| NULL == radius_srv) {
		eag_log_err("sock_radius_req input error");
		return -1;
	}

	radius = sockradius->radius;
	eag_time_gettimeofday(&tv, NULL);
	timenow = tv.tv_sec;
	
	packet->id = sockradius->roll_id;
	if (RADIUS_CODE_ACCOUNTING_REQUEST == packet->code) {
		radius_acctreq_authenticator(packet, radius_srv->acct_secret,
				radius_srv->acct_secretlen);
	}
	
	radid = &(sockradius->id[packet->id]);
	radid->status = RADIUS_ID_USED;
	memcpy(&(radid->req_packet), packet, sizeof(radid->req_packet));
	radid->retry_count = 0;
	radid->vice_retry_count = 0;
	radid->total_time = radius->retry_interval
			*(radius->retry_times+radius->vice_retry_times+1)*2;
	radid->start_time = timenow;
	memcpy(&(radid->radius_srv), radius_srv, sizeof(radid->radius_srv));
	memcpy(&(radid->user_addr), user_addr, sizeof(user_addr_t));
	
	if (0 == radid->radius_srv.backup_auth_ip) {
		radid->radius_srv.backup_auth_ip = radid->radius_srv.auth_ip;
		radid->radius_srv.backup_auth_port = radid->radius_srv.auth_port;
	}
	if (0 == radid->radius_srv.backup_acct_ip) {
		radid->radius_srv.backup_acct_ip = radid->radius_srv.acct_ip;
		radid->radius_srv.backup_acct_port = radid->radius_srv.acct_port;
	}
	
	if (RADIUS_CODE_ACCESS_REQUEST == packet->code) {
		radius_ip = radius_srv->auth_ip;
		radius_port = radius_srv->auth_port;
	} else {
		radius_ip = radius_srv->acct_ip;
		radius_port = radius_srv->acct_port;
	}

    ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	eag_log_debug("eag_radius","sock_radius_req userip %s, fd=%d, "
		"radius server=%#x:%u, packet code=%u, id=%u, wait=%d",
		user_ipstr, sockradius->sockfd,
		radius_ip, radius_port, packet->code, packet->id, wait);
	sock_radius_send_packet(sockradius, radius_ip, radius_port, packet);
	if (wait) {
		radid->timeout = radius->retry_interval;
		radius_id_event(RADIUS_ID_RETRY_TIMEOUT, radid);
	} else {
		radid->timeout = radid->total_time;
		radius_id_event(RADIUS_ID_STATUS_TIMEOUT, radid);
	}

	return EAG_RETURN_OK;
}

static int
radius_req(eag_radius_t *radius,
		user_addr_t *user_addr,
		struct radius_packet_t *packet,
		struct radius_srv_t *radius_srv,
		int wait)
{
	sock_radius_t *sockradius = NULL;
	int found = 0;
	
	list_for_each_entry(sockradius, &(radius->sockradius_list), node) {
		if (0 == sock_radius_detect_free_id(sockradius)) {
			found = 1;
			break;
		}
	}

	if (found) {
		sock_radius_req(sockradius, user_addr, packet, radius_srv, wait);
		eag_log_debug("eag_radius", "radius_req ok");
		return EAG_RETURN_OK;
	}

	sockradius = sock_radius_new(radius);
	if (NULL == sockradius) {
		eag_log_err("radius_req sock_radius_new failed");
		return -1;
	}
	sock_radius_start(sockradius);
	sock_radius_req(sockradius, user_addr, packet, radius_srv, wait);
	eag_log_debug("eag_radius", "radius_req ok");
	
	return EAG_RETURN_OK;
}

int
radius_auth(eag_radius_t *radius,
		struct app_conn_t *appconn,
		uint8_t auth_type)
{
	struct radius_packet_t packet = {0};
	uint32_t len = 0;
	char ap_macstr[32] = "";
	char user_macstr[32] = "";
	char user_ipstr[IPX_LEN] = "";
	char nas_ipstr[32]= "";
	char portal_ipstr[32]= "";
	char radius_auth_ipstr[32]= "";
	
	if (NULL == radius || NULL == appconn) {
		eag_log_err("radius_auth input error");
		return -1;
	}

	ipx2str(&(appconn->session.user_addr), user_ipstr, sizeof(user_ipstr));
	mac2str(appconn->session.apmac, ap_macstr, sizeof(ap_macstr), '-');
	mac2str(appconn->session.usermac, user_macstr, sizeof(user_macstr), '-');
	ip2str(appconn->session.nasip, nas_ipstr, sizeof(nas_ipstr));
	ip2str(appconn->portal_srv.ip, portal_ipstr, sizeof(portal_ipstr));
	ip2str(appconn->session.radius_srv.auth_ip, radius_auth_ipstr, sizeof(radius_auth_ipstr));
	
	radius_packet_init(&packet, RADIUS_CODE_ACCESS_REQUEST);

	len = strlen(appconn->session.username);
	if (0 == len || len > RADIUS_ATTR_VLEN) {
		eag_log_err("radius_auth failed, username=%s len=%u",
			appconn->session.username, len);
		return EAG_ERR_RADIUS_UESRNAME_TOOLEN;
	}
	radius_addattr(&packet, RADIUS_ATTR_USER_NAME, 0, 0, 0,
			(uint8_t *)appconn->session.username, len);
	
	len = strlen(appconn->session.sessionid);
	radius_addattr(&packet, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0,
			(uint8_t *)appconn->session.sessionid, len);

	radius_addattr(&packet, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0,
			appconn->session.nasip, NULL, 0);
	
	radius_add_public_attr(&packet, appconn);
	
	radius_addattr(&packet, RADIUS_ATTR_SERVICE_TYPE, 0, 0,
		       RADIUS_SERVICE_TYPE_LOGIN, NULL, 0);

	if (AUTH_CHAP == auth_type) {
		uint8_t chappasswd[RADIUS_PASSWORD_LEN + 1] = { 0 };

		radius_addattr(&packet, RADIUS_ATTR_CHAP_CHALLENGE, 0, 0, 0,
			(uint8_t *)appconn->session.challenge, RADIUS_CHAP_CHAL_LEN);

		chappasswd[0] = appconn->session.chapid;
		memcpy(&(chappasswd[1]), appconn->session.chappasswd,
				RADIUS_PASSWORD_LEN);
		radius_addattr(&packet, RADIUS_ATTR_CHAP_PASSWORD, 0, 0, 0,
				(uint8_t *)chappasswd, RADIUS_PASSWORD_LEN + 1);
	} else {
		radius_add_userpasswd(&packet,
				(uint8_t *)appconn->session.passwd,
				strlen(appconn->session.passwd),
				appconn->session.radius_srv.auth_secret,
				appconn->session.radius_srv.auth_secretlen);
	}
	
	radius_addattr(&packet, RADIUS_ATTR_NAS_IPV6_ADDRESS, 0, 0, 0,
			(uint8_t *)&(appconn->session.nasipv6), sizeof(struct in6_addr));

	if (EAG_IPV4 != appconn->session.user_addr.family) {
		radius_addattr(&packet, RADIUS_ATTR_FRAMED_INTERFACE_ID, 0, 0, 0,
				(uint8_t *)&(appconn->session.framed_interface_id), sizeof(uint64_t));

		radius_addattr(&packet, RADIUS_ATTR_FRAMED_IPV6_PREFIX, 0, 0, 0,
				appconn->session.framed_ipv6_prefix, MAX_FRAMED_IPV6_PREFIX_LEN);

		radius_addattr(&packet, RADIUS_ATTR_LOGIN_IPV6_HOST, 0, 0, 0,
				(uint8_t *)&(appconn->session.login_ipv6_host), sizeof(struct in6_addr));

		radius_addattr(&packet, RADIUS_ATTR_DELEGATED_IPV6_PREFIX, 0, 0, 0,
				appconn->session.delegated_ipv6_prefix, MAX_FRAMED_IPV6_PREFIX_LEN);

		if (radius->vendor_id && radius->radius_specific_attr.framed_ipv6_address) {
			radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
					radius->vendor_id,
					radius->radius_specific_attr.framed_ipv6_address,
					0, (uint8_t *)&(appconn->session.user_addr.user_ipv6), sizeof(struct in6_addr));
		}
	}

	if (radius->vendor_id && radius->radius_specific_attr.audit_ip) {
		radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
			radius->vendor_id, 
			radius->radius_specific_attr.audit_ip,
			0, (uint8_t *)&(appconn->session.audit_ip), 4);
	}

	if (radius->vendor_id && radius->radius_specific_attr.user_agent) {
		len = strlen(appconn->user_agent);
		len = (len > 253)?253:len;
		if (0 < len) {
			radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
				radius->vendor_id, 
				radius->radius_specific_attr.user_agent,
				0, (uint8_t *)appconn->user_agent, len);
		}
	}
	admin_log_notice("RadiusAccessRequest___UserName:%s,UserIP:%s,UserMAC:%s,ApMAC:%s,SSID:%s,NasIP:%s,PortalIP:%s,RadiusAuthIP:%s,Interface:%s,NasID:%s,Authtype:%s",
		appconn->session.username, user_ipstr, user_macstr, ap_macstr, appconn->session.essid, 
		nas_ipstr, portal_ipstr, radius_auth_ipstr, appconn->session.intf, appconn->session.nasid,
		(EAG_AUTH_TYPE_MAC==appconn->session.server_auth_type)?"MAC":"Portal");
	log_app_filter(appconn,"RadiusAccessRequest___UserName:%s,UserIP:%s,UserMAC:%s,ApMAC:%s,SSID:%s,NasIP:%s,PortalIP:%s,RadiusAuthIP:%s,Interface:%s,NasID:%s,Authtype:%s",
		appconn->session.username, user_ipstr, user_macstr, ap_macstr, appconn->session.essid, 
		nas_ipstr, portal_ipstr, radius_auth_ipstr, appconn->session.intf, appconn->session.nasid,
		(EAG_AUTH_TYPE_MAC==appconn->session.server_auth_type)?"MAC":"Portal");

	eag_bss_message_count(radius->eagstat, appconn, BSS_ACCESS_REQUEST_COUNT, 1);
	radius_req(radius, &(appconn->session.user_addr), &packet,
		&(appconn->session.radius_srv), RADIUS_REQ_WAIT);

	return EAG_RETURN_OK;
}

static int
radius_acct_req(eag_radius_t *radius,
		struct app_conn_t *appconn,
		uint8_t status_type,
		int wait)
{
	struct radius_packet_t packet;
	uint32_t len = 0;
	uint64_t input_octets = 0;
	uint64_t output_octets = 0;
	uint32_t input_packets = 0;
	uint32_t output_packets = 0;
	uint64_t ipv6_input_octets = 0;
	uint64_t ipv6_output_octets = 0;
	uint32_t ipv6_input_packets = 0;
	uint32_t ipv6_output_packets = 0;
	uint32_t ipv6_input_octets_low = 0;
	uint32_t ipv6_input_octets_high = 0;
	uint32_t ipv6_output_octets_low = 0;
	uint32_t ipv6_output_octets_high = 0;
	struct timeval tv = {0};
	time_t timenow = 0;
	unsigned long timediff = 0;
	char user_ipstr[IPX_LEN] = "";
	char ap_macstr[32] = "";
	char user_macstr[32] = "";
	char nas_ipstr[32]= "";
	char radius_acct_ipstr[32]= "";
	struct radius_srv_t *acct_srv_t = NULL;
	const char *terminate_cause_str = "";
	char *user_type = "";
	
	if (NULL == radius || NULL == appconn) {
		eag_log_err("radius_acct_req input error");
		return -1;
	}

	mac2str(appconn->session.apmac, ap_macstr, sizeof(ap_macstr), '-');
	mac2str(appconn->session.usermac, user_macstr, sizeof(user_macstr), '-');
	ip2str(appconn->session.nasip, nas_ipstr, sizeof(nas_ipstr));
	acct_srv_t = &(appconn->session.radius_srv);
	ip2str(acct_srv_t->acct_ip, radius_acct_ipstr, sizeof(radius_acct_ipstr));
	
	if (RADIUS_STATUS_TYPE_START != status_type
		&& RADIUS_STATUS_TYPE_INTERIM_UPDATE != status_type
		&& RADIUS_STATUS_TYPE_STOP != status_type) {
		eag_log_err("radius_acct_req input error, status_type=%d",
			status_type);
		return -1;
	}
	
	eag_time_gettimeofday(&tv, NULL);
	timenow = tv.tv_sec;
	ipx2str(&(appconn->session.user_addr), user_ipstr, sizeof(user_ipstr));
	
	radius_packet_init(&packet, RADIUS_CODE_ACCOUNTING_REQUEST);

	radius_addattr(&packet, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0,
			status_type, NULL, 0);

	len = strlen(appconn->session.username);
	if (0 == len || len > RADIUS_ATTR_VLEN) {
		eag_log_err("radius_auth failed, username=%s len=%u",
			appconn->session.username, len);
		return EAG_ERR_RADIUS_UESRNAME_TOOLEN;
	}
	radius_addattr(&packet, RADIUS_ATTR_USER_NAME, 0, 0, 0,
			(uint8_t *)appconn->session.username, len);

	len = strlen(appconn->session.sessionid);
	radius_addattr(&packet, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0,
			(uint8_t *)appconn->session.sessionid, len);
	
	radius_addattr(&packet, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0,
			appconn->session.nasip, NULL, 0);

	if (radius->vendor_id && radius->radius_specific_attr.audit_ip) {
		radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
			radius->vendor_id, 
			radius->radius_specific_attr.audit_ip,
			0, (uint8_t *)&(appconn->session.audit_ip), 4);
	}

	if (0 == appconn->session.radius_srv.class_to_bandwidth) {
		if (appconn->session.class_attr_len > 0) {
			radius_addattr(&packet, RADIUS_ATTR_CLASS, 0, 0, 0,
			      	(uint8_t *)appconn->session.class_attr,
			      	appconn->session.class_attr_len);
		}
	}
	radius_add_public_attr(&packet, appconn);
	
	radius_addattr(&packet, RADIUS_ATTR_NAS_IPV6_ADDRESS, 0, 0, 0,
			(uint8_t *)&(appconn->session.nasipv6), sizeof(struct in6_addr));

	if (EAG_IPV4 != appconn->session.user_addr.family) {
		if (radius->vendor_id && radius->radius_specific_attr.framed_ipv6_address) {
			radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
				radius->vendor_id,
				radius->radius_specific_attr.framed_ipv6_address,
				0, (uint8_t *)&(appconn->session.user_addr.user_ipv6), sizeof(struct in6_addr));
		}
		radius_addattr(&packet, RADIUS_ATTR_FRAMED_INTERFACE_ID, 0, 0, 0,
				(uint8_t *)&(appconn->session.framed_interface_id), sizeof(uint64_t));

		radius_addattr(&packet, RADIUS_ATTR_FRAMED_IPV6_PREFIX, 0, 0, 0,
				appconn->session.framed_ipv6_prefix, MAX_FRAMED_IPV6_PREFIX_LEN);

		radius_addattr(&packet, RADIUS_ATTR_LOGIN_IPV6_HOST, 0, 0, 0,
				(uint8_t *)&(appconn->session.login_ipv6_host), sizeof(struct in6_addr));

		radius_addattr(&packet, RADIUS_ATTR_DELEGATED_IPV6_PREFIX, 0, 0, 0,
				appconn->session.delegated_ipv6_prefix, MAX_FRAMED_IPV6_PREFIX_LEN);

		len = strlen(appconn->session.framed_ipv6_route);
		radius_addattr(&packet, RADIUS_ATTR_FRAMED_IPV6_ROUTE, 0, 0, 0,
				(uint8_t *)appconn->session.framed_ipv6_route, len);

		len = strlen(appconn->session.framed_ipv6_pool);
		radius_addattr(&packet, RADIUS_ATTR_FRAMED_IPV6_POOL, 0, 0, 0,
				(uint8_t *)appconn->session.framed_ipv6_pool, len);

	}

	if (EAG_IPV4 == appconn->session.user_addr.family) {
		user_type = "v4-only";
	} else if (EAG_IPV6 == appconn->session.user_addr.family) {
		user_type = "v6-only";
	} else if (EAG_MIX == appconn->session.user_addr.family) {
		user_type = "dual-stack";
	} else {
		user_type = "";
	}
	if (radius->vendor_id && radius->radius_specific_attr.user_type) {
		radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
			radius->vendor_id,
			radius->radius_specific_attr.user_type,
			0, (uint8_t *)user_type, strlen(user_type));
	}
	switch (status_type) {
	case RADIUS_STATUS_TYPE_START:
		eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_REQUEST_START_COUNT, 1);
		radius_addattr(&packet, RADIUS_ATTR_SESSION_TIMEOUT, 0, 0,
				appconn->session.sessiontimeout, NULL, 0);
		admin_log_notice("RadiusAcctRequestStart___UserName:%s,UserIP:%s,UserMAC:%s,SSID:%s,"\
			"Authtype:%s,ApMAC:%s,NasIP:%s,RadiusAcctIP:%s,Interface:%s,NasID:%s",
			appconn->session.username, user_ipstr, user_macstr, appconn->session.essid,
			(EAG_AUTH_TYPE_MAC==appconn->session.server_auth_type)?"MAC":"Portal",
			ap_macstr, nas_ipstr, radius_acct_ipstr, appconn->session.intf, appconn->session.nasid);
		log_app_filter(appconn,"RadiusAcctRequestStart___UserName:%s,UserIP:%s,UserMAC:%s,SSID:%s,"\
			"Authtype:%s,APMAC:%s,NasIP:%s,RadiusAcctIP:%s,Interface:%s,NasID:%s",
			appconn->session.username, user_ipstr, user_macstr, appconn->session.essid,
			(EAG_AUTH_TYPE_MAC==appconn->session.server_auth_type)?"MAC":"Portal",
			ap_macstr,nas_ipstr, radius_acct_ipstr, appconn->session.intf, appconn->session.nasid);
		break;
	case RADIUS_STATUS_TYPE_STOP:
		radius_addattr(&packet, RADIUS_ATTR_ACCT_TERMINATE_CAUSE, 0, 0,
				(uint32_t)appconn->session.terminate_cause, NULL, 0);
		/*do not break!*/
	case RADIUS_STATUS_TYPE_INTERIM_UPDATE:
		flush_all_appconn_flux_immediate(radius->appdb);
		if (1 == radius->swap_octets) {
			input_octets = appconn->session.input_octets;
			output_octets = appconn->session.output_octets;
			input_packets = appconn->session.input_packets;
			output_packets = appconn->session.output_packets;
			ipv6_input_octets = appconn->session.ipv6_input_octets;
			ipv6_output_octets = appconn->session.ipv6_output_octets;
			ipv6_input_packets = appconn->session.ipv6_input_packets;
			ipv6_output_packets = appconn->session.ipv6_output_packets;
		} else {
			input_octets = appconn->session.output_octets;
			output_octets = appconn->session.input_octets;
			input_packets = appconn->session.output_packets;
			output_packets = appconn->session.input_packets;
			ipv6_input_octets = appconn->session.ipv6_output_octets;
			ipv6_output_octets = appconn->session.ipv6_input_octets;
			ipv6_input_packets = appconn->session.ipv6_output_packets;
			ipv6_output_packets = appconn->session.ipv6_input_packets;
		}

		/* ACCT_INPUT_OCTETS */
		radius_addattr(&packet, RADIUS_ATTR_ACCT_INPUT_OCTETS, 0, 0,
					(uint32_t)input_octets, NULL, 0);

		/* ACCT_OUTPUT_OCTETS */
		radius_addattr(&packet, RADIUS_ATTR_ACCT_OUTPUT_OCTETS, 0, 0,
					(uint32_t)output_octets, NULL, 0);

		/* ACCT_INPUT_GIGAWORDS */
		radius_addattr(&packet, RADIUS_ATTR_ACCT_INPUT_GIGAWORDS, 0, 0,
					(uint32_t)(input_octets >> 32), NULL, 0);

		/* ACCT_OUTPUT_GIGAWORDS */
		radius_addattr(&packet, RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS, 0, 0,
					(uint32_t)(output_octets >> 32), NULL, 0);

		/* ACCT_INPUT_PACKETS */
		radius_addattr(&packet, RADIUS_ATTR_ACCT_INPUT_PACKETS, 0, 0,
					input_packets, NULL, 0);

		/* ACCT_OUTPUT_PACKETS */
		radius_addattr(&packet, RADIUS_ATTR_ACCT_OUTPUT_PACKETS, 0, 0,
					output_packets, NULL, 0);

		ipv6_input_octets_low = (uint32_t)ipv6_input_octets;
		ipv6_output_octets_low = (uint32_t)ipv6_output_octets;
		ipv6_input_octets_high = (uint32_t)(ipv6_input_octets >> 32);
		ipv6_output_octets_high = (uint32_t)(ipv6_output_octets >> 32);

		if (EAG_IPV4 != appconn->session.user_addr.family) {
			/* ACCT_IPV6_INPUT_OCTETS */
			if (radius->vendor_id && radius->radius_specific_attr.acct_ipv6_input_octets) {
			radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
						radius->vendor_id,
						radius->radius_specific_attr.acct_ipv6_input_octets,
						0, (uint8_t *)&ipv6_input_octets_low, sizeof(uint32_t));
			}
			/* ACCT_IPV6_OUTPUT_OCTETS */
			if (radius->vendor_id && radius->radius_specific_attr.acct_ipv6_output_octets) {
			radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
						radius->vendor_id,
						radius->radius_specific_attr.acct_ipv6_output_octets,
						0, (uint8_t *)&ipv6_output_octets_low, sizeof(uint32_t));
			}
			/* ACCT_IPV6_INPUT_PACKETS */
			if (radius->vendor_id && radius->radius_specific_attr.acct_ipv6_input_packets) {
			radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
						radius->vendor_id,
						radius->radius_specific_attr.acct_ipv6_input_packets,
						0, (uint8_t *)&ipv6_input_packets, sizeof(uint32_t));
			}
			/* ACCT_IPV6_OUTPUT_PACKETS */
			if (radius->vendor_id && radius->radius_specific_attr.acct_ipv6_output_packets) {
			radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
						radius->vendor_id,
						radius->radius_specific_attr.acct_ipv6_output_packets,
						0, (uint8_t *)&ipv6_output_packets, sizeof(uint32_t));
			}
			/* ACCT_IPV6_INPUT_GIGAWORDS */
			if (radius->vendor_id && radius->radius_specific_attr.acct_ipv6_input_gigawords) {
			radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
						radius->vendor_id,
						radius->radius_specific_attr.acct_ipv6_input_gigawords,
						0, (uint8_t *)&ipv6_input_octets_high, sizeof(uint32_t));
			}
			/* ACCT_IPV6_OUTPUT_GIGAWORDS */
			if (radius->vendor_id && radius->radius_specific_attr.acct_ipv6_output_gigawords) {
			radius_addattr(&packet, RADIUS_ATTR_VENDOR_SPECIFIC,
						radius->vendor_id,
						radius->radius_specific_attr.acct_ipv6_output_gigawords,
						0, (uint8_t *)&ipv6_output_octets_high, sizeof(uint32_t));
			}
		}

		eag_log_debug("eag_radius", "radius_acct_req add attr, userip=%s, "
			"INPUT_OCTETS=%u, OUTPUT_OCTETS=%u, "
			"INPUT_GIGAWORDS=%u, OUTPUT_GIGAWORDS=%u, "
			"INPUT_PACKETS=%u, OUTPUT_PACKETS=%u, "
			"IPV6_INPUT_OCTETS=%u, IPV6_OUTPUT_OCTETS=%u, "
			"IPV6_INPUT_GIGAWORDS=%u, IPV6_OUTPUT_GIGAWORDS=%u, "
			"IPV6_INPUT_PACKETS=%u, IPV6_OUTPUT_PACKETS=%u",
			user_ipstr, (uint32_t)input_octets, (uint32_t)output_octets,
			(uint32_t)(input_octets >> 32), (uint32_t)(output_octets >> 32),
			input_packets, output_packets, (uint32_t)ipv6_input_octets, (uint32_t)ipv6_output_octets,
			(uint32_t)(ipv6_input_octets >> 32), (uint32_t)(ipv6_output_octets >> 32),
			ipv6_input_packets, ipv6_output_packets);

		if (RADIUS_STATUS_TYPE_INTERIM_UPDATE == status_type) {
			timediff = timenow - appconn->session.session_start_time;
			eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_REQUEST_UPDATE_COUNT, 1);
			admin_log_notice("RadiusAcctRequestUpdate___UserName:%s,UserIP:%s,UserMAC:%s,"
				"Sessiontime:%lu,SSID:%s,Authtype:%s,ApMAC:%s,NasIP:%s,RadiusAcctIP:%s,Interface:%s,NasID:%s,"
				"InputOctets:%llu,InputPackets:%u,OutputOctets:%llu,OutputPackets:%u,"
				"IPV6InputOctets:%llu,IPV6InputPackets:%u,IPV6OutputOctets:%llu,IPV6OutputPackets:%u",
				appconn->session.username, user_ipstr, user_macstr, timediff, appconn->session.essid,
				(EAG_AUTH_TYPE_MAC==appconn->session.server_auth_type)?"MAC":"Portal",
				ap_macstr, nas_ipstr, radius_acct_ipstr, appconn->session.intf, appconn->session.nasid, 
				appconn->session.output_octets, appconn->session.output_packets, 
				appconn->session.input_octets, appconn->session.input_packets, 
				appconn->session.ipv6_output_octets, appconn->session.ipv6_output_packets,
				appconn->session.ipv6_input_octets, appconn->session.ipv6_input_packets);
			log_app_filter(appconn,"RadiusAcctRequestUpdate___UserName:%s,UserIP:%s,UserMAC:%s,"
				"Sessiontime:%lu,SSID:%s,Authtype:%s,ApMAC:%s,NasIP:%s,RadiusAcctIP:%s,Interface:%s,NasID:%s,"
				"InputOctets:%llu,InputPackets:%u,OutputOctets:%llu,OutputPackets:%u,"
				"IPV6InputOctets:%llu,IPV6InputPackets:%u,IPV6OutputOctets:%llu,IPV6OutputPackets:%u",
				appconn->session.username, user_ipstr, user_macstr, timediff, appconn->session.essid,
				(EAG_AUTH_TYPE_MAC==appconn->session.server_auth_type)?"MAC":"Portal",
				ap_macstr, nas_ipstr, radius_acct_ipstr, appconn->session.intf, appconn->session.nasid, 
				appconn->session.output_octets, appconn->session.output_packets, 
				appconn->session.input_octets, appconn->session.input_packets, 
				appconn->session.ipv6_output_octets, appconn->session.ipv6_output_packets,
				appconn->session.ipv6_input_octets, appconn->session.ipv6_input_packets);
		} else {
			timediff = appconn->session.session_stop_time - appconn->session.session_start_time;
			eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_REQUEST_STOP_COUNT, 1);
			terminate_cause_str = radius_terminate_cause_to_str(appconn->session.terminate_cause);
			admin_log_notice("RadiusAcctRequestStop___UserName:%s,UserIP:%s,UserMAC:%s,"
				"Sessiontime:%lu,SSID:%s,Authtype:%s,ApMAC:%s,NasIP:%s,RadiusAcctIP:%s,Interface:%s,NasID:%s,"
				"InputOctets:%llu,InputPackets:%u,OutputOctets:%llu,OutputPackets:%u,OfflineReason:%s(%d)",
				appconn->session.username, user_ipstr, user_macstr, timediff, appconn->session.essid,
				(EAG_AUTH_TYPE_MAC==appconn->session.server_auth_type)?"MAC":"Portal",
				ap_macstr, nas_ipstr, radius_acct_ipstr, appconn->session.intf, 
				appconn->session.nasid, appconn->session.output_octets,
				appconn->session.output_packets, appconn->session.input_octets, appconn->session.input_packets,
				terminate_cause_str, appconn->session.terminate_cause);
			log_app_filter(appconn,"RadiusAcctRequestStop___UserName:%s,UserIP:%s,UserMAC:%s,"
				"Sessiontime:%lu,SSID:%s,Authtype:%s,ApMAC:%s,NasIP:%s,RadiusAcctIP:%s,Interface:%s,NasID:%s,"
				"InputOctets:%llu,InputPackets:%u,OutputOctets:%llu,OutputPackets:%u,OfflineReason:%s(%d)",
				appconn->session.username, user_ipstr, user_macstr, timediff, appconn->session.essid,
				(EAG_AUTH_TYPE_MAC==appconn->session.server_auth_type)?"MAC":"Portal",
				ap_macstr, nas_ipstr, radius_acct_ipstr, appconn->session.intf, 
				appconn->session.nasid, appconn->session.output_octets,
				appconn->session.output_packets, appconn->session.input_octets, appconn->session.input_packets,
				terminate_cause_str, appconn->session.terminate_cause);
		}
		/* ACCT_SESSION_TIME */
		radius_addattr(&packet, RADIUS_ATTR_ACCT_SESSION_TIME, 0, 0,
					timediff, NULL, 0);
		
		eag_log_debug("eag_radius", "radius_acct_req add attr, userip=%s, "
			"SESSION_TIME=%lu(%lu-%lu)",
			user_ipstr, timediff, timenow, 
			appconn->session.session_start_time);
		break;
	default:
		eag_log_err("radius_acct, unexpected acct_status_type %u",
			status_type);
		break;
	}

	radius_req(radius, &(appconn->session.user_addr),
			&packet, &(appconn->session.radius_srv), wait);

	return EAG_RETURN_OK;
}

int
radius_acct(eag_radius_t *radius,
		struct app_conn_t *appconn,
		uint8_t status_type)
{
	return radius_acct_req(radius, appconn, status_type, RADIUS_REQ_WAIT);
}

int
radius_acct_nowait(eag_radius_t *radius,
			struct app_conn_t *appconn, uint8_t status_type)
{
	return radius_acct_req(radius, appconn, status_type, RADIUS_REQ_NOWAIT);
}

int
eag_set_idletime_valuecheck(int value_check)
{
	idletime_valuecheck = value_check;

	return 0;
}

static void
config_radius_session(struct app_conn_t *appconn,
		struct radius_packet_t *pack)
{
	struct radius_attr_t *attr = NULL;
	char user_ipstr[IPX_LEN] = "";

	ipx2str(&(appconn->session.user_addr), user_ipstr, sizeof(user_ipstr));
	/* Session timeout */
	if (!radius_getattr(pack, &attr, RADIUS_ATTR_SESSION_TIMEOUT, 0, 0, 0)) {
		appconn->session.sessiontimeout = ntohl(attr->v.i);
		eag_log_debug("eag_radius",
			"config_radius_session ip=%s sessiontimeout=%lu",
			user_ipstr, appconn->session.sessiontimeout);
	}

	/* Idle timeout */
	if (!radius_getattr(pack, &attr, RADIUS_ATTR_IDLE_TIMEOUT, 0, 0, 0)) {
		unsigned long idle_timeout = ntohl(attr->v.i);
		if (1 == idletime_valuecheck) {
			if (idle_timeout < appconn->session.idle_timeout) {
				appconn->session.idle_timeout = idle_timeout;
			}
		} else {
			appconn->session.idle_timeout = ntohl(attr->v.i);
		}
		eag_log_debug("eag_radius",
			"config_radius_session ip=%s idletimeout=%lu",
			user_ipstr, appconn->session.idle_timeout);
	}

	/* Interim interval */
	if (!radius_getattr(pack,
			&attr, RADIUS_ATTR_ACCT_INTERIM_INTERVAL, 0, 0, 0)) 
	{
		unsigned long interim_interval = ntohl(attr->v.i);

		if (interim_interval >= 60) {
			appconn->session.interim_interval = interim_interval;
			eag_log_debug("eag_radius",
				"config_radius_session ip=%s interim_interval=%lu",
				user_ipstr, appconn->session.interim_interval);
		} else {
			eag_log_warning("config_radius_session "
				"ip=%s interim_interval keep old %lu",
				user_ipstr, appconn->session.interim_interval);
		}
	}

	/* Bandwidth up */
	if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_WISPR,
			RADIUS_ATTR_WISPR_BANDWIDTH_MAX_UP, 0)) 
	{
		appconn->session.bandwidthmaxup = ntohl(attr->v.i) / 1024;
		eag_log_debug("eag_radius", "session.bandwidthmaxup =%u Kbps",
			appconn->session.bandwidthmaxup);
	}

	/* Bandwidth down */
	if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_WISPR,
			RADIUS_ATTR_WISPR_BANDWIDTH_MAX_DOWN, 0)) 
	{
		appconn->session.bandwidthmaxdown = ntohl(attr->v.i) / 1024;
		eag_log_debug("eag_radius", "session.bandwidthmaxup =%u Kbps\n",
			appconn->session.bandwidthmaxdown);
	}

/*
	RFC-2865  ��Class���ԵĶ����ǲ�����radius client���н�����
֮ǰ����Կ������WinRadiusʵ��radius�·������ܵ�
����ӿ��ؿ����Ƿ���Ҫradius�·�����Ĭ��Ϊdisable
*/	
	if ( !appconn->session.radius_srv.class_to_bandwidth )
	{
		if (!radius_getattr(pack, &attr, RADIUS_ATTR_CLASS, 0, 0, 0)) {
			memcpy(appconn->session.class_attr, attr->v.t, attr->l-2);
			appconn->session.class_attr_len = attr->l-2;
			eag_log_debug("eag_radius",
					"config_radius_session class_attr =%s, len = %u",
					appconn->session.class_attr,
					appconn->session.class_attr_len);
		} else {
			memset(appconn->session.class_attr, 0, sizeof(appconn->session.class_attr)-1);
			appconn->session.class_attr_len = 0;
		}
	}
	else
	{
		/*class for bandwidth!  4*8 byte  
		   ���з�ֵ���ʣ�����ƽ�����ʣ����з�ֵ���ʣ�����ƽ������ */
		if (!radius_getattr(pack, &attr, RADIUS_ATTR_CLASS, 0, 0, 0)) {
			char class_str[128] = "";
	#define MIN(a,b) ((a)>(b))?(b):(a)

			memset(class_str, 0, sizeof (class_str));
			memcpy(class_str, attr->v.t, 
				MIN(attr->l - 2, sizeof (class_str) - 1));

			if (strlen(class_str) == 32) {
				char up_peak[9] = "";
				char up_average[9] = "";
				char down_peak[9] = "";
				char down_average[9] = "";

				memset(up_peak, 0, sizeof (up_peak));
				memcpy(up_peak, class_str, 8);

				memset(up_average, 0, sizeof (up_average));
				memcpy(up_average, class_str + 8, 8);

				memset(down_peak, 0, sizeof (down_peak));
				memcpy(down_peak, class_str + 16, 8);

				memset(down_average, 0, sizeof (down_average));
				memcpy(down_average, class_str + 24, 8);

				appconn->session.bandwidthmaxup = 
					strtoul(up_average, NULL, 10) / 1024;
				appconn->session.bandwidthmaxdown =
					strtoul(down_average, NULL, 10) / 1024;
				eag_log_debug("eag_radius",
						"config_radius_session bandwidthmaxup =%u Kbps",
						appconn->session.bandwidthmaxup);
				eag_log_debug("eag_radius",
						"config_radius_session bandwidthmaxdown =%u Kbps",
						appconn->session.bandwidthmaxdown);
			}
		}
	}

	if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_HUAWEI,
			RADIUS_ATTR_HUAWEI_AVE_BANDWIDTH_MAX_UP, 0))
	{
		/*  v.i bytes / 1024 = ? KB for speed limit on AP */
		appconn->session.bandwidthmaxup = ntohl(attr->v.i) / 1024;
		eag_log_debug("eag_radius", "HUAWEI:bandwidthmaxup =%u",
			appconn->session.bandwidthmaxup);
	}

	if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_HUAWEI, 
			RADIUS_ATTR_HUAWEI_AVE_BANDWIDTH_MAX_DOWN, 0))
	{
		/*  v.i bytes / 1024 = ? KB for speed limit on AP */
		appconn->session.bandwidthmaxdown = ntohl(attr->v.i) / 1024;
		eag_log_debug("eag_radius", "HUAWEI:bandwidthmaxdown =%u",
			appconn->session.bandwidthmaxdown);
	}
	
    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
            RADIUS_VENDOR_AUTELAN, 
            RADIUS_ATTR_AUTELAN_INPUT_AVERAGE_RATE, 0))
    {
		/*  v.i bytes / 1024 = ? KB for speed limit on AP */
        appconn->session.bandwidthmaxup = ntohl(attr->v.i) / 1024;
        eag_log_debug("eag_radius", "AUTELAN:bandwidthmaxup =%u",
            appconn->session.bandwidthmaxup);
    }

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
            RADIUS_VENDOR_AUTELAN, 
            RADIUS_ATTR_AUTELAN_OUTPUT_AVERAGE_RATE, 0))
    {
		/*  v.i bytes / 1024 = ? KB for speed limit on AP */
        appconn->session.bandwidthmaxdown = ntohl(attr->v.i) / 1024;
        eag_log_debug("eag_radius", "AUTELAN:bandwidthmaxdown =%u",
            appconn->session.bandwidthmaxdown);
    }

	if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_AUTELAN, 
			RADIUS_ATTR_AUTELAN_BANDWIDTH_MAX_UP, 0))
	{
		/*  v.i KB for speed limit on AP */
		appconn->session.bandwidthmaxup = ntohl(attr->v.i);
		eag_log_debug("eag_radius", "AUTELAN:bandwidthmaxup =%u",
			appconn->session.bandwidthmaxup);
	}

	if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_AUTELAN, 
			RADIUS_ATTR_AUTELAN_BANDWIDTH_MAX_DOWN, 0))
    {
        /*  v.i KB for speed limit on AP */
        appconn->session.bandwidthmaxdown = ntohl(attr->v.i);
        eag_log_debug("eag_radius", "AUTELAN:bandwidthmaxdown =%u",
            appconn->session.bandwidthmaxdown);
    }
    
	if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_AUTELAN, 
			RADIUS_ATTR_AUTELAN_SESSION_FLOWOUT_KB, 0))
	{
		/*  v.i = ? KB, maxtotaloctets = ? bps  */
		appconn->session.maxtotaloctets = ntohl(attr->v.i) * 1024;
		eag_log_debug("eag_radius", "AUTELAN:maxtotaloctets=%llu",
			appconn->session.maxtotaloctets);
	}

	if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_AUTELAN, 
			RADIUS_ATTR_AUTELAN_SESSION_FLOWOUT_MB, 0))
	{
		/*  v.i = ? MB, maxtotaloctets = ? bps  */
		appconn->session.maxtotaloctets = ntohl(attr->v.i) * 1024 * 1024;
		eag_log_debug("eag_radius", "AUTELAN:maxtotaloctets=%llu",
			appconn->session.maxtotaloctets);
	}
	
/*
	if (!radius_getattr(pack, &attr, RADIUS_ATTR_REPLY_MESSAGE, 0, 0, 0)) {
		memset(appconn->session.replymsg, 0, sizeof(appconn->session.replymsg));
		memcpy(appconn->session.replymsg, attr->v.t, attr->l - 2);
		eag_log_debug("eag_radius",
			"config_radius_session reply message=%s, attr_len=%u",
			appconn->session.replymsgreplymsg, attr->l - 2);
	}
*/
}

static int
access_accept_proc(eag_radius_t *radius,
		user_addr_t *user_addr,
		struct radius_packet_t *packet)
{
	struct app_conn_t *appconn = NULL;
	const char *errid = NULL;
	char user_ipstr[IPX_LEN] = "";
	struct radius_attr_t *attr = NULL;
	char facl_name[256] = "";

	if (NULL == radius || NULL == packet) {
		eag_log_err("access_accept_proc input error");
		return -1;
	}
	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	appconn = appconn_find_by_useripx(radius->appdb, user_addr);
	eag_bss_message_count(radius->eagstat, appconn, BSS_ACCESS_ACCEPT_COUNT, 1);
	if (NULL == appconn) {
		eag_log_warning("access_accept_proc "
			"appconn_find_by_userip failed, userip %s",
			user_ipstr);
		if (PORTAL_PROTOCOL_MOBILE == portal_get_protocol_type()) {
			errid = ERRID_AC999;
		}
		eag_portal_auth_failure(radius->portal, user_addr,
				PORTAL_AUTH_REJECT, errid);
		return 0;
	}
	if (0 == appconn->on_auth) {
		eag_log_warning("access_accept_proc "
			"userip %s is not on_auth, appconn is new, auth failed",
			user_ipstr);
		if (PORTAL_PROTOCOL_MOBILE == portal_get_protocol_type()) {
			errid = ERRID_AC999;
		}
		eag_portal_auth_failure(radius->portal, user_addr,
				PORTAL_AUTH_REJECT, errid);
		return 0;
	}

	appconn->on_auth = 0;
	appconn->session.interim_interval = radius->acct_interval;
	
	config_radius_session(appconn, packet);

	if (!radius_getattr(packet, &attr, RADIUS_ATTR_FILTER_ID, 0, 0 ,0)) {
		memset(facl_name, 0, sizeof(facl_name));
		memcpy(facl_name, attr->v.t, attr->l-2);
		facl_name[sizeof(facl_name)-1] = '\0';
		if (!strcmp(facl_name, "actest1")){
			appconn->session.bandwidthmaxup= 1024;
			appconn->session.bandwidthmaxdown = 1024;
        		eag_log_debug("eag_radius", "AUTELAN:bandwidthmaxup =%u, bandwidthmaxdown =%u",
            			appconn->session.bandwidthmaxup, appconn->session.bandwidthmaxdown);
		} else if (!strcmp(facl_name, "actest2")){
			appconn->session.bandwidthmaxup= 2048;
			appconn->session.bandwidthmaxdown = 2048;
        		eag_log_debug("eag_radius", "AUTELAN:bandwidthmaxup =%u, bandwidthmaxdown =%u",
            			appconn->session.bandwidthmaxup, appconn->session.bandwidthmaxdown);
		}
	}

	eag_portal_auth_success(radius->portal, appconn);

	return 0;
}

static int
access_reject_proc(eag_radius_t *radius,
		user_addr_t *user_addr,
		struct radius_packet_t *packet)
{
	struct app_conn_t *appconn = NULL;
	struct radius_attr_t *attr = NULL;
	char replymsg[256] = "";
	uint8_t err_code = PORTAL_AUTH_REJECT;
	const char *err_id = NULL;
	char user_ipstr[IPX_LEN] = "";

	if (NULL == radius || NULL == packet) {
		eag_log_err("access_reject_proc input error");
		return -1;
	}

	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	
	appconn = appconn_find_by_useripx(radius->appdb, user_addr);
	if (NULL == appconn) {
		eag_log_warning("access_reject_proc "
			"appconn_find_by_userip failed, userip %s",
			user_ipstr);
	} else {
		appconn->on_auth = 0;
	}

	if (!radius_getattr(packet, &attr, RADIUS_ATTR_REPLY_MESSAGE, 0, 0, 0)) {
		memcpy(replymsg, attr->v.t, attr->l - 2);
		eag_log_debug("eag_radius",
			"access_reject_proc reply message=%s, attr_len=%u",
			replymsg, attr->l - 2);
	} else {
		eag_log_warning("access_reject_proc userip %s "
			"not get reply-message attr from radius server",
			user_ipstr);
	}

	err_id = replymsg;
	err_code = get_portal_err_code_by_radius_err(radius,
						replymsg);

	eag_bss_message_count(radius->eagstat, appconn, BSS_ACCESS_REJECT_COUNT, 1);
	eag_portal_auth_failure(radius->portal, user_addr,
			err_code, err_id);

	return EAG_RETURN_OK;
}

static int
acct_response_proc(eag_radius_t *radius,
			struct radius_id *radid,
			struct radius_packet_t *packet)
{
	struct app_conn_t *appconn = NULL;
	struct radius_attr_t *attr = NULL;
	char user_ipstr[IPX_LEN] = "";
	char username[256] = "";
	char user_macstr[32] = "";
	char radius_ipstr[32] = "";
	char nasid[RADIUS_MAX_NASID_LEN] = "";
	char session_filter_prefix[512] = ""; /* add for debug-filter */
	uint32_t nasip = 0;
	char nas_ipstr[32]= "";

	if (NULL == radius || NULL == radid  || NULL == packet) {
		eag_log_err("acct_response_proc input error");
		return -1;
	}

	nasip = eag_ins_get_nasip(radius->eagins);
	ip2str(nasip, nas_ipstr, sizeof(nas_ipstr));
	ipx2str(&(radid->user_addr), user_ipstr, sizeof(user_ipstr));
	ip2str(radid->radius_srv.acct_ip, radius_ipstr, sizeof(radius_ipstr));
	if (!radius_getattr(&(radid->req_packet), &attr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
		memcpy(username, attr->v.t, attr->l-2);
	}
	if (!radius_getattr(&radid->req_packet, &attr, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0)) {
		memcpy(user_macstr, attr->v.t, attr->l-2);
	}
	if (!radius_getattr(&radid->req_packet, &attr, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0)) {
		memcpy(nasid, attr->v.t, attr->l-2);
	}
	eag_get_debug_filter_key(session_filter_prefix, sizeof(session_filter_prefix),
				user_ipstr, user_macstr, username,radius->hansi_type,radius->hansi_id);
	
	appconn = appconn_find_by_useripx(radius->appdb, &(radid->user_addr));
	if (NULL == appconn) { /* may be acct-stop */
		eag_log_debug("eag_radius",
			"acct_response_proc, not found appconn by userip %s",
			user_ipstr);
	}
	if (!radius_getattr(&(radid->req_packet), &attr, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0, 0)) {
		switch (attr->v.i) {
		case RADIUS_STATUS_TYPE_START:
			eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_RESPONSE_START_COUNT, 1);
			admin_log_notice("RadiusAcctResponseStart___UserName:%s,UserIP:%s,UserMAC:%s,NasIP:%s,RadiusAcctIP:%s,NasID:%s",
				username, user_ipstr, user_macstr, nas_ipstr, radius_ipstr, nasid);
			eag_log_filter(session_filter_prefix,"RadiusAcctResponseStart___UserName:%s,UserIP:%s,UserMAC:%s,NasIP:%s,RadiusAcctIP:%s,NasID:%s",
				username, user_ipstr, user_macstr, nas_ipstr, radius_ipstr, nasid);
			break;
		case RADIUS_STATUS_TYPE_STOP:
			eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_RESPONSE_STOP_COUNT, 1);
			admin_log_notice("RadiusAcctResponseStop___UserName:%s,UserIP:%s,UserMAC:%s,NasIP:%s,RadiusAcctIP:%s,NasID:%s",
				username, user_ipstr, user_macstr, nas_ipstr, radius_ipstr, nasid);
			eag_log_filter(session_filter_prefix,"RadiusAcctResponseStop___UserName:%s,UserIP:%s,UserMAC:%s,NasIP:%s,RadiusAcctIP:%s,NasID:%s",
				username, user_ipstr, user_macstr, nas_ipstr, radius_ipstr, nasid);
			break;
		case RADIUS_STATUS_TYPE_INTERIM_UPDATE:
			eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_RESPONSE_UPDATE_COUNT, 1);
			admin_log_notice("RadiusAcctResponseUpdate___UserName:%s,UserIP:%s,UserMAC:%s,NasIP:%s,RadiusAcctIP:%s,NasID:%s",
				username, user_ipstr, user_macstr, nas_ipstr, radius_ipstr, nasid);
			eag_log_filter(session_filter_prefix,"RadiusAcctResponseUpdate___UserName:%s,UserIP:%s,UserMAC:%s,NasIP:%s,RadiusAcctIP:%s,NasID:%s",
				username, user_ipstr, user_macstr, nas_ipstr, radius_ipstr, nasid);
			break;
		default:
			break;
		}
	}

	return EAG_RETURN_OK;
}

static int
sockradius_receive(eag_thread_t *thread)
{
	sock_radius_t *sockradius = NULL;
	ssize_t nbyte = 0;
	struct radius_packet_t packet = {0};
	struct sockaddr_in addr = {0};
	socklen_t len = 0;
	uint32_t radius_ip = 0;
	uint16_t radius_port = 0;
	char radius_ipstr[32] = "";
	struct radius_id *radid = NULL;
	struct timeval tv = {0};
	time_t timenow = 0;
	eag_radius_t *radius = NULL;
	//int is_distributed = 0;
	int rdc_distributed = 0;
	struct radius_attr_t *attr = NULL;
	char username[256] = "";
	char user_ipstr[IPX_LEN] = "";
	char user_macstr[32] = "";
	char nasid[RADIUS_MAX_NASID_LEN] = "";
	char session_filter_prefix[512] = ""; /* add for debug-filter */
	char replymsg[256] = "";
	uint32_t nasip = 0;
	char nas_ipstr[32]= "";

	if (NULL == thread) {
		eag_log_err("sockradius_receive input error");
		return EAG_ERR_INPUT_PARAM_ERR;	
	}

	sockradius = eag_thread_get_arg(thread);
	if (NULL == sockradius) {
		eag_log_err("sockradius_receive sockradius null");
		return EAG_ERR_NULL_POINTER;
	}
	radius = sockradius->radius;
		
	rdc_distributed = eag_ins_get_rdc_distributed(radius->eagins);
	nasip = eag_ins_get_nasip(radius->eagins);
	
	len = sizeof(addr);
	if (rdc_distributed) {
		nbyte = rdc_recvfrom(sockradius->sockfd, &(packet), sizeof(packet), 0,
						(struct sockaddr *)&addr, &len);
		if (nbyte < 0) {
			eag_log_err("sockradius_receive rdc_recvfrom failed fd(%d)",
				sockradius->sockfd);
			return EAG_ERR_SOCKET_RECV_FAILED;
		}
	} else {
		nbyte = recvfrom(sockradius->sockfd, &(packet), sizeof(packet), 0,
					(struct sockaddr *)&addr, &len);
		if (nbyte < 0) {
			eag_log_err("sockradius_receive recvfrom failed: fd(%d) %s",
				sockradius->sockfd, safe_strerror(errno));
			return EAG_ERR_SOCKET_RECV_FAILED;
		}
	}

	radius_ip = ntohl(addr.sin_addr.s_addr);
	radius_port = ntohs(addr.sin_port);
	ip2str(radius_ip, radius_ipstr, sizeof(radius_ipstr));
	ip2str(nasip, nas_ipstr, sizeof(nas_ipstr));
	
	eag_log_debug("eag_radius", "sockradius fd(%d) receive %d bytes from %s:%u",
				sockradius->sockfd, nbyte, radius_ipstr, radius_port);
		
	if (nbyte < RADIUS_PACKET_HEADSIZE) {
		eag_log_warning("sockradius_receive packet size %d < min %d",
			nbyte, RADIUS_PACKET_HEADSIZE);
		return -1;
	}

	if (nbyte > sizeof(packet)) {
		eag_log_warning("sockradius_receive packet size %d > max %d",
			nbyte, sizeof(packet));
		return -1;
	}

	if (nbyte != ntohs(packet.length)) {
		eag_log_warning("sockradius_receive packet size %d != len %d",
			nbyte, ntohs(packet.length));
		return -1;
	}

	radid = &(sockradius->id[packet.id]);
	if (RADIUS_ID_USED != radid->status) {
		eag_log_warning("sockradius_receive fd(%d) id(%u) status %u not used",
			sockradius->sockfd, packet.id, radid->status);
		return -1;
	}

	eag_time_gettimeofday(&tv, NULL);
	timenow = tv.tv_sec;
	
	if (RADIUS_CODE_ACCESS_ACCEPT == packet.code
		|| RADIUS_CODE_ACCESS_REJECT == packet.code) {
		if (radius_ip != radid->radius_srv.auth_ip 
			&& radius_ip != radid->radius_srv.backup_auth_ip) {
			eag_log_warning("sockradius_receive fd(%d) id(%u), radius_ip %s is neither "
				"auth_ip %#X nor backup_auth_ip %#X",
				sockradius->sockfd, packet.id, radius_ipstr,
				radid->radius_srv.auth_ip, radid->radius_srv.backup_auth_ip);
			return -1;
		}
		if (radius_port != radid->radius_srv.auth_port
			&& radius_port != radid->radius_srv.backup_auth_port) {
			eag_log_warning("sockradius_receive fd(%d) id(%u), radius_port %u is neither "
				"auth_port %u nor backup_auth_port %u",
				sockradius->sockfd, packet.id, radius_port,
				radid->radius_srv.auth_port, radid->radius_srv.backup_auth_port);
			return -1;
		}
		if (0 != radius_reply_check(&packet, &(radid->req_packet),
						radid->radius_srv.auth_secret,
						radid->radius_srv.auth_secretlen)) {
			eag_log_warning("sockradius_receive fd(%d) id(%u) authenticator not match",
				sockradius->sockfd, packet.id);
			return -1;
		}
	}else if (RADIUS_CODE_ACCOUNTING_RESPONSE == packet.code) {
		if (radius_ip != radid->radius_srv.acct_ip
			&& radius_ip != radid->radius_srv.backup_acct_ip) {
			eag_log_warning("sockradius_receive fd(%d) id(%u), radius_ip %s is neither "
				"acct_ip %#X nor backup_acct_ip %#X",
				sockradius->sockfd, packet.id, radius_ipstr,
				radid->radius_srv.acct_ip, radid->radius_srv.backup_acct_ip);
			return -1;
		}
		if (radius_port != radid->radius_srv.acct_port
			&& radius_port != radid->radius_srv.backup_acct_port) {
			eag_log_warning("sockradius_receive fd(%d) id(%u), radius_port %u is neither "
				"acct_port %u nor backup_acct_port %u",
				sockradius->sockfd, packet.id, radius_port,
				radid->radius_srv.acct_port, radid->radius_srv.backup_acct_port);
			return -1;
		}
		if (0 != radius_reply_check(&packet, &(radid->req_packet),
						radid->radius_srv.acct_secret,
						radid->radius_srv.acct_secretlen)) {
			eag_log_warning("sockradius_receive fd(%d) id(%u) acct authenticator not match, "
				"acct_secret=%s, acct_secretlen=%u",
				sockradius->sockfd, packet.id,
				radid->radius_srv.acct_secret, radid->radius_srv.acct_secretlen);
			return -1;
		}
	} else {
		eag_log_warning("sockradius_receive fd(%d) id(%u), "
			"receive an unexpected packet code %u",
				sockradius->sockfd, packet.id, packet.code);
			return -1;
	}

	eag_log_debug("eag_radius","sockradius fd(%d) receive packet: id(%u) code(%u) from radius %s:%u",
				sockradius->sockfd, packet.id, packet.code, radius_ipstr, radius_port);
	if (NULL != radid->t_timeout) {
		eag_thread_cancel(radid->t_timeout);
		radid->t_timeout = NULL;
	}
	radid->timeout = radid->total_time + radid->start_time - timenow;
	radid->status = RADIUS_ID_WAIT;
	radius_id_event(RADIUS_ID_STATUS_TIMEOUT, radid);
	eag_log_debug("eag_radius","radid fd(%d), id(%u) receive response, "
			"wait %u second for free status",
			sockradius->sockfd, packet.id, radid->timeout);

	ipx2str(&(radid->user_addr), user_ipstr, sizeof(user_ipstr));
	if (!radius_getattr(&(radid->req_packet), &attr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
		memcpy(username, attr->v.t, attr->l-2);
	}
	
	if (!radius_getattr(&radid->req_packet, &attr, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0)) {
		memcpy(user_macstr, attr->v.t, attr->l-2);
	}
	if (!radius_getattr(&radid->req_packet, &attr, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0)) {
		memcpy(nasid, attr->v.t, attr->l-2);
	}
	
	eag_get_debug_filter_key(session_filter_prefix, sizeof(session_filter_prefix), user_ipstr,
								user_macstr, username, sockradius->radius->hansi_type,
								sockradius->radius->hansi_id);
	if (RADIUS_CODE_ACCESS_ACCEPT == packet.code) {
		admin_log_notice("RadiusAccessAccept___UserName:%s,UserIP:%s,UserMAC:%s,NasIP:%s,RadiusAuthIP:%s,NasID:%s",
			username, user_ipstr, user_macstr, nas_ipstr, radius_ipstr, nasid);
		eag_log_filter(session_filter_prefix,"RadiusAccessAccept___UserName:%s,UserIP:%s,UserMAC:%s,NasIP:%s,RadiusAuthIP:%s,NasID:%s",
			username, user_ipstr, user_macstr, nas_ipstr, radius_ipstr, nasid);
		access_accept_proc(sockradius->radius, &(radid->user_addr), &packet);
	} else if (RADIUS_CODE_ACCESS_REJECT == packet.code) {
		if (!radius_getattr(&packet, &attr, RADIUS_ATTR_REPLY_MESSAGE, 0, 0, 0)) {
			memcpy(replymsg, attr->v.t, attr->l - 2);
		}
		admin_log_notice("RadiusAccessReject___UserName:%s,UserIP:%s,UserMAC:%s,NasIP:%s,RadiusAuthIP:%s,NasID:%s,ReplyMessage:%s",
			username, user_ipstr, user_macstr, nas_ipstr, radius_ipstr, nasid, replymsg);
		eag_log_filter(session_filter_prefix,"RadiusAccessReject___UserName:%s,UserIP:%s,UserMAC:%s,NasIP:%s,RadiusAuthIP:%s,NasID:%s,ReplyMessage:%s",
			username, user_ipstr, user_macstr, nas_ipstr, radius_ipstr, nasid, replymsg);
		access_reject_proc(sockradius->radius, &(radid->user_addr), &packet);
	} else {
		acct_response_proc(sockradius->radius, radid, &packet);
	}
	
	return EAG_RETURN_OK;
}

static int
radius_id_retry_timeout(eag_thread_t *thread)
{
	struct app_conn_t *appconn = NULL;	//add only used for bss stat
	struct radius_id *radid = NULL;
	sock_radius_t *sockradius = NULL;
	eag_radius_t *radius = NULL;
	uint32_t radius_ip = 0;
	uint16_t radius_port = 0;
	struct timeval tv = {0};
	time_t timenow = 0;
	char user_ipstr[IPX_LEN] = "";
	char user_macstr[32] = "";
	char nasid[RADIUS_MAX_NASID_LEN] = "";
	struct radius_attr_t *attr = NULL;
	char username[256] = "";
	char session_filter_prefix[512] = ""; /* add for debug-filter */
	
	if (NULL == thread) {
		eag_log_err("radius_id_retry_timeout input error");
		return -1;
	}

	radid = eag_thread_get_arg(thread);
	if (NULL == radid) {
		eag_log_err("radius_id_retry_timeout radid null");
		return -1;
	}

	if (NULL != radid->t_timeout) {
		eag_thread_cancel(radid->t_timeout);
		radid->t_timeout = NULL;
	}

	sockradius = radid->sockradius;
	if (NULL == sockradius) {
		eag_log_err("radius_id_retry_timeout sockradius null");
		return -1;
	}
	radius = sockradius->radius;

	appconn = appconn_find_by_useripx(radius->appdb, &(radid->user_addr));	//used for bss stat
	
	eag_time_gettimeofday(&tv, NULL);
	timenow = tv.tv_sec;
	
	eag_log_debug("eag_radius","radius_id_retry_timeout sockradius fd(%d) id(%u)",
			sockradius->sockfd, radid->index);

	ipx2str(&(radid->user_addr), user_ipstr, sizeof(user_ipstr));
	if (!radius_getattr(&(radid->req_packet), &attr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
		memcpy(username, attr->v.t, attr->l-2);
	}
	if (!radius_getattr(&radid->req_packet, &attr, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0)) {
		memcpy(user_macstr, attr->v.t, attr->l-2);
	}
	if (!radius_getattr(&radid->req_packet, &attr, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0)) {
		memcpy(nasid, attr->v.t, attr->l-2);
	}
	
	eag_get_debug_filter_key(session_filter_prefix, sizeof(session_filter_prefix), user_ipstr,
						user_macstr, username, radius->hansi_type, radius->hansi_id);
	if (radid->retry_count < radius->retry_times) {
		if (RADIUS_CODE_ACCESS_REQUEST == radid->req_packet.code) {
			radius_ip = radid->radius_srv.auth_ip;
			radius_port = radid->radius_srv.auth_port;
			eag_bss_message_count(radius->eagstat, appconn, BSS_ACCESS_REQUEST_RETRY_COUNT, 1);
			eag_log_filter(session_filter_prefix, "RadiusAccessRequestRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
					username, user_ipstr, user_macstr, nasid);
			admin_log_notice("RadiusAccessRequestRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
					username, user_ipstr, user_macstr, nasid);
		} else {
			radius_ip = radid->radius_srv.acct_ip;
			radius_port = radid->radius_srv.acct_port;
			radius_getattr(&(radid->req_packet), &attr, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0, 0);
			switch (attr->v.i) {
			case RADIUS_STATUS_TYPE_START:
				eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_REQUEST_START_RETRY_COUNT, 1);
				eag_log_filter(session_filter_prefix, "RadiusAcctRequestStartRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				admin_log_notice("RadiusAcctRequestStartRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				break;
			case RADIUS_STATUS_TYPE_INTERIM_UPDATE:
				eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_REQUEST_UPDATE_RETRY_COUNT, 1);
				eag_log_filter(session_filter_prefix, "RadiusAcctRequestUpdateRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				admin_log_notice("RadiusAcctRequestUpdateRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				break;
			case RADIUS_STATUS_TYPE_STOP:
				eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_REQUEST_STOP_RETRY_COUNT, 1);
				eag_log_filter(session_filter_prefix, "RadiusAcctRequestStopRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				admin_log_notice("RadiusAcctRequestStopRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				break;
			default:
				break;
			}
		}
		radid->retry_count++;
	} else if (radid->vice_retry_count < radius->vice_retry_times) {
		if (RADIUS_CODE_ACCESS_REQUEST == radid->req_packet.code) {
			radius_ip = radid->radius_srv.backup_auth_ip;
			radius_port = radid->radius_srv.backup_auth_port;
			eag_log_filter(session_filter_prefix, "RadiusAccessRequestRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
					username, user_ipstr, user_macstr, nasid);
			admin_log_notice("RadiusAccessRequestRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
					username, user_ipstr, user_macstr, nasid);
			eag_bss_message_count(radius->eagstat, appconn, BSS_ACCESS_REQUEST_RETRY_COUNT, 1);
		} else {
			radius_ip = radid->radius_srv.backup_acct_ip;
			radius_port = radid->radius_srv.backup_acct_port;
			radius_getattr(&(radid->req_packet), &attr, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0, 0);
			switch (attr->v.i) {
			case RADIUS_STATUS_TYPE_START:
				eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_REQUEST_START_RETRY_COUNT, 1);
				eag_log_filter(session_filter_prefix, "RadiusAcctRequestStartRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				admin_log_notice("RadiusAcctRequestStartRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				break;
			case RADIUS_STATUS_TYPE_INTERIM_UPDATE:
				eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_REQUEST_UPDATE_RETRY_COUNT, 1);
				eag_log_filter(session_filter_prefix, "RadiusAcctRequestUpdateRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				admin_log_notice("RadiusAcctRequestUpdateRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				break;
			case RADIUS_STATUS_TYPE_STOP:
				eag_bss_message_count(radius->eagstat, appconn, BSS_ACCT_REQUEST_STOP_RETRY_COUNT, 1);
				eag_log_filter(session_filter_prefix, "RadiusAcctRequestStopRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				admin_log_notice("RadiusAcctRequestStopRetry___UserName:%s,UserIP:%s,UserMAC:%s,NasID:%s",
						username, user_ipstr, user_macstr, nasid);
				break;
			default:
				break;
			}
		}
		radid->vice_retry_count++;
	} else {
		if (RADIUS_CODE_ACCESS_REQUEST == radid->req_packet.code) {
			eag_bss_message_count(radius->eagstat, appconn, BSS_ACCESS_REQUEST_TIMEOUT_COUNT, 1);
		}
		radid->timeout = radid->total_time + radid->start_time - timenow;
		radid->status = RADIUS_ID_WAIT;
		radius_id_event(RADIUS_ID_STATUS_TIMEOUT, radid);
		eag_log_debug("eag_radius","radius_id_retry_timeout, retry times reach limit, "
			"radius id become wait status, fd=%d, id=%u",
			sockradius->sockfd, radid->index);
		return 0;
	}

	eag_log_debug("eag_radius","radius_id_retry_timeout userip %s, fd=%d, "
		"radius server=%#x:%u, packet code=%u, id=%u",
		user_ipstr, sockradius->sockfd,
		radius_ip, radius_port, radid->req_packet.code, radid->req_packet.id);

	sock_radius_send_packet(sockradius, radius_ip, radius_port, &(radid->req_packet));
	radid->timeout = radius->retry_interval;
	radius_id_event(RADIUS_ID_RETRY_TIMEOUT, radid);
	
	return 0;
}

static int
radius_id_status_timeout(eag_thread_t *thread)
{
	struct radius_id *radid = NULL;
	sock_radius_t *sockradius = NULL;
	
	if (NULL == thread) {
		eag_log_err("radius_id_status_timeout input error");
		return -1;
	}

	radid = eag_thread_get_arg(thread);
	if (NULL == radid) {
		eag_log_err("radius_id_status_timeout radid null");
		return -1;
	}

	if (NULL != radid->t_timeout) {
		eag_thread_cancel(radid->t_timeout);
		radid->t_timeout = NULL;
	}

	sockradius = radid->sockradius;
	if (NULL == sockradius) {
		eag_log_err("radius_id_status_timeout sockradius null");
		return -1;
	}

	eag_log_debug("eag_radius","radius_id_status_timeout sockradius fd(%d) id(%u)",
				sockradius->sockfd, radid->index);
	
	radid->status = RADIUS_ID_FREE;
	memset(&(radid->req_packet), 0, sizeof(radid->req_packet));
	radid->timeout = 0;
	radid->retry_count = 0;
	radid->vice_retry_count = 0;
	radid->total_time = 0;
	radid->start_time = 0;
	memset(&(radid->radius_srv), 0, sizeof(radid->radius_srv));
	
	return 0;
}

eag_radius_t *
eag_radius_new(int hansitype, int insid)
{
	eag_radius_t *radius = NULL;

	radius = eag_malloc(sizeof(*radius));
	if (NULL == radius) {
		eag_log_err("eag_radius_new eag_malloc failed");
		return NULL;
	}

	memset(radius, 0, sizeof(*radius));
	if (EAG_RETURN_OK != eag_blkmem_create(&(radius->sockradius_blkmem),
					       SOCKRADIUS_BLKMEM_NAME,
					       sizeof(sock_radius_t),
					       SOCKRADIUS_BLKMEM_ITEMNUM,
					       SOCKRADIUS_BLKMEM_MAXNUM)) {
		eag_log_err("eag_radius_new blkmem_create failed");
		eag_free(radius);
		radius = NULL;
		return NULL;
	}
	INIT_LIST_HEAD(&(radius->sockradius_list));
	INIT_LIST_HEAD(&(radius->errcode_head));
	init_radius_err_code(&(radius->errcode_head),
				 RADIUS_ERR_CODE_PROFILE_CMCC);
	radius->acct_interval = DEFAULT_ACCT_INTERVAL;
	radius->retry_interval = DEFAULT_RADIUS_RETRY_INTERVAL;
	radius->retry_times = DEFAULT_RADIUS_RETRY_TIMES;
	radius->vice_retry_times = DEFAULT_VICE_RADIUS_RETRY_TIMES;
	radius->hansi_type = hansitype;
	radius->hansi_id = insid;

	eag_log_info("radius new ok");
	return radius;
}

int
eag_radius_free(eag_radius_t *radius)
{
	if (NULL == radius) {
		eag_log_err("eag_radius_free input error");
		return -1;
	}
	
	if (NULL != radius->sockradius_blkmem) {
		eag_blkmem_destroy(&(radius->sockradius_blkmem));
	}
	release_radius_err_code(&(radius->errcode_head));
	eag_free(radius);

	eag_log_info("radius free ok");
	
	return 0;
}

int
eag_radius_start(eag_radius_t *radius)
{
	sock_radius_t *sockradius = NULL;
	int ret = 0;
	
	if (NULL == radius) {
		eag_log_err("eag_radius_start input error");
		return -1;
	}

	list_for_each_entry(sockradius, &(radius->sockradius_list), node) {
		if ( (ret = sock_radius_start(sockradius)) != EAG_RETURN_OK) {
			break;
		}
	}

	if (EAG_RETURN_OK != ret) {
		list_for_each_entry(sockradius, &(radius->sockradius_list), node) {
			sock_radius_stop(sockradius);
		}
		return ret;
	}

	eag_log_info("radius start ok");
	return EAG_RETURN_OK;
}

int
eag_radius_stop(eag_radius_t *radius)
{
	sock_radius_t *sockradius = NULL;
	sock_radius_t *next = NULL;
		
	if (NULL == radius) {
		eag_log_err("eag_radius_stop input error");
		return -1;
	}
	
	list_for_each_entry_safe(sockradius, next, 
		&(radius->sockradius_list), node) {
		sock_radius_stop(sockradius);
		sock_radius_free(sockradius);
	}

	eag_log_info("radius stop ok");
	return EAG_RETURN_OK;
}

int
div_username_and_domain(char *username,
		char *domain,
		size_t domain_size,
		struct radius_conf *radiusconf)
{
	char *at_point = NULL;
	char *get_domain = NULL;
	int i = 0;

	if (NULL == username || NULL == domain || domain_size <= 0 
		|| NULL == radiusconf) {
		eag_log_err("div_username_and_domain input error");
		return EAG_RETURN_OK;
	}

	eag_log_debug("eag_radius", "div_username_and_domain username=%s, "
				"origin domain=%s",
				username, domain);

	at_point = strrchr(username,'@');
	if (NULL == at_point) {
		eag_log_debug("eag_radius", "div_username_and_domain not found @");
		return EAG_RETURN_OK;
	}
	get_domain = at_point + 1;
	if ('\0' == *get_domain) {
		eag_log_debug("eag_radius", "div_username_and_domain not found domain");
		return EAG_RETURN_OK;
	}

	eag_log_debug("eag_radius", "div_username_and_domain "
		"get_domain=%s, radiusconf->current_num=%d",
		get_domain, radiusconf->current_num);
	
	for (i = 0; i < radiusconf->current_num; i++) {
		if (strcmp(radiusconf->radius_srv[i].domain, get_domain) == 0) {
			if (1 == radiusconf->radius_srv[i].remove_domain_name) {
				*at_point = '\0';
				eag_log_debug("eag_radius", "div_username_and_domain " 
					"div the domain after username,new username:%s",
					username);
			}
			strncpy(domain, get_domain, domain_size-1);
			eag_log_debug("eag_radius", "div_username_and_domain "
				"match domain %s, new username=%s",
				domain, username);
			return EAG_RETURN_OK;
		}
	}

	for (i = 0; i < radiusconf->current_num; i++) {
		if (strcmp(radiusconf->radius_srv[i].domain, domain) == 0) {
			if (1 == radiusconf->radius_srv[i].remove_domain_name) {
				*at_point = '\0';
				eag_log_debug("eag_radius", "div_username_and_domain " 
					"div the domain after username,new username:%s",
					username);
			}
			return EAG_RETURN_OK;
		}
	}

	if (radiusconf->current_num > 0) {
		if (1 == radiusconf->radius_srv[0].remove_domain_name) {
			*at_point = '\0';
			eag_log_debug("eag_radius", "div_username_and_domain " 
				"div the domain after username,new username:%s",
				username);
		}
		return EAG_RETURN_OK;
		
	}

	eag_log_debug("eag_radius", "div_username_and_domain not match domain, "
		"domain=%s, usename=%s", domain, username);

	return EAG_RETURN_OK;
}

int
eag_radius_set_local_ip(eag_radius_t *radius,
		uint32_t local_ip)
{
	if (NULL == radius) {
		eag_log_err("eag_radius_set_local_ip input error");
		return -1;
	}

	radius->local_ip = local_ip;

	return 0;
}

int
eag_radius_set_retry_params(eag_radius_t *radius,
		int retry_interval,
		int retry_times,
		int vice_retry_times)
{
	if (NULL == radius) {
		eag_log_err("eag_radius_set_retry_params input error");
		return -1;
	}
	
	radius->retry_interval = retry_interval;
	radius->retry_times = retry_times;
	radius->vice_retry_times = vice_retry_times;
	
	return 0;
}

int
eag_radius_get_retry_params(eag_radius_t *radius,
		int *retry_interval,
		int *retry_times,
		int *vice_retry_times)
{
	if (NULL == radius) {
		eag_log_err("eag_radius_set_retry_params input error");
		return -1;
	}
	
	*retry_interval = radius->retry_interval;
	*retry_times = radius->retry_times;
	*vice_retry_times = radius->vice_retry_times;
	
	return 0;
}

int
eag_radius_set_acct_interval(eag_radius_t *radius,
		int acct_interval)
{
	if (NULL == radius) {
		eag_log_err("eag_radius_set_acct_interval input error");
		return -1;
	}
	
	radius->acct_interval = acct_interval;

	return 0;
}

int
eag_radius_get_acct_interval(eag_radius_t *radius)
{
	if (NULL == radius) {
		eag_log_err("eag_radius_get_acct_interval input error");
		return -1;
	}
	
	return radius->acct_interval;
}

int
eag_radius_set_thread_master(eag_radius_t *radius,
		eag_thread_master_t *master)
{
	if (NULL == radius) {
		eag_log_err("eag_radius_set_thread_master input error");
		return -1;
	}

	radius->master = master;

	return 0;
}

int
eag_radius_get_single_specific_attr(char *line, int size,
								radius_specific_attr_t *radius_specific_attr)
{
	char *specific_attr = NULL;

	if (NULL == radius_specific_attr || NULL == line) {
		eag_log_err("eag_radius_get_single_specific_attr input error");
		return -1;
	}
	specific_attr = strstr(line, "audit_ip");
	if (NULL != specific_attr) {
		specific_attr = strchr(line, '=');
		*specific_attr = '\0';
		specific_attr++;
		radius_specific_attr->audit_ip = strtol(specific_attr, NULL, 10);
		eag_log_debug("eag_radius", 
				"eag_radius_get_single_specific_attr  audit_ip=%u\n",
				radius_specific_attr->audit_ip);
		return 0;
	}
	specific_attr = strstr(line, "user_agent");
	if (NULL != specific_attr) {
		specific_attr = strchr(line, '=');
		*specific_attr = '\0';
		specific_attr++;
		radius_specific_attr->user_agent = strtol(specific_attr, NULL, 10);
		eag_log_debug("eag_radius", 
				"eag_radius_get_single_specific_attr  user_agent=%u\n",
				radius_specific_attr->user_agent);
		return 0;
	}
	specific_attr = strstr(line, "user_type");
	if (NULL != specific_attr) {
		specific_attr = strchr(line, '=');
		*specific_attr = '\0';
		specific_attr++;
		radius_specific_attr->user_type = strtol(specific_attr, NULL, 10);
		eag_log_debug("eag_radius", 
				"eag_radius_get_single_specific_attr  user_type=%u\n",
				radius_specific_attr->user_type);
		return 0;
	}
	specific_attr = strstr(line, "framed_ipv6_address");
	if (NULL != specific_attr) {
		specific_attr = strchr(line, '=');
		*specific_attr = '\0';
		specific_attr++;
		radius_specific_attr->framed_ipv6_address = strtol(specific_attr, NULL, 10);
		eag_log_debug("eag_radius", 
				"eag_radius_get_single_specific_attr  framed_ipv6_address=%u\n",
				radius_specific_attr->framed_ipv6_address);
		return 0;
	}
	specific_attr = strstr(line, "acct_ipv6_input_octets");
	if (NULL != specific_attr) {
		specific_attr = strchr(line, '=');
		*specific_attr = '\0';
		specific_attr++;
		radius_specific_attr->acct_ipv6_input_octets = strtol(specific_attr, NULL, 10);
		eag_log_debug("eag_radius", 
				"eag_radius_get_single_specific_attr  acct_ipv6_input_octets=%u\n",
				radius_specific_attr->acct_ipv6_input_octets);
		return 0;
	}
	specific_attr = strstr(line, "acct_ipv6_output_octets");
	if (NULL != specific_attr) {
		specific_attr = strchr(line, '=');
		*specific_attr = '\0';
		specific_attr++;
		radius_specific_attr->acct_ipv6_output_octets = strtol(specific_attr, NULL, 10);
		eag_log_debug("eag_radius", 
				"eag_radius_get_single_specific_attr  acct_ipv6_output_octets=%u\n",
				radius_specific_attr->acct_ipv6_output_octets);
		return 0;
	}
	specific_attr = strstr(line, "acct_ipv6_input_packets");
	if (NULL != specific_attr) {
		specific_attr = strchr(line, '=');
		*specific_attr = '\0';
		specific_attr++;
		radius_specific_attr->acct_ipv6_input_packets = strtol(specific_attr, NULL, 10);
		eag_log_debug("eag_radius", 
				"eag_radius_get_single_specific_attr  acct_ipv6_input_packets=%u\n",
				radius_specific_attr->acct_ipv6_input_packets);
		return 0;
	}
	specific_attr = strstr(line, "acct_ipv6_output_packets");
	if (NULL != specific_attr) {
		specific_attr = strchr(line, '=');
		*specific_attr = '\0';
		specific_attr++;
		radius_specific_attr->acct_ipv6_output_packets = strtol(specific_attr, NULL, 10);
		eag_log_debug("eag_radius", 
				"eag_radius_get_single_specific_attr  acct_ipv6_output_packets=%u\n",
				radius_specific_attr->acct_ipv6_output_packets);
		return 0;
	}
	specific_attr = strstr(line, "acct_ipv6_input_gigawords");
	if (NULL != specific_attr) {
		specific_attr = strchr(line, '=');
		*specific_attr = '\0';
		specific_attr++;
		radius_specific_attr->acct_ipv6_input_gigawords = strtol(specific_attr, NULL, 10);
		eag_log_debug("eag_radius", 
				"eag_radius_get_single_specific_attr  acct_ipv6_input_gigawords=%u\n",
				radius_specific_attr->acct_ipv6_input_gigawords);
		return 0;
	}
	specific_attr = strstr(line, "acct_ipv6_output_gigawords");
	if (NULL != specific_attr) {
		specific_attr = strchr(line, '=');
		*specific_attr = '\0';
		specific_attr++;
		radius_specific_attr->acct_ipv6_output_gigawords = strtol(specific_attr, NULL, 10);
		eag_log_debug("eag_radius", 
				"eag_radius_get_single_specific_attr  acct_ipv6_output_gigawords=%u\n",
				radius_specific_attr->acct_ipv6_output_gigawords);
		return 0;
	}
	/* TODO: Please expand new specific attr */

	return 0;
}

int
eag_radius_get_radius_specific_attr(radius_specific_attr_t *radius_specific_attr,
											const char *file_path)
{
	FILE *fp = NULL;
	char line[1024];
	int iret = -1;

	if (NULL == radius_specific_attr) {
		eag_log_err("eag_radius_get_radius_specific_attr input error");
		return iret;
	}

	if (NULL == file_path || strlen(file_path) == 0) {
		eag_log_err("eag_radius_get_radius_specific_attr  file path error!");
		return iret;
	}

	fp = fopen(file_path, "r");
	if (NULL != fp) {
		do {
			memset(line, 0, sizeof(line));
			if ( NULL == fgets(line, sizeof (line), fp) ) {
				break;
			}
			eag_log_debug("eag_radius", 
    					"eag_radius_get_radius_specific_attr  get line=%s\n", line);
			eag_radius_get_single_specific_attr(line, sizeof(line), radius_specific_attr);
		} while (!feof(fp));

		fclose(fp);
		iret = 0;
	} else {
		eag_log_err("file %s open error!\n", file_path);
		return iret;
	}

	eag_log_debug("eag_radius", 
			"eag_radius_get_radius_specific_attr load radius specific attr success!\n");

	return iret;
}

int
eag_radius_set_radius_vendor_id(eag_radius_t *radius,
										uint32_t vendor_id)
{
	char radius_specific_attr_file[64] = "";
	radius_specific_attr_t *radius_specific_attr = NULL;

	if (NULL == radius) {
		eag_log_err("eag_radius_set_radius_vendor_id input error");
		return -1;
	}

	if (0 == vendor_id) {
		radius->vendor_id = RADIUS_VENDOR_AUTELAN;
	} else if (RADIUS_OID_DTT == vendor_id){
		radius->vendor_id = RADIUS_VENDOR_DTT;
	} else {
		radius->vendor_id = vendor_id;
	}
	eag_log_info("eag_radius_set_radius_vendor_id vendor_id: %u\n", radius->vendor_id);
	radius_specific_attr = &(radius->radius_specific_attr);

	memset(radius_specific_attr_file, 0, sizeof(radius_specific_attr_file));
	snprintf(radius_specific_attr_file, sizeof(radius_specific_attr_file)-1, 
				"%s/%u", RADIUS_SPECIFIC_ATTR_PATH, radius->vendor_id);
	eag_log_debug("eag_radius", 
			"eag_radius_set_radius_vendor_id  specific_attr_file=%s\n", radius_specific_attr_file);

	eag_radius_get_radius_specific_attr(radius_specific_attr, radius_specific_attr_file);

	return EAG_RETURN_OK;
}

int
eag_radius_set_radius_conf(eag_radius_t *radius,
		struct radius_conf *radiusconf)
{
	if (NULL == radius) {
		eag_log_err("eag_radius_set_radius_conf input error");
		return -1;
	}

	radius->radiusconf = radiusconf;

	return EAG_RETURN_OK;
}

int
eag_radius_set_eagins(eag_radius_t *radius,
		eag_ins_t *eagins)
{
	if (NULL == radius) {
		eag_log_err("eag_radius_set_eagins input error");
		return -1;
	}

	radius->eagins = eagins;

	return EAG_RETURN_OK;
}

int
eag_radius_set_appdb(eag_radius_t *radius,
		appconn_db_t *appdb)
{
	if (NULL == radius) {
		eag_log_err("eag_radius_set_appdb input error");
		return -1;
	}

	radius->appdb = appdb;

	return EAG_RETURN_OK;
}

int
eag_radius_set_eagstat(eag_radius_t *radius,
		eag_statistics_t *eagstat)
{
	if (NULL == radius || NULL == eagstat) {
		eag_log_err("eag_portal_set_eagstat input error");
		return -1;
	}

	radius->eagstat = eagstat;
	
	return EAG_RETURN_OK;
}

int
eag_radius_set_portal(eag_radius_t *radius,
		eag_portal_t *portal)
{
	if (NULL == portal) {
		eag_log_err("eag_radius_set_portal input error");
		return -1;
	}

	radius->portal = portal;

	return EAG_RETURN_OK;
}

static void
sock_radius_event(sock_radius_event_t event,
		sock_radius_t *sockradius)
{
	if (NULL == sockradius) {
		eag_log_err("sock_radius_event input error");
		return;
	}
	
	switch (event) {
	case SOCK_RADIUS_READ:
		sockradius->t_read =
		    eag_thread_add_read(sockradius->master, sockradius_receive,
					sockradius, sockradius->sockfd);
		if (NULL == sockradius->t_read) {
			eag_log_err("sock_radius_event thread_add_read failed");
		}
		break;
	default:
		break;
	}
}

static void
radius_id_event(radius_id_event_t event,
		struct radius_id *radid)
{
	if (NULL == radid) {
		eag_log_err("radius_id_event input error");
		return;
	}
	
	switch (event) {
	case RADIUS_ID_RETRY_TIMEOUT:
		radid->t_timeout =
		    eag_thread_add_timer(radid->master, radius_id_retry_timeout,
					radid, radid->timeout);
		if (NULL == radid->t_timeout) {
			eag_log_err("radius_id_event thread_add_timer failed");
		}
		break;
	case RADIUS_ID_STATUS_TIMEOUT:
		radid->t_timeout =
		    eag_thread_add_timer(radid->master, radius_id_status_timeout,
					radid, radid->timeout);
		if (NULL == radid->t_timeout) {
			eag_log_err("radius_id_event thread_add_timer failed");
		}
		break;
	default:
		break;
	}
}

const char *
radius_terminate_cause_to_str(int terminate_cause)
{
	switch (terminate_cause) {
	case RADIUS_TERMINATE_CAUSE_USER_REQUEST:
		return "UserRequest";
	case RADIUS_TERMINATE_CAUSE_LOST_CARRIER:
		return "LostCarrier";
	case RADIUS_TERMINATE_CAUSE_LOST_SERVICE:
		return "LostService";
	case RADIUS_TERMINATE_CAUSE_IDLE_TIMEOUT:
		return "IdleTimeout";
	case RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT:
		return "SessionTimeout";
	case RADIUS_TERMINATE_CAUSE_ADMIN_RESET:
		return "AdminReset";
	case RADIUS_TERMINATE_CAUSE_ADMIN_REBOOT:
		return "AdminReboot";
	case RADIUS_TERMINATE_CAUSE_PORT_ERROR:
		return "PortError";
	case RADIUS_TERMINATE_CAUSE_NAS_ERROR:
		return "NasError";
	case RADIUS_TERMINATE_CAUSE_NAS_REQUEST:
		return "NasRequest";
	case RADIUS_TERMINATE_CAUSE_NAS_REBOOT:
		return "NasReboot";
	case RADIUS_TERMINATE_CAUSE_PORT_UNNEEDED:
		return "PortUnneeded";
	case RADIUS_TERMINATE_CAUSE_PORT_PREEMPTED:
		return "PortPreempted";
	case RADIUS_TERMINATE_CAUSE_PORT_SUSPEND:
		return "PortSuspend";
	case RADIUS_TERMINATE_CAUSE_SERVICE_UNAVAILABLE:
		return "ServiceUnavailable";
	case RADIUS_TERMINATE_CAUSE_CALLBACK:
		return "CallBack";
	case RADIUS_TERMINATE_CAUSE_USER_ERROR:
		return "UserError";
	case RADIUS_TERMINATE_CAUSE_HOST_REQUEST:
		return "HostRequest";
	default:
		return "Unknow Reason";
	}
}

