/*
 * Definitions for Authentication VERSION 2 NUMBER 14.
 */
#ifndef __Authentication2
#define __Authentication2
#include <xnscourier/courier.h>
#include <xnscourier/courierconnection.h>


/*
 * Definitions from DEPENDS UPON Time inclusion
 * (must be linked with Time2_support.c also)
 */
#include <xnscourier/Time2.h>
typedef String Authentication2_Organization;
#define sizeof_Authentication2_Organization sizeof_String
#define clear_Authentication2_Organization clear_String
#define externalize_Authentication2_Organization externalize_String
#define internalize_Authentication2_Organization internalize_String

typedef String Authentication2_Domain;
#define sizeof_Authentication2_Domain sizeof_String
#define clear_Authentication2_Domain clear_String
#define externalize_Authentication2_Domain externalize_String
#define internalize_Authentication2_Domain internalize_String

typedef String Authentication2_Object;
#define sizeof_Authentication2_Object sizeof_String
#define clear_Authentication2_Object clear_String
#define externalize_Authentication2_Object externalize_String
#define internalize_Authentication2_Object internalize_String


typedef struct {
	String organization;
	String domain;
	String object;
} Authentication2_ThreePartName;
typedef Authentication2_ThreePartName Authentication2_Clearinghouse_Name;
#define sizeof_Authentication2_Clearinghouse_Name sizeof_Authentication2_ThreePartName
#define clear_Authentication2_Clearinghouse_Name clear_Authentication2_ThreePartName
#define externalize_Authentication2_Clearinghouse_Name externalize_Authentication2_ThreePartName
#define internalize_Authentication2_Clearinghouse_Name internalize_Authentication2_ThreePartName


typedef enum {
	simple = 0,
	strong = 1
} Authentication2_CredentialsType;
#define sizeof_Authentication2_CredentialsType sizeof_enumeration
#define clear_Authentication2_CredentialsType clear_enumeration
#define externalize_Authentication2_CredentialsType externalize_enumeration
#define internalize_Authentication2_CredentialsType internalize_enumeration

typedef Authentication2_ThreePartName Authentication2_SimpleCredentials;
#define sizeof_Authentication2_SimpleCredentials sizeof_Authentication2_ThreePartName
#define clear_Authentication2_SimpleCredentials clear_Authentication2_ThreePartName
#define externalize_Authentication2_SimpleCredentials externalize_Authentication2_ThreePartName
#define internalize_Authentication2_SimpleCredentials internalize_Authentication2_ThreePartName


typedef struct {
	Authentication2_CredentialsType type;
	Authentication2_ThreePartName value;
} Authentication2_Credentials;
typedef Cardinal Authentication2_HashedPassword;
#define sizeof_Authentication2_HashedPassword sizeof_Cardinal
#define clear_Authentication2_HashedPassword clear_Cardinal
#define externalize_Authentication2_HashedPassword externalize_Cardinal
#define internalize_Authentication2_HashedPassword internalize_Cardinal

typedef Cardinal Authentication2_SimpleVerifier;
#define sizeof_Authentication2_SimpleVerifier sizeof_Cardinal
#define clear_Authentication2_SimpleVerifier clear_Cardinal
#define externalize_Authentication2_SimpleVerifier externalize_Cardinal
#define internalize_Authentication2_SimpleVerifier internalize_Cardinal


typedef struct {
	Cardinal length;
	Unspecified *sequence;
} Authentication2_Verifier;

#define sizeof_Authentication2_Verifier(p) (1 + (p)->length * 1)

typedef enum {
	credentialsInvalid = 0,
	verifierInvalid = 1,
	verifierExpiered = 2,
	verifierReused = 3,
	credentialsExpired = 4,
	inappropriateCredeitnals = 5
} Authentication2_Problem;
#define sizeof_Authentication2_Problem sizeof_enumeration
#define clear_Authentication2_Problem clear_enumeration
#define externalize_Authentication2_Problem externalize_enumeration
#define internalize_Authentication2_Problem internalize_enumeration


typedef struct {
	Authentication2_Problem problem;
} T_cn14_1;

#define sizeof_T_cn14_1(p) 1

#define clear_T_cn14_1(p)

#define Authentication2_AuthenticationError (ERROR_OFFSET+2)
#define Authentication2_AuthenticationErrorArgs T_cn14_1

typedef enum {
	tooBusy = 0,
	accessRightsInsufficient = 1,
	keysUnavailable = 2,
	strongKeyDoesNotExist = 3,
	simpleKeyDoesNotExist = 4,
	strongKeyAlreadyRegistered = 5,
	simpleKeyAlreadyRegistered = 6,
	domainForNewKeyUnavailable = 7,
	domainForNewKeyUnknown = 8,
	badKey = 9,
	badName = 10,
	databaseFull = 11,
	other = 12
} Authentication2_CallProblem;
#define sizeof_Authentication2_CallProblem sizeof_enumeration
#define clear_Authentication2_CallProblem clear_enumeration
#define externalize_Authentication2_CallProblem externalize_enumeration
#define internalize_Authentication2_CallProblem internalize_enumeration


typedef enum {
	notApplicable = 0,
	initiator = 1,
	recipient = 2,
	client = 3
} Authentication2_Which;
#define sizeof_Authentication2_Which sizeof_enumeration
#define clear_Authentication2_Which clear_enumeration
#define externalize_Authentication2_Which externalize_enumeration
#define internalize_Authentication2_Which internalize_enumeration


typedef struct {
	Authentication2_CallProblem problem;
	Authentication2_Which whichArg;
} T_cn14_2;

#define sizeof_T_cn14_2(p) 2

#define clear_T_cn14_2(p)

#define Authentication2_CallError (ERROR_OFFSET+1)
#define Authentication2_CallErrorArgs T_cn14_2

typedef struct {
	Boolean ok;
} Authentication2_CheckSimpleCredentialsResults;

#define sizeof_Authentication2_CheckSimpleCredentialsResults(p) 1

#define clear_Authentication2_CheckSimpleCredentialsResults(p)

extern Authentication2_CheckSimpleCredentialsResults Authentication2_CheckSimpleCredentials();

#endif __Authentication

