/*
 * Definitions for GAP VERSION 3 NUMBER 3.
 */
#ifndef __GAP3
#define __GAP3
#include <xnscourier/courier.h>
#include <xnscourier/courierconnection.h>


/*
 * Definitions from DEPENDS UPON Authentication inclusion
 * (must be linked with Authentication1_support.c also)
 */
#include <xnscourier/Authentication1.h>
typedef Cardinal GAP3_WaitTime;
#define sizeof_GAP3_WaitTime sizeof_Cardinal
#define clear_GAP3_WaitTime clear_Cardinal
#define externalize_GAP3_WaitTime externalize_Cardinal
#define internalize_GAP3_WaitTime internalize_Cardinal


typedef Unspecified GAP3_SessionHandle[2];

#define sizeof_GAP3_SessionHandle(p) 2

#define clear_GAP3_SessionHandle(p)

typedef enum {
	five = 0,
	six = 1,
	seven = 2,
	eight = 3
} GAP3_CharLength;
#define sizeof_GAP3_CharLength sizeof_enumeration
#define clear_GAP3_CharLength clear_enumeration
#define externalize_GAP3_CharLength externalize_enumeration
#define internalize_GAP3_CharLength internalize_enumeration


typedef enum {
	none = 0,
	odd = 1,
	even = 2,
	one = 3,
	zero = 4
} GAP3_Parity;
#define sizeof_GAP3_Parity sizeof_enumeration
#define clear_GAP3_Parity clear_enumeration
#define externalize_GAP3_Parity externalize_enumeration
#define internalize_GAP3_Parity internalize_enumeration


typedef enum {
	oneStopBit = 0,
	twoStopBits = 1
} GAP3_StopBits;
#define sizeof_GAP3_StopBits sizeof_enumeration
#define clear_GAP3_StopBits clear_enumeration
#define externalize_GAP3_StopBits externalize_enumeration
#define internalize_GAP3_StopBits internalize_enumeration


typedef enum {
	flowControlNone = 0,
	xOnXOff = 1
} GAP3_T_r3_2;
#define sizeof_GAP3_T_r3_2 sizeof_enumeration
#define clear_GAP3_T_r3_2 clear_enumeration
#define externalize_GAP3_T_r3_2 externalize_enumeration
#define internalize_GAP3_T_r3_2 internalize_enumeration


typedef struct {
	GAP3_T_r3_2 type;
	Unspecified xOn;
	Unspecified xOFF;
} GAP3_FlowControl;

#define sizeof_GAP3_FlowControl(p) 3

#define clear_GAP3_FlowControl(p)

typedef enum {
	wack = 0,
	nack = 1,
	defaultBidReply = 2
} GAP3_BidReply;
#define sizeof_GAP3_BidReply sizeof_enumeration
#define clear_GAP3_BidReply clear_enumeration
#define externalize_GAP3_BidReply externalize_enumeration
#define internalize_GAP3_BidReply internalize_enumeration


typedef enum {
	true = 0,
	false = 1,
	defaultExtendedBoolean = 2
} GAP3_ExtendedBoolean;
#define sizeof_GAP3_ExtendedBoolean sizeof_enumeration
#define clear_GAP3_ExtendedBoolean clear_enumeration
#define externalize_GAP3_ExtendedBoolean externalize_enumeration
#define internalize_GAP3_ExtendedBoolean internalize_enumeration


typedef enum {
	undefined = 0,
	terminal = 1,
	printer = 2
} GAP3_DeviceType;
#define sizeof_GAP3_DeviceType sizeof_enumeration
#define clear_GAP3_DeviceType clear_enumeration
#define externalize_GAP3_DeviceType externalize_enumeration
#define internalize_GAP3_DeviceType internalize_enumeration


typedef enum {
	xerox800 = 0,
	xerox850 = 1,
	xerox860 = 2,
	system6 = 3,
	cmcll = 4,
	imb2770 = 5,
	ibm2770Host = 6,
	ibm6670 = 7,
	ibm6670Host = 8,
	ibm3270 = 9,
	ibm3270Host = 10,
	oldTtyHost = 11,
	oldTty = 12,
	otherSessionType = 13,
	unknown = 14,
	ibm2780 = 15,
	ibm2780Host = 16,
	ibm3780 = 17,
	ibm3780Host = 18,
	siemens9750 = 19,
	siemens9750Host = 20,
	ttyHost = 21,
	tty = 22
} T_d3_3;
#define sizeof_T_d3_3 sizeof_enumeration
#define clear_T_d3_3 clear_enumeration
#define externalize_T_d3_3 externalize_enumeration
#define internalize_T_d3_3 internalize_enumeration


