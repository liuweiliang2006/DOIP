/*
 * doip_pdu.h
 *
 *  Created on: 2022年4月7日
 *      Author: Administrator
 */

#ifndef DOIPC_DOIP_PDU_H_
#define DOIPC_DOIP_PDU_H_
#include <stdint.h>

struct doip_hdr_s;
typedef struct doip_hdr_s doip_hdr_t;
typedef doip_hdr_t doip_request_t;
typedef doip_hdr_t doip_response_t;
typedef doip_hdr_t doip_message_t;

struct generic_nack;
typedef struct generic_nack generic_nack_t;

struct vehicle_identification_request;
struct vehicle_identification_request_eid;
struct vehicle_identification_request_vin;
struct vehicle_identification_response;
typedef struct vehicle_identification_request vehicle_identification_request_t;
typedef struct vehicle_identification_request_eid vehicle_identification_request_eid_t;
typedef struct vehicle_identification_request_vin vehicle_identification_request_vin_t;
typedef struct vehicle_identification_response vehicle_identification_response_t;
typedef struct vehicle_identification_response vehicle_announcement_t;

struct routing_activation_request;
struct routing_activation_response;
typedef struct routing_activation_request routing_activation_request_t;
typedef struct routing_activation_response routing_activation_response_t;

struct diagnostic_message;
struct diagnostic_nack;
struct diagnostic_ack;
typedef struct diagnostic_message diagnostic_message_t;
typedef struct diagnostic_nack diagnostic_nack_t;
typedef struct diagnostic_ack diagnostic_ack_t;

struct alive_check_request;
struct alive_check_response;
typedef struct alive_check_request alive_check_request_t;
typedef struct alive_check_response alive_check_response_t;

struct power_mode_request;
struct power_mode_response;
typedef struct power_mode_request power_mode_request_t;
typedef struct power_mode_response power_mode_response_t;

struct entity_status_request;
struct entity_status_response;
typedef struct entity_status_request entity_status_request_t;
typedef struct entity_status_response entity_status_response_t;

#if defined(WIN32)
#pragma pack(push)
#pragma pack(1)
#else
#pragma pack(1)
#endif

/*WARNING: Don't change the order, it maybe due to fatal error !!!! */
enum {
	IDX_GENERIC_NACK,
	IDX_VI_REQ,
	IDX_VI_REQ_EID,
	IDX_VI_REQ_VIN,
	IDX_VI_RSP,
	IDX_RA_REQ,
	IDX_RA_RSP,
	IDX_DIAG_MSG,
	IDX_DIAG_NACK,
	IDX_DIAG_ACK,
	IDX_AC_REQ,
	IDX_AC_RSP,
	IDX_PM_REQ,
	IDX_PM_RSP,
	IDX_ES_REQ,
	IDX_ES_RSP,
	IDX_MAX_NUM
};
extern doip_message_t *DOIP_MSGS[IDX_MAX_NUM];

/*
0x00: reserved
0x01: DoIP ISO/DIS 13400-2:2010
0x02: DoIP ISO 13400-2:2012
0x03��0xFE: reserved by this part of ISO 13400
0xFF: default value for vehicle identification request messages
*/
#define DOIP_PROTOCOL_VERSION_RESERVED  0x00u
#define DOIP_PROTOCOL_VERSION_2010      0x01u
#define DOIP_PROTOCOL_VERSION_2012      0x02u
#define DOIP_PROTOCOL_VERSION_DEFAULT   0xFFu
#define DOIP_PROTOCOL_VERSION           DOIP_PROTOCOL_VERSION_2012

