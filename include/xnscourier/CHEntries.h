/*
 * Definitions for CHEntries VERSION 0 NUMBER 0.
 */
#ifndef __CHEntries0
#define __CHEntries0
#include <xnscourier/courier.h>
#include <xnscourier/courierconnection.h>


/*
 * Definitions from DEPENDS UPON Clearinghouse inclusion
 * (must be linked with Clearinghouse2_support.c also)
 */
#include <xnscourier/Clearinghouse2.h>

/*
 * Definitions from DEPENDS UPON Time inclusion
 * (must be linked with Time2_support.c also)
 */
#include <xnscourier/Time2.h>

static LongCardinal CHEntries0_members = {3};

static LongCardinal CHEntries0_addressList = {4};

static LongCardinal CHEntries0_mailboxes = {31};

static LongCardinal CHEntries0_authenticationLevel = {8};

static LongCardinal CHEntries0_fileService = {10000};

static LongCardinal CHEntries0_printService = {10001};

static LongCardinal CHEntries0_user = {10003};

static LongCardinal CHEntries0_mailService = {10004};

static LongCardinal CHEntries0_clearinghouseService = {10021};

static LongCardinal CHEntries0_userGroup = {10022};

static LongCardinal CHEntries0_userData = {20000};
typedef String CHEntries0_Description;
#define sizeof_CHEntries0_Description sizeof_String
#define clear_CHEntries0_Description clear_String
#define externalize_CHEntries0_Description externalize_String
#define internalize_CHEntries0_Description internalize_String

typedef Clearinghouse2_NetworkAddressList CHEntries0_AddressListValue;
#define sizeof_CHEntries0_AddressListValue sizeof_Clearinghouse2_NetworkAddressList
#define clear_CHEntries0_AddressListValue clear_Clearinghouse2_NetworkAddressList
#define externalize_CHEntries0_AddressListValue externalize_Clearinghouse2_NetworkAddressList
#define internalize_CHEntries0_AddressListValue internalize_Clearinghouse2_NetworkAddressList


typedef Clearinghouse2_ThreePartName CHEntries0_T_r0_28[4];

typedef struct {
	LongCardinal time;
	CHEntries0_T_r0_28 mailService;
} CHEntries0_MailboxesValue;

typedef struct {
	Boolean simpleSupported;
	Boolean strongSupported;
} CHEntries0_AuthenticationLevelValue;

#define sizeof_CHEntries0_AuthenticationLevelValue(p) 2

#define clear_CHEntries0_AuthenticationLevelValue(p)

typedef struct {
	Cardinal lastNameIndex;
	Clearinghouse2_ThreePartName fileService;
} CHEntries0_UserDataValue;

static LongCardinal CHEntries0_authKeys = {6};

static LongCardinal CHEntries0_services = {51};

static LongCardinal CHEntries0_internetworkRoutingService = {10002};

static LongCardinal CHEntries0_workstation = {10005};

static LongCardinal CHEntries0_externalCommunicationsService = {10006};

static LongCardinal CHEntries0_rs232CPort = {10007};

static LongCardinal CHEntries0_interactiveTerminalService = {10008};

static LongCardinal CHEntries0_gatewayService = {10009};

static LongCardinal CHEntries0_ibm3270Host = {10010};

static LongCardinal CHEntries0_mailGateway = {10011};

static LongCardinal CHEntries0_siemens9750Host = {10012};

static LongCardinal CHEntries0_adobeService = {10013};

static LongCardinal CHEntries0_librarianService = {10014};

static LongCardinal CHEntries0_ttxGateway = {10015};

static LongCardinal CHEntries0_authenticationService = {10016};

static LongCardinal CHEntries0_remoteBatchService = {10017};

static LongCardinal CHEntries0_network = {10018};

static LongCardinal CHEntries0_networkServers = {10019};

static LongCardinal CHEntries0_communicationsInterfaceUnit = {10020};

static LongCardinal CHEntries0_fetchService = {10023};

static LongCardinal CHEntries0_rs232CData = {20001};

static LongCardinal CHEntries0_ibm3270HostData = {20002};

static LongCardinal CHEntries0_siemens9750HostData = {20003};

static LongCardinal CHEntries0_canMailTo = {20005};

static LongCardinal CHEntries0_mailGatewayRouteData = {20006};

static LongCardinal CHEntries0_foreignMailSystemName = {20007};

static LongCardinal CHEntries0_userPassword = {20101};

static LongCardinal CHEntries0_rs232CBack = {20102};

static LongCardinal CHEntries0_ibm3270HostBack = {20103};

static LongCardinal CHEntries0_associatedWorkstation = {30005};
typedef Cardinal CHEntries0_Duplexity;
#define sizeof_CHEntries0_Duplexity sizeof_Cardinal
#define clear_CHEntries0_Duplexity clear_Cardinal
#define externalize_CHEntries0_Duplexity externalize_Cardinal
#define internalize_CHEntries0_Duplexity internalize_Cardinal

