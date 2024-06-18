
#include <stdio.h>
#include <stdlib.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <bt_uss.h>
#include <user_settings.h>

LOG_MODULE_REGISTER(main);

/* BLUETOOTH HANDLING */
static char device_name[] = "USER_SETTINGS_BT";
static const struct bt_data ad[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, device_name, sizeof(device_name))};
static const struct bt_data sd[] = {};

static struct bt_conn *prv_current_conn;

/**
 * @brief Bluetooth connected callback
 *
 * This is called when a central device connects (or fails to connect)
 * to this device.
 *
 * @param[in] conn The connection object
 * @param[in] err  BT_HCI_ERR_* error code from zephyr/bluetooth/hci_err.h
 */
static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Connected. Peer address: %s", addr);

	if (err) {
		LOG_ERR("Establishing connection failed (err %u)\n", err);
		return;
	}

	prv_current_conn = bt_conn_ref(conn);

	/* Enable USS */
	bt_uss_enable(prv_current_conn);
}

/**
 * @brief Bluetooth disconnected callback
 *
 * This is called when a central device disconnects or we disconnect from it.
 *
 * @param[in] conn The connection object
 * @param[in] reason BT_HCI_ERR_* error code from zephyr/bluetooth/hci_err.h
 */
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Disconnected. Peer address: %s (reason %02x)\n", addr, reason);

	bt_uss_disable(prv_current_conn);

	if (prv_current_conn) {
		bt_conn_unref(prv_current_conn);
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static int ble_start(void)
{
	int err = 0;

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (bt_enable, err %d)", err);
		return err;
	}

	/* Register connection callbacks */
	bt_conn_cb_register(&conn_callbacks);

	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return err;
	}

	return 0;
}

/* MAIN */

void main(void)
{
	LOG_INF("Testing settings bluetooth service");

	/* Initialize settings defaults */
	int err = user_settings_init();
	if (err) {
		LOG_ERR("err: %d", err);
	}

	/* Provide all application settings - one setting of each type is set here for demonstration
	 * purposes */
	user_settings_add(1, "t1", USER_SETTINGS_TYPE_BOOL);
	user_settings_add(2, "t2", USER_SETTINGS_TYPE_U8);
	user_settings_add(3, "t3", USER_SETTINGS_TYPE_U16);
	user_settings_add(4, "t4", USER_SETTINGS_TYPE_U32);
	user_settings_add(5, "t5", USER_SETTINGS_TYPE_U64);
	user_settings_add(6, "t6", USER_SETTINGS_TYPE_I8);
	user_settings_add(7, "t7", USER_SETTINGS_TYPE_I16);
	user_settings_add(8, "t8", USER_SETTINGS_TYPE_I32);
	user_settings_add(9, "t9", USER_SETTINGS_TYPE_I64);
	user_settings_add_sized(10, "t10", USER_SETTINGS_TYPE_STR, 10);
	user_settings_add_sized(11, "t11", USER_SETTINGS_TYPE_BYTES, 8);

	/* Load settings - this will set every setting to its value from NVS, or a default value (if
	 * it is set)
	 */
	user_settings_load();

	/* Start bluetooth operation */
	err = ble_start();
	if (err) {
		LOG_ERR("Unable to start Bluetooth, err: %d", err);
	}

	k_sleep(K_FOREVER);
}
