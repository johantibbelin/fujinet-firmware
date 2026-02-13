#ifdef BUILD_ATARI16BIT
#ifndef ACSIFUJI_H
#define ACSIFUJI_H

#include "fujiDevice.h"
#include "../../include/debug.h"
#include <cassert>

class acsiFuji : public fujiDevice
{
private:

protected:
    bool _transaction_did_ack = false;
    void transaction_continue(bool expectMoreData) override {
        assert(!_transaction_did_ack);
        //acsi_ack();
        _transaction_did_ack = true;
    }
    void transaction_complete() override {
        assert(_transaction_did_ack);
        acsi_complete();
        _transaction_did_ack = false;
    }
    void transaction_error() override {
        if (_transaction_did_ack)
            acsi_error();
        else
            //rs232_nak();
        _transaction_did_ack = false;
    }
    bool transaction_get(void *data, size_t len) override {
        assert(_transaction_did_ack);
        uint8_t ck = bus_to_peripheral((uint8_t *) data, len);
        if (acsi_checksum((uint8_t *) data, len) != ck)
            return false;
        return true;
    }
    void transaction_put(const void *data, size_t len, bool err) override {
        assert(_transaction_did_ack);
        bus_to_computer((uint8_t *) data, len, err);
        _transaction_did_ack = false;
    }

    size_t set_additional_direntry_details(fsdir_entry_t *f, uint8_t *dest,
                                           uint8_t maxlen) override;

    void acsi_new_disk();                 // 0xE7
    void acsi_copy_file();                // 0xD8
    void acsi_test();                     // 0x00
    void acsi_net_set_ssid(bool save);     // 0xFB
public:
    acsiFuji();
    void setup() override;
    void acsi_status() override;
    void acsi_process(uint32_t commanddata, uint8_t checksum) override;

    // ============ Wrapped Fuji commands ============
};

#endif /* ACSIFUJI_H */
#endif /* BUILD_ATARI16BIT */