extern struct GAP3_SessionParameterObject;
typedef struct GAP3_SessionParameterObject GAP3_SessionParameterObject;

typedef struct {
	Unspecified pollProc;
} GAP3_T_c3_4;

#define sizeof_GAP3_T_c3_4(p) 1

#define clear_GAP3_T_c3_4(p)

typedef struct {
	Cardinal sendBlocksize;
	Cardinal receiveBlocksize;
} GAP3_T_c3_5;

#define sizeof_GAP3_T_c3_5(p) 2

#define clear_GAP3_T_c3_5(p)

typedef struct {
	GAP3_CharLength charLength;
	GAP3_Parity parity;
	GAP3_StopBits stopBits;
	Cardinal frameTimeout;
} GAP3_T_c3_6;

#define sizeof_GAP3_T_c3_6(p) 4

#define clear_GAP3_T_c3_6(p)

typedef struct {
	Cardinal sendBlocksize;
	Cardinal receiveBlocksize;
} GAP3_T_c3_7;

#define sizeof_GAP3_T_c3_7(p) 2

#define clear_GAP3_T_c3_7(p)

typedef struct {
	GAP3_CharLength charLength;
	GAP3_Parity parity;
	GAP3_StopBits stopBits;
	Cardinal frameTimeout;
	GAP3_FlowControl flowControl;
} GAP3_T_c3_8;

#define sizeof_GAP3_T_c3_8(p) 7

#define clear_GAP3_T_c3_8(p)

struct GAP3_SessionParameterObject {
	T_d3_3 designator;
	union {
		NilRecord u_xerox800;
#define xerox800_case u.u_xerox800
		GAP3_T_c3_4 u_xerox850;
#define xerox850_case u.u_xerox850
		GAP3_T_c3_4 u_xerox860;
#define xerox860_case u.u_xerox860
		GAP3_T_c3_5 u_system6;
#define system6_case u.u_system6
		GAP3_T_c3_5 u_cmcll;
#define cmcll_case u.u_cmcll
		GAP3_T_c3_5 u_imb2770;
#define imb2770_case u.u_imb2770
		GAP3_T_c3_5 u_ibm2770Host;
#define ibm2770Host_case u.u_ibm2770Host
		GAP3_T_c3_5 u_ibm6670;
#define ibm6670_case u.u_ibm6670
		GAP3_T_c3_5 u_ibm6670Host;
#define ibm6670Host_case u.u_ibm6670Host
		NilRecord u_ibm3270;
#define ibm3270_case u.u_ibm3270
		NilRecord u_ibm3270Host;
#define ibm3270Host_case u.u_ibm3270Host
		GAP3_T_c3_6 u_oldTtyHost;
#define oldTtyHost_case u.u_oldTtyHost
		GAP3_T_c3_6 u_oldTty;
#define oldTty_case u.u_oldTty
		NilRecord u_otherSessionType;
#define otherSessionType_case u.u_otherSessionType
		NilRecord u_unknown;
#define unknown_case u.u_unknown
		GAP3_T_c3_7 u_ibm2780;
#define ibm2780_case u.u_ibm2780
		GAP3_T_c3_7 u_ibm2780Host;
#define ibm2780Host_case u.u_ibm2780Host
		GAP3_T_c3_7 u_ibm3780;
#define ibm3780_case u.u_ibm3780
		GAP3_T_c3_7 u_ibm3780Host;
#define ibm3780Host_case u.u_ibm3780Host
		NilRecord u_siemens9750;
#define siemens9750_case u.u_siemens9750
		NilRecord u_siemens9750Host;
#define siemens9750Host_case u.u_siemens9750Host
		GAP3_T_c3_8 u_ttyHost;
#define ttyHost_case u.u_ttyHost
		GAP3_T_c3_8 u_tty;
#define tty_case u.u_tty
	} u;
};

