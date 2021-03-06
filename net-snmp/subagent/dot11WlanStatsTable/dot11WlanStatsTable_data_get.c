/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.18.2.1 $ of : mfd-data-get.m2c,v $ 
 *
 * $Id:$
 */
/* standard Net-SNMP includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/* include our parent header */
#include "dot11WlanStatsTable.h"


/** @defgroup data_get data_get: Routines to get data
 *
 * TODO:230:M: Implement dot11WlanStatsTable get routines.
 * TODO:240:M: Implement dot11WlanStatsTable mapping routines (if any).
 *
 * These routine are used to get the value for individual objects. The
 * row context is passed, along with a pointer to the memory where the
 * value should be copied.
 *
 * @{
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table dot11WlanStatsTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * dot11WlanStatsTable is subid 4 of dot11wlanConfig.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.31656.6.1.2.13.4, length: 12
*/

/* ---------------------------------------------------------------------
 * TODO:200:r: Implement dot11WlanStatsTable data context functions.
 */

/*---------------------------------------------------------------------
 * DOT11-AC-MIB::dot11VlanConfigEntry.wlanID
 * wlanID is subid 1 of dot11VlanConfigEntry.
 * Its status is Current, and its access level is ReadWrite.
 * OID: .1.3.6.1.4.1.31656.6.1.2.7.1.1
 * Description:
Vlan ID.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   1
 *
 *
 * Its syntax is INTEGER (based on perltype INTEGER)
 * The net-snmp type is ASN_INTEGER. The C type decl is long (long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wlanID.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wlanID_dot11WlanStatsTable_map(long *mib_wlanID_val_ptr, long raw_wlanID_val)
{
    netsnmp_assert(NULL != mib_wlanID_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanID_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wlanID mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wlanID_val_ptr) = raw_wlanID_val;

    return MFD_SUCCESS;
} /* wlanID_map */


/**
 * set mib index(es)
 *
 * @param tbl_idx mib index structure
 *
 * @retval MFD_SUCCESS     : success.
 * @retval MFD_ERROR       : other error.
 *
 * @remark
 *  This convenience function is useful for setting all the MIB index
 *  components with a single function call. It is assume that the C values
 *  have already been mapped from their native/rawformat to the MIB format.
 */
int
dot11WlanStatsTable_indexes_set_tbl_idx(dot11WlanStatsTable_mib_index *tbl_idx, long wlanID_val)
{
    DEBUGMSGTL(("verbose:dot11WlanStatsTable:dot11WlanStatsTable_indexes_set_tbl_idx","called\n"));

    /* wlanID(1)/INTEGER/ASN_INTEGER/long(long)//l/A/W/e/r/d/h */
    tbl_idx->wlanID = wlanID_val;
    

    return MFD_SUCCESS;
} /* dot11WlanStatsTable_indexes_set_tbl_idx */

/**
 * @internal
 * set row context indexes
 *
 * @param reqreq_ctx the row context that needs updated indexes
 *
 * @retval MFD_SUCCESS     : success.
 * @retval MFD_ERROR       : other error.
 *
 * @remark
 *  This function sets the mib indexs, then updates the oid indexs
 *  from the mib index.
 */
int
dot11WlanStatsTable_indexes_set(dot11WlanStatsTable_rowreq_ctx *rowreq_ctx, long wlanID_val)
{
    DEBUGMSGTL(("verbose:dot11WlanStatsTable:dot11WlanStatsTable_indexes_set","called\n"));

    if(MFD_SUCCESS != dot11WlanStatsTable_indexes_set_tbl_idx(&rowreq_ctx->tbl_idx
                                   , wlanID_val
           ))
        return MFD_ERROR;

    /*
     * convert mib index to oid index
     */
    rowreq_ctx->oid_idx.len = sizeof(rowreq_ctx->oid_tmp) / sizeof(oid);
    if(0 != dot11WlanStatsTable_index_to_oid(&rowreq_ctx->oid_idx,
                                    &rowreq_ctx->tbl_idx)) {
        return MFD_ERROR;
    }

    return MFD_SUCCESS;
} /* dot11WlanStatsTable_indexes_set */


