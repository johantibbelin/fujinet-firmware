#ifndef FUJI_H
#define FUJI_H

#include <cstdint>
#include <cstring>

#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"
#include "mbedtls/md5.h"

#include "bus.h"
#include "disk.h"
#include "network.h"
#include "cassette.h"

#include "fujiHost.h"
#include "fujiDisk.h"
#include "fujiCmd.h"

#define MAX_HOSTS 8
#define MAX_DISK_DEVICES 8
#define MAX_NETWORK_DEVICES 8

#define MAX_WIFI_PASS_LEN 64

#define MAX_APPKEY_LEN 64

#define READ_DEVICE_SLOTS_DISKS1 0x00
#define READ_DEVICE_SLOTS_TAPE 0x10

typedef struct
{
    char ssid[33];
    char hostname[64];
    unsigned char localIP[4];
    unsigned char gateway[4];
    unsigned char netmask[4];
    unsigned char dnsIP[4];
    unsigned char macAddress[6];
    unsigned char bssid[6];
    char fn_version[15];
} AdapterConfig;

enum appkey_mode : uint8_t
{
    APPKEYMODE_READ = 0,
    APPKEYMODE_WRITE,
    APPKEYMODE_INVALID
};

struct appkey
{
    uint16_t creator = 0;
    uint8_t app = 0;
    uint8_t key = 0;
    appkey_mode mode = APPKEYMODE_INVALID;
    uint8_t reserved = 0;
} __attribute__((packed));

class sioFuji : public virtualDevice
{
private:
    systemBus *_sio_bus;

    fujiHost _fnHosts[MAX_HOSTS];

    fujiDisk _fnDisks[MAX_DISK_DEVICES];

    sioCassette _cassetteDev;

    int _current_open_directory_slot = -1;

    sioDisk _bootDisk; // special disk drive just for configuration

    uint8_t bootMode = 0; // Boot mode 0 = CONFIG, 1 = MINI-BOOT

    uint8_t _countScannedSSIDs = 0;

    appkey _current_appkey;

    std::string base64_buffer;

    mbedtls_md5_context _md5;
    mbedtls_sha1_context _sha1;
    mbedtls_sha256_context _sha256;
    mbedtls_sha512_context _sha512;

    char hash_mode = 0;
    unsigned char _md5_output[16];
    unsigned char _sha1_output[20];
    unsigned char _sha256_output[32];
    unsigned char _sha512_output[64];

protected:
    void sio_reset_fujinet();          // 0xFF
    void sio_net_get_ssid();           // 0xFE
    void sio_net_scan_networks();      // 0xFD
    void sio_net_scan_result();        // 0xFC
    void sio_net_set_ssid();           // 0xFB
    void sio_net_get_wifi_status();    // 0xFA
    void sio_mount_host();             // 0xF9
    void sio_disk_image_mount();       // 0xF8
    void sio_open_directory();         // 0xF7
    void sio_read_directory_entry();   // 0xF6
    void sio_close_directory();        // 0xF5
    void sio_read_host_slots();        // 0xF4
    void sio_write_host_slots();       // 0xF3
    void sio_read_device_slots();      // 0xF2
    void sio_write_device_slots();     // 0xF1
    void sio_enable_udpstream();       // 0xF0
    void sio_net_get_wifi_enabled();   // 0xEA
    void sio_disk_image_umount();      // 0xE9
    void sio_get_adapter_config();     // 0xE8
    void sio_new_disk();               // 0xE7
    void sio_unmount_host();           // 0xE6
    void sio_get_directory_position(); // 0xE5
    void sio_set_directory_position(); // 0xE4
    void sio_set_hsio_index();         // 0xE3
    void sio_set_device_filename();    // 0xE2
    void sio_set_host_prefix();        // 0xE1
    void sio_get_host_prefix();        // 0xE0
    void sio_set_sio_external_clock(); // 0xDF
    void sio_write_app_key();          // 0xDE
    void sio_read_app_key();           // 0xDD
    void sio_open_app_key();           // 0xDC
    void sio_close_app_key();          // 0xDB
    void sio_get_device_filename();    // 0xDA
    void sio_set_boot_config();        // 0xD9
    void sio_copy_file();              // 0xD8
    void sio_set_boot_mode();          // 0xD6
    void sio_base64_encode_input();    // 0xD0
    void sio_base64_encode_compute();  // 0xCF
    void sio_base64_encode_length();   // 0xCE
    void sio_base64_encode_output();   // 0xCD
    void sio_base64_decode_input();    // 0xCC
    void sio_base64_decode_compute();  // 0xCB
    void sio_base64_decode_length();   // 0xCA
    void sio_base64_decode_output();   // 0xC9
    void sio_hash_input();             // 0xC8
    void sio_hash_compute();           // 0xC7
    void sio_hash_length();            // 0xC6
    void sio_hash_output();            // 0xC5

    void sio_status() override;
    void sio_process(uint32_t commanddata, uint8_t checksum) override;

    void shutdown() override;

public:
    bool boot_config = true;

    bool status_wait_enabled = true;

    sioDisk *bootdisk();

    sioNetwork *network();

    sioCassette *cassette() { return &_cassetteDev; };
    void debug_tape();

    void insert_boot_device(uint8_t d);

    void setup(systemBus *siobus);

    void image_rotate();
    int get_disk_id(int drive_slot);
    std::string get_host_prefix(int host_slot);

    fujiHost *get_hosts(int i) { return &_fnHosts[i]; }
    fujiDisk *get_disks(int i) { return &_fnDisks[i]; }

    void _populate_slots_from_config();
    void _populate_config_from_slots();

    void mount_all();              // 0xD7

    sioFuji();
};

extern sioFuji theFuji;

#endif // FUJI_H