typedef Cardinal CHEntries0_CharLength;
#define sizeof_CHEntries0_CharLength sizeof_Cardinal
#define clear_CHEntries0_CharLength clear_Cardinal
#define externalize_CHEntries0_CharLength externalize_Cardinal
#define internalize_CHEntries0_CharLength internalize_Cardinal

typedef Cardinal CHEntries0_FlowControl;
#define sizeof_CHEntries0_FlowControl sizeof_Cardinal
#define clear_CHEntries0_FlowControl clear_Cardinal
#define externalize_CHEntries0_FlowControl externalize_Cardinal
#define internalize_CHEntries0_FlowControl internalize_Cardinal

typedef Cardinal CHEntries0_LineSpeed;
#define sizeof_CHEntries0_LineSpeed sizeof_Cardinal
#define clear_CHEntries0_LineSpeed clear_Cardinal
#define externalize_CHEntries0_LineSpeed externalize_Cardinal
#define internalize_CHEntries0_LineSpeed internalize_Cardinal

typedef Cardinal CHEntries0_Parity;
#define sizeof_CHEntries0_Parity sizeof_Cardinal
#define clear_CHEntries0_Parity clear_Cardinal
#define externalize_CHEntries0_Parity externalize_Cardinal
#define internalize_CHEntries0_Parity internalize_Cardinal

typedef Cardinal CHEntries0_StopBits;
#define sizeof_CHEntries0_StopBits sizeof_Cardinal
#define clear_CHEntries0_StopBits clear_Cardinal
#define externalize_CHEntries0_StopBits externalize_Cardinal
#define internalize_CHEntries0_StopBits internalize_Cardinal


typedef enum {
	unassigned = 0,
	outOfService = 1,
	its = 2,
	irs = 3,
	gws = 4,
	ibm3270Host = 5,
	ttyEmulation = 6,
	rbs = 7,
	fax = 9,
	mailGateway = 10,
	phototypesetter = 11
} CHEntries0_PortClientType;
#define sizeof_CHEntries0_PortClientType sizeof_enumeration
#define clear_CHEntries0_PortClientType clear_enumeration
#define externalize_CHEntries0_PortClientType externalize_enumeration
#define internalize_CHEntries0_PortClientType internalize_enumeration


typedef enum {
	dialerNone = 0,
	vadic = 1,
	hayes = 2,
	ventel = 3,
	rs366 = 4
} CHEntries0_PortDialerType;
#define sizeof_CHEntries0_PortDialerType sizeof_enumeration
#define clear_CHEntries0_PortDialerType clear_enumeration
#define externalize_CHEntries0_PortDialerType externalize_enumeration
#define internalize_CHEntries0_PortDialerType internalize_enumeration


typedef enum {
	asynchronous = 0,
	synchronous = 1,
	bitSynchronous = 2,
	byteSynchronous = 3,
	syncAny = 4
} CHEntries0_PortSyncType;
#define sizeof_CHEntries0_PortSyncType sizeof_enumeration
#define clear_CHEntries0_PortSyncType clear_enumeration
#define externalize_CHEntries0_PortSyncType externalize_enumeration
#define internalize_CHEntries0_PortSyncType internalize_enumeration


typedef enum {
	echoLocal = 0,
	echoRemote = 1
} CHEntries0_PortEchoingLocation;
#define sizeof_CHEntries0_PortEchoingLocation sizeof_enumeration
#define clear_CHEntries0_PortEchoingLocation clear_enumeration
#define externalize_CHEntries0_PortEchoingLocation externalize_enumeration
#define internalize_CHEntries0_PortEchoingLocation internalize_enumeration


typedef struct {
	Cardinal length;
	Cardinal *sequence;
} CHEntries0_T_r0_29;

#define sizeof_CHEntries0_T_r0_29(p) (1 + (p)->length * 1)

typedef struct {
	Boolean cIUPort;
	CHEntries0_PortClientType owningClientType;
	Boolean preemptionAllowed;
	Cardinal lineNumber;
	Cardinal dialerNumber;
	Cardinal duplexity;
	CHEntries0_PortDialerType dialingHardware;
	Cardinal charLength;
	CHEntries0_PortEchoingLocation echoing;
	LongCardinal xxxxpaddingxxx;
	Cardinal flowControl;
	Cardinal lineSpeed;
	Cardinal parity;
	Cardinal stopBits;
	Boolean portActsAsDCE;
	Clearinghouse2_ThreePartName accessControl;
	CHEntries0_T_r0_29 validLineSpeeds;
} CHEntries0_RS232CData;

#endif __CHEntries

