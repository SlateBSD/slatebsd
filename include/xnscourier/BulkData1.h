/*
 * Definitions for BulkData VERSION 1 NUMBER 0.
 */
#ifndef __BulkData1
#define __BulkData1
#include <xnscourier/courier.h>
#include <xnscourier/courierconnection.h>


typedef Unspecified BulkData1_T_r0_1[3];

#define sizeof_BulkData1_T_r0_1(p) 3

#define clear_BulkData1_T_r0_1(p)

typedef Unspecified BulkData1_T_r0_2[2];

#define sizeof_BulkData1_T_r0_2(p) 2

#define clear_BulkData1_T_r0_2(p)

typedef struct {
	BulkData1_T_r0_1 host;
	BulkData1_T_r0_2 hostRelativeIdentifier;
} BulkData1_Identifier;

#define sizeof_BulkData1_Identifier(p) 5

#define clear_BulkData1_Identifier(p)

typedef enum {
	null = 0,
	immediate = 1,
	passive = 2,
	active = 3
} T_d0_3;
#define sizeof_T_d0_3 sizeof_enumeration
#define clear_T_d0_3 clear_enumeration
#define externalize_T_d0_3 externalize_enumeration
#define internalize_T_d0_3 internalize_enumeration


extern struct BulkData1_Descriptor;
typedef struct BulkData1_Descriptor BulkData1_Descriptor;

typedef Unspecified BulkData1_T_r0_5[2];

#define sizeof_BulkData1_T_r0_5(p) 2

#define clear_BulkData1_T_r0_5(p)

typedef Unspecified BulkData1_T_r0_6[3];

#define sizeof_BulkData1_T_r0_6(p) 3

#define clear_BulkData1_T_r0_6(p)

typedef struct {
	BulkData1_T_r0_5 network;
	BulkData1_T_r0_6 host;
	BulkData1_Identifier identifier;
} BulkData1_T_c0_4;

#define sizeof_BulkData1_T_c0_4(p) 10

#define clear_BulkData1_T_c0_4(p)

struct BulkData1_Descriptor {
	T_d0_3 designator;
	union {
		NilRecord u_null;
#define null_case u.u_null
		NilRecord u_immediate;
#define immediate_case u.u_immediate
		BulkData1_T_c0_4 u_passive;
#define passive_case u.u_passive
		BulkData1_T_c0_4 u_active;
#define active_case u.u_active
	} u;
};
typedef BulkData1_Descriptor BulkData1_Sink;
#define sizeof_BulkData1_Sink sizeof_BulkData1_Descriptor
#define clear_BulkData1_Sink clear_BulkData1_Descriptor
#define externalize_BulkData1_Sink externalize_BulkData1_Descriptor
#define internalize_BulkData1_Sink internalize_BulkData1_Descriptor


static BulkData1_Descriptor BulkData1_immediateSink = 
	{ immediate
	};

static BulkData1_Descriptor BulkData1_nullSink = 
	{ null
	};
typedef BulkData1_Descriptor BulkData1_Source;
#define sizeof_BulkData1_Source sizeof_BulkData1_Descriptor
#define clear_BulkData1_Source clear_BulkData1_Descriptor
#define externalize_BulkData1_Source externalize_BulkData1_Descriptor
#define internalize_BulkData1_Source internalize_BulkData1_Descriptor


static BulkData1_Descriptor BulkData1_immediateSource = 
	{ immediate
	};

static BulkData1_Descriptor BulkData1_nullSource = 
	{ null
	};

#define BulkData1_InvalidDescriptor (ERROR_OFFSET+0)
#define BulkData1_InvalidDescriptorArgs T_cn0_7

#define BulkData1_NoSuchIdentifier (ERROR_OFFSET+1)
#define BulkData1_NoSuchIdentifierArgs T_cn0_8

#define BulkData1_IdentifierBusy (ERROR_OFFSET+2)
#define BulkData1_IdentifierBusyArgs T_cn0_9

#define BulkData1_WrongHost (ERROR_OFFSET+3)
#define BulkData1_WrongHostArgs T_cn0_10

#define BulkData1_WrongDirection (ERROR_OFFSET+4)
#define BulkData1_WrongDirectionArgs T_cn0_11

#define BulkData1_TransferAborted (ERROR_OFFSET+5)
#define BulkData1_TransferAbortedArgs T_cn0_12

extern void BulkData1_Send();

extern void BulkData1_Receive();

extern void BulkData1_Cancel();

#endif __BulkData

