/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2006  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <time.h>
#include <sys/types.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "glib-ectomy.h"

#define HCID_CONFIG_FILE CONFIGDIR "/hcid.conf"

#define HCID_DEFAULT_DISCOVERABLE_TIMEOUT 180 /* 3 minutes */

/*
 * Scanning modes, used by DEV_SET_MODE
 * off: remote devices are not allowed to find or connect to this device
 * connectable: remote devices are allowed to connect, but they are not
 *              allowed to find it.
 * discoverable: remote devices are allowed to connect and find this device
 * unknown: reserved to not allowed/future modes
 */
#define MODE_OFF		"off"
#define MODE_CONNECTABLE	"connectable"
#define MODE_DISCOVERABLE	"discoverable"
#define MODE_UNKNOWN		"unknown"

enum {
	HCID_SET_NAME,
	HCID_SET_CLASS,
	HCID_SET_VOICE,
	HCID_SET_INQMODE,
	HCID_SET_PAGETO,
	HCID_SET_DISCOVTO,
	HCID_SET_PTYPE,
	HCID_SET_LM,
	HCID_SET_LP,
};

struct device_opts {
	unsigned long flags;
	char    *name;
	uint32_t class;
	uint16_t voice;
	uint8_t  inqmode;
	uint16_t pageto;
	uint16_t pkt_type;
	uint16_t link_mode;
	uint16_t link_policy;
	uint16_t scan;
	int      discovto;
};

extern struct device_opts default_device;
extern struct device_opts *parser_device;

struct device_list {
	char *ref;			/* HCI device or Bluetooth address */
	struct device_list *next;
	struct device_opts opts;
};

struct hcid_opts {
	char    host_name[40];
	int     auto_init;
	int     security;
	int     pairing;

	char   *config_file;

	uint8_t pin_code[16];
	int     pin_len;

	int     sock;
};
extern struct hcid_opts hcid;

typedef enum {
	REQ_PENDING,
	REQ_SENT
} req_status_t;

struct hci_req_data {
	int dev_id;
	int event;
	req_status_t status;
	bdaddr_t dba;
	uint16_t ogf;
	uint16_t ocf;
	void *cparam;
	int clen;
};

struct hci_req_data *hci_req_data_new(int dev_id, const bdaddr_t *dba, uint16_t ogf, uint16_t ocf, int event, const void *cparam, int clen);
void hci_req_queue_append(struct hci_req_data *data);
void hci_req_queue_remove(int dev_id, bdaddr_t *dba);

#define HCID_SEC_NONE	0
#define HCID_SEC_AUTO	1
#define HCID_SEC_USER	2

#define HCID_PAIRING_NONE	0
#define HCID_PAIRING_MULTI	1
#define HCID_PAIRING_ONCE	2

int read_config(char *file);

struct device_opts *alloc_device_opts(char *ref);

int get_discoverable_timeout(int dev_id);

void init_security_data(void);
void start_security_manager(int hdev);
void stop_security_manager(int hdev);
void toggle_pairing(int enable);

void set_pin_length(bdaddr_t *sba, int length);

int hcid_dbus_init(void);
void hcid_dbus_exit(void);
void hcid_dbus_set_experimental();
int hcid_dbus_use_experimental();
int hcid_dbus_register_device(uint16_t id);
int hcid_dbus_unregister_device(uint16_t id);
int hcid_dbus_start_device(uint16_t id);
int hcid_dbus_stop_device(uint16_t id);
void hcid_dbus_pending_pin_req_add(bdaddr_t *sba, bdaddr_t *dba);
int hcid_dbus_request_pin(int dev, bdaddr_t *sba, struct hci_conn_info *ci);