/*
0x0000 	7.1.2 mandatory mandatory
0x0001  7.1.4 mandatory mandatory
0x0002	7.1.4 optional optional
0x0003	7.1.4 mandatory mandatory
0x0004	7.1.4 mandatory mandatory
0x0005 	7.1.5 mandatory mandatory
0x0006 	7.1.5 mandatory mandatory
0x0007  7.1.7 mandatory mandatory
0x0008  7.1.7 mandatory mandatory

0x0009 - 0x4000	Reserved by this part of ISO 13400

0x4001  7.1.9 optional optional
0x4002  7.1.9 optional optional
0x4003	7.1.8 mandatory mandatory
0x4004	7.1.8 mandatory mandatory

0x4005 - 0x8000	Reserved by this part of ISO 13400

0x8001  7.1.6 mandatory mandatory
0x8002	7.1.6 mandatory mandatory
0x8003	7.1.6 mandatory mandatory

0x8004 - 0xEFFF	Reserved by this part of ISO 13400

0xF000 - 0xFFFF Reserved for manufacturer-specific use
*/
enum doip_msg_type {
	/*Generic DoIP header negative acknowledge */
	DOIP_GENERIC_NACK = 0x0000,	/*UDP_DISCOVERY, UDP_TEST_EQUIPMENT_REQUEST, TCP_DATA */

	/*Vehicle identification request message */
	DOIP_VI_REQUEST = 0x0001,	/*UDP_DISCOVERY */
	/*Vehicle identification request message with EID */
	DOIP_VI_REQUEST_BY_EID = 0x0002,	/*UDP_DISCOVERY */
	/*Vehicle identification request message with VIN */
	DOIP_VI_REQUEST_BY_VIN = 0x0003,	/*UDP_DISCOVERY */
	/*Vehicle announcement message/vehicle identification response message */
	DOIP_VI_RESPONSE = 0x0004,	/*UDP_DISCOVERY UDP_TEST_EQUIPMENT_REQUEST */

	/*Routing activation request */
	DOIP_ROUTING_ACTIVATION_REQUEST = 0x0005,	/*TCP_DATA */
	/*Routing activation response */
	DOIP_ROUTING_ACTIVATION_RESPONSE = 0x0006,	/*TCP_DATA */

	/*Alive check request */
	DOIP_ALIVE_CHECK_REQUEST = 0x0007,	/*TCP_DATA */
	/*Alive check response */
	DOIP_ALIVE_CHECK_RESPONSE = 0x0008,	/*TCP_DATA */

	/*DoIP entity status request */
	DOIP_ENTITY_STATUS_REQUEST = 0x4001,	/*UDP_DISCOVERY */
	/*DoIP entity status response */
	DOIP_ENTITY_STATUS_RESPONSE = 0x4002,	/*UDP_TEST_EQUIPMENT_REQUEST */

	/*Diagnostic power mode information request */
	DOIP_POWER_MODE_REQUEST = 0x4003,	/*UDP_DISCOVERY */
	/*Diagnostic power mode information response */
	DOIP_POWER_MODE_RESPONSE = 0x4004,	/*UDP_TEST_EQUIPMENT_REQUEST */

	/*Diagnostic message */
	DOIP_DIAGNOSTIC = 0x8001,		/*TCP_DATA */
	/*Diagnostic message positive acknowledgement */
	DOIP_DIAGNOSTIC_ACK = 0x8002,	/*TCP_DATA */
	/*Diagnostic message negative acknowledgement */
	DOIP_DIAGNOSTIC_NACK = 0x8003,	/*TCP_DATA */
};
typedef enum doip_msg_type doip_msg_type_t;

struct doip_hdr_s {
	uint8_t ProtocolVersion;
	uint8_t InverseProtocolVersion;
	uint16_t PayloadType;
	uint32_t PayloadLength;
};
void doip_msg_hdr_init(doip_hdr_t *hdr);

/*
0x00 Incorrect pattern format, Close socket mandatory
0x01 Unknown payload type, Discard DoIP message mandatory
0x02 Message too large, Discard DoIP message mandatory
0x03 Out of memory, Discard DoIP message mandatory
0x04 Invalid payload length, Close socket mandatory
*/
#define DOIP_GEN_NACK_BAD_FORMAT				0x00u
#define DOIP_GEN_NACK_BAD_PAYLOAD_TYPE			0x01u
#define DOIP_GEN_NACK_TOO_LARGE					0x02u
#define DOIP_GEN_NACK_OUT_OF_MEMORY				0x03u
#define DOIP_GEN_NACK_INVALID_PAYLOAD_LENGTH	0x04u
#define DOIP_GEN_NACK_NONE						0xFFu

