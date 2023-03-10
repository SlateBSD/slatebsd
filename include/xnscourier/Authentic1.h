/*
 * Definitions for Authentication VERSION 1 NUMBER 14.
 */
#ifndef __Authentication1
#define __Authentication1
#include <xnscourier/courier.h>
#include <xnscourier/courierconnection.h>

typedef String Authentication1_ClearinghouseOrganization;
#define sizeof_Authentication1_ClearinghouseOrganization sizeof_String
#define clear_Authentication1_ClearinghouseOrganization clear_String
#define externalize_Authentication1_ClearinghouseOrganization externalize_String
#define internalize_Authentication1_ClearinghouseOrganization internalize_String

typedef String Authentication1_ClearinghouseDomain;
#define sizeof_Authentication1_ClearinghouseDomain sizeof_String
#define clear_Authentication1_ClearinghouseDomain clear_String
#define externalize_Authentication1_ClearinghouseDomain externalize_String
#define internalize_Authentication1_ClearinghouseDomain internalize_String

typedef String Authentication1_ClearinghouseObject;
#define sizeof_Authentication1_ClearinghouseObject sizeof_String
#define clear_Authentication1_ClearinghouseObject clear_String
#define externalize_Authentication1_ClearinghouseObject externalize_String
#define internalize_Authentication1_ClearinghouseObject internalize_String


typedef struct {
	String organization;
	String domain;
	String object;
} Authentication1_ClearinghouseThreePartName;
typedef Authentication1_ClearinghouseThreePartName Authentication1_ClearinghouseName;
#define sizeof_Authentication1_ClearinghouseName sizeof_Authentication1_ClearinghouseThreePartName
#define clear_Authentication1_ClearinghouseName clear_Authentication1_ClearinghouseThreePartName
#define externalize_Authentication1_ClearinghouseName externalize_Authentication1_ClearinghouseThreePartName
#define internalize_Authentication1_ClearinghouseName internalize_Authentication1_ClearinghouseThreePartName

typedef Cardinal Authentication1_CredentialsType;
#define sizeof_Authentication1_CredentialsType sizeof_Cardinal
#define clear_Authentication1_CredentialsType clear_Cardinal
#define externalize_Authentication1_CredentialsType externalize_Cardinal
#define internalize_Authentication1_CredentialsType internalize_Cardinal


typedef struct {
	Cardinal length;
	Unspecified *sequence;
} Authentication1_T_r14_1;

#define sizeof_Authentication1_T_r14_1(p) (1 + (p)->length * 1)

typedef struct {
	Cardinal type;
	Authentication1_T_r14_1 value;
} Authentication1_Credentials;

static Cardinal Authentication1_simpleCredentials = {0};
typedef Authentication1_ClearinghouseThreePartName Authentication1_SimpleCredentials;
#define sizeof_Authentication1_SimpleCredentials sizeof_Authentication1_ClearinghouseThreePartName
#define clear_Authentication1_SimpleCredentials clear_Authentication1_ClearinghouseThreePartName
#define externalize_Authentication1_SimpleCredentials externalize_Authentication1_ClearinghouseThreePartName
#define internalize_Authentication1_SimpleCredentials internalize_Authentication1_ClearinghouseThreePartName


typedef struct {
	Cardinal length;
	Unspecified *sequence;
} Authentication1_Verifier;

#define sizeof_Authentication1_Verifier(p) (1 + (p)->length * 1)
typedef Cardinal Authentication1_HashedPassword;
#define sizeof_Authentication1_HashedPassword sizeof_Cardinal
#define clear_Authentication1_HashedPassword clear_Cardinal
#define externalize_Authentication1_HashedPassword externalize_Cardinal
#define internalize_Authentication1_HashedPassword internalize_Cardinal

typedef Cardinal Authentication1_SimpleVerifier;
#define sizeof_Authentication1_SimpleVerifier sizeof_Cardinal
#define clear_Authentication1_SimpleVerifier clear_Cardinal
#define externalize_Authentication1_SimpleVerifier externalize_Cardinal
#define internalize_Authentication1_SimpleVerifier internalize_Cardinal


typedef enum {
	credentialsInvalid = 0,
	verifierInvalid = 1
} Authentication1_Problem;
#define sizeof_Authentication1_Problem sizeof_enumeration
#define clear_Authentication1_Problem clear_enumeration
#define externalize_Authentication1_Problem externalize_enumeration
#define internalize_Authentication1_Problem internalize_enumeration


typedef struct {
	Authentication1_Problem problem;
} T_cn14_2;

#define sizeof_T_cn14_2(p) 1

#define clear_T_cn14_2(p)

#define Authentication1_AuthenticationError (ERROR_OFFSET+2)
#define Authentication1_AuthenticationErrorArgs T_cn14_2

#endif __Authentication

