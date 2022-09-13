
#include "host-binding-table.h"

static EmberBindingTableEntry bindingTable[BINDING_TABLE_SIZE];

/** @brief Ncp Init
 *
 * This function is called when the network coprocessor is being initialized,
 * either at startup or upon reset.  It provides applications on opportunity to
 * perform additional configuration of the NCP.  The function is always called
 * twice when the NCP is initialized.  In the first invocation, memoryAllocation
 * will be true and the application should only issue EZSP commands that affect
 * memory allocation on the NCP.  For example, tables on the NCP can be resized
 * in the first call.  In the second invocation, memoryAllocation will be false
 * and the application should only issue EZSP commands that do not affect memory
 * allocation.  For example, tables on the NCP can be populated in the second
 * call.  This callback is not called on SoCs.
 *
 * @param memoryAllocation   Ver.: always
 */
void emberAfPluginHostBindingTableNcpInitCallback(boolean memoryAllocation)
{

    if(memoryAllocation == FALSE )
    {
        uint16_t currentSetting = 0;
        EzspStatus ezspStatus = ezspGetConfigurationValue(EZSP_CONFIG_APPLICATION_ZDO_FLAGS,
                                                            &currentSetting);

        if(ezspStatus == EZSP_SUCCESS){
            currentSetting |= EMBER_APP_HANDLES_ZDO_BINDING_REQUESTS;
            ezspStatus = ezspSetConfigurationValue( EZSP_CONFIG_APPLICATION_ZDO_FLAGS,
                                                    currentSetting);
            emberAfAppFlush();
            emberAfAppPrint("Host Binding Table - Ezsp Config - EZSP_CONFIG_APPLICATION_ZDO_FLAGS to 0x%2x: 0x%2x:\n", currentSetting, ezspStatus);
        }
    }

	emberAfPluginHostBindingTableInit();
}
/** @brief Pre ZDO Message Received
 *
 * This function passes the application an incoming ZDO message and gives the
 * appictation the opportunity to handle it. By default, this callback returns
 * false indicating that the incoming ZDO message has not been handled and
 * should be handled by the Application Framework.
 *
 * @param emberNodeId   Ver.: always
 * @param apsFrame   Ver.: always
 * @param message   Ver.: always
 * @param length   Ver.: always
 */
bool emberAfPluginHostBindingTablePreZDOMessageReceivedCallback(EmberNodeId emberNodeId,
                                             EmberApsFrame* apsFrame,
                                             int8u* message,
                                             int16u length)
{
	static int test = 0;
	boolean returnValue;
	EmberBindingTableEntry bindingEntry;
	EmberZdoStatus zdoStatus;
	int status;
	int index=0;

	switch (apsFrame->clusterId) {
	    case BIND_REQUEST:
	    {
	    	emberAppbindUnbindReqContents bindReq;
	    	EmberZdoStatus zdoStatus;

	    	status = emberAppParseBindUnbindReq(message, length, &bindReq);

	    	if(status != EMBER_SUCCESS){
	    		emberAfAppPrint("\nError Parsing\n");
	    	}

	    	if(bindReq.destAddressMode == 3) {
	    		bindingEntry.type = EMBER_UNICAST_BINDING;
	    		MEMCOPY(bindingEntry.identifier, &bindReq.dsteui, EUI64_SIZE);
	    	} else if(bindReq.destAddressMode == 1) {
	    		bindingEntry.type = EMBER_MULTICAST_BINDING;
	    		MEMCOPY(bindingEntry.identifier, &bindReq.groupAddress, sizeof(bindReq.groupAddress));
	    	}
	    	bindingEntry.local = bindReq.srcendpoint;
	    	bindingEntry.clusterId = bindReq.clusterId;
	    	bindingEntry.remote = bindReq.dstendpoint;

	    	status = emberAppSetBinding(&bindingEntry);

	    	zdoStatus = (status == EMBER_SUCCESS?EMBER_ZDP_SUCCESS:EMBER_ZDP_DEVICE_BINDING_TABLE_FULL);

	    	emberAfAppPrintln("emberAppSetBinding()  returned : %d", status);

	    	status = emberAppSendZdoResponses(apsFrame,
	    							 status,
									 bindReq.transactionSeqNum,
									 emberNodeId);

	    	emberAfAppPrintln("emberAppSendZdoResponses()  returned : %d", status);

	    	returnValue = true;

	    }
	    break;

	    case UNBIND_REQUEST :
	    {
	    	emberAfAppPrint("UNBIND_REQUEST:\n");
	    	emberAppbindUnbindReqContents bindReq;

	    	status = emberAppParseBindUnbindReq(message, length, &bindReq);

	    	if(status != EMBER_SUCCESS){
	    		emberAfAppPrint("\nError Parsing\n");
	    	}

	    	if(bindReq.destAddressMode == 3) {
	    		bindingEntry.type = EMBER_UNICAST_BINDING;
	    		MEMCOPY(bindingEntry.identifier, &bindReq.dsteui, EUI64_SIZE);
	    	} else if(bindReq.destAddressMode == 1) {
	    		bindingEntry.type = EMBER_MULTICAST_BINDING;
	    		MEMCOPY(bindingEntry.identifier, &bindReq.groupAddress, sizeof(bindReq.groupAddress));
	    	}
	    	bindingEntry.local = bindReq.srcendpoint;
	    	bindingEntry.clusterId = bindReq.clusterId;
	    	bindingEntry.remote = bindReq.dstendpoint;

	    	status = emberAppDeleteBinding(&bindingEntry);

	    	emberAfAppPrintln("emberAppDeleteBinding()  retrned : %d", status);

	    	zdoStatus = (status == EMBER_SUCCESS?EMBER_ZDP_SUCCESS:EMBER_ZDP_NO_ENTRY);

	    	status = emberAppSendZdoResponses(apsFrame,
	    							 zdoStatus,
									 bindReq.transactionSeqNum,
									 emberNodeId);

	    	emberAfAppPrintln("emberAppSendZdoResponses()  returned : %d", status);

	    	returnValue = true;
	    }
	    break;
	    case BINDING_TABLE_REQUEST:
	    {
	    	emberAfAppPrint("BINDING_TABLE_REQUEST:\n");
	    	returnValue = true;
	    }
	    break;
	    default:
	    {
	      emberAfCorePrintln("Untracked ZDO %2x", apsFrame->clusterId);
	      returnValue = false;
	    }
	    break;
	  }
	return returnValue;
}