struct generic_nack {
	doip_hdr_t hdr;
	uint8_t code;
};


/*
Vehicle discovery, Vehicle identification request, External test equipment, UDP_TEST_EQUIPMENT_REQUEST, DoIP entity, UDP_DISCOVERY, UDP Multi- or unicast
Vehicle discovery, Vehicle identification response, DoIP entity, UDP_DISCOVERY or dynamically assigned, External test equipment,UDP_TEST_EQUIPMENT_REQUEST,UDP Unicast
Vehicle discovery, Vehicle announcement, DoIP entity, UDP_DISCOVERY or dynamically assigned, External test equipment, UDP_DISCOVERY, UDP Multicast
Data transmission e.g. Routing activation request, External test equipment, Dynamically assigned, DoIP entity, TCP_DATA TCP, Unicast
Data transmission e.g. Routing activation response, DoIP entity, TCP_DATA External test equipment, Dynamically assigned, TCP Unicast
*/
struct vehicle_identification_request {
	doip_hdr_t hdr;
};

struct vehicle_identification_request_eid {
	doip_hdr_t hdr;
	uint8_t EID[6];
};

struct vehicle_identification_request_vin {
	doip_hdr_t hdr;
	uint8_t VIN[17];
};

/*
Each DoIP entity shall send the vehicle announcement message as specified in Table 19 A_
DoIP_Announce_Num times with A_DoIP_Announce_Interval seconds inter-message time
between each transmission starting immediately after configuration of a valid IP address
*/

/*
[DoIP-141] Each DoIP entity shall be uniquely identifiable by either the VIN, the EID or both at any time.
[DoIP-142] If it cannot be guaranteed that a vehicle can be identified by the VIN at any time, support for EID and GID shall be provided.
*/

/*
0x00 No further action required mandatory
0x01 to 0x0F Reserved by this part of ISO 13400 mandatory
0x10 Routing activation required to initiate central security. optional
0x11 to 0xFF Available for additional OEM-specific use. optional
*/

/*
DOIP_VIR_ROUTING_ACTIVATION_REQUIRED
external test equipment may send a routing activation request message(see Table 22) with the
activation type set to 0xE0 (see Table 23) to that DoIP entity and determine the specific action
from the OEM - specific field in the routing activation response message(see Table 24).
*/

#define DOIP_VI_NO_FURTHER_ACTION_REQUIRED		0x00
#define DOIP_VI_ROUTING_ACTIVATION_REQUIRED	0x10

/*
0x00 VIN and/or GID are synchronized mandatory
0x01��0x0F Reserved by this part of ISO 13400 mandatory
0x10 Incomplete: VIN and GID are NOT synchronized. mandatory
0x11��0xFF Reserved by this part of ISO 13400 mandatory
*/
#define DOIP_VI_SYNCHRONIZED		0x00
#define DOIP_VI_NOT_SYNCHRONIZED	0x10

struct vehicle_identification_response {
	doip_hdr_t hdr;
	uint8_t VIN[17];
	uint8_t LogicalAddress[2];
	uint8_t EID[6];
	uint8_t GID[6];
	uint8_t FurtherActionRequired;
	uint8_t SyncStatus; /*VIN / GID sync.status */
};

/*
This subclause specifies the DoIP messages that are necessary to activate routing on a TCP_DATA socket.
For deactivation of routing on a TCP_DATA socket, no additional payload types are defined as this can be
achieved simply by closing the TCP_DATA socket. Figure 25 in Clause 11 shows an example sequence of
external test equipment trying to activate routing on a newly established TCP_DATA socket.
*/

