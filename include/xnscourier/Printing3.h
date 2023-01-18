/*
 * Definitions for Printing VERSION 3 NUMBER 4.
 */
#ifndef __Printing3
#define __Printing3
#include <xnscourier/courier.h>
#include <xnscourier/courierconnection.h>


/*
 * Definitions from DEPENDS UPON BulkData inclusion
 * (must be linked with BulkData1_support.c also)
 */
#include <xnscourier/BulkData1.h>

/*
 * Definitions from DEPENDS UPON Authentication inclusion
 * (must be linked with Authentication1_support.c also)
 */
#include <xnscourier/Authentication1.h>

/*
 * Definitions from DEPENDS UPON Time inclusion
 * (must be linked with Time2_support.c also)
 */
#include <xnscourier/Time2.h>
typedef LongCardinal Printing3_Time;
#define sizeof_Printing3_Time sizeof_LongCardinal
#define clear_Printing3_Time clear_LongCardinal
#define externalize_Printing3_Time externalize_LongCardinal
#define internalize_Printing3_Time internalize_LongCardinal


typedef Unspecified Printing3_RequestID[5];

#define sizeof_Printing3_RequestID(p) 5

#define clear_Printing3_RequestID(p)

typedef enum {
	printObjectName = 0,
	printObjectCreateDate = 1,
	senderName = 2
} T_d4_8;
#define sizeof_T_d4_8 sizeof_enumeration
#define clear_T_d4_8 clear_enumeration
#define externalize_T_d4_8 externalize_enumeration
#define internalize_T_d4_8 internalize_enumeration


extern struct Printing3_T_s4_9;
typedef struct Printing3_T_s4_9 Printing3_T_s4_9;

struct Printing3_T_s4_9 {
	T_d4_8 designator;
	union {
		String u_printObjectName;
#define printObjectName_case u.u_printObjectName
		LongCardinal u_printObjectCreateDate;
#define printObjectCreateDate_case u.u_printObjectCreateDate
		String u_senderName;
#define senderName_case u.u_senderName
	} u;
};

typedef struct {
	Cardinal length;
	Printing3_T_s4_9 *sequence;
} Printing3_PrintAttributes;

typedef enum {
	unknown = 0,
	knownSize = 1,
	otherSize = 2
} T_d4_10;
#define sizeof_T_d4_10 sizeof_enumeration
#define clear_T_d4_10 clear_enumeration
#define externalize_T_d4_10 externalize_enumeration
#define internalize_T_d4_10 internalize_enumeration


extern struct Printing3_Paper;
typedef struct Printing3_Paper Printing3_Paper;

typedef enum {
	usLetter = 1,
	usLegal = 2,
	a0 = 3,
	a1 = 4,
	a2 = 5,
	a3 = 6,
	a4 = 7,
	a5 = 8,
	a6 = 9,
	a7 = 10,
	a8 = 11,
	a9 = 12,
	a10 = 35,
	isoB0 = 13,
	isoB1 = 14,
	isoB2 = 15,
	isoB3 = 16,
	isoB4 = 17,
	isoB5 = 18,
	isoB6 = 19,
	isoB7 = 20,
	isoB8 = 21,
	isoB9 = 22,
	isoB10 = 23,
	jisB0 = 24,
	jisB1 = 25,
	jisB2 = 26,
	jisB3 = 27,
	jisB4 = 28,
	jisB5 = 29,
	jisB6 = 30,
	jisB7 = 31,
	jisB8 = 32,
	jisB9 = 33,
	jisB10 = 34
} Printing3_T_c4_11;
#define sizeof_Printing3_T_c4_11 sizeof_enumeration
#define clear_Printing3_T_c4_11 clear_enumeration
#define externalize_Printing3_T_c4_11 externalize_enumeration
#define internalize_Printing3_T_c4_11 internalize_enumeration


typedef struct {
	Cardinal width;
	Cardinal length;
} Printing3_T_c4_12;

#define sizeof_Printing3_T_c4_12(p) 2

#define clear_Printing3_T_c4_12(p)

