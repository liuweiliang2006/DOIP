/*
 * SoAd_Cfg.c
 *
 *  Created on: 2022��3��21��
 *      Author: Administrator
 */

#include "SoAd_Cfg.h"
#include "SoAd_ConfigTypes.h"
#include "stddef.h"
#include "Std_Types.h"
static const SoAd_SocketConnectionType SoAd_SocketConnection [SOAD_SOCKET_COUNT] =
{
	{	/* for DCM */
		.SocketRemoteIpAddress = "172.18.0.200",
		.SocketRemotePort = 8989,
		.SocketProtocol = SOAD_SOCKET_PROT_TCP,
		.SocketLocalPort = 8989,
		.AutosarConnectorType = SOAD_AUTOSAR_CONNECTOR_DOIP,
	},
	{	/* for COM */
		.SocketRemoteIpAddress = "172.18.0.200",
		.SocketRemotePort = 3344,
		.SocketProtocol = SOAD_SOCKET_PROT_TCP,
		.SocketLocalPort = 3344,
		.AutosarConnectorType = SOAD_AUTOSAR_CONNECTOR_PDUR,
	}
};
static const DoIp_TargetAddressConfigType SoAd_DoIpTargetAddresses[DOIP_TARGET_COUNT]=
{
	{
		.addressValue=0xfeed,  /* this 16 bit ta */
		.txPdu=SOADTP_ID_SOAD_TX,
//		.rxPdu=PDUR_ID_SOAD_RX
		.rxPdu=NULL
	},
};
static const DoIp_TesterConfigType SoAd_DoIpTesters[DOIP_TESTER_COUNT] =
{
	{
		.address = 0xbeef,
	}
};
static const DoIp_RoutingActivationConfigType SoAd_DoIpRoutingActivations[DOIP_ROUTINGACTIVATION_COUNT] =
{
	{
		.activationNumber = 0xda,
		.authenticationCallback = NULL,
		.confirmationCallback = NULL
	}
};

static const DoIp_RoutingActivationToTargetAddressMappingType SoAd_DoIpRoutingActivationToTargetAddressMap[DOIP_ROUTINGACTIVATION_TO_TARGET_RELATION_COUNT] =
{
	{
		.routingActivation = 0,
		.target = 0,
	}
};

static const SoAd_PduRouteType SoAd_PduRoute[SOAD_PDU_ROUTE_COUNT] =
{
	{	/* for DCM */
		.DestinationSocketRef = &SoAd_SocketConnection[0],
//		.SourcePduId = PDUR_ID_SOAD_TX,
		.SourcePduId = NULL,
	},
	{	/* for COM */
		.DestinationSocketRef = &SoAd_SocketConnection[1]
	},
};

const SoAd_ConfigType SoAd_Config =
{
	.SocketConnection = SoAd_SocketConnection,
	.DoIpTargetAddresses = SoAd_DoIpTargetAddresses,
	.DoIpTesters= SoAd_DoIpTesters,
	.DoIpRoutingActivations = SoAd_DoIpRoutingActivations,
	.DoIpRoutingActivationToTargetAddressMap = SoAd_DoIpRoutingActivationToTargetAddressMap,
	.PduRoute = SoAd_PduRoute
};

Std_ReturnType SoAd_DoIp_Arc_GetVin(uint8* buf, uint8 len)
{
	return E_OK;
}

Std_ReturnType SoAd_DoIp_Arc_GetEid(uint8* buf, uint8 len)
{
	return E_OK;
}

Std_ReturnType SoAd_DoIp_Arc_GetGid(uint8* buf, uint8 len)
{
	return E_OK;
}

Std_ReturnType SoAd_DoIp_Arc_GetFurtherActionRequired(uint8* buf)
{
	return E_OK;
}

