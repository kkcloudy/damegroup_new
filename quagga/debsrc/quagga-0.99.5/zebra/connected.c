/*
 * Address linked list routine.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
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

#include <zebra.h>

#include "prefix.h"
#include "linklist.h"
#include "if.h"
#include "table.h"
#include "rib.h"
#include "table.h"
#include "log.h"
#include "memory.h"
#include "debug.h"

#include "zebra/zserv.h"
#include "zebra/redistribute.h"
#include "zebra/interface.h"
#include "zebra/connected.h"

extern product_inf *product;
extern int keep_kernel_mode;
/* withdraw a connected address */
static void
connected_withdraw (struct connected *ifc)
{
  if (! ifc)
    return;

  /* Update interface address information to protocol daemon. */
  if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
    {
      zebra_interface_address_delete_update (ifc->ifp, ifc);

      if_subnet_delete (ifc->ifp, ifc);
      
      if (ifc->address->family == AF_INET)
        connected_down_ipv4 (ifc->ifp, ifc);
#ifdef HAVE_IPV6
      else
        connected_down_ipv6 (ifc->ifp, ifc);
#endif

      UNSET_FLAG (ifc->conf, ZEBRA_IFC_REAL);
    }

  if (!CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
    {
	  /*  
		router_id_del_address(ifc);
	    listnode_delete (ifc->ifp->connected, ifc);
	    connected_free (ifc);*/
		
	  if(ifc->address->family == AF_INET)
	  {
		  router_id_del_address(ifc);
		  listnode_delete (ifc->ifp->connected, ifc);
		  connected_free (ifc);
	  }
	  else
	  {
		 if(!CHECK_FLAG(ifc->ipv6_config, RTMD_IPV6_ADDR_CONFIG))
		  {
			 router_id_del_address(ifc);
			 listnode_delete (ifc->ifp->connected, ifc);
			 connected_free (ifc);
		  }
		 else
		  {
			 zlog_debug("%s: line %d, Cannot delete IPv6 config addr .\n",__func__,__LINE__);
		  }
	   }
		
    }
}

static void
connected_announce (struct interface *ifp, struct connected *ifc)
{
  if (!ifc)
    return;
  
  listnode_add (ifp->connected, ifc);

  /* Update interface address information to protocol daemon. */
  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
    {
      if (ifc->address->family == AF_INET)
        if_subnet_add (ifp, ifc);
	  
      SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);
	  /*gjd : add for Rtsuit restart, ip address not write show running .So use add flag RTMD_RESTART_IP_CONFIG ,
	  when set it by netlink info from kernel through the IFA_BROADCAST.  Becaus when setup vrrp , the virtual ip will insert,
	  the IFA_BROADCAST will not get from kenerl . 2011-12-20 pm 7:00*/
	  if((!CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
	  	&&(keep_kernel_mode == 1)
	  	&&(CHECK_FLAG(ifc->ip_config,RTMD_RESTART_IP_CONFIG)))
	  {
	  	SET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);
		/*zlog_err("%s : line %d ifc->conf(%u), ifc->ipconfig(%u).\n",
				__func__,__LINE__,ifc->conf,ifc->ip_config);*/
	  	}
	  if(ifp && (strncmp(ifp->name,"lo",2)==0)&&(!LOOPBACK(ifc->address->u.prefix4.s_addr)))
	  	SET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);
	  	
      zebra_interface_address_add_update (ifp, ifc);

      if (if_is_up(ifp))
        {
          if (ifc->address->family == AF_INET)
	    connected_up_ipv4 (ifp, ifc);
#ifdef HAVE_IPV6
          else
            connected_up_ipv6 (ifp, ifc);
#endif
        }
    }
}

/* If same interface address is already exist... */
struct connected *
connected_check (struct interface *ifp, struct prefix *p)
{
  struct connected *ifc;
  struct listnode *node;
  if(ifp && ifp->connected)
  {
	  for (ALL_LIST_ELEMENTS_RO (ifp->connected, node, ifc))
		if (prefix_same (ifc->address, p))
		  return ifc;
  }

  return NULL;
}