static uint8_t emAppGetExistingOrUnusedIndex(EmberBindingTableEntry *value)
{
  uint8_t emptyIndex = EMBER_APP_INVALID_INDEX;
  uint8_t existingIndex = EMBER_APP_INVALID_INDEX;
  uint8_t index = 0;

  for(index = 0; index < BINDING_TABLE_SIZE; index++){
    if( (emptyIndex == EMBER_APP_INVALID_INDEX) &&
       (bindingTable[index].type == EMBER_UNUSED_BINDING) ) {
      //Try to find a empty index and record it.
      emptyIndex = index;
    } else if( (bindingTable[index].clusterId == value->clusterId) &&
              (bindingTable[index].local == value->local)&&
              (bindingTable[index].type == value->type) &&
              (bindingTable[index].remote == value->remote) ){
      //if the binding matches record the exsisting binding index. Break, because we
      //already have the index, no need to keep looking.

      if(value->type == EMBER_UNICAST_BINDING){
        if(!memcmp(bindingTable[index].identifier, value->identifier, EUI64_SIZE)){
          existingIndex = index;
          break;
        }
      }
    }
  }

  //if existingIndex is found
  //    return existingIndex
  //else
  //    return emptyIndex (which will be EMBER_APP_INVALID_INDEX, if table is full).
  index = (existingIndex != EMBER_APP_INVALID_INDEX)?existingIndex:emptyIndex;

  return index;
}

/************************************************************************************/
EmberStatus emberAppGetBinding(uint8_t index, EmberBindingTableEntry *result)
{
  if(index != EMBER_APP_INVALID_INDEX){
    //Valid index, copy the bindingEntry to result and return SUCCESS.
	  memcpy(result, &bindingTable[index], sizeof(bindingTable[index]));
    return EMBER_SUCCESS;
  }
  //Invalid request
  return EMBER_BINDING_INDEX_OUT_OF_RANGE;
}

/************************************************************************************/
EmberStatus emberAppSetBinding(EmberBindingTableEntry *value)
{
  EmberStatus status;
  //try to get the existingIndex or a freeIndex
  uint8_t index = emAppGetExistingOrUnusedIndex(value);

  if(index == EMBER_APP_INVALID_INDEX){
    //emAppGetExistingOrUnusedIndex returned EMBER_APP_INVALID_INDEX,
    //so table must be full
    return EMBER_BINDING_INDEX_OUT_OF_RANGE;
  }

  if(bindingTable[index].type == EMBER_UNUSED_BINDING){
    //not an existing binding so add it to binding table.
    memcpy(&bindingTable[index], value, sizeof(EmberBindingTableEntry));
  } else {
	  return EMBER_BINDING_INDEX_OUT_OF_RANGE;
  }

  return EMBER_SUCCESS;
}

