/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.12 $ of : mfd-data-access.m2c,v $ 
 *
 * $Id:$
 */
/* standard Net-SNMP includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* include our parent header */
#include "dot11WtpNetworkaddrTable.h"


#include "dot11WtpNetworkaddrTable_data_access.h"
#include "wcpss/wid/WID.h"
#include "ws_dcli_ac.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "../mibs_public.h"

/** @defgroup data_access data_access: Routines to access data
 *
 * These routines are used to locate the data used to satisfy
 * requests.
 * 
 * @{
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table dot11WtpNetworkaddrTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * dot11WtpNetworkaddrTable is subid 5 of wtpGeneralInfo.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.31656.6.1.1.1.5, length: 12
*/

/**
 * initialization for dot11WtpNetworkaddrTable data access
 *
 * This function is called during startup to allow you to
 * allocate any resources you need for the data table.
 *
 * @param dot11WtpNetworkaddrTable_reg
 *        Pointer to dot11WtpNetworkaddrTable_registration
 *
 * @retval MFD_SUCCESS : success.
 * @retval MFD_ERROR   : unrecoverable error.
 */
int
dot11WtpNetworkaddrTable_init_data(dot11WtpNetworkaddrTable_registration_ptr dot11WtpNetworkaddrTable_reg)
{
    DEBUGMSGTL(("verbose:dot11WtpNetworkaddrTable:dot11WtpNetworkaddrTable_init_data","called\n"));

    /*
     * TODO:303:o: Initialize dot11WtpNetworkaddrTable data.
     */
    /*
    ***************************************************
    ***             START EXAMPLE CODE              ***
    ***---------------------------------------------***/
    /*
     * if you are the sole writer for the file, you could
     * open it here. However, as stated earlier, we are assuming
     * the worst case, which in this case means that the file is
     * written to by someone else, and might not even exist when
     * we start up. So we can't do anything here.
     */
    /*
    ***---------------------------------------------***
    ***              END  EXAMPLE CODE              ***
    ***************************************************/

    return MFD_SUCCESS;
} /* dot11WtpNetworkaddrTable_init_data */

/**
 * container-cached overview
 *
 */

/***********************************************************************
 *
 * cache
 *
 ***********************************************************************/
/**
 * container initialization
 *
 * @param container_ptr_ptr A pointer to a container pointer. If you
 *        create a custom container, use this parameter to return it
 *        to the MFD helper. If set to NULL, the MFD helper will
 *        allocate a container for you.
 * @param  cache A pointer to a cache structure. You can set the timeout
 *         and other cache flags using this pointer.
 *
 *  This function is called at startup to allow you to customize certain
 *  aspects of the access method. For the most part, it is for advanced
 *  users. The default code should suffice for most cases. If no custom
 *  container is allocated, the MFD code will create one for your.
 *
 *  This is also the place to set up cache behavior. The default, to
 *  simply set the cache timeout, will work well with the default
 *  container. If you are using a custom container, you may want to
 *  look at the cache helper documentation to see if there are any
 *  flags you want to set.
 *
 * @remark
 *  This would also be a good place to do any initialization needed
 *  for you data source. For example, opening a connection to another
 *  process that will supply the data, opening a database, etc.
 */
void
dot11WtpNetworkaddrTable_container_init(netsnmp_container **container_ptr_ptr,
                        netsnmp_cache *cache)
{
    DEBUGMSGTL(("verbose:dot11WtpNetworkaddrTable:dot11WtpNetworkaddrTable_container_init","called\n"));
    
    if((NULL == cache) || (NULL == container_ptr_ptr)) {
        snmp_log(LOG_ERR,"bad params to dot11WtpNetworkaddrTable_container_init\n");
        return;
    }

    /*
     * For advanced users, you can use a custom container. If you
     * do not create one, one will be created for you.
     */
    *container_ptr_ptr = NULL;

    /*
     * TODO:345:A: Set up dot11WtpNetworkaddrTable cache properties.
     *
     * Also for advanced users, you can set parameters for the
     * cache. Do not change the magic pointer, as it is used
     * by the MFD helper. To completely disable caching, set
     * cache->enabled to 0.
     */
    cache->timeout = DOT11WTPNETWORKADDRTABLE_CACHE_TIMEOUT; /* seconds */
} /* dot11WtpNetworkaddrTable_container_init */