/* Check if two ifc's describe the same address */
static int
connected_same (struct connected *ifc1, struct connected *ifc2)
{
  if (ifc1->ifp != ifc2->ifp)
    return 0;
  
  if (ifc1->destination)
    if (!ifc2->destination)
      return 0;
  if (ifc2->destination)
    if (!ifc1->destination)
      return 0;
  
  if (ifc1->destination && ifc2->destination)
    if (!prefix_same (ifc1->destination, ifc2->destination))
      return 0;

  if (ifc1->flags != ifc2->flags)
    return 0;
  
  return 1;
}

/* Handle implicit withdrawals of addresses, where a system ADDs an address
 * to an interface which already has the same address configured.
 *
 * Returns the struct connected which must be announced to clients,
 * or NULL if nothing to do.
 */
static struct connected *
connected_implicit_withdraw (struct interface *ifp, struct connected *ifc)
{
  struct connected *current;
  
  /* Check same connected route. */
  if ((current = connected_check (ifp, (struct prefix *) ifc->address)))
    {
      if (CHECK_FLAG(current->conf, ZEBRA_IFC_CONFIGURED))
        SET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);
	  
	  if (CHECK_FLAG (current->ipv6_config, RTMD_IPV6_ADDR_CONFIG))
		SET_FLAG (ifc->ipv6_config, RTMD_IPV6_ADDR_CONFIG);
	
      /* Avoid spurious withdraws, this might be just the kernel 'reflecting'
       * back an address we have already added.
       */
      if (connected_same (current, ifc))
        {
          /* nothing to do */
          connected_free (ifc);
          return NULL;
        }
      
      UNSET_FLAG(current->conf, ZEBRA_IFC_CONFIGURED);
      UNSET_FLAG(current->ipv6_config, RTMD_IPV6_ADDR_CONFIG);
      connected_withdraw (current); /* implicit withdraw - freebsd does this */
    }
  return ifc;
}

/* Called from if_up(). */
void
connected_up_ipv4 (struct interface *ifp, struct connected *ifc)
{
  struct prefix_ipv4 p;
  struct prefix_ipv4 *addr;
  struct prefix_ipv4 *dest;

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start", __func__);


 /* if(product != NULL)*/
  if(product != NULL && product->board_type != BOARD_IS_ACTIVE_MASTER)/*gjd : change for active master 在删除ip时，直连路由没删除，因为重复添加，导致refcnt不为0释放不掉*/
  	goto skip;
  if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER &&CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
  	goto skip;/*local interface , skip the check the ifc->conf, because the ip address not install in the kernel .*/

  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
    return;

 skip:

  addr = (struct prefix_ipv4 *) ifc->address;
  dest = (struct prefix_ipv4 *) ifc->destination;

  memset (&p, 0, sizeof (struct prefix_ipv4));
  p.family = AF_INET;
  p.prefixlen = addr->prefixlen;

  /* Point-to-point check. */
  if (CONNECTED_POINTOPOINT_HOST(ifc))
    p.prefix = dest->prefix;
  else
    p.prefix = addr->prefix;

  /* Apply mask to the network. */
  apply_mask_ipv4 (&p);

  /* In case of connected address is 0.0.0.0/0 we treat it tunnel
     address. */
  if (prefix_ipv4_any (&p))
    return;

  rib_add_ipv4 (ZEBRA_ROUTE_CONNECT, 0, &p, NULL, ifp->ifindex, RT_TABLE_MAIN, 
                ifp->metric, 0);

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: goto rib_update ", __func__);
  rib_update ();
}

