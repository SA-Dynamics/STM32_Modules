#include "OPCUA_Task.h"
#include "open62541.h"




//static struct
//{
//	int32_t s32Sock;
//	struct sockaddr_in sClientAddr;
//	char s8SendBuf[64];
//	char s8RecvBuf[64];
//}g_sClientParams;


//void ClientTaskInit(void)
//{
////	g_sClientParams.s32Sock = -1;
////	
////	g_sClientParams.s32Sock = socket(AF_INET, SOCK_STREAM);
////	
////	g_sClientParams.sClientAddr.sin_family = AF_INET;
////	g_sClientParams.sClientAddr.sin_port = htons(4840);
////	
////	if (connect(g_sClientParams.s32Sock, (struct sockaddr *)&g_sClientParams.sClientAddr, sizeof(struct sockaddr) == -1)
////	{		
////		closesocket(g_sClientParams.s32Sock);
////		g_sClientParams.s32Sock = -1;
////	}
//}


//void TCP_ClientTask(void *argument)
//{
//	MX_LWIP_Init();
//	
//	while (1)
//	{
//		vTaskDelay(1);
//	}
//}


void OPCUA_Task(void *pvParameter)
{
	//The default 64KB of memory for sending and receicing buffer caused problems to many users. With the code below, they are reduced to ~16KB
	UA_UInt32 sendBufferSize = 160;       //64 KB was too much for my platform
	UA_UInt32 recvBufferSize = 160;       //64 KB was too much for my platform
	UA_UInt16 portNumber = 4840;

	UA_Server* mUaServer = UA_Server_new();
//	UA_ServerConfig *uaServerConfig = UA_Server_getConfig(mUaServer);
//	UA_ServerConfig_setMinimalCustomBuffer(uaServerConfig, portNumber, 0, sendBufferSize, recvBufferSize);

//	//VERY IMPORTANT: Set the hostname with your IP before starting the server
//	UA_ServerConfig_setCustomHostname(uaServerConfig, UA_STRING("192.168.1.100"));

//	//The rest is the same as the example

//	UA_Boolean running = true;

//	// add a variable node to the adresspace
//	UA_VariableAttributes attr = UA_VariableAttributes_default;
//	UA_Int32 myInteger = 42;
//	UA_Variant_setScalarCopy(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
//	attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US","the answer");
//	attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US","the answer");
//	UA_NodeId myIntegerNodeId = UA_NODEID_STRING_ALLOC(1, "the.answer");
//	UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME_ALLOC(1, "the answer");
//	UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
//	UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
//	UA_Server_addVariableNode(mUaServer, myIntegerNodeId, parentNodeId,
//																													parentReferenceNodeId, myIntegerName,
//																													UA_NODEID_NULL, attr, NULL, NULL);

//	/* allocations on the heap need to be freed */
//	UA_VariableAttributes_clear(&attr);
//	UA_NodeId_clear(&myIntegerNodeId);
//	UA_QualifiedName_clear(&myIntegerName);

//	UA_StatusCode retval = UA_Server_run(mUaServer, &running);
//	UA_Server_delete(mUaServer);	

	while (1)
	{
		osDelay(1);
	}

}
