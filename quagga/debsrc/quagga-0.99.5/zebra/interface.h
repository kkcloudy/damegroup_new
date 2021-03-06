/* Interface function header.
 * Copyright (C) 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#ifndef _ZEBRA_INTERFACE_H
#define _ZEBRA_INTERFACE_H

#include "redistribute.h"

#ifdef HAVE_IRDP
#include "zebra/irdp.h"
#endif
/*addad by gxd*/
#define IFC_MAX 8
/*2008-12-3 10:55:41*/
/* For interface multicast configuration. */
#define IF_ZEBRA_MULTICAST_UNSPEC 0
#define IF_ZEBRA_MULTICAST_ON     1
#define IF_ZEBRA_MULTICAST_OFF    2

/* For interface shutdown configuration. */
#define IF_ZEBRA_SHUTDOWN_UNSPEC 0
#define IF_ZEBRA_SHUTDOWN_ON     1
#define IF_ZEBRA_SHUTDOWN_OFF    2

/* Router advertisement feature. */
#if (defined(LINUX_IPV6) && (defined(__GLIBC__) && __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1)) || defined(KAME)
	#ifdef HAVE_RTADV
		#define RTADV
	#endif
#endif

#ifdef RTADV

/*niehy add max count of uplink|downlink interface */
#define VRRP_LINK_MAX_CNT		(8)
typedef struct {
	struct in6_addr sin6_addr;
	uint32_t    mask;
	int		deletable;	/* TRUE if one of my primary addr */
} vipv6_addr;

/* Router advertisement parameter.  From RFC2461 and RFC3775. */
struct rtadvconf
{
  /* A flag indicating whether or not the router sends periodic Router
     Advertisements and responds to Router Solicitations.
     Default: FALSE */
     struct in6_addr dst;
   int adv_if_count;
   int ra_flag;
  int AdvSendAdvertisements;

  /* The maximum time allowed between sending unsolicited multicast
     Router Advertisements from the interface, in milliseconds.
     MUST be no less than 70 ms (RFC3775, section 7.4) and no greater 
     than 1800000 ms (See RFC2461).

     Default: 600000 milliseconds */
  int MaxRtrAdvInterval;
#define RTADV_MAX_RTR_ADV_INTERVAL 600000

  /* The minimum time allowed between sending unsolicited multicast
     Router Advertisements from the interface, in milliseconds.
     MUST be no less than 30 ms (See RFC3775, section 7.4). 
     MUST be no greater than .75 * MaxRtrAdvInterval.

     Default: 0.33 * MaxRtrAdvInterval */
  int MinRtrAdvInterval;
#define RTADV_MIN_RTR_ADV_INTERVAL (0.33 * RTADV_MAX_RTR_ADV_INTERVAL)

  /* Unsolicited Router Advertisements' interval timer. */
  int AdvIntervalTimer;

  /* The TRUE/FALSE value to be placed in the "Managed address
     configuration" flag field in the Router Advertisement.  See
     [ADDRCONF].
 
     Default: FALSE */
  int AdvManagedFlag;


  /* The TRUE/FALSE value to be placed in the "Other stateful
     configuration" flag field in the Router Advertisement.  See
     [ADDRCONF].

     Default: FALSE */
  int AdvOtherConfigFlag;
  /*gujd : 2012-05-29,am 9:53 . Add for IPv6 Ready Test.*/
  int AdvLinkMtuOption;

  /* The value to be placed in MTU options sent by the router.  A
     value of zero indicates that no MTU options are sent.

     Default: 0 */
  int AdvLinkMTU;


  /* The value to be placed in the Reachable Time field in the Router
     Advertisement messages sent by the router.  The value zero means
     unspecified (by this router).  MUST be no greater than 3,600,000
     milliseconds (1 hour).

     Default: 0 */
  u_int32_t AdvReachableTime;
#define RTADV_MAX_REACHABLE_TIME 3600000


  /* The value to be placed in the Retrans Timer field in the Router
     Advertisement messages sent by the router.  The value zero means
     unspecified (by this router).

     Default: 0 */
  /*int AdvRetransTimer;*/
  u_int32_t AdvRetransTimer;/*gujd : 2012-05-29,am 9:53 . Change for IPv6 Ready Test.*/

  /* The default value to be placed in the Cur Hop Limit field in the
     Router Advertisement messages sent by the router.  The value
     should be set to that current diameter of the Internet.  The
     value zero means unspecified (by this router).

     Default: The value specified in the "Assigned Numbers" RFC
     [ASSIGNED] that was in effect at the time of implementation. */
  int AdvCurHopLimit;