typedef enum {
	bitSynchronous = 0,
	byteSynchronous = 1,
	asynchronous = 2,
	autoRecognition = 3
} GAP3_LineType;
#define sizeof_GAP3_LineType sizeof_enumeration
#define clear_GAP3_LineType clear_enumeration
#define externalize_GAP3_LineType externalize_enumeration
#define internalize_GAP3_LineType internalize_enumeration


typedef enum {
	bps50 = 0,
	bps75 = 1,
	bps110 = 2,
	bps135 = 3,
	bps150 = 4,
	bps300 = 5,
	bps600 = 6,
	bps1200 = 7,
	bps2400 = 8,
	bps3600 = 9,
	bps4800 = 10,
	bps7200 = 11,
	bps9600 = 12,
	bps19200 = 13,
	bps28800 = 14,
	bps38400 = 15,
	bps48000 = 16,
	bps56000 = 17,
	bps57600 = 18
} GAP3_LineSpeed;
#define sizeof_GAP3_LineSpeed sizeof_enumeration
#define clear_GAP3_LineSpeed clear_enumeration
#define externalize_GAP3_LineSpeed externalize_enumeration
#define internalize_GAP3_LineSpeed internalize_enumeration


typedef enum {
	fullduplex = 0,
	halfduplex = 1
} GAP3_Duplexity;
#define sizeof_GAP3_Duplexity sizeof_enumeration
#define clear_GAP3_Duplexity clear_enumeration
#define externalize_GAP3_Duplexity externalize_enumeration
#define internalize_GAP3_Duplexity internalize_enumeration


typedef enum {
	directConn = 0,
	dialConn = 1
} T_d3_9;
#define sizeof_T_d3_9 sizeof_enumeration
#define clear_T_d3_9 clear_enumeration
#define externalize_T_d3_9 externalize_enumeration
#define internalize_T_d3_9 internalize_enumeration


extern struct GAP3_T_r3_10;
typedef struct GAP3_T_r3_10 GAP3_T_r3_10;

typedef struct {
	GAP3_Duplexity duplex;
	GAP3_LineType lineType;
	GAP3_LineSpeed lineSpeed;
} GAP3_T_c3_11;

#define sizeof_GAP3_T_c3_11(p) 3

#define clear_GAP3_T_c3_11(p)

typedef enum {
	manualDial = 0,
	autoDial = 1
} GAP3_T_r3_13;
#define sizeof_GAP3_T_r3_13 sizeof_enumeration
#define clear_GAP3_T_r3_13 clear_enumeration
#define externalize_GAP3_T_r3_13 externalize_enumeration
#define internalize_GAP3_T_r3_13 internalize_enumeration


typedef struct {
	GAP3_Duplexity duplex;
	GAP3_LineType lineType;
	GAP3_LineSpeed lineSpeed;
	GAP3_T_r3_13 dialMode;
	Cardinal dialerNumber;
	Cardinal retryCount;
} GAP3_T_c3_12;

#define sizeof_GAP3_T_c3_12(p) 6

#define clear_GAP3_T_c3_12(p)

struct GAP3_T_r3_10 {
	T_d3_9 designator;
	union {
		GAP3_T_c3_11 u_directConn;
#define directConn_case u.u_directConn
		GAP3_T_c3_12 u_dialConn;
#define dialConn_case u.u_dialConn
	} u;
};

typedef struct {
	GAP3_T_r3_10 accessDetail;
} GAP3_CommParamObject;

typedef enum {
	preemptNever = 0,
	preemptAlways = 1,
	preemptInactive = 2
} GAP3_ReserveType;
#define sizeof_GAP3_ReserveType sizeof_enumeration
#define clear_GAP3_ReserveType clear_enumeration
#define externalize_GAP3_ReserveType externalize_enumeration
#define internalize_GAP3_ReserveType internalize_enumeration


typedef Unspecified GAP3_Resource[2];

#define sizeof_GAP3_Resource(p) 2

#define clear_GAP3_Resource(p)

typedef enum {
	primary = 0,
	secondary = 1
} GAP3_LineControl;
#define sizeof_GAP3_LineControl sizeof_enumeration
#define clear_GAP3_LineControl clear_enumeration
#define externalize_GAP3_LineControl externalize_enumeration
#define internalize_GAP3_LineControl internalize_enumeration