struct Printing3_Paper {
	T_d4_10 designator;
	union {
		NilRecord u_unknown;
#define unknown_case u.u_unknown
		Printing3_T_c4_11 u_knownSize;
#define knownSize_case u.u_knownSize
		Printing3_T_c4_12 u_otherSize;
#define otherSize_case u.u_otherSize
	} u;
};

typedef enum {
	paper = 0
} T_d4_13;
#define sizeof_T_d4_13 sizeof_enumeration
#define clear_T_d4_13 clear_enumeration
#define externalize_T_d4_13 externalize_enumeration
#define internalize_T_d4_13 internalize_enumeration


extern struct Printing3_Medium;
typedef struct Printing3_Medium Printing3_Medium;

struct Printing3_Medium {
	T_d4_13 designator;
	union {
		Printing3_Paper u_paper;
#define paper_case u.u_paper
	} u;
};

typedef struct {
	Cardinal length;
	Printing3_Medium *sequence;
} Printing3_Media;

typedef enum {
	printObjectSize = 0,
	recipientName = 1,
	message = 2,
	copyCount = 3,
	pagesToPrint = 4,
	mediumHint = 5,
	priorityHint = 6,
	releaseKey = 7,
	staple = 8,
	twoSided = 9
} T_d4_14;
#define sizeof_T_d4_14 sizeof_enumeration
#define clear_T_d4_14 clear_enumeration
#define externalize_T_d4_14 externalize_enumeration
#define internalize_T_d4_14 internalize_enumeration


extern struct Printing3_T_s4_15;
typedef struct Printing3_T_s4_15 Printing3_T_s4_15;

typedef struct {
	Cardinal beginningPageNumber;
	Cardinal endingPageNumber;
} Printing3_T_c4_16;

#define sizeof_Printing3_T_c4_16(p) 2

#define clear_Printing3_T_c4_16(p)

typedef enum {
	low = 0,
	normal = 1,
	high = 2
} Printing3_T_c4_17;
#define sizeof_Printing3_T_c4_17 sizeof_enumeration
#define clear_Printing3_T_c4_17 clear_enumeration
#define externalize_Printing3_T_c4_17 externalize_enumeration
#define internalize_Printing3_T_c4_17 internalize_enumeration


struct Printing3_T_s4_15 {
	T_d4_14 designator;
	union {
		LongCardinal u_printObjectSize;
#define printObjectSize_case u.u_printObjectSize
		String u_recipientName;
#define recipientName_case u.u_recipientName
		String u_message;
#define message_case u.u_message
		Cardinal u_copyCount;
#define copyCount_case u.u_copyCount
		Printing3_T_c4_16 u_pagesToPrint;
#define pagesToPrint_case u.u_pagesToPrint
		Printing3_Medium u_mediumHint;
#define mediumHint_case u.u_mediumHint
		Printing3_T_c4_17 u_priorityHint;
#define priorityHint_case u.u_priorityHint
		Cardinal u_releaseKey;
#define releaseKey_case u.u_releaseKey
		Boolean u_staple;
#define staple_case u.u_staple
		Boolean u_twoSided;
#define twoSided_case u.u_twoSided
	} u;
};

typedef struct {
	Cardinal length;
	Printing3_T_s4_15 *sequence;
} Printing3_PrintOptions;

typedef enum {
	ppmedia = 0,
	ppstaple = 1,
	pptwoSided = 2
} T_d4_18;
#define sizeof_T_d4_18 sizeof_enumeration
#define clear_T_d4_18 clear_enumeration
#define externalize_T_d4_18 externalize_enumeration
#define internalize_T_d4_18 internalize_enumeration


extern struct Printing3_T_s4_19;
typedef struct Printing3_T_s4_19 Printing3_T_s4_19;

struct Printing3_T_s4_19 {
	T_d4_18 designator;
	union {
		Printing3_Media u_ppmedia;
#define ppmedia_case u.u_ppmedia
		Boolean u_ppstaple;
#define ppstaple_case u.u_ppstaple
		Boolean u_pptwoSided;
#define pptwoSided_case u.u_pptwoSided
	} u;
};

typedef struct {
	Cardinal length;
	Printing3_T_s4_19 *sequence;
} Printing3_PrinterProperties;