/* Add connected IPv4 route to the interface. */
void
connected_add_ipv4 (struct interface *ifp, int flags, struct in_addr *addr, 
		    u_char prefixlen, struct in_addr *broad, 
		    const char *label)
{
  struct prefix_ipv4 *p;
  struct connected *ifc;

  /* Make connected structure. */
  ifc = connected_new ();
  ifc->ifp = ifp;
  ifc->flags = flags;

  /* Allocate new connected address. */
  p = prefix_ipv4_new ();
  p->family = AF_INET;
  p->prefix = *addr;
  p->prefixlen = prefixlen;
  ifc->address = (struct prefix *) p;
  
  /* If there is broadcast or pointopoint address. */
  if (broad)
    {
      p = prefix_ipv4_new ();
      p->family = AF_INET;
      p->prefix = *broad;
      ifc->destination = (struct prefix *) p;

      /* validate the destination address */
      if (ifp->flags & IFF_POINTOPOINT)
        {
	  if (IPV4_ADDR_SAME(addr,broad))
	    zlog_warn("warning: PtP interface %s has same local and peer "
		      "address %s, routing protocols may malfunction",
		      ifp->name,inet_ntoa(*addr));
	  else if ((prefixlen != IPV4_MAX_PREFIXLEN) &&
	   	   (ipv4_network_addr(addr->s_addr,prefixlen) !=
	   	    ipv4_network_addr(broad->s_addr,prefixlen)))
	    {
	      char buf[2][INET_ADDRSTRLEN];
	      zlog_warn("warning: PtP interface %s network mismatch: local "
	      		"%s/%d vs. peer %s, routing protocols may malfunction",
	    		ifp->name,
			inet_ntop (AF_INET, addr, buf[0], sizeof(buf[0])),
			prefixlen,
			inet_ntop (AF_INET, broad, buf[1], sizeof(buf[1])));
	    }
        }
      else
        {
	  if (broad->s_addr != ipv4_broadcast_addr(addr->s_addr,prefixlen))
	    {
	      char buf[2][INET_ADDRSTRLEN];
	      struct in_addr bcalc;
	      bcalc.s_addr = ipv4_broadcast_addr(addr->s_addr,prefixlen);
	      zlog_warn("warning: interface %s broadcast addr %s/%d != "
	       		"calculated %s, routing protocols may malfunction",
	    		ifp->name,
			inet_ntop (AF_INET, broad, buf[0], sizeof(buf[0])),
			prefixlen,
			inet_ntop (AF_INET, &bcalc, buf[1], sizeof(buf[1])));
	    }
	  /*gjd : for Rtsuit restart , ip address write show running.*/
	  if(!CHECK_FLAG(ifc->ip_config,RTMD_RESTART_IP_CONFIG)&&(keep_kernel_mode == 1))
	  	{
	  		SET_FLAG(ifc->ip_config,RTMD_RESTART_IP_CONFIG);			
			/*zlog_err("%s : line %d ifc->conf(%u), ifc->ipconfig(%u).\n",
				__func__,__LINE__,ifc->conf,ifc->ip_config);*/
	  	}
      }

    }
  else
    /* no broadcast or destination address was supplied */
    if ((prefixlen == IPV4_MAX_PREFIXLEN) && if_is_pointopoint(ifp))
      zlog_warn("warning: PtP interface %s with addr %s/%d needs a "
      		"peer address",ifp->name,inet_ntoa(*addr),prefixlen);

  /* Label of this address. */
  if (label)
    ifc->label = XSTRDUP (MTYPE_CONNECTED_LABEL, label);

  /* nothing to do? */
  if ((ifc = connected_implicit_withdraw (ifp, ifc)) == NULL)
    return;
  
  connected_announce (ifp, ifc);
}

