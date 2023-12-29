#ifdef BUILD_APPLE
#ifndef DISK2_H
#define DISK2_H

#include "bus.h"
#include "../media/media.h"

class iwmDisk2 : public iwmDevice
{

protected:
    MediaType *_disk = nullptr;

    // unused because not a smartport device
    void send_status_reply_packet() override {};
    void send_extended_status_reply_packet() override {};
    void send_status_dib_reply_packet() override {};
    void send_extended_status_dib_reply_packet() override {};
    void process(iwm_decoded_cmd_t cmd) override {}; 
    void iwm_readblock(iwm_decoded_cmd_t cmd) override {};
    void iwm_writeblock(iwm_decoded_cmd_t cmd) override {};
   
    void shutdown() override;
    char disk_num;
    bool enabled;
    int track_pos;
    int old_pos;
    uint8_t oldphases;

public:
    iwmDisk2();
    void init();
    mediatype_t mount(FILE *f, uint32_t disksize, mediatype_t disk_type = MEDIATYPE_UNKNOWN);
    void unmount();
    bool write_blank(FILE *f, uint16_t sectorSize, uint16_t numSectors);
    int get_track_pos() { return track_pos; };
    bool phases_valid(uint8_t phases);
    bool move_head();
    void change_track(int indicator);
    
    // void set_disk_number(char c) { disk_num = c; }
    // char get_disk_number() { return disk_num; };
    mediatype_t disktype() { return _disk == nullptr ? MEDIATYPE_UNKNOWN : _disk->_mediatype; };

    ~iwmDisk2();
};

#endif
#endif /* BUILD_APPLE */