typedef enum {
	spooler = 0,
	formatter = 1,
	printer = 2,
	media = 3
} T_d4_26;
#define sizeof_T_d4_26 sizeof_enumeration
#define clear_T_d4_26 clear_enumeration
#define externalize_T_d4_26 externalize_enumeration
#define internalize_T_d4_26 internalize_enumeration


extern struct Printing3_T_s4_27;
typedef struct Printing3_T_s4_27 Printing3_T_s4_27;

typedef enum {
	available = 0,
	busy = 1,
	disabled = 2,
	full = 3
} Printing3_T_c4_28;
#define sizeof_Printing3_T_c4_28 sizeof_enumeration
#define clear_Printing3_T_c4_28 clear_enumeration
#define externalize_Printing3_T_c4_28 externalize_enumeration
#define internalize_Printing3_T_c4_28 internalize_enumeration


typedef enum {
	available4_20 = 0,
	busy4_21 = 1,
	disabled4_22 = 2
} Printing3_T_c4_29;
#define sizeof_Printing3_T_c4_29 sizeof_enumeration
#define clear_Printing3_T_c4_29 clear_enumeration
#define externalize_Printing3_T_c4_29 externalize_enumeration
#define internalize_Printing3_T_c4_29 internalize_enumeration


typedef enum {
	available4_23 = 0,
	busy4_24 = 1,
	disabled4_25 = 2,
	needsAttention = 3,
	needsKeyOperator = 4
} Printing3_T_c4_30;
#define sizeof_Printing3_T_c4_30 sizeof_enumeration
#define clear_Printing3_T_c4_30 clear_enumeration
#define externalize_Printing3_T_c4_30 externalize_enumeration
#define internalize_Printing3_T_c4_30 internalize_enumeration


struct Printing3_T_s4_27 {
	T_d4_26 designator;
	union {
		Printing3_T_c4_28 u_spooler;
#define spooler_case u.u_spooler
		Printing3_T_c4_29 u_formatter;
#define formatter_case u.u_formatter
		Printing3_T_c4_30 u_printer;
#define printer_case u.u_printer
		Printing3_Media u_media;
#define media_case u.u_media
	} u;
};

typedef struct {
	Cardinal length;
	Printing3_T_s4_27 *sequence;
} Printing3_PrinterStatus;

typedef enum {
	status = 0,
	statusMessage = 1
} T_d4_32;
#define sizeof_T_d4_32 sizeof_enumeration
#define clear_T_d4_32 clear_enumeration
#define externalize_T_d4_32 externalize_enumeration
#define internalize_T_d4_32 internalize_enumeration


extern struct Printing3_T_s4_33;
typedef struct Printing3_T_s4_33 Printing3_T_s4_33;

typedef enum {
	pending = 0,
	inProgress = 1,
	completed = 2,
	completedWithWarning = 3,
	unknown4_31 = 4,
	rejected = 5,
	aborted = 6,
	canceled = 7,
	held = 8
} Printing3_T_c4_34;
#define sizeof_Printing3_T_c4_34 sizeof_enumeration
#define clear_Printing3_T_c4_34 clear_enumeration
#define externalize_Printing3_T_c4_34 externalize_enumeration
#define internalize_Printing3_T_c4_34 internalize_enumeration


struct Printing3_T_s4_33 {
	T_d4_32 designator;
	union {
		Printing3_T_c4_34 u_status;
#define status_case u.u_status
		String u_statusMessage;
#define statusMessage_case u.u_statusMessage
	} u;
};

typedef struct {
	Cardinal length;
	Printing3_T_s4_33 *sequence;
} Printing3_RequestStatus;
typedef Cardinal Printing3_UndefinedProblem;
#define sizeof_Printing3_UndefinedProblem sizeof_Cardinal
#define clear_Printing3_UndefinedProblem clear_Cardinal
#define externalize_Printing3_UndefinedProblem externalize_Cardinal
#define internalize_Printing3_UndefinedProblem internalize_Cardinal


