/*
 * Definitions for GAP VERSION 2 NUMBER 3.
 */
#ifndef __GAP2
#define __GAP2
#include <xnscourier/courier.h>
#include <xnscourier/courierconnection.h>

typedef Cardinal GAP2_WaitTime;
#define sizeof_GAP2_WaitTime sizeof_Cardinal
#define clear_GAP2_WaitTime clear_Cardinal
#define externalize_GAP2_WaitTime externalize_Cardinal
#define internalize_GAP2_WaitTime internalize_Cardinal


typedef Unspecified GAP2_SessionHandle[2];

#define sizeof_GAP2_SessionHandle(p) 2

#define clear_GAP2_SessionHandle(p)

typedef enum {
	five = 0,
	six = 1,
	seven = 2,
	eight = 3
} GAP2_CharLength;
#define sizeof_GAP2_CharLength sizeof_enumeration
#define clear_GAP2_CharLength clear_enumeration
#define externalize_GAP2_CharLength externalize_enumeration
#define internalize_GAP2_CharLength internalize_enumeration


typedef enum {
	none = 0,
	odd = 1,
	even = 2,
	one = 3,
	zero = 4
} GAP2_Parity;
#define sizeof_GAP2_Parity sizeof_enumeration
#define clear_GAP2_Parity clear_enumeration
#define externalize_GAP2_Parity externalize_enumeration
#define internalize_GAP2_Parity internalize_enumeration


typedef enum {
	oneStopBit = 0,
	twoStopBits = 1
} GAP2_StopBits;
#define sizeof_GAP2_StopBits sizeof_enumeration
#define clear_GAP2_StopBits clear_enumeration
#define externalize_GAP2_StopBits externalize_enumeration
#define internalize_GAP2_StopBits internalize_enumeration


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
	ttyHost = 11,
	tty = 12,
	other = 13,
	unknown = 14
} T_d3_1;
#define sizeof_T_d3_1 sizeof_enumeration
#define clear_T_d3_1 clear_enumeration
#define externalize_T_d3_1 externalize_enumeration
#define internalize_T_d3_1 internalize_enumeration


extern struct GAP2_SessionParameterObject;
typedef struct GAP2_SessionParameterObject GAP2_SessionParameterObject;

typedef struct {
	Unspecified pollProc;
} GAP2_T_c3_2;

#define sizeof_GAP2_T_c3_2(p) 1

#define clear_GAP2_T_c3_2(p)

typedef struct {
	Cardinal sendBlocksize;
	Cardinal receiveBlocksize;
} GAP2_T_c3_3;

#define sizeof_GAP2_T_c3_3(p) 2

#define clear_GAP2_T_c3_3(p)

typedef struct {
	GAP2_CharLength charLength;
	GAP2_Parity parity;
	GAP2_StopBits stopBits;
	Cardinal frameTimeout;
} GAP2_T_c3_4;

#define sizeof_GAP2_T_c3_4(p) 4

#define clear_GAP2_T_c3_4(p)

struct GAP2_SessionParameterObject {
	T_d3_1 designator;
	union {
		NilRecord u_xerox800;
#define xerox800_case u.u_xerox800
		GAP2_T_c3_2 u_xerox850;
#define xerox850_case u.u_xerox850
		GAP2_T_c3_2 u_xerox860;
#define xerox860_case u.u_xerox860
		GAP2_T_c3_3 u_system6;
#define system6_case u.u_system6
		GAP2_T_c3_3 u_cmcll;
#define cmcll_case u.u_cmcll
		GAP2_T_c3_3 u_imb2770;
#define imb2770_case u.u_imb2770
		GAP2_T_c3_3 u_ibm2770Host;
#define ibm2770Host_case u.u_ibm2770Host
		GAP2_T_c3_3 u_ibm6670;
#define ibm6670_case u.u_ibm6670
		GAP2_T_c3_3 u_ibm6670Host;
#define ibm6670Host_case u.u_ibm6670Host
		NilRecord u_ibm3270;
#define ibm3270_case u.u_ibm3270
		NilRecord u_ibm3270Host;
#define ibm3270Host_case u.u_ibm3270Host
		GAP2_T_c3_4 u_ttyHost;
#define ttyHost_case u.u_ttyHost
		GAP2_T_c3_4 u_tty;
#define tty_case u.u_tty
		NilRecord u_other;
#define other_case u.u_other
		NilRecord u_unknown;
#define unknown_case u.u_unknown
	} u;
};

typedef enum {
	bitSynchronous = 0,
	byteSynchronous = 1,
	asynchronous = 2,
	autoRecognition = 3
} GAP2_LineType;
#define sizeof_GAP2_LineType sizeof_enumeration
#define clear_GAP2_LineType clear_enumeration
#define externalize_GAP2_LineType externalize_enumeration
#define internalize_GAP2_LineType internalize_enumeration


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
	bps9600 = 12
} GAP2_LineSpeed;
#define sizeof_GAP2_LineSpeed sizeof_enumeration
#define clear_GAP2_LineSpeed clear_enumeration
#define externalize_GAP2_LineSpeed externalize_enumeration
#define internalize_GAP2_LineSpeed internalize_enumeration


typedef enum {
	fullduplex = 0,
	halfduplex = 1
} GAP2_Duplexity;
#define sizeof_GAP2_Duplexity sizeof_enumeration
#define clear_GAP2_Duplexity clear_enumeration
#define externalize_GAP2_Duplexity externalize_enumeration
#define internalize_GAP2_Duplexity internalize_enumeration


typedef enum {
	directConn = 0,
	dialConn = 1
} T_d3_5;
#define sizeof_T_d3_5 sizeof_enumeration
#define clear_T_d3_5 clear_enumeration
#define externalize_T_d3_5 externalize_enumeration
#define internalize_T_d3_5 internalize_enumeration


extern struct GAP2_T_r3_6;
typedef struct GAP2_T_r3_6 GAP2_T_r3_6;

typedef enum {
	manualDial = 0,
	autoDial = 1
} GAP2_T_r3_8;
#define sizeof_GAP2_T_r3_8 sizeof_enumeration
#define clear_GAP2_T_r3_8 clear_enumeration
#define externalize_GAP2_T_r3_8 externalize_enumeration
#define internalize_GAP2_T_r3_8 internalize_enumeration


typedef struct {
	GAP2_T_r3_8 dialMode;
	Cardinal dialerNumber;
	Cardinal retryCount;
} GAP2_T_c3_7;

#define sizeof_GAP2_T_c3_7(p) 3

#define clear_GAP2_T_c3_7(p)

struct GAP2_T_r3_6 {
	T_d3_5 designator;
	union {
		NilRecord u_directConn;
#define directConn_case u.u_directConn
		GAP2_T_c3_7 u_dialConn;
#define dialConn_case u.u_dialConn
	} u;
};

typedef struct {
	GAP2_Duplexity duplex;
	GAP2_LineType lineType;
	GAP2_LineSpeed lineSpeed;
	GAP2_T_r3_6 accessDetail;
} GAP2_CommParamObject;

typedef enum {
	preemptNever = 0,
	preemptAlways = 1,
	preemptInactive = 2
} GAP2_ReserveType;
#define sizeof_GAP2_ReserveType sizeof_enumeration
#define clear_GAP2_ReserveType clear_enumeration
#define externalize_GAP2_ReserveType externalize_enumeration
#define internalize_GAP2_ReserveType internalize_enumeration


typedef Unspecified GAP2_Resource[2];

#define sizeof_GAP2_Resource(p) 2

#define clear_GAP2_Resource(p)

typedef enum {
	primary = 0,
	secondary = 1
} GAP2_LineControl;
#define sizeof_GAP2_LineControl sizeof_enumeration
#define clear_GAP2_LineControl clear_enumeration
#define externalize_GAP2_LineControl externalize_enumeration
#define internalize_GAP2_LineControl internalize_enumeration