/*---------------------------------------------------------------------
 * DOT11-AC-MIB::dot11WlanStatsEntry.wlanAssocedStaNum
 * wlanAssocedStaNum is subid 1 of dot11WlanStatsEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.31656.6.1.2.13.4.1.1
 * Description:
The  total ssociated station numbers.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   0
 *
 *
 * Its syntax is COUNTER (based on perltype COUNTER)
 * The net-snmp type is ASN_COUNTER. The C type decl is u_long (u_long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wlanAssocedStaNum.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wlanAssocedStaNum_map(u_long *mib_wlanAssocedStaNum_val_ptr, u_long raw_wlanAssocedStaNum_val)
{
    netsnmp_assert(NULL != mib_wlanAssocedStaNum_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanAssocedStaNum_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wlanAssocedStaNum mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wlanAssocedStaNum_val_ptr) = raw_wlanAssocedStaNum_val;

    return MFD_SUCCESS;
} /* wlanAssocedStaNum_map */

/**
 * Extract the current value of the wlanAssocedStaNum data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wlanAssocedStaNum_val_ptr
 *        Pointer to storage for a u_long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wlanAssocedStaNum_get( dot11WlanStatsTable_rowreq_ctx *rowreq_ctx, u_long * wlanAssocedStaNum_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wlanAssocedStaNum_val_ptr );


    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanAssocedStaNum_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wlanAssocedStaNum data.
 * set (* wlanAssocedStaNum_val_ptr ) from rowreq_ctx->data
 */
    (* wlanAssocedStaNum_val_ptr ) = rowreq_ctx->data.wlanAssocedStaNum;

    return MFD_SUCCESS;
} /* wlanAssocedStaNum_get */

/*---------------------------------------------------------------------
 * DOT11-AC-MIB::dot11WlanStatsEntry.wlanAssocReqTimes
 * wlanAssocReqTimes is subid 2 of dot11WlanStatsEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.31656.6.1.2.13.4.1.2
 * Description:
The associated request times.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   0
 *
 *
 * Its syntax is COUNTER (based on perltype COUNTER)
 * The net-snmp type is ASN_COUNTER. The C type decl is u_long (u_long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wlanAssocReqTimes.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wlanAssocReqTimes_map(u_long *mib_wlanAssocReqTimes_val_ptr, u_long raw_wlanAssocReqTimes_val)
{
    netsnmp_assert(NULL != mib_wlanAssocReqTimes_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanAssocReqTimes_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wlanAssocReqTimes mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wlanAssocReqTimes_val_ptr) = raw_wlanAssocReqTimes_val;

    return MFD_SUCCESS;
} /* wlanAssocReqTimes_map */

/**
 * Extract the current value of the wlanAssocReqTimes data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wlanAssocReqTimes_val_ptr
 *        Pointer to storage for a u_long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wlanAssocReqTimes_get( dot11WlanStatsTable_rowreq_ctx *rowreq_ctx, u_long * wlanAssocReqTimes_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wlanAssocReqTimes_val_ptr );


    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanAssocReqTimes_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wlanAssocReqTimes data.
 * set (* wlanAssocReqTimes_val_ptr ) from rowreq_ctx->data
 */
    (* wlanAssocReqTimes_val_ptr ) = rowreq_ctx->data.wlanAssocReqTimes;

    return MFD_SUCCESS;
} /* wlanAssocReqTimes_get */

/*---------------------------------------------------------------------
 * DOT11-AC-MIB::dot11WlanStatsEntry.wlanAssocRespTimes
 * wlanAssocRespTimes is subid 3 of dot11WlanStatsEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.31656.6.1.2.13.4.1.3
 * Description:
The AP responsed station request associated Times.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   0
 *
 *
 * Its syntax is COUNTER (based on perltype COUNTER)
 * The net-snmp type is ASN_COUNTER. The C type decl is u_long (u_long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wlanAssocRespTimes.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wlanAssocRespTimes_map(u_long *mib_wlanAssocRespTimes_val_ptr, u_long raw_wlanAssocRespTimes_val)
{
    netsnmp_assert(NULL != mib_wlanAssocRespTimes_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanAssocRespTimes_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wlanAssocRespTimes mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wlanAssocRespTimes_val_ptr) = raw_wlanAssocRespTimes_val;

    return MFD_SUCCESS;
} /* wlanAssocRespTimes_map */

/**
 * Extract the current value of the wlanAssocRespTimes data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wlanAssocRespTimes_val_ptr
 *        Pointer to storage for a u_long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wlanAssocRespTimes_get( dot11WlanStatsTable_rowreq_ctx *rowreq_ctx, u_long * wlanAssocRespTimes_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wlanAssocRespTimes_val_ptr );


    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanAssocRespTimes_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wlanAssocRespTimes data.
 * set (* wlanAssocRespTimes_val_ptr ) from rowreq_ctx->data
 */
    (* wlanAssocRespTimes_val_ptr ) = rowreq_ctx->data.wlanAssocRespTimes;

    return MFD_SUCCESS;
} /* wlanAssocRespTimes_get */