void
connected_down_ipv4 (struct interface *ifp, struct connected *ifc)
{
  struct prefix_ipv4 p;
  struct prefix_ipv4 *addr;
  struct prefix_ipv4 *dest;

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start ", __func__);
  
  /*gujd: 2012-10-19, pm 3:49. When vrrp use virtual IP address , destination is NULL, so can't return.*/
  #if 0
  if(!ifc || !ifc->address || !ifc->destination|| !ifc->ifp)
  {
  	zlog_err("ifc(%p), ifc->address(%p), ifc->destination(%p).\n",
			ifc,ifc->address,ifc->destination);
  	return;
  	}
  #else
	if(!ifc || !ifc->address || !ifc->ifp)
	  {
	   /*CID 11026 (#1 of 1): Dereference after null check (FORWARD_NULL)
		4. var_deref_op: Dereferencing null pointer "ifc".
		Make sure , here is bug. So changed.*/
		
	  	/*zlog_err("ifc(%p), ifc->address(%p), ifc->ifp(%p).\n",
				ifc,ifc->address,ifc->ifp);
		   return ;*/
		   
		if(!ifc)
		{
			zlog_err("%s: line %d, ifc is null.\n",__func__,__LINE__);
			return ;
		}
		else
		{
			zlog_err("%s: line %d, ifc->address(%p), ifc->ifp(%p).\n",
				__func__,__LINE__,ifc->address,ifc->ifp);
			return ;
		}
	  }
  #endif
  /*  if(product != NULL)*/
   if(product != NULL && product->board_type != BOARD_IS_ACTIVE_MASTER)/*gjd : change for active master 在删除ip时，直连路由没删除，因为重复添加，导致refcnt不为0释放不掉*/
  	goto skip;

   if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER &&CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
  	goto skip;/*local interface , skip the check the ifc->conf, because the ip address not install in the kernel .*/
  
  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
    return;
  
skip:
  addr = (struct prefix_ipv4 *)ifc->address;
  dest = (struct prefix_ipv4 *)ifc->destination;

  memset (&p, 0, sizeof (struct prefix_ipv4));
  p.family = AF_INET;
  p.prefixlen = addr->prefixlen;

  /* Point-to-point check. */
  if (CONNECTED_POINTOPOINT_HOST(ifc))
    p.prefix = dest->prefix;
  else
    p.prefix = addr->prefix;

  /* Apply mask to the network. */
  apply_mask_ipv4 (&p);

  /* In case of connected address is 0.0.0.0/0 we treat it tunnel
     address. */
  if (prefix_ipv4_any (&p))
    return;

  rib_delete_ipv4 (ZEBRA_ROUTE_CONNECT, 0, &p, NULL, ifp->ifindex, 0);

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: goto rib_update ", __func__);

   rib_update ();
}

/* Delete connected IPv4 route to the interface. */
void
connected_delete_ipv4 (struct interface *ifp, int flags, struct in_addr *addr,
		       u_char prefixlen, struct in_addr *broad)
{
  struct prefix_ipv4 p;
  struct connected *ifc;

  memset (&p, 0, sizeof (struct prefix_ipv4));
  p.family = AF_INET;
  p.prefix = *addr;
  p.prefixlen = prefixlen;

  ifc = connected_check (ifp, (struct prefix *) &p);
  if (! ifc)
    return;
    
  connected_withdraw (ifc);
}

#ifdef HAVE_IPV6
void
connected_up_ipv6 (struct interface *ifp, struct connected *ifc)
{
  struct prefix_ipv6 p;
  struct prefix_ipv6 *addr;
  struct prefix_ipv6 *dest;

  
  if(product != NULL && product->board_type != BOARD_IS_ACTIVE_MASTER)
  	goto skip;
 #if 0
/*ipv6 not support interface loacal*/
  if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER &&CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
  	goto skip;/*local interface , skip the check the ifc->conf, because the ip address not install in the kernel .*/
#endif
  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
    return;

skip:
  addr = (struct prefix_ipv6 *) ifc->address;
  dest = (struct prefix_ipv6 *) ifc->destination;

  memset (&p, 0, sizeof (struct prefix_ipv6));
  p.family = AF_INET6;
  p.prefixlen = addr->prefixlen;

  if (if_is_pointopoint (ifp) && dest)
    {
      if (IN6_IS_ADDR_UNSPECIFIED (&dest->prefix))
	p.prefix = addr->prefix;
      else
	p.prefix = dest->prefix;
    }
  else
    p.prefix = addr->prefix;

  /* Apply mask to the network. */
  apply_mask_ipv6 (&p);

#if ! defined (MUSICA) && ! defined (LINUX)
  /* XXX: It is already done by rib_bogus_ipv6 within rib_add_ipv6 */
  if (IN6_IS_ADDR_UNSPECIFIED (&p.prefix))
    return;
#endif

  rib_add_ipv6 (ZEBRA_ROUTE_CONNECT, 0, &p, NULL, ifp->ifindex, 0,
                ifp->metric, 0);

  rib_update ();
}