/**
 * load cache data
 *
 * TODO:350:M: Implement dot11WtpNetworkaddrTable cache load
 *
 * @param container container to which items should be inserted
 *
 * @retval MFD_SUCCESS              : success.
 * @retval MFD_RESOURCE_UNAVAILABLE : Can't access data source
 * @retval MFD_ERROR                : other error.
 *
 *  This function is called to cache the index(es) (and data, optionally)
 *  for the every row in the data set.
 *
 * @remark
 *  While loading the cache, the only important thing is the indexes.
 *  If access to your data is cheap/fast (e.g. you have a pointer to a
 *  structure in memory), it would make sense to update the data here.
 *  If, however, the accessing the data invovles more work (e.g. parsing
 *  some other existing data, or peforming calculations to derive the data),
 *  then you can limit yourself to setting the indexes and saving any
 *  information you will need later. Then use the saved information in
 *  dot11WtpNetworkaddrTable_row_prep() for populating data.
 *
 * @note
 *  If you need consistency between rows (like you want statistics
 *  for each row to be from the same time frame), you should set all
 *  data here.
 *
 */
int
dot11WtpNetworkaddrTable_cache_load(netsnmp_container *container)
{
	snmp_log(LOG_DEBUG, "enter dot11WtpNetworkaddrTable_cache_load\n");

    dot11WtpNetworkaddrTable_rowreq_ctx *rowreq_ctx;
    size_t                 count = 0;
    
    /*
     * this example code is based on a data source that is a
     * text file to be read and parsed.
     */
    //FILE *filep;
    //char line[MAX_LINE_SIZE];
    /*
     * temporary storage for index values
     */
        /*
         * wtpMacAddr(1)/DisplayString/ASN_OCTET_STR/char(char)//L/A/w/e/R/d/H
         */
        /** 128 - 1(entry) - 1(col) - 0(other indexes) = 112 */
   char   wtpMacAddr[20] = { 0 };
   size_t      wtpMacAddr_len;

    DEBUGMSGTL(("verbose:dot11WtpNetworkaddrTable:dot11WtpNetworkaddrTable_cache_load","called\n"));

    /*
    ***************************************************
    ***             START EXAMPLE CODE              ***
    ***---------------------------------------------***/
    /*
     * open our data file.
     */
    //filep = fopen("/etc/dummy.conf", "r");
    //if(NULL ==  filep) {
        //return MFD_RESOURCE_UNAVAILABLE;
    //}

    /*
    ***---------------------------------------------***
    ***              END  EXAMPLE CODE              ***
    ***************************************************/
    /*
     * TODO:351:M: |-> Load/update data in the dot11WtpNetworkaddrTable container.
     * loop over your dot11WtpNetworkaddrTable data, allocate a rowreq context,
     * set the index(es) [and data, optionally] and insert into
     * the container.
     */
     
	long wtpIPAddress = 0;
	int retval = 0;
	char ip[WTP_WTP_IP_LEN+1] = { 0 };
	long wtpNetmask = 0;
	unsigned char mymaskBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *mymaskPtr = mymaskBuf;
	long wtpGateway = 0;
	unsigned char mygatewayBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *mygatewayPtr = mygatewayBuf;
	long wtpFirstDNSServer = 0;
	unsigned char myfstdnsBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *myfstdnsPtr = myfstdnsBuf;
	long wtpSeconDNSServer = 0;
	unsigned char mysnddnsBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *mysnddnsPtr = mysnddnsBuf;

	
    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_all_wtp_network_info_cmd\n");
    messageHead = list_connection_call_dbus_method(show_all_wtp_network_info_cmd, SHOW_ALL_WTP_TABLE_METHOD);
	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_all_wtp_network_info_cmd,messageHead=%p\n", messageHead);
	
	if(messageHead)
	{
	    for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
		{
		    struct WtpNetworkInfo *WtpHead = messageNode->message;
		    if(WtpHead)
		    {
		        struct WtpNetworkInfo *WtpShowNode = NULL;
        		for (WtpShowNode = WtpHead->WtpNetworkInfo_list; NULL != WtpShowNode; WtpShowNode = WtpShowNode->next) 
        	    {
        			memset(wtpMacAddr,0,20);
        			if(WtpShowNode->WTPMAC)
        			{
        				snprintf(wtpMacAddr,
        						 sizeof(wtpMacAddr)-1,
        						 "%02X:%02X:%02X:%02X:%02X:%02X",
        						 WtpShowNode->WTPMAC[0],WtpShowNode->WTPMAC[1],WtpShowNode->WTPMAC[2],WtpShowNode->WTPMAC[3],WtpShowNode->WTPMAC[4],WtpShowNode->WTPMAC[5]);
        			}

        			memset(mygatewayBuf,0,DCLIAC_BUF_LEN);
        			memset(myfstdnsBuf,0,DCLIAC_BUF_LEN);
        			memset(mysnddnsBuf,0,DCLIAC_BUF_LEN);
        			memset(mymaskBuf,0,DCLIAC_BUF_LEN);

        			if(WtpShowNode->WTPIP)
        			{
        				retval = wtp_check_wtp_ip_addr(ip,WtpShowNode->WTPIP);
        				if(retval == 0)
        				{
        					INET_ATON(wtpIPAddress,WtpShowNode->WTPIP);
        				}
        				else
        				{
        					INET_ATON(wtpIPAddress,ip);
        				}
        			}
        			
        			ip_long2str(WtpShowNode->ap_mask_new,&mymaskPtr);
        			INET_ATON(wtpNetmask,mymaskBuf);

        			ip_long2str(WtpShowNode->ap_gateway,&mygatewayPtr);
        			INET_ATON(wtpGateway,mygatewayBuf);

        			ip_long2str(WtpShowNode->ap_dnsfirst,&myfstdnsPtr);
        			INET_ATON(wtpFirstDNSServer,myfstdnsBuf);

        			ip_long2str(WtpShowNode->ap_dnssecend,&mysnddnsPtr);
        			INET_ATON(wtpSeconDNSServer,mysnddnsBuf);

	    /*
	    ***************************************************
	    ***             START EXAMPLE CODE              ***
	    ***---------------------------------------------***/
	    /*
	     * get a line (skip blank lines)
	     */
	    //do {
	        //if (!fgets(line, sizeof(line), filep)) {
	            /* we're done */
	            //fclose(filep);
	            //filep = NULL;
	        //}
	    //} while (filep && (line[0] == '\n'));

	    /*
	     * check for end of data
	     */
	    //if(NULL == filep)
	       // break;

	    /*
	     * parse line into variables
	     */
	    /*
	    ***---------------------------------------------***
	    ***              END  EXAMPLE CODE              ***
	    ***************************************************/

	        /*
	         * TODO:352:M: |   |-> set indexes in new dot11WtpNetworkaddrTable rowreq context.
	         */
	        rowreq_ctx = dot11WtpNetworkaddrTable_allocate_rowreq_ctx();
	        if (NULL == rowreq_ctx) {
	            snmp_log(LOG_ERR, "memory allocation failed\n");
                free_dbus_message_list(&messageHead, Free_show_all_wtp_network_info_cmd);
	            return MFD_RESOURCE_UNAVAILABLE;
	        }
			
			wtpMacAddr_len = strlen(wtpMacAddr);
	        if(MFD_SUCCESS != dot11WtpNetworkaddrTable_indexes_set(rowreq_ctx
	                               , wtpMacAddr, wtpMacAddr_len
	               )) {
	            snmp_log(LOG_ERR,"error setting index while loading "
	                     "dot11WtpNetworkaddrTable cache.\n");
	            dot11WtpNetworkaddrTable_release_rowreq_ctx(rowreq_ctx);
	            continue;
	        }

	        /*
	         * TODO:352:r: |   |-> populate dot11WtpNetworkaddrTable data context.
	         * Populate data context here. (optionally, delay until row prep)
	         */
	    /*
	     * TRANSIENT or semi-TRANSIENT data:
	     * copy data or save any info needed to do it in row_prep.
	     */
	    /*
	     * setup/save data for wtpIPAddress
	     * wtpIPAddress(1)/IPADDR/ASN_IPADDRESS/u_long(u_long)//l/A/W/e/r/d/h
	     */
	    /*
	     * TODO:246:r: |-> Define wtpIPAddress mapping.
	     * Map values between raw/native values and MIB values
	     *
	     * Integer based value can usually just do a direct copy.
	     */
	    memcpy(&(rowreq_ctx->data.parameter), &(messageNode->parameter), sizeof(dbus_parameter));
		
		rowreq_ctx->data.wtpCurrID = WtpShowNode->WTPID;
			
	    rowreq_ctx->data.wtpIPAddress = wtpIPAddress;
	    
	    /*
	     * setup/save data for wtpNetmask
	     * wtpNetmask(2)/IPADDR/ASN_IPADDRESS/u_long(u_long)//l/A/W/e/r/d/h
	     */
	    /*
	     * TODO:246:r: |-> Define wtpNetmask mapping.
	     * Map values between raw/native values and MIB values
	     *
	     * Integer based value can usually just do a direct copy.
	     */
	    rowreq_ctx->data.wtpNetmask = wtpNetmask;
	    
	    /*
	     * setup/save data for wtpGateway
	     * wtpGateway(3)/IPADDR/ASN_IPADDRESS/u_long(u_long)//l/A/W/e/r/d/h
	     */
	    /*
	     * TODO:246:r: |-> Define wtpGateway mapping.
	     * Map values between raw/native values and MIB values
	     *
	     * Integer based value can usually just do a direct copy.
	     */
	    rowreq_ctx->data.wtpGateway = wtpGateway;
	    
	    /*
	     * setup/save data for wtpFirstDNSServer
	     * wtpFirstDNSServer(4)/IPADDR/ASN_IPADDRESS/u_long(u_long)//l/A/W/e/r/d/h
	     */
	    /*
	     * TODO:246:r: |-> Define wtpFirstDNSServer mapping.
	     * Map values between raw/native values and MIB values
	     *
	     * Integer based value can usually just do a direct copy.
	     */
	    rowreq_ctx->data.wtpFirstDNSServer = wtpFirstDNSServer;
	    
	    /*
	     * setup/save data for wtpSeconDNSServer
	     * wtpSeconDNSServer(5)/IPADDR/ASN_IPADDRESS/u_long(u_long)//l/A/W/e/r/d/h
	     */
	    /*
	     * TODO:246:r: |-> Define wtpSeconDNSServer mapping.
	     * Map values between raw/native values and MIB values
	     *
	     * Integer based value can usually just do a direct copy.
	     */
	    rowreq_ctx->data.wtpSeconDNSServer = wtpSeconDNSServer;
	    
	    /*
	     * setup/save data for wtpSetAction
	     * wtpSetAction(6)/INTEGER/ASN_INTEGER/long(u_long)//l/A/W/E/r/d/h
	     */
	    /*
	     * TODO:246:r: |-> Define wtpSetAction mapping.
	     * Map values between raw/native values and MIB values
	     *
	    * enums usually need mapping.
	    */
	    if(MFD_SUCCESS !=
	       wtpSetAction_map(&rowreq_ctx->data.wtpSetAction, 1 )) {
			free_dbus_message_list(&messageHead, Free_show_all_wtp_network_info_cmd);
	        return MFD_ERROR;
	    }

	        
	        /*
	         * insert into table container
	         */
        	        if(CONTAINER_INSERT(container, rowreq_ctx))			
        			{
        	        	dot11WtpNetworkaddrTable_release_rowreq_ctx(rowreq_ctx);
        	        }
        	        ++count;
        	    }
        	}	
		}
		free_dbus_message_list(&messageHead, Free_show_all_wtp_network_info_cmd);
	}

    /*
    ***************************************************
    ***             START EXAMPLE CODE              ***
    ***---------------------------------------------***/
    //if(NULL != filep)
      //  fclose(filep);
    /*
    ***---------------------------------------------***
    ***              END  EXAMPLE CODE              ***
    ***************************************************/

    DEBUGMSGT(("verbose:dot11WtpNetworkaddrTable:dot11WtpNetworkaddrTable_cache_load",
               "inserted %d records\n", count));
	
	snmp_log(LOG_DEBUG, "exit dot11WtpNetworkaddrTable_cache_load\n");

    return MFD_SUCCESS;
} /* dot11WtpNetworkaddrTable_cache_load */