/*---------------------------------------------------------------------
 * DOT11-AC-MIB::dot11WlanStatsEntry.wlanAssocSuccessTimes
 * wlanAssocSuccessTimes is subid 4 of dot11WlanStatsEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.31656.6.1.2.13.4.1.4
 * Description:
The AP responsed station request associated Times.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   0
 *
 *
 * Its syntax is COUNTER (based on perltype COUNTER)
 * The net-snmp type is ASN_COUNTER. The C type decl is u_long (u_long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wlanAssocSuccessTimes.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wlanAssocSuccessTimes_map(u_long *mib_wlanAssocSuccessTimes_val_ptr, u_long raw_wlanAssocSuccessTimes_val)
{
    netsnmp_assert(NULL != mib_wlanAssocSuccessTimes_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanAssocSuccessTimes_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wlanAssocSuccessTimes mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wlanAssocSuccessTimes_val_ptr) = raw_wlanAssocSuccessTimes_val;

    return MFD_SUCCESS;
} /* wlanAssocSuccessTimes_map */

/**
 * Extract the current value of the wlanAssocSuccessTimes data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wlanAssocSuccessTimes_val_ptr
 *        Pointer to storage for a u_long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wlanAssocSuccessTimes_get( dot11WlanStatsTable_rowreq_ctx *rowreq_ctx, u_long * wlanAssocSuccessTimes_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wlanAssocSuccessTimes_val_ptr );


    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanAssocSuccessTimes_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wlanAssocSuccessTimes data.
 * set (* wlanAssocSuccessTimes_val_ptr ) from rowreq_ctx->data
 */
    (* wlanAssocSuccessTimes_val_ptr ) = rowreq_ctx->data.wlanAssocSuccessTimes;

    return MFD_SUCCESS;
} /* wlanAssocSuccessTimes_get */

/*---------------------------------------------------------------------
 * DOT11-AC-MIB::dot11WlanStatsEntry.wlanAssocFailTimes
 * wlanAssocFailTimes is subid 5 of dot11WlanStatsEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.31656.6.1.2.13.4.1.5
 * Description:
The AP responsed station request associated Times.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   0
 *
 *
 * Its syntax is COUNTER (based on perltype COUNTER)
 * The net-snmp type is ASN_COUNTER. The C type decl is u_long (u_long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wlanAssocFailTimes.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wlanAssocFailTimes_map(u_long *mib_wlanAssocFailTimes_val_ptr, u_long raw_wlanAssocFailTimes_val)
{
    netsnmp_assert(NULL != mib_wlanAssocFailTimes_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanAssocFailTimes_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wlanAssocFailTimes mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wlanAssocFailTimes_val_ptr) = raw_wlanAssocFailTimes_val;

    return MFD_SUCCESS;
} /* wlanAssocFailTimes_map */

/**
 * Extract the current value of the wlanAssocFailTimes data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wlanAssocFailTimes_val_ptr
 *        Pointer to storage for a u_long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wlanAssocFailTimes_get( dot11WlanStatsTable_rowreq_ctx *rowreq_ctx, u_long * wlanAssocFailTimes_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wlanAssocFailTimes_val_ptr );


    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanAssocFailTimes_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wlanAssocFailTimes data.
 * set (* wlanAssocFailTimes_val_ptr ) from rowreq_ctx->data
 */
    (* wlanAssocFailTimes_val_ptr ) = rowreq_ctx->data.wlanAssocFailTimes;

    return MFD_SUCCESS;
} /* wlanAssocFailTimes_get */

/*---------------------------------------------------------------------
 * DOT11-AC-MIB::dot11WlanStatsEntry.wlanUnassocNormallyTimes
 * wlanUnassocNormallyTimes is subid 6 of dot11WlanStatsEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.31656.6.1.2.13.4.1.6
 * Description:
The AP responsed station request unassociated Times.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   0
 *
 *
 * Its syntax is COUNTER (based on perltype COUNTER)
 * The net-snmp type is ASN_COUNTER. The C type decl is u_long (u_long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wlanUnassocNormallyTimes.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wlanUnassocNormallyTimes_map(u_long *mib_wlanUnassocNormallyTimes_val_ptr, u_long raw_wlanUnassocNormallyTimes_val)
{
    netsnmp_assert(NULL != mib_wlanUnassocNormallyTimes_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanUnassocNormallyTimes_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wlanUnassocNormallyTimes mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wlanUnassocNormallyTimes_val_ptr) = raw_wlanUnassocNormallyTimes_val;

    return MFD_SUCCESS;
} /* wlanUnassocNormallyTimes_map */

