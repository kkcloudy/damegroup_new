/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.18.2.1 $ of : mfd-data-get.m2c,v $
 *
 * $Id:$
 *
 * @file dot11NewIllegalDeviceTable_data_get.h
 *
 * @addtogroup get
 *
 * Prototypes for get functions
 *
 * @{
 */
#ifndef DOT11NEWILLEGALDEVICETABLE_DATA_GET_H
#define DOT11NEWILLEGALDEVICETABLE_DATA_GET_H

#ifdef __cplusplus
extern "C" {
#endif

/* *********************************************************************
 * GET function declarations
 */

/* *********************************************************************
 * GET Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table dot11NewIllegalDeviceTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * dot11NewIllegalDeviceTable is subid 3 of dot11WhiteBlackList.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.31656.6.1.2.8.3, length: 12
*/
    /*
     * indexes
     */
    int NewblackListDeviceMacID_map(long *mib_NewblackListDeviceMacID_val_ptr, long raw_NewblackListDeviceMacID_val);

    int NewblackListDeviceMAC_map(char **mib_NewblackListDeviceMAC_val_ptr_ptr, size_t *mib_NewblackListDeviceMAC_val_ptr_len_ptr, char *raw_NewblackListDeviceMAC_val_ptr, size_t raw_NewblackListDeviceMAC_val_ptr_len, int allow_realloc);
    int NewblackListDeviceMAC_get( dot11NewIllegalDeviceTable_rowreq_ctx *rowreq_ctx, char **NewblackListDeviceMAC_val_ptr_ptr, size_t *NewblackListDeviceMAC_val_ptr_len_ptr );
    int NewblackListAddReason_map(char **mib_NewblackListAddReason_val_ptr_ptr, size_t *mib_NewblackListAddReason_val_ptr_len_ptr, char *raw_NewblackListAddReason_val_ptr, size_t raw_NewblackListAddReason_val_ptr_len, int allow_realloc);
    int NewblackListAddReason_get( dot11NewIllegalDeviceTable_rowreq_ctx *rowreq_ctx, char **NewblackListAddReason_val_ptr_ptr, size_t *NewblackListAddReason_val_ptr_len_ptr );
    int NewblackListDuration_map(u_long *mib_NewblackListDuration_val_ptr, u_long raw_NewblackListDuration_val);
    int NewblackListDuration_get( dot11NewIllegalDeviceTable_rowreq_ctx *rowreq_ctx, u_long * NewblackListDuration_val_ptr );
    int NewblackListAddType_map(u_long *mib_NewblackListAddType_val_ptr, u_long raw_NewblackListAddType_val);
    int NewblackListAddType_get( dot11NewIllegalDeviceTable_rowreq_ctx *rowreq_ctx, u_long * NewblackListAddType_val_ptr );


int dot11NewIllegalDeviceTable_indexes_set_tbl_idx(dot11NewIllegalDeviceTable_mib_index *tbl_idx, long NewblackListDeviceMacID_val);
int dot11NewIllegalDeviceTable_indexes_set(dot11NewIllegalDeviceTable_rowreq_ctx *rowreq_ctx, long NewblackListDeviceMacID_val);




#ifdef __cplusplus
}
#endif

#endif /* DOT11NEWILLEGALDEVICETABLE_DATA_GET_H */
/** @} */