/**
 * cache clean up
 *
 * @param container container with all current items
 *
 *  This optional callback is called prior to all
 *  item's being removed from the container. If you
 *  need to do any processing before that, do it here.
 *
 * @note
 *  The MFD helper will take care of releasing all the row contexts.
 *
 */
void
dot11WtpNetworkaddrTable_cache_free(netsnmp_container *container)
{
    DEBUGMSGTL(("verbose:dot11WtpNetworkaddrTable:dot11WtpNetworkaddrTable_cache_free","called\n"));

    /*
     * TODO:380:M: Free dot11WtpNetworkaddrTable cache.
     */
} /* dot11WtpNetworkaddrTable_cache_free */

/**
 * prepare row for processing.
 *
 *  When the agent has located the row for a request, this function is
 *  called to prepare the row for processing. If you fully populated
 *  the data context during the index setup phase, you may not need to
 *  do anything.
 *
 * @param rowreq_ctx pointer to a context.
 *
 * @retval MFD_SUCCESS     : success.
 * @retval MFD_ERROR       : other error.
 */
int
dot11WtpNetworkaddrTable_row_prep( dot11WtpNetworkaddrTable_rowreq_ctx *rowreq_ctx)
{
    DEBUGMSGTL(("verbose:dot11WtpNetworkaddrTable:dot11WtpNetworkaddrTable_row_prep","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

    /*
     * TODO:390:o: Prepare row for request.
     * If populating row data was delayed, this is the place to
     * fill in the row for this request.
     */

    return MFD_SUCCESS;
} /* dot11WtpNetworkaddrTable_row_prep */

/** @} */