/************************************************************************************/
EmberStatus emberAppDeleteBinding(EmberBindingTableEntry *value)
{
  //tryi to get the existingIndex or a freeIndex
  uint8_t index = emAppGetExistingOrUnusedIndex(value);

  if(index == EMBER_APP_INVALID_INDEX){
    //emAppGetExistingOrUnusedIndex returned EMBER_APP_INVALID_INDEX,
    //so table must be full
    return EMBER_INVALID_BINDING_INDEX;
  }

  if(bindingTable[index].type != EMBER_UNUSED_BINDING){
    //this is an existing binding so set this to unused in the binding table.
	bindingTable[index].type = EMBER_UNUSED_BINDING;
  } else {
	return EMBER_INVALID_BINDING_INDEX;
  }

  return EMBER_SUCCESS;
}

/************************************************************************************/
EmberStatus emberAppParseBindUnbindReq(int8u* message, int16u length, emberAppbindUnbindReqContents *bindReq)
{
  uint8_t index = 0;

  //-----------------------------------------------------------------------------------
  //                                BIND REQ FORMAT
  //-----------------------------------------------------------------------------------
  //  SEQ NO  |  SRC EP  |  Cluster ID  | AddrMode | Dest EUI or GRP ID | DST EP
  // (1 byte) | (1 byte) |  (2 bytes)   | (1 byte) |   (2 or 8 bytes)   | 0  or 1 byte
  //-----------------------------------------------------------------------------------

  if (length < 15) {
    //Bind Req is atleast 15 bytes in length
    return EMBER_APP_PARSE_LENGTH_ERROR;
  }

  bindReq->transactionSeqNum = message[index];
  index++;
  MEMCOPY(bindReq->srceui, &message[index], EUI64_SIZE);
  index += EUI64_SIZE;
  bindReq->srcendpoint = message[index];
  index++;
  bindReq->clusterId = (((uint16_t)message[index]) | ((uint16_t)message[index + 1]) << 8);
  index += 2;
  bindReq->destAddressMode = message[index];
  index++;

  if(bindReq->destAddressMode == 1) {
    //destAddressMode == 1, group address, no EP
    bindReq->groupAddress = (((uint16_t)message[index]) | ((uint16_t)message[index + 1]) << 8);
    index += 2;
  } else if((bindReq->destAddressMode == 3) &&
            (length - index) > 8){
    //destAddressMode == 3, EUI with EP
	  memcpy(bindReq->dsteui, &message[index], EUI64_SIZE);
    index += EUI64_SIZE;
    bindReq->dstendpoint = message[index];
  } else {
    return EMBER_APP_PARSE_LENGTH_ERROR;
  }

  return EMBER_SUCCESS;
}

/************************************************************************************/
EmberStatus emberAfPluginHostBindingTableInit(void)
{
	memset(bindingTable, 0, sizeof(bindingTable));
}

/************************************************************************************/
EmberStatus emberAppSendZdoResponses(EmberApsFrame *requestFrame,
									 EmberZdoStatus status,
									 uint8_t seqNum,
									 EmberNodeId destination)
{
 EmberApsFrame apsFrame;
 static uint8_t zdoResponseFrame[50], zdoMessageLength = 0;

 apsFrame.profileId = requestFrame->profileId;
 apsFrame.clusterId = requestFrame->clusterId | CLUSTER_ID_RESPONSE_MINIMUM;
 apsFrame.sourceEndpoint = EMBER_ZDO_ENDPOINT;
 apsFrame.destinationEndpoint = EMBER_ZDO_ENDPOINT;
 apsFrame.options = (EMBER_APS_OPTION_RETRY
                     | EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY);


 if(requestFrame->clusterId == BIND_REQUEST ||
		 requestFrame->clusterId == UNBIND_REQUEST){
	 zdoResponseFrame[0] = seqNum;
	 zdoResponseFrame[1] = status;
	 zdoMessageLength = 3;
 }

 return emberSendZigDevRequest(destination,
		 apsFrame.clusterId,
		 apsFrame.options,
		 zdoResponseFrame,
		 zdoMessageLength);
}