  /* The value to be placed in the Router Lifetime field of Router
     Advertisements sent from the interface, in seconds.  MUST be
     either zero or between MaxRtrAdvInterval and 9000 seconds.  A
     value of zero indicates that the router is not to be used as a
     default router.

     Default: 3 * MaxRtrAdvInterval */
  int AdvDefaultLifetime;
#define RTADV_ADV_DEFAULT_LIFETIME (3 * RTADV_MAX_RTR_ADV_INTERVAL)


  /* A list of prefixes to be placed in Prefix Information options in
     Router Advertisement messages sent from the interface.

     Default: all prefixes that the router advertises via routing
     protocols as being on-link for the interface from which the
     advertisement is sent. The link-local prefix SHOULD NOT be
     included in the list of advertised prefixes. */
  struct list *AdvPrefixList;
  struct list *pool;
  int prefix_flag;

  /* The TRUE/FALSE value to be placed in the "Home agent"
     flag field in the Router Advertisement.  See [RFC3775 7.1].

     Default: FALSE */
  int AdvHomeAgentFlag;
#ifndef ND_RA_FLAG_HOME_AGENT
#define ND_RA_FLAG_HOME_AGENT 	0x20
#endif

  /* The value to be placed in Home Agent Information option if Home 
     Flag is set.
     Default: 0 */
  int HomeAgentPreference;

  /* The value to be placed in Home Agent Information option if Home 
     Flag is set. Lifetime (seconds) MUST not be greater than 18.2 
     hours. 
     The value 0 has special meaning: use of AdvDefaultLifetime value.
     
     Default: 0 */
  int HomeAgentLifetime;
#define RTADV_MAX_HALIFETIME 65520 /* 18.2 hours */

  /* The TRUE/FALSE value to insert or not an Advertisement Interval
     option. See [RFC 3775 7.3]

     Default: FALSE */
  int AdvIntervalOption;
/*vipv6_addr *vaddr*/        /* point on the ip address array */
    vipv6_addr uplink_ipv6_vaddr[VRRP_LINK_MAX_CNT];
								/* point on the uplink ip address array */
	vipv6_addr downlink_ipv6_vaddr[VRRP_LINK_MAX_CNT];
								/* point on the downlink ip address array */
	vipv6_addr vgateway_ipv6_vaddr[VRRP_LINK_MAX_CNT];

};

#endif /* RTADV */

struct ipv6_pool
{
	struct in6_addr prefix;
	struct in6_addr link_addr;
	struct ipv6_pool *next;

};


/* `zebra' daemon local interface structure. */
struct zebra_if
{
  /* Shutdown configuration. */
  u_char shutdown;

  /* Multicast configuration. */
  u_char multicast;

  /* Router advertise configuration. */
  u_char rtadv_enable;
  
  /*gujd : 2012-05-29,am 9:53 . Add for IPv6 Ready Test.*/
  struct timeval time_memory;

  /* Interface's address. */
  struct list *address;

  /* Installed addresses chains tree. */
  struct route_table *ipv4_subnets;

#ifdef RTADV
  struct rtadvconf rtadv;
#endif /* RTADV */

struct ipv6_pool **rtadv_prefix_pool;


#ifdef HAVE_IRDP
  struct irdp_interface irdp;
#endif

#ifdef SUNOS_5
  /* the real IFF_UP state of the primary interface.
   * need this to differentiate between all interfaces being
   * down (but primary still plumbed) and primary having gone
   * ~IFF_UP, and all addresses gone.
   */
  u_char primary_state;
#endif /* SUNOS_5 */
};

extern void if_delete_update (struct interface *ifp);
extern void if_add_update (struct interface *ifp);
extern void if_up (struct interface *);
extern void if_down (struct interface *);
extern void if_refresh (struct interface *);
extern void if_flags_update (struct interface *, uint64_t);
extern int if_subnet_add (struct interface *, struct connected *);
extern int if_subnet_delete (struct interface *, struct connected *);

/*gujd: 2012-02-08: pm 5:25 . In order to decrease the warning when make img . For declaration the func .*/
extern void if_up_redistribute(struct interface * ifp);
extern void if_down_redistribute(struct interface * ifp);


#ifdef HAVE_PROC_NET_DEV
extern void ifstat_update_proc (void);
extern void ipv6_ifstat_update_proc (void); //lilong add 2015.2.8
#endif /* HAVE_PROC_NET_DEV */
#ifdef HAVE_NET_RT_IFLIST
extern void ifstat_update_sysctl (void);

#endif /* HAVE_NET_RT_IFLIST */
#ifdef HAVE_PROC_NET_DEV
extern int interface_list_proc (void);
#endif /* HAVE_PROC_NET_DEV */
#ifdef HAVE_PROC_NET_IF_INET6
extern int ifaddr_proc_ipv6 (void);
#endif /* HAVE_PROC_NET_IF_INET6 */

#ifdef BSDI
extern int if_kvm_get_mtu (struct interface *);
#endif /* BSDI */

#endif /* _ZEBRA_INTERFACE_H */
