/** @file bt_uss.h
 *
 * @brief Bluetooth service for user settings
 *
 * UUID of service: 8467a290-5ac0-4856-b3bc-85ae11a5bd81
 * UUID of write+notify characteristic: 8467a291-5ac0-4856-b3bc-85ae11a5bd81
 *
 * The service uses the binary user settings protocol.
 * Command can be written to the write characteristic and responses will be sent out on the
 * notify characteristic.
 *
 * Details on the binary protocol can be found in library/protocol/binary
 *
 * Writing to the characteristic can fail, in which case it will return one of the following errors:
 * - BT_ATT_ERR_ATTRIBUTE_NOT_FOUND (0x0a) if the setting ID sent does not exists
 * - BT_ATT_ERR_NOT_SUPPORTED (0x06) if the command could not be parsed
 * - BT_ATT_ERR_UNLIKELY (0x0e) if notification response could not be sent or if some other error
 *  occurred
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas.  All rights reserved.
 */

#ifndef BT_USS_H
#define BT_USS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/bluetooth/conn.h>

/**
 * @brief Enable the USS Service
 *
 * This must be called in the Bluetooth connected handler
 *
 * @param[in] conn The bluetooth connection of the connected central device
 */
void bt_uss_enable(struct bt_conn *conn);

/**
 * @brief Disable the USS Service
 *
 * This must be called in the Bluetooth disconnected handler
 *
 * @param[in] conn The bluetooth connection of the connected central device
 */
void bt_uss_disable(struct bt_conn *conn);

#ifdef __cplusplus
}
#endif

#endif /* BT_USS_H */