/* Add connected IPv6 route to the interface. */
void
connected_add_ipv6 (struct interface *ifp, struct in6_addr *addr,
		    u_char prefixlen, struct in6_addr *broad,
		    const char *label)
{
  struct prefix_ipv6 *p;
  struct connected *ifc;

  /* Make connected structure. */
  ifc = connected_new ();
  ifc->ifp = ifp;

  /* Allocate new connected address. */
  p = prefix_ipv6_new ();
  p->family = AF_INET6;
  IPV6_ADDR_COPY (&p->prefix, addr);
  p->prefixlen = prefixlen;
  ifc->address = (struct prefix *) p;

  /* If there is broadcast or pointopoint address. */
  if (broad)
    {
      p = prefix_ipv6_new ();
      p->family = AF_INET6;
      IPV6_ADDR_COPY (&p->prefix, broad);
      ifc->destination = (struct prefix *) p;
    }

  /* Label of this address. */
  if (label)
    ifc->label = XSTRDUP (MTYPE_CONNECTED_LABEL, label);
  
  if ((ifc = connected_implicit_withdraw (ifp, ifc)) == NULL)
    return;
  
  connected_announce (ifp, ifc);
}

void
connected_down_ipv6 (struct interface *ifp, struct connected *ifc)
{
  struct prefix_ipv6 p;
  struct prefix_ipv6 *addr;
  struct prefix_ipv6 *dest;

  if(product != NULL && product->board_type != BOARD_IS_ACTIVE_MASTER)
   goto skip;
  
  if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER &&CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
   goto skip;/*local interface , skip the check the ifc->conf, because the ip address not install in the kernel .*/

  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
    return;
  
skip:
  addr = (struct prefix_ipv6 *) ifc->address;
  dest = (struct prefix_ipv6 *) ifc->destination;

  memset (&p, 0, sizeof (struct prefix_ipv6));
  p.family = AF_INET6;
  p.prefixlen = addr->prefixlen;

  if (if_is_pointopoint (ifp) && dest)
    {
      if (IN6_IS_ADDR_UNSPECIFIED (&dest->prefix))
	p.prefix = addr->prefix;
      else
	p.prefix = dest->prefix;
    }
  else
    p.prefix = addr->prefix;

  apply_mask_ipv6 (&p);

  if (IN6_IS_ADDR_UNSPECIFIED (&p.prefix))
    return;

  rib_delete_ipv6 (ZEBRA_ROUTE_CONNECT, 0, &p, NULL, ifp->ifindex, 0);

  rib_update ();
}

void
connected_delete_ipv6 (struct interface *ifp, struct in6_addr *address,
		       u_char prefixlen, struct in6_addr *broad)
{
  struct prefix_ipv6 p;
  struct connected *ifc;
  
  memset (&p, 0, sizeof (struct prefix_ipv6));
  p.family = AF_INET6;
  memcpy (&p.prefix, address, sizeof (struct in6_addr));
  p.prefixlen = prefixlen;

  ifc = connected_check (ifp, (struct prefix *) &p);
  if (! ifc)
    return;

  connected_withdraw (ifc);
}
#endif /* HAVE_IPV6 */