/**
 * Extract the current value of the wlanUnassocNormallyTimes data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wlanUnassocNormallyTimes_val_ptr
 *        Pointer to storage for a u_long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wlanUnassocNormallyTimes_get( dot11WlanStatsTable_rowreq_ctx *rowreq_ctx, u_long * wlanUnassocNormallyTimes_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wlanUnassocNormallyTimes_val_ptr );


    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanUnassocNormallyTimes_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wlanUnassocNormallyTimes data.
 * set (* wlanUnassocNormallyTimes_val_ptr ) from rowreq_ctx->data
 */
    (* wlanUnassocNormallyTimes_val_ptr ) = rowreq_ctx->data.wlanUnassocNormallyTimes;

    return MFD_SUCCESS;
} /* wlanUnassocNormallyTimes_get */

/*---------------------------------------------------------------------
 * DOT11-AC-MIB::dot11WlanStatsEntry.wlanUnassocAnomalyTimes
 * wlanUnassocAnomalyTimes is subid 7 of dot11WlanStatsEntry.
 * Its status is Current, and its access level is ReadOnly.
 * OID: .1.3.6.1.4.1.31656.6.1.2.13.4.1.7
 * Description:
The AP no responsed station request unassociated Times.
 *
 * Attributes:
 *   accessible 1     isscalar 0     enums  0      hasdefval 0
 *   readable   1     iscolumn 1     ranges 0      hashint   0
 *   settable   0
 *
 *
 * Its syntax is COUNTER (based on perltype COUNTER)
 * The net-snmp type is ASN_COUNTER. The C type decl is u_long (u_long)
 */
/**
 * map a value from its original native format to the MIB format.
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_ERROR           : Any other error
 *
 * @note parameters follow the memset convention (dest, src).
 *
 * @note generation and use of this function can be turned off by re-running
 * mib2c after adding the following line to the file
 * default-node-wlanUnassocAnomalyTimes.m2d :
 *   @eval $m2c_node_skip_mapping = 1@
 *
 * @remark
 *  If the values for your data type don't exactly match the
 *  possible values defined by the mib, you should map them here.
 *  Otherwise, just do a direct copy.
 */
int
wlanUnassocAnomalyTimes_map(u_long *mib_wlanUnassocAnomalyTimes_val_ptr, u_long raw_wlanUnassocAnomalyTimes_val)
{
    netsnmp_assert(NULL != mib_wlanUnassocAnomalyTimes_val_ptr);
    
    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanUnassocAnomalyTimes_map","called\n"));
    
    /*
     * TODO:241:o: |-> Implement wlanUnassocAnomalyTimes mapping.
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them here.
     */
    (*mib_wlanUnassocAnomalyTimes_val_ptr) = raw_wlanUnassocAnomalyTimes_val;

    return MFD_SUCCESS;
} /* wlanUnassocAnomalyTimes_map */

/**
 * Extract the current value of the wlanUnassocAnomalyTimes data.
 *
 * Set a value using the data context for the row.
 *
 * @param rowreq_ctx
 *        Pointer to the row request context.
 * @param wlanUnassocAnomalyTimes_val_ptr
 *        Pointer to storage for a u_long variable
 *
 * @retval MFD_SUCCESS         : success
 * @retval MFD_SKIP            : skip this node (no value for now)
 * @retval MFD_ERROR           : Any other error
 */
int
wlanUnassocAnomalyTimes_get( dot11WlanStatsTable_rowreq_ctx *rowreq_ctx, u_long * wlanUnassocAnomalyTimes_val_ptr )
{
   /** we should have a non-NULL pointer */
   netsnmp_assert( NULL != wlanUnassocAnomalyTimes_val_ptr );


    DEBUGMSGTL(("verbose:dot11WlanStatsTable:wlanUnassocAnomalyTimes_get","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);

/*
 * TODO:231:o: |-> Extract the current value of the wlanUnassocAnomalyTimes data.
 * set (* wlanUnassocAnomalyTimes_val_ptr ) from rowreq_ctx->data
 */
    (* wlanUnassocAnomalyTimes_val_ptr ) = rowreq_ctx->data.wlanUnassocAnomalyTimes;

    return MFD_SUCCESS;
} /* wlanUnassocAnomalyTimes_get */



/** @} */