/*
0x00 Default none mandatory
0x01 WWH-OBD none mandatory
0x02 to 0xDF ISO/SAE reserved
0xE0 Central security OEM-specific optional
0xE1 to 0xFF Available for additional OEM-specific use OEM-specific optional
*/
#define DOIP_RA_DEFAULT			0x00
#define DOIP_RA_WWH_OBD			0x01
#define DOIP_RA_OEM_SPECIFIC	0xE0

struct routing_activation_request {
	doip_hdr_t hdr;
	uint8_t SourceAddress[2];
	uint8_t ActivationType;
	uint8_t	iso_rsv[4];
	uint8_t oem_specific[4];
};

/*
0x00 Routing activation denied due to unknown source address.
	 Do not activate routing and close this TCP_DATA socket. mandatory
0x01 Routing activation denied because all concurrently supported TCP_DATA sockets are registered and active.
	 Do not activate routing and close this TCP_DATA socket. mandatory
0x02 Routing activation denied because an SA different from the table connection entry was received on the already activated TCP_DATA socket.
	 Do not activate routing and close this TCP_DATA socket. mandatory
0x03 Routing activation denied because the SA is already registered and active on a different TCP_DATA socket.
	 Do not activate routing and close this TCP_DATA socket. mandatory
0x04 Routing activation denied due to missing authentication.
	 Do not activate routing and register. optional
0x05 Routing activation denied due to rejected confirmation.
	 Do not activate routing and close this TCP_DATA socket. optional
0x06 Routing activation denied due to unsupported routing activation type.
	 Do not activate routing and close this TCP_DATA socket. mandatory
0x07 �C 0x0F Reserved by this part of ISO 13400. �� ��
0x10 Routing successfully activated.
	 Activate routing and register SA on this TCP_DATA socket. mandatory
0x11 Routing will be activated; confirmation required.
	 Only activate routing after confirmation from within the vehicle. optional
0x12 �C 0xDF Reserved by this part of ISO 13400. �� ��
0xE0 �C 0xFE Vehicle-manufacturer specific. �� optional
0xFF Reserved by this part of ISO 13400. �� ��
*/
#define DOIP_RA_UNKNOWN_SA	0x00
#define DOIP_RA_REGISTERED_AND_ACTIVE	0x01
#define DOIP_RA_DIFFERENT_SA	0x02
#define DOIP_RA_DIFFERENT_SOCKET	0x03
#define DOIP_RA_MISSING_AUTHENTICATION	0x04
#define DOIP_RA_REJECTED_CONFIRMATION	0x05
#define DOIP_RA_UNSUPPORTED_ACTIVATION_TYPE	0x06
#define DOIP_RA_SUCCESSFULLY	0x10
#define DOIP_RA_WILL_BE_ACTIVATED	0x11
#define DOIP_RA_MANUFACTURER_SPECIFIC	0xE0	//to 0xFE


struct routing_activation_response {
	doip_hdr_t hdr;
	uint8_t equipment_addr[2];	/*logical address of external test equipment */
	uint8_t enity_addr[2];	/*logical address of DoIP entity */
	uint8_t code;	/*Routing activation response code */
	uint8_t rsv[4];
	uint8_t oem_specific[4];
};


/*
ISO 27145-3
*/
struct diagnostic_message {
	doip_hdr_t hdr;
	uint8_t sa[2]; /*source address */
	uint8_t ta[2]; /*target address */
	uint8_t userdata[0];
};

/*
00 ISO 13400 �C protocol version			0x01	��
01 ISO 13400 �C inverse protocol version	0xFE	��
02 ISO 13400 �C payload type				0x8001	GH_PT
03 ISO 13400 �C payload type				0x8001	GH_PT
04 ISO 13400 �C payload length			7		GH_PL
05 ISO 13400 �C payload length			7		GH_PL
06 ISO 13400 �C payload length			7		GH_PL
07 ISO 13400 �C payload length			7		GH_PL
08 ISO 13400 �C source address e.g.		0x0E00	SA
09 ISO 13400 �C source address			0x0E00	SA
10 ISO 13400 �C target address			0xE000	TA
11 ISO 13400 �C target address			0xE000	TA
12 ISO 13400 �C user data					0x22	UD / RDBI		[ ISO 27145-3 �C ReadDataByIdentifier request SID ]
13 ISO 13400 �C user data					0xF8	UD / DID_HB		[ ISO 27145-3 �C DataIdentifier #1 (HB) = ITID = protocol identification ]
14 ISO 13400 �C user data					0x10	UD / DID_LB		[ ISO 27145-3 �C DataIdentifier #1 (LB) = ITID = protocol identification ]
*/