typedef Cardinal GAP2_ControllerAddress;
#define sizeof_GAP2_ControllerAddress sizeof_Cardinal
#define clear_GAP2_ControllerAddress clear_Cardinal
#define externalize_GAP2_ControllerAddress externalize_Cardinal
#define internalize_GAP2_ControllerAddress internalize_Cardinal

typedef Cardinal GAP2_TerminalAddress;
#define sizeof_GAP2_TerminalAddress sizeof_Cardinal
#define clear_GAP2_TerminalAddress clear_Cardinal
#define externalize_GAP2_TerminalAddress externalize_Cardinal
#define internalize_GAP2_TerminalAddress internalize_Cardinal


typedef enum {
	alreadyReserved = 0,
	reserveNeeded = 1
} T_d3_9;
#define sizeof_T_d3_9 sizeof_enumeration
#define clear_T_d3_9 clear_enumeration
#define externalize_T_d3_9 externalize_enumeration
#define internalize_T_d3_9 internalize_enumeration


typedef enum {
	rs232c = 0,
	bsc = 1,
	teletype = 2,
	polledBSCController = 3,
	polledSDLCController = 5,
	polledBSCTerminal = 4,
	polledSDLCTerminal = 6
} T_d3_10;
#define sizeof_T_d3_10 sizeof_enumeration
#define clear_T_d3_10 clear_enumeration
#define externalize_T_d3_10 externalize_enumeration
#define internalize_T_d3_10 internalize_enumeration


extern struct GAP2_TransportObject;
typedef struct GAP2_TransportObject GAP2_TransportObject;

extern struct GAP2_T_r3_12;
typedef struct GAP2_T_r3_12 GAP2_T_r3_12;

typedef struct {
	GAP2_Resource resource;
} GAP2_T_c3_13;

#define sizeof_GAP2_T_c3_13(p) 2

#define clear_GAP2_T_c3_13(p)

typedef struct {
	Cardinal lineNumber;
} GAP2_T_c3_14;

#define sizeof_GAP2_T_c3_14(p) 1

#define clear_GAP2_T_c3_14(p)

struct GAP2_T_r3_12 {
	T_d3_9 designator;
	union {
		GAP2_T_c3_13 u_alreadyReserved;
#define alreadyReserved_case u.u_alreadyReserved
		GAP2_T_c3_14 u_reserveNeeded;
#define reserveNeeded_case u.u_reserveNeeded
	} u;
};

typedef struct {
	GAP2_CommParamObject commParams;
	GAP2_ReserveType preemptOthers;
	GAP2_ReserveType preemptMe;
	String phoneNumber;
	GAP2_T_r3_12 line;
} GAP2_T_c3_11;

typedef struct {
	String localTerminalID;
	String localSecurityID;
	GAP2_LineControl lineControl;
	Unspecified authenticateProc;
} GAP2_T_c3_15;

typedef struct {
	String hostControllerName;
	Cardinal controllerAddress;
	Cardinal portsOnController;
} GAP2_T_c3_16;

typedef struct {
	String hostControllerName;
	Cardinal terminalAddress;
} GAP2_T_c3_17;

struct GAP2_TransportObject {
	T_d3_10 designator;
	union {
		GAP2_T_c3_11 u_rs232c;
#define rs232c_case u.u_rs232c
		GAP2_T_c3_15 u_bsc;
#define bsc_case u.u_bsc
		NilRecord u_teletype;
#define teletype_case u.u_teletype
		GAP2_T_c3_16 u_polledBSCController;
#define polledBSCController_case u.u_polledBSCController
		GAP2_T_c3_16 u_polledSDLCController;
#define polledSDLCController_case u.u_polledSDLCController
		GAP2_T_c3_17 u_polledBSCTerminal;
#define polledBSCTerminal_case u.u_polledBSCTerminal
		GAP2_T_c3_17 u_polledSDLCTerminal;
#define polledSDLCTerminal_case u.u_polledSDLCTerminal
	} u;
};

