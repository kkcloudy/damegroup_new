/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.12 $ of : mfd-data-access.m2c,v $
 *
 * $Id:$
 */
#ifndef DOT11WTPSTATSTABLE_DATA_ACCESS_H
#define DOT11WTPSTATSTABLE_DATA_ACCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "autelanWtpGroup.h"


/* *********************************************************************
 * function declarations
 */

/* *********************************************************************
 * Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table dot11WtpStatsTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * dot11WtpStatsTable is subid 4 of wtpStats.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.33050.6.1.1.2.4, length: 12
*/


    int dot11WtpStatsTable_init_data(dot11WtpStatsTable_registration_ptr dot11WtpStatsTable_reg);


/*
 * TODO:180:o: Review dot11WtpStatsTable cache timeout.
 * The number of seconds before the cache times out
 */
#define DOT11WTPSTATSTABLE_CACHE_TIMEOUT   DOT1DTPFDBTABLE_CACHE_TIMEOUT


void dot11WtpStatsTable_container_init(netsnmp_container **container_ptr_ptr,
                             netsnmp_cache *cache);
int dot11WtpStatsTable_cache_load(netsnmp_container *container);
void dot11WtpStatsTable_cache_free(netsnmp_container *container);

    /*
    ***************************************************
    ***             START EXAMPLE CODE              ***
    ***---------------------------------------------***/
/* *********************************************************************
 * Since we have no idea how you really access your data, we'll go with
 * a worst case example: a flat text file.
 */
#define MAX_LINE_SIZE 256
    /*
    ***---------------------------------------------***
    ***              END  EXAMPLE CODE              ***
    ***************************************************/
    int dot11WtpStatsTable_row_prep( dot11WtpStatsTable_rowreq_ctx *rowreq_ctx);


#ifdef __cplusplus
}
#endif

#endif /* DOT11WTPSTATSTABLE_DATA_ACCESS_H */
