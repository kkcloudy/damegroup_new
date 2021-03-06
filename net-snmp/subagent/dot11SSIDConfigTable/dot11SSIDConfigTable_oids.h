/*
 * Note: this file originally auto-generated by mib2c using
 *  : generic-table-oids.m2c 12855 2005-09-27 15:56:08Z rstory $
 *
 * $Id:$
 */
#ifndef DOT11SSIDCONFIGTABLE_OIDS_H
#define DOT11SSIDCONFIGTABLE_OIDS_H

#ifdef __cplusplus
extern "C" {
#endif


/* column number definitions for table dot11SSIDConfigTable */
//#define DOT11SSIDCONFIGTABLE_OID              1,3,6,1,4,1,31656,6,1,2,13,6

#define COLUMN_NEWSSIDNAME         1
#define COLUMN_NEWSSIDNAME_FLAG    (0x1 << 0)
    
#define COLUMN_NEWSSIDENABLED         2
#define COLUMN_NEWSSIDENABLED_FLAG    (0x1 << 1)
    
#define COLUMN_NEWSSIDHIDDEN         3
#define COLUMN_NEWSSIDHIDDEN_FLAG    (0x1 << 2)
    
#define COLUMN_NEWSTAISOLATE         4
#define COLUMN_NEWSTAISOLATE_FLAG    (0x1 << 3)
    
#define COLUMN_NEWDOT11AUTH         5
#define COLUMN_NEWDOT11AUTH_FLAG    (0x1 << 4)
    
#define COLUMN_NEWSECURITY         6
#define COLUMN_NEWSECURITY_FLAG    (0x1 << 5)
    
#define COLUMN_NEWAUTHENMODE         7
#define COLUMN_NEWAUTHENMODE_FLAG    (0x1 << 6)
    
#define COLUMN_NEWSECURITYCIPHERS         8
#define COLUMN_NEWSECURITYCIPHERS_FLAG    (0x1 << 7)
    
#define COLUMN_NEWENCRINPUTTYPE         9
#define COLUMN_NEWENCRINPUTTYPE_FLAG    (0x1 << 8)
    
#define COLUMN_NEWSECURITYKEY         10
#define COLUMN_NEWSECURITYKEY_FLAG    (0x1 << 9)
    
#define COLUMN_NEWEXTENSIBLEAUTH         11
#define COLUMN_NEWEXTENSIBLEAUTH_FLAG    (0x1 << 10)
    
#define COLUMN_NEWAUTHIP         12
#define COLUMN_NEWAUTHIP_FLAG    (0x1 << 11)
    
#define COLUMN_NEWAUTHPORT         13
#define COLUMN_NEWAUTHPORT_FLAG    (0x1 << 12)
    
#define COLUMN_NEWAUTHSHAREDSECRET         14
#define COLUMN_NEWAUTHSHAREDSECRET_FLAG    (0x1 << 13)
    
#define COLUMN_NEWACCTIP         15
#define COLUMN_NEWACCTIP_FLAG    (0x1 << 14)
    
#define COLUMN_NEWACCTPORT         16
#define COLUMN_NEWACCTPORT_FLAG    (0x1 << 15)
    
#define COLUMN_NEWACCTSHAREDSECRET         17
#define COLUMN_NEWACCTSHAREDSECRET_FLAG    (0x1 << 16)
    
#define COLUMN_NEWVLANID         18
#define COLUMN_NEWVLANID_FLAG    (0x1 << 17)
    
#define COLUMN_NEWMAXSIMULTUSERS         19
#define COLUMN_NEWMAXSIMULTUSERS_FLAG    (0x1 << 18)
    
#define COLUMN_NEWSTAUPLINKMAXRATE         20
#define COLUMN_NEWSTAUPLINKMAXRATE_FLAG    (0x1 << 19)
    
#define COLUMN_NEWSTADWLINKMAXRATE         21
#define COLUMN_NEWSTADWLINKMAXRATE_FLAG    (0x1 << 20)
    
#define COLUMN_SSIDROWSTATUS         22
#define COLUMN_SSIDROWSTATUS_FLAG    (0x1 << 21)
    
#define COLUMN_STASDEADTIME         25
#define COLUMN_STASDEADTIME_FLAG    (0x1 << 24)
    
#define COLUMN_AUTHDEADTIME         26
#define COLUMN_AUTHDEADTIME_FLAG    (0x1 << 25)
    
#define COLUMN_NEWWAPIAUTHIP         27
#define COLUMN_NEWWAPIAUTHIP_FLAG    (0x1 << 26)
    
#define COLUMN_NEWWAPICERTIFICATIONTYPE         28
#define COLUMN_NEWWAPICERTIFICATIONTYPE_FLAG    (0x1 << 27)
    
#define COLUMN_NEWWAPIASCERTIFICATIONPATH         29
#define COLUMN_NEWWAPIASCERTIFICATIONPATH_FLAG    (0x1 << 28)
    
#define COLUMN_NEWWAPIAECERTIFICATIONPATH         30
#define COLUMN_NEWWAPIAECERTIFICATIONPATH_FLAG    (0x1 << 29)
    

#define DOT11SSIDCONFIGTABLE_MIN_COL   COLUMN_NEWSSIDNAME
#define DOT11SSIDCONFIGTABLE_MAX_COL   COLUMN_NEWWAPIAECERTIFICATIONPATH
    

    /*
     * TODO:405:r: Review DOT11SSIDCONFIGTABLE_SETTABLE_COLS macro.
     * OR together all the writable cols.
     */
#define DOT11SSIDCONFIGTABLE_SETTABLE_COLS (COLUMN_NEWSSIDNAME_FLAG | COLUMN_NEWSSIDENABLED_FLAG | COLUMN_NEWSSIDHIDDEN_FLAG | COLUMN_NEWSTAISOLATE_FLAG | COLUMN_NEWDOT11AUTH_FLAG | COLUMN_NEWSECURITY_FLAG | COLUMN_NEWAUTHENMODE_FLAG | COLUMN_NEWSECURITYCIPHERS_FLAG | COLUMN_NEWENCRINPUTTYPE_FLAG | COLUMN_NEWSECURITYKEY_FLAG | COLUMN_NEWEXTENSIBLEAUTH_FLAG | COLUMN_NEWAUTHIP_FLAG | COLUMN_NEWAUTHPORT_FLAG | COLUMN_NEWAUTHSHAREDSECRET_FLAG | COLUMN_NEWACCTIP_FLAG | COLUMN_NEWACCTPORT_FLAG | COLUMN_NEWACCTSHAREDSECRET_FLAG | COLUMN_NEWVLANID_FLAG | COLUMN_NEWMAXSIMULTUSERS_FLAG | COLUMN_NEWSTAUPLINKMAXRATE_FLAG | COLUMN_NEWSTADWLINKMAXRATE_FLAG | COLUMN_SSIDROWSTATUS_FLAG | COLUMN_STASDEADTIME_FLAG | COLUMN_AUTHDEADTIME_FLAG | COLUMN_NEWWAPIAUTHIP_FLAG | COLUMN_NEWWAPICERTIFICATIONTYPE_FLAG | COLUMN_NEWWAPIASCERTIFICATIONPATH_FLAG | COLUMN_NEWWAPIAECERTIFICATIONPATH_FLAG)
    /*
     * TODO:405:r: Review DOT11SSIDCONFIGTABLE_REQUIRED_COLS macro.
     * OR together all the required rows for row creation.
     * default is writable cols w/out defaults.
     */
#define DOT11SSIDCONFIGTABLE_REQUIRED_COLS (COLUMN_NEWSSIDNAME_FLAG | COLUMN_NEWSSIDENABLED_FLAG | COLUMN_NEWSSIDHIDDEN_FLAG | COLUMN_NEWSTAISOLATE_FLAG | COLUMN_NEWDOT11AUTH_FLAG | COLUMN_NEWSECURITY_FLAG | COLUMN_NEWAUTHENMODE_FLAG | COLUMN_NEWSECURITYCIPHERS_FLAG | COLUMN_NEWENCRINPUTTYPE_FLAG | COLUMN_NEWSECURITYKEY_FLAG | COLUMN_NEWEXTENSIBLEAUTH_FLAG | COLUMN_NEWAUTHIP_FLAG | COLUMN_NEWAUTHPORT_FLAG | COLUMN_NEWAUTHSHAREDSECRET_FLAG | COLUMN_NEWACCTIP_FLAG | COLUMN_NEWACCTPORT_FLAG | COLUMN_NEWACCTSHAREDSECRET_FLAG | COLUMN_NEWVLANID_FLAG | COLUMN_NEWMAXSIMULTUSERS_FLAG | COLUMN_NEWSTAUPLINKMAXRATE_FLAG | COLUMN_NEWSTADWLINKMAXRATE_FLAG | COLUMN_SSIDROWSTATUS_FLAG | COLUMN_STASDEADTIME_FLAG | COLUMN_AUTHDEADTIME_FLAG | COLUMN_NEWWAPIAUTHIP_FLAG | COLUMN_NEWWAPICERTIFICATIONTYPE_FLAG | COLUMN_NEWWAPIASCERTIFICATIONPATH_FLAG | COLUMN_NEWWAPIAECERTIFICATIONPATH_FLAG)
    

#ifdef __cplusplus
}
#endif

#endif /* DOT11SSIDCONFIGTABLE_OIDS_H */
