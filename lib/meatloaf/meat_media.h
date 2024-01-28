
#ifndef MEATLOAF_CBM_MEDIA
#define MEATLOAF_CBM_MEDIA

#include "meat_io.h"

#include <map>
#include <bitset>
#include <unordered_map>
#include <sstream>

#include "../../include/debug.h"

#include "string_utils.h"


/********************************************************
 * Streams
 ********************************************************/

class MImageStream: public MStream {

public:
    MImageStream(std::shared_ptr<MStream> is) {
        containerStream = is;
        _is_open = true;
        has_subdirs = false;
    }

    // MStream methods
    bool open() override;
    void close() override;

    ~MImageStream() {
        //Debug_printv("close");
        close();
    }

    // MStream methods
    bool isBrowsable() override { return false; };
    bool isRandomAccess() override { return true; };

    // read = (size) => this.containerStream.read(size);
    virtual uint8_t read() {
        uint8_t b = 0;
        containerStream->read( &b, 1 );
        return b;
    }
    // readUntil = (delimiter = 0x00) => this.containerStream.readUntil(delimiter);
    virtual std::string readUntil( uint8_t delimiter = 0x00 )
    {
        uint8_t b = 0, r = 0;
        std::string bytes = "";
        do
        {
            r = containerStream->read( &b, 1 );
            if ( b != delimiter )
                bytes += b;
            else
                break;
        } while ( r );

        return bytes;
    }
    // readString = (size) => this.containerStream.readString(size);
    virtual std::string readString( uint8_t size )
    {
        uint8_t b[size];
        if ( containerStream->read( b, size ) )
            return std::string((char *)b);
        
        return std::string();
    }
    // readStringUntil = (delimiter = 0x00) => this.containerStream.readStringUntil(delimiter);
    virtual std::string readStringUntil( uint8_t delimiter = '\0' )
    {
        uint8_t b[1];
        std::stringstream ss;
        while( containerStream->read( b, 1 ) )
        {
            if ( b[0] == delimiter )
                ss << b;
        }
        return ss.str();
    }

    // seek = (offset) => this.containerStream.seek(offset + this.media_header_size);
    bool seek(uint32_t offset) override { return containerStream->seek(offset + media_header_size); }
    // seekCurrent = (offset) => this.containerStream.seekCurrent(offset);
    bool seekCurrent(uint32_t offset) { return containerStream->seek(offset); }

    bool seekPath(std::string path) override { return false; };
    std::string seekNextEntry() override { return ""; };

    virtual uint32_t seekFileSize( uint8_t start_track, uint8_t start_sector );

    uint32_t read(uint8_t* buf, uint32_t size) override;
    uint32_t write(const uint8_t *buf, uint32_t size) override;
    void reset() override {
        seekCalled = false;
        _position = 0;
        _size = block_size;
        //m_load_address = {0, 0};
    }

    bool isOpen() override;
    std::string url;

protected:

    bool seekCalled = false;
    std::shared_ptr<MStream> containerStream;

    bool _is_open = false;

    MImageStream* decodedStream;

    bool show_hidden = false;

    size_t media_header_size = 0x00;
    size_t entry_index = 0;  // Currently selected directory entry
    size_t entry_count = -1; // Directory list entry count (-1 unknown)

    enum open_modes { OPEN_READ, OPEN_WRITE, OPEN_APPEND, OPEN_MODIFY };
    std::string file_type_label[12] = { "DEL", "SEQ", "PRG", "USR", "REL", "CBM", "DIR", "SUS", "NAT", "CMD", "CFS", "???" };

    virtual void seekHeader() = 0;
    virtual bool seekNextImageEntry() = 0;
    void resetEntryCounter() {
        entry_index = 0;
    }

    // Disks
    virtual uint16_t blocksFree() { return 0; };
	virtual uint8_t speedZone(uint8_t track) { return 0; };

    virtual uint32_t blocks() {
        if ( _size > 0 && _size < block_size )
            return 1;
        else
            return ( _size / block_size );
    }

    virtual bool seekEntry( std::string filename ) { return false; };
    virtual bool seekEntry( uint16_t index ) { return false; };

    virtual uint16_t readFile(uint8_t* buf, uint16_t size) = 0;
    virtual std::string decodeType(uint8_t file_type, bool show_hidden = false);

private:

    // Commodore Media
    // FILE
    friend class P00File;

    // FLOPPY DISK
    friend class D64File;
    friend class D71File;
    friend class D80File;
    friend class D81File;
    friend class D82File;

    // HARD DRIVE
    friend class DNPFile;
    friend class D90File;

    // CONTAINER
    friend class D8BFile;
    friend class DFIFile;

    // CASSETTE TAPE
    friend class T64File;
    friend class TCRTFile;

    // CARTRIDGE
    friend class CRTFile;
};



/********************************************************
 * Utility implementations
 ********************************************************/
class ImageBroker {
    static std::unordered_map<std::string, MImageStream*> repo;
public:
    template<class T> static T* obtain(std::string url) {
        // obviously you have to supply STREAMFILE.url to this function!
        if(repo.find(url)!=repo.end()) {
            return (T*)repo.at(url);
        }

        // create and add stream to broker if not found
        auto newFile = MFSOwner::File(url);
        T* newStream = (T*)newFile->getSourceStream();

        // Are we at the root of the pathInStream?
        if ( newFile->pathInStream == "")
        {
            Debug_printv("DIRECTORY [%s]", url.c_str());
        }
        else
        {
            Debug_printv("SINGLE FILE [%s]", url.c_str());
        }

        repo.insert(std::make_pair(url, newStream));
        delete newFile;
        return newStream;
    }

    static MImageStream* obtain(std::string url) {
        return obtain<MImageStream>(url);
    }

    static void dispose(std::string url) {
        if(repo.find(url)!=repo.end()) {
            auto toDelete = repo.at(url);
            repo.erase(url);
            delete toDelete;
        }
    }
};

#endif // MEATLOAF_CBM_MEDIA
