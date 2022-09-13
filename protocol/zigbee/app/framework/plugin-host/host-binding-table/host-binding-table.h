//
//  binding-table-app-impl.h
//
//

#ifndef HOST_BINDING_TABLE_H
#define HOST_BINDING_TABLE_H

/**
 * @addtogroup binding-table-app-impl
 * @brief Binding table implementation on the app.
 *
 * @{
 */

#include "app/framework/include/af.h"

#define BINDING_TABLE_SIZE            60
#define EMBER_APP_PARSE_LENGTH_ERROR  0xFF
#define EMBER_APP_INVALID_INDEX       0xFF

typedef struct bindUnbindReqContents{
    uint8_t transactionSeqNum;
    uint8_t srcendpoint;
    uint8_t destAddressMode;
    uint8_t dstendpoint;
    uint16_t clusterId;
    uint16_t groupAddress;
    EmberEUI64 srceui;
    EmberEUI64 dsteui;
}emberAppbindUnbindReqContents;

/** @brief Copies a binding table entry to the structure that the
 * \c result points to.
 *
 * @param index  The index of a binding table entry.
 *
 * @param result A pointer to the location to which to copy the binding
 * table entry.
 *
 * @return An ::EmberStatus value that indicates the success
 * or failure of the command.
 */
EmberStatus emberAppGetBinding(uint8_t index, EmberBindingTableEntry *result);

/** @brief Sets an entry in the binding table by copying the structure
 * pointed to by \c value into the binding table.
 * @note You do not need to reserve memory for \c value.
 *
 * @param value  A pointer to a structure.
 *
 * @return An ::EmberStatus value that indicates the success
 * or failure of the command.
 */
EmberStatus emberAppSetBinding(EmberBindingTableEntry *value);

/** @brief Deletes an entry in the binding table by removing the structure
 * pointed to by \c value from the binding table.
 * @note You do not need to reserve memory for \c value.
 *
 * @param value  A pointer to a structure.
 *
 * @return An ::EmberStatus value that indicates the success
 * or failure of the command.
 */
EmberStatus emberAppDeleteBinding(EmberBindingTableEntry *value);

/** @brief Parses the binding table request and populates the stucture
 * provided by the app.
 *
 * @param bindReq  A pointer to the received ZDO request.
 *
 * @param length  length of the received request.
 *
 * @param bindReq  output pointer for the parsed data..
 *
 * @return An ::EmberStatus value that indicates the success
 * or failure of the command.
 */
EmberStatus emberAppParseBindUnbindReq(int8u* message, int16u length, emberAppbindUnbindReqContents *bindReq);

EmberStatus emberAppSendZdoResponses(EmberApsFrame *requestFrame,
									 EmberZdoStatus status,
									 uint8_t seqNum,
									 EmberNodeId destination);

/** @} END addtogroup */
#endif /* HOST_BINDING_TABLE_H */