void hcid_dbus_inquiry_start(bdaddr_t *local);
void hcid_dbus_inquiry_complete(bdaddr_t *local);
void hcid_dbus_periodic_inquiry_start(bdaddr_t *local, uint8_t status);
void hcid_dbus_periodic_inquiry_exit(bdaddr_t *local, uint8_t status);
void hcid_dbus_inquiry_result(bdaddr_t *local, bdaddr_t *peer, uint32_t class, int8_t rssi, uint8_t *data);
void hcid_dbus_remote_class(bdaddr_t *local, bdaddr_t *peer, uint32_t class);
void hcid_dbus_remote_name(bdaddr_t *local, bdaddr_t *peer, uint8_t status, char *name);
void hcid_dbus_conn_complete(bdaddr_t *local, uint8_t status, uint16_t handle, bdaddr_t *peer);
void hcid_dbus_disconn_complete(bdaddr_t *local, uint8_t status, uint16_t handle, uint8_t reason);
void hcid_dbus_bonding_process_complete(bdaddr_t *local, bdaddr_t *peer, uint8_t status);
void hcid_dbus_setname_complete(bdaddr_t *local);
void hcid_dbus_setscan_enable_complete(bdaddr_t *local);
void hcid_dbus_pin_code_reply(bdaddr_t *local, void *ptr);

void init_devices(void);
int add_device(uint16_t dev_id);
int remove_device(uint16_t dev_id);
int start_device(uint16_t dev_id);
int stop_device(uint16_t dev_id);

int get_device_address(uint16_t dev_id, char *address, size_t size);
int get_device_version(uint16_t dev_id, char *version, size_t size);
int get_device_revision(uint16_t dev_id, char *revision, size_t size);
int get_device_manufacturer(uint16_t dev_id, char *manufacturer, size_t size);
int get_device_company(uint16_t dev_id, char *company, size_t size);

int get_device_name(uint16_t dev_id, char *name, size_t size);
int set_device_name(uint16_t dev_id, const char *name);
int get_device_alias(uint16_t dev_id, const bdaddr_t *bdaddr, char *alias, size_t size);
int set_device_alias(uint16_t dev_id, const bdaddr_t *bdaddr, const char *alias);

int get_encryption_key_size(uint16_t dev_id, const bdaddr_t *baddr);

int write_discoverable_timeout(bdaddr_t *bdaddr, int timeout);
int read_discoverable_timeout(bdaddr_t *bdaddr, int *timeout);
int write_device_mode(bdaddr_t *bdaddr, const char *mode);
int read_device_mode(bdaddr_t *bdaddr, char *mode, int length);
int write_local_name(bdaddr_t *bdaddr, char *name);
int read_local_name(bdaddr_t *bdaddr, char *name);
int write_local_class(bdaddr_t *bdaddr, uint8_t *class);
int read_local_class(bdaddr_t *bdaddr, uint8_t *class);
int write_remote_class(bdaddr_t *local, bdaddr_t *peer, uint32_t class);
int read_remote_class(bdaddr_t *local, bdaddr_t *peer, uint32_t *class);
int write_device_name(bdaddr_t *local, bdaddr_t *peer, char *name);
int read_device_name(bdaddr_t *local, bdaddr_t *peer, char *name);
int write_l2cap_info(bdaddr_t *local, bdaddr_t *peer,
			uint16_t mtu_result, uint16_t mtu,
			uint16_t mask_result, uint32_t mask);
int read_l2cap_info(bdaddr_t *local, bdaddr_t *peer,
			uint16_t *mtu_result, uint16_t *mtu,
			uint16_t *mask_result, uint32_t *mask);
int write_version_info(bdaddr_t *local, bdaddr_t *peer, uint16_t manufacturer, uint8_t lmp_ver, uint16_t lmp_subver);
int write_features_info(bdaddr_t *local, bdaddr_t *peer, unsigned char *features);
int write_lastseen_info(bdaddr_t *local, bdaddr_t *peer, struct tm *tm);
int write_lastused_info(bdaddr_t *local, bdaddr_t *peer, struct tm *tm);
int write_link_key(bdaddr_t *local, bdaddr_t *peer, unsigned char *key, int type, int length);
int read_link_key(bdaddr_t *local, bdaddr_t *peer, unsigned char *key);
int read_pin_length(bdaddr_t *local, bdaddr_t *peer);
int read_pin_code(bdaddr_t *local, bdaddr_t *peer, char *pin);

void info(const char *format, ...);
void error(const char *format, ...);
void debug(const char *format, ...);
void enable_debug();
void disable_debug();
void start_logging(const char *ident, const char *message);
void stop_logging(void);