/*
0x00
Routing confirmation acknowledge (ACK) message indicating that the diagnostic
message was correctly received, processed and put into the transmission buffer of
the destination network.
mandatory
0x01��0xFF Reserved by this part of ISO 13400. ��
*/
#define DOIP_DIAG_ACK		0x00

/*diagnostic message positive acknowledgment */
struct diagnostic_ack {
	doip_hdr_t hdr;
	uint8_t sa[2];	/*source address */
	uint8_t ta[2];	/*target address */
	uint8_t code;	/*ack code */
	uint8_t previous[0];	/*Previous diagnostic message data */
};

/*
0x00��0x01 Reserved by this part of ISO 13400 ��
0x02 Invalid source address mandatory
0x03 Unknown target address mandatory
0x04 Diagnostic message too large mandatory
0x05 Out of memory mandatory
0x06 Target unreachable optional
0x07 Unknown network optional
0x08 Transport protocol error optional
0x09��0xFF Reserved by this part of ISO 13400 ��
*/
#define DOIP_DIAG_NACK_INVALID_SA			0x02
#define DOIP_DIAG_NACK_UNKNOWN_TA			0x03
#define DOIP_DIAG_NACK_TOO_LARGE_MSG		0x04
#define DOIP_DIAG_NACK_OUT_OF_MEMORY		0x05
#define DOIP_DIAG_NACK_TARGET_UNREACHABLE	0x06
#define DOIP_DIAG_NACK_UNKNOWN_NETWORK		0x07
#define DOIP_DIAG_NACK_TP_PROTO_ERRORK		0x08

struct diagnostic_nack {
	doip_hdr_t hdr;
	uint8_t sa[2];	/*source address */
	uint8_t ta[2];	/*target address */
	uint8_t code;	/*ack code */
	uint8_t previous[0];	/*Previous diagnostic message data */
};

/*alive check request */
struct alive_check_request {
	doip_hdr_t hdr;
};

/*alive check response */
struct alive_check_response {
	doip_hdr_t hdr;
	uint8_t sa[2];	/*source address */
};

/*power mode information request */
struct power_mode_request {
	doip_hdr_t hdr;
};

/*
0x00: not ready
0x01: ready
0x02: not supported
0x03��0xFF: reserved by this part of ISO 13400
*/
#define DOIP_PM_NOT_READY	0x00
#define DOIP_PM_READY		0x01
#define DOIP_PM_UNSUPPORTED	0x02

/*power mode information response */
struct power_mode_response {
	doip_hdr_t hdr;
	uint8_t mode;	/*Diagnostic power mode */
};


/*entity status request */
struct entity_status_request {
	doip_hdr_t hdr;
};

/*
0x00: DoIP gateway
0x01: DoIP node
0x02��0xFF: reserved by this part of ISO 13400
*/
#define DOIP_ES_TYPE_GATEWAY	0x00
#define DOIP_ES_TYPE_NODE		0x01

/*entity status response */
struct entity_status_response {
	doip_hdr_t hdr;
	uint8_t nt; /*Node type */
	uint8_t mcts; /*Max. concurrent TCP_DATA sockets : 1 ~ 255 */
	uint8_t ncts; /*Currently open TCP_DATA sockets : 0 ~ 255 */
	uint8_t mds[4];	/*Max.data size(MDS) : 0 ~ 4GB */
};

#endif /* DOIPC_DOIP_PDU_H_ */