typedef enum {
	callOnAutoRecognition = 0,
	callOnActive = 1,
	dontCall = 2
} GAP2_CallBackType;
#define sizeof_GAP2_CallBackType sizeof_enumeration
#define clear_GAP2_CallBackType clear_enumeration
#define externalize_GAP2_CallBackType externalize_enumeration
#define internalize_GAP2_CallBackType internalize_enumeration


static Cardinal GAP2_infiniteTime = {0177777};

static Unspecified GAP2_NopPollProc = {00};

static Cardinal GAP2_unspecifiedTerminalAddr = {0177777};

#define GAP2_unimplemented (ERROR_OFFSET+0)
#define GAP2_unimplementedArgs T_cn3_18

#define GAP2_noCommunicationHardware (ERROR_OFFSET+1)
#define GAP2_noCommunicationHardwareArgs T_cn3_19

#define GAP2_illegalTransport (ERROR_OFFSET+2)
#define GAP2_illegalTransportArgs T_cn3_20

#define GAP2_mediumConnectFailed (ERROR_OFFSET+3)
#define GAP2_mediumConnectFailedArgs T_cn3_21

#define GAP2_badAddressFormat (ERROR_OFFSET+4)
#define GAP2_badAddressFormatArgs T_cn3_22

#define GAP2_noDialingHardware (ERROR_OFFSET+5)
#define GAP2_noDialingHardwareArgs T_cn3_23

#define GAP2_dialingHardwareProblem (ERROR_OFFSET+6)
#define GAP2_dialingHardwareProblemArgs T_cn3_24

#define GAP2_transmissionMediumUnavailable (ERROR_OFFSET+7)
#define GAP2_transmissionMediumUnavailableArgs T_cn3_25

#define GAP2_inconsistentParams (ERROR_OFFSET+8)
#define GAP2_inconsistentParamsArgs T_cn3_26

#define GAP2_tooManyGateStreams (ERROR_OFFSET+9)
#define GAP2_tooManyGateStreamsArgs T_cn3_27

#define GAP2_bugInGAPCode (ERROR_OFFSET+10)
#define GAP2_bugInGAPCodeArgs T_cn3_28

#define GAP2_gapNotExported (ERROR_OFFSET+11)
#define GAP2_gapNotExportedArgs T_cn3_29

#define GAP2_gapCommunicationError (ERROR_OFFSET+12)
#define GAP2_gapCommunicationErrorArgs T_cn3_30

#define GAP2_controllerAlreadyExists (ERROR_OFFSET+13)
#define GAP2_controllerAlreadyExistsArgs T_cn3_31

#define GAP2_controllerDoesNotExist (ERROR_OFFSET+14)
#define GAP2_controllerDoesNotExistArgs T_cn3_32

#define GAP2_terminalAddressInUse (ERROR_OFFSET+15)
#define GAP2_terminalAddressInUseArgs T_cn3_33

#define GAP2_terminalAddressInvalid (ERROR_OFFSET+16)
#define GAP2_terminalAddressInvalidArgs T_cn3_34

extern void GAP2_Reset();

extern void GAP2_IAmStillHere();

typedef struct {
	GAP2_SessionHandle session;
} GAP2_CreateResults;

#define sizeof_GAP2_CreateResults(p) 2

#define clear_GAP2_CreateResults(p)

typedef struct {
	Cardinal length;
	GAP2_TransportObject *sequence;
} GAP2_T_p3_38;

extern GAP2_CreateResults GAP2_Create();

extern void GAP2_Delete();

typedef struct {
	GAP2_Resource resource;
} GAP2_ReserveResults;

#define sizeof_GAP2_ReserveResults(p) 2

#define clear_GAP2_ReserveResults(p)

extern GAP2_ReserveResults GAP2_Reserve();

extern void GAP2_AbortReserve();

extern void GAP2_UseMediumForOISCP();

#endif __GAP