typedef enum {
	noRoute = 0,
	noResponse = 1,
	transmissionHardware = 2,
	transportTimeout = 3,
	tooManyLocalConnections = 4,
	tooManyRemoteConnections = 5,
	missingCourier = 6,
	missingProgram = 7,
	missingProcedure = 8,
	protocolMismatch = 9,
	parameterInconsistency = 10,
	invalidMessage = 11,
	returnTimedOut = 12,
	otherCallProblem = 0177777
} Printing3_ConnectionProblem;
#define sizeof_Printing3_ConnectionProblem sizeof_enumeration
#define clear_Printing3_ConnectionProblem clear_enumeration
#define externalize_Printing3_ConnectionProblem externalize_enumeration
#define internalize_Printing3_ConnectionProblem internalize_enumeration


typedef enum {
	aborted4_35 = 0,
	formatIncorrect = 2,
	noRendezvous = 3,
	wrongDirection = 4
} Printing3_TransferProblem;
#define sizeof_Printing3_TransferProblem sizeof_enumeration
#define clear_Printing3_TransferProblem clear_enumeration
#define externalize_Printing3_TransferProblem externalize_enumeration
#define internalize_Printing3_TransferProblem internalize_enumeration


#define Printing3_Busy (ERROR_OFFSET+0)
#define Printing3_BusyArgs T_cn4_36

#define Printing3_InsufficientSpoolSpace (ERROR_OFFSET+1)
#define Printing3_InsufficientSpoolSpaceArgs T_cn4_37

#define Printing3_InvalidPrintParameters (ERROR_OFFSET+2)
#define Printing3_InvalidPrintParametersArgs T_cn4_38

#define Printing3_MasterTooLarge (ERROR_OFFSET+3)
#define Printing3_MasterTooLargeArgs T_cn4_39

#define Printing3_ServiceUnavailable (ERROR_OFFSET+4)
#define Printing3_ServiceUnavailableArgs T_cn4_40

#define Printing3_MediumUnavailable (ERROR_OFFSET+5)
#define Printing3_MediumUnavailableArgs T_cn4_41

#define Printing3_SpoolingDisabled (ERROR_OFFSET+6)
#define Printing3_SpoolingDisabledArgs T_cn4_42

#define Printing3_SpoolingQueueFull (ERROR_OFFSET+7)
#define Printing3_SpoolingQueueFullArgs T_cn4_43

#define Printing3_SystemError (ERROR_OFFSET+8)
#define Printing3_SystemErrorArgs T_cn4_44

#define Printing3_TooManyClients (ERROR_OFFSET+9)
#define Printing3_TooManyClientsArgs T_cn4_45

typedef struct {
	Cardinal problem;
} T_cn4_46;

#define sizeof_T_cn4_46(p) 1

#define clear_T_cn4_46(p)

#define Printing3_Undefined (ERROR_OFFSET+10)
#define Printing3_UndefinedArgs T_cn4_46

typedef struct {
	Printing3_ConnectionProblem problem;
} T_cn4_47;

#define sizeof_T_cn4_47(p) 1

#define clear_T_cn4_47(p)

#define Printing3_ConnectionError (ERROR_OFFSET+11)
#define Printing3_ConnectionErrorArgs T_cn4_47

typedef struct {
	Printing3_TransferProblem problem;
} T_cn4_48;

#define sizeof_T_cn4_48(p) 1

#define clear_T_cn4_48(p)

#define Printing3_TransferError (ERROR_OFFSET+12)
#define Printing3_TransferErrorArgs T_cn4_48

typedef struct {
	Printing3_RequestID printRequestID;
} Printing3_PrintResults;

#define sizeof_Printing3_PrintResults(p) 5

#define clear_Printing3_PrintResults(p)

extern Printing3_PrintResults Printing3_Print();

typedef struct {
	Printing3_PrinterProperties properties;
} Printing3_GetPrinterPropertiesResults;

extern Printing3_GetPrinterPropertiesResults Printing3_GetPrinterProperties();

typedef struct {
	Printing3_RequestStatus status;
} Printing3_GetPrintRequestStatusResults;

extern Printing3_GetPrintRequestStatusResults Printing3_GetPrintRequestStatus();

typedef struct {
	Printing3_PrinterStatus status;
} Printing3_GetPrinterStatusResults;

extern Printing3_GetPrinterStatusResults Printing3_GetPrinterStatus();

#endif __Printing

