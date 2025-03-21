#ifndef _MEDIA_TYPE_
#define _MEDIA_TYPE_

#include <string>

#define INVALID_SECTOR_VALUE 0xFFFFFFFF

#define DISK_BYTES_PER_SECTOR_SINGLE 512
#define DISK_BYTES_PER_SECTOR_BLOCK 512

#define DISK_CTRL_STATUS_CLEAR 0x00

enum mediatype_t 
{
    MEDIATYPE_UNKNOWN = 0,
    MEDIATYPE_IMG,    // Hard drive img file
    MEDIATYPE_ST,     // .ST RAW floppy image
    MEDIATYPE_STX,    // Pasti floppy image (unimplemented (for now))
    MEDIATYPE_MSA,    // .MSA floppy image from Magic Shadow Archiver
    MEDIATYPE_COUNT
};

class MediaType
{
protected:
    FILE *_media_fileh = nullptr;
    uint32_t _media_image_size = 0;
    uint32_t _media_num_sectors = 0;
    uint16_t _media_sector_size = DISK_BYTES_PER_SECTOR_SINGLE;

public:
    typedef struct intelw_t {
        uint8_t low_byte;
        uint8_t high_byte;
    };

    struct GEMDOS_BPB {
        intelw_t pbs; //Bytes per sector (always 512)
        intelw_t cpc; //sectors per cluster
        intelw_t csb; //Cluster size in bytes
        intelw_t lsec; // Lib size in bytes
        intelw_t fsc; // FAT size in sectors
        intelw_t sssf; // Start sector secodn fat
        uint16_t noc; // Number of clusters
        uint16_t flags; // Flags bit0 - 0 12 bit FAT - 1 16 bit FAT
    };

    struct DiskImageDetails {
        mediatype_t media_type;
        std::string media_type_string;
        std::string file_extension;
        uint32_t media_size;
    };

    uint8_t _media_sectorbuff[DISK_BYTES_PER_SECTOR_SINGLE];
    uint32_t _media_last_sector = INVALID_SECTOR_VALUE-1;
    uint8_t _media_controller_status = DISK_CTRL_STATUS_CLEAR;

    mediatype_t _mediatype = MEDIATYPE_UNKNOWN;

    virtual mediatype_t mount(FILE *f, uint32_t disksize, mediatype_t disk_type) = 0;
    virtual void unmount();

    // Returns TRUE if an error condition occurred
    virtual bool format(uint16_t *respopnsesize);

    // Returns TRUE if an error condition occurred
    virtual bool read(uint16_t sectornum, uint16_t *readcount) = 0;
    // Returns TRUE if an error condition occurred
    virtual bool write(uint16_t sectornum, bool verify);
    
    virtual void status(uint8_t statusbuff[4]) = 0;

    static mediatype_t discover_mediatype(const char *filename, uint32_t disksize);
    uint16_t sector_size(uint16_t sector);

    virtual ~MediaType();
};

#endif // _MEDIA_TYPE_