typedef Cardinal GAP3_ControllerAddress;
#define sizeof_GAP3_ControllerAddress sizeof_Cardinal
#define clear_GAP3_ControllerAddress clear_Cardinal
#define externalize_GAP3_ControllerAddress externalize_Cardinal
#define internalize_GAP3_ControllerAddress internalize_Cardinal

typedef Cardinal GAP3_TerminalAddress;
#define sizeof_GAP3_TerminalAddress sizeof_Cardinal
#define clear_GAP3_TerminalAddress clear_Cardinal
#define externalize_GAP3_TerminalAddress externalize_Cardinal
#define internalize_GAP3_TerminalAddress internalize_Cardinal


typedef enum {
	alreadyReserved = 0,
	reserveNeeded = 1
} T_d3_14;
#define sizeof_T_d3_14 sizeof_enumeration
#define clear_T_d3_14 clear_enumeration
#define externalize_T_d3_14 externalize_enumeration
#define internalize_T_d3_14 internalize_enumeration


typedef enum {
	rs232c = 0,
	bsc = 1,
	teletype = 2,
	polledBSCController = 3,
	sdlcController = 5,
	polledBSCTerminal = 4,
	sdlcTerminal = 6,
	service = 7,
	unused = 8,
	polledBSCPrinter = 9,
	sdlcPrinter = 10
} T_d3_15;
#define sizeof_T_d3_15 sizeof_enumeration
#define clear_T_d3_15 clear_enumeration
#define externalize_T_d3_15 externalize_enumeration
#define internalize_T_d3_15 internalize_enumeration


extern struct GAP3_TransportObject;
typedef struct GAP3_TransportObject GAP3_TransportObject;

extern struct GAP3_T_r3_17;
typedef struct GAP3_T_r3_17 GAP3_T_r3_17;

typedef struct {
	GAP3_Resource resource;
} GAP3_T_c3_18;

#define sizeof_GAP3_T_c3_18(p) 2

#define clear_GAP3_T_c3_18(p)

typedef struct {
	Cardinal lineNumber;
} GAP3_T_c3_19;

#define sizeof_GAP3_T_c3_19(p) 1

#define clear_GAP3_T_c3_19(p)

struct GAP3_T_r3_17 {
	T_d3_14 designator;
	union {
		GAP3_T_c3_18 u_alreadyReserved;
#define alreadyReserved_case u.u_alreadyReserved
		GAP3_T_c3_19 u_reserveNeeded;
#define reserveNeeded_case u.u_reserveNeeded
	} u;
};

typedef struct {
	GAP3_CommParamObject commParams;
	GAP3_ReserveType preemptOthers;
	GAP3_ReserveType preemptMe;
	String phoneNumber;
	GAP3_T_r3_17 line;
} GAP3_T_c3_16;

typedef struct {
	String localTerminalID;
	String localSecurityID;
	GAP3_LineControl lineControl;
	Unspecified authenticateProc;
	GAP3_BidReply bidReply;
	GAP3_ExtendedBoolean sendLineHoldingEOTs;
	GAP3_ExtendedBoolean expectLineHoldingEOTs;
} GAP3_T_c3_20;

typedef struct {
	String hostControllerName;
	Cardinal controllerAddress;
	Cardinal portsOnController;
} GAP3_T_c3_21;

typedef struct {
	String hostControllerName;
	Cardinal terminalAddress;
} GAP3_T_c3_22;

typedef struct {
	LongCardinal id;
} GAP3_T_c3_23;

#define sizeof_GAP3_T_c3_23(p) 2

#define clear_GAP3_T_c3_23(p)

typedef struct {
	String hostControllerName;
	Cardinal printerAddress;
} GAP3_T_c3_24;

struct GAP3_TransportObject {
	T_d3_15 designator;
	union {
		GAP3_T_c3_16 u_rs232c;
#define rs232c_case u.u_rs232c
		GAP3_T_c3_20 u_bsc;
#define bsc_case u.u_bsc
		NilRecord u_teletype;
#define teletype_case u.u_teletype
		GAP3_T_c3_21 u_polledBSCController;
#define polledBSCController_case u.u_polledBSCController
		GAP3_T_c3_21 u_sdlcController;
#define sdlcController_case u.u_sdlcController
		GAP3_T_c3_22 u_polledBSCTerminal;
#define polledBSCTerminal_case u.u_polledBSCTerminal
		GAP3_T_c3_22 u_sdlcTerminal;
#define sdlcTerminal_case u.u_sdlcTerminal
		GAP3_T_c3_23 u_service;
#define service_case u.u_service
		NilRecord u_unused;
#define unused_case u.u_unused
		GAP3_T_c3_24 u_polledBSCPrinter;
#define polledBSCPrinter_case u.u_polledBSCPrinter
		GAP3_T_c3_24 u_sdlcPrinter;
#define sdlcPrinter_case u.u_sdlcPrinter
	} u;
};

static Cardinal GAP3_infiniteTime = {0177777};

static Unspecified GAP3_NopPollProc = {00};

static Cardinal GAP3_unspecifiedTerminalAddr = {0177777};

#define GAP3_unimplemented (ERROR_OFFSET+0)
#define GAP3_unimplementedArgs T_cn3_25

#define GAP3_noCommunicationHardware (ERROR_OFFSET+1)
#define GAP3_noCommunicationHardwareArgs T_cn3_26

#define GAP3_illegalTransport (ERROR_OFFSET+2)
#define GAP3_illegalTransportArgs T_cn3_27

#define GAP3_mediumConnectFailed (ERROR_OFFSET+3)
#define GAP3_mediumConnectFailedArgs T_cn3_28

#define GAP3_badAddressFormat (ERROR_OFFSET+4)
#define GAP3_badAddressFormatArgs T_cn3_29

#define GAP3_noDialingHardware (ERROR_OFFSET+5)
#define GAP3_noDialingHardwareArgs T_cn3_30

#define GAP3_dialingHardwareProblem (ERROR_OFFSET+6)
#define GAP3_dialingHardwareProblemArgs T_cn3_31

#define GAP3_transmissionMediumUnavailable (ERROR_OFFSET+7)
#define GAP3_transmissionMediumUnavailableArgs T_cn3_32

#define GAP3_inconsistentParams (ERROR_OFFSET+8)
#define GAP3_inconsistentParamsArgs T_cn3_33

#define GAP3_tooManyGateStreams (ERROR_OFFSET+9)
#define GAP3_tooManyGateStreamsArgs T_cn3_34

#define GAP3_bugInGAPCode (ERROR_OFFSET+10)
#define GAP3_bugInGAPCodeArgs T_cn3_35

#define GAP3_gapNotExported (ERROR_OFFSET+11)
#define GAP3_gapNotExportedArgs T_cn3_36

#define GAP3_gapCommunicationError (ERROR_OFFSET+12)
#define GAP3_gapCommunicationErrorArgs T_cn3_37

#define GAP3_controllerAlreadyExists (ERROR_OFFSET+13)
#define GAP3_controllerAlreadyExistsArgs T_cn3_38

#define GAP3_controllerDoesNotExist (ERROR_OFFSET+14)
#define GAP3_controllerDoesNotExistArgs T_cn3_39

#define GAP3_terminalAddressInUse (ERROR_OFFSET+15)
#define GAP3_terminalAddressInUseArgs T_cn3_40

#define GAP3_terminalAddressInvalid (ERROR_OFFSET+16)
#define GAP3_terminalAddressInvalidArgs T_cn3_41

#define GAP3_serviceTooBusy (ERROR_OFFSET+17)
#define GAP3_serviceTooBusyArgs T_cn3_42

#define GAP3_userNotAuthenticated (ERROR_OFFSET+18)
#define GAP3_userNotAuthenticatedArgs T_cn3_43

#define GAP3_userNotAuthorized (ERROR_OFFSET+19)
#define GAP3_userNotAuthorizedArgs T_cn3_44

#define GAP3_serviceNotFound (ERROR_OFFSET+20)
#define GAP3_serviceNotFoundArgs T_cn3_45

extern void GAP3_Reset();

typedef struct {
	GAP3_SessionHandle session;
} GAP3_CreateResults;

#define sizeof_GAP3_CreateResults(p) 2

#define clear_GAP3_CreateResults(p)

typedef struct {
	Cardinal length;
	GAP3_TransportObject *sequence;
} GAP3_T_p3_48;

extern GAP3_CreateResults GAP3_Create();

extern void GAP3_Delete();

#endif __GAP

