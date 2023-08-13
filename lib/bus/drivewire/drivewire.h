//
// http://www.boisypitre.com/retrocomputing/drivewire/
// https://www.frontiernet.net/~mmarlette/Cloud-9/Hardware/DriveWire3.html
// https://www.cocopedia.com/wiki/index.php/DRIVEWIRE.ZIP
//
// https://sourceforge.net/projects/drivewireserver/
// https://github.com/qbancoffee/drivewire4
// https://github.com/n6il/toolshed/tree/master/hdbdos
//
// https://github.com/MyTDT-Mysoft/COCO-FastLoader
//
// https://www.cocopedia.com/wiki/index.php/Main_Page
// https://github.com/qbancoffee/coco_motherboards
// https://archive.worldofdragon.org/index.php?title=Main_Page
// https://sites.google.com/site/dabarnstudio/drivewire-4-3-4e
// https://sites.google.com/site/dabarnstudio/coco-midi-drivewire
//

#ifndef COCO_H
#define COCO_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <forward_list>

#define DRIVEWIRE_BAUDRATE 62500

// /* Operation Codes */
// #define		OP_NOP		0
// #define		OP_GETSTAT	'G'
// #define		OP_SETSTAT	'S'
// #define		OP_READ		'R'
// #define		OP_READEX	'R'+128
// #define		OP_WRITE	'W'
// #define		OP_REREAD	'r'
// #define		OP_REREADEX	'r'+128
// #define		OP_REWRITE	'w'
// #define		OP_INIT		'I'
// #define		OP_TERM		'T'
// #define		OP_TIME		'#'
// #define		OP_RESET2	0xFE
// #define		OP_RESET1	0xFF
// #define		OP_PRINT	'P'
// #define		OP_PRINTFLUSH	'F'
// #define     OP_VPORT_READ    'C'

// struct dwTransferData
// {
// 	int		dw_protocol_vrsn;
// 	FILE		*devpath;
// 	FILE		*dskpath[4];
// 	int		cocoType;
// 	int		baudRate;
// 	unsigned char	lastDrive;
// 	uint32_t	readRetries;
// 	uint32_t	writeRetries;
// 	uint32_t	sectorsRead;
// 	uint32_t	sectorsWritten;
// 	unsigned char	lastOpcode;
// 	unsigned char	lastLSN[3];
// 	unsigned char	lastSector[256];
// 	unsigned char	lastGetStat;
// 	unsigned char	lastSetStat;
// 	uint16_t	lastChecksum;
// 	unsigned char	lastError;
// 	FILE	*prtfp;
// 	unsigned char	lastChar;
// 	char	prtcmd[80];
// };

// EXTERN char device[256];
// EXTERN char dskfile[4][256];
// EXTERN int maxy, maxx;
// EXTERN int updating;
// EXTERN int thread_dead;
// EXTERN FILE *logfp;
// EXTERN WINDOW *window0, *window1, *window2, *window3;
// EXTERN struct dwTransferData datapack;
// EXTERN int interactive;



union cmdFrame_t
{
    struct
    {
        uint8_t device;
        uint8_t comnd;
        uint8_t aux1;
        uint8_t aux2;
        uint8_t cksum;
    };
    struct
    {
        uint32_t commanddata;
        uint8_t checksum;
    } __attribute__((packed));
};

// class def'ns
class drivewireModem;          // declare here so can reference it, but define in modem.h
class drivewireFuji;        // declare here so can reference it, but define in fuji.h
class systemBus;      // declare early so can be friend
class drivewireNetwork;     // declare here so can reference it, but define in network.h
class drivewireUDPStream;   // declare here so can reference it, but define in udpstream.h
class drivewireCassette;    // Cassette forward-declaration.
class drivewireCPM;         // CPM device.
class drivewirePrinter;     // Printer device

class virtualDevice
{
protected:
    friend systemBus;

    int _devnum;

    cmdFrame_t cmdFrame;
    bool listen_to_type3_polls = false;
    
    /**
     * @brief All DRIVEWIRE devices repeatedly call this routine to fan out to other methods for each command. 
     * This is typcially implemented as a switch() statement.
     */
    virtual void drivewire_process(uint32_t commanddata, uint8_t checksum) = 0;

    // Optional shutdown/reboot cleanup routine
    virtual void shutdown(){};

public:
    /**
     * @brief get the DRIVEWIRE device Number (1-255)
     * @return The device number registered for this device
     */
    int id() { return _devnum; };

    /**
     * @brief Is this virtualDevice holding the virtual disk drive used to boot CONFIG?
     */
    bool is_config_device = false;

    /**
     * @brief is device active (turned on?)
     */
    bool device_active = true;

    /**
     * @brief Get the systemBus object that this virtualDevice is attached to.
     */
    systemBus get_bus();
};

enum drivewire_message : uint16_t
{
    DRIVEWIREMSG_DISKSWAP,  // Rotate disk
    DRIVEWIREMSG_DEBUG_TAPE // Tape debug msg
};

struct drivewire_message_t
{
    drivewire_message message_id;
    uint16_t message_arg;
};

// typedef drivewire_message_t drivewire_message_t;

class systemBus
{
private:
    std::forward_list<virtualDevice *> _daisyChain;

    int _command_frame_counter = 0;

    virtualDevice *_activeDev = nullptr;
    drivewireModem *_modemDev = nullptr;
    drivewireFuji *_fujiDev = nullptr;
    drivewireNetwork *_netDev[8] = {nullptr};
    drivewireUDPStream *_udpDev = nullptr;
    drivewireCassette *_cassetteDev = nullptr;
    drivewireCPM *_cpmDev = nullptr;
    drivewirePrinter *_printerdev = nullptr;

    bool useUltraHigh = false; // Use fujinet derived clock.

    void _drivewire_process_cmd();
    void _drivewire_process_queue();

    /**
     * @brief Current Baud Rate
     */
    int _drivewireBaud = 0;


    // int readSector(struct dwTransferData *dp);
    // int writeSector(struct dwTransferData *dp);
    // int seekSector(struct dwTransferData *dp, int sector);
    // void DoOP_INIT(struct dwTransferData *dp);
    // void DoOP_TERM(struct dwTransferData *dp);
    // void DoOP_RESET(struct dwTransferData *dp);
    // void DoOP_READ(struct dwTransferData *dp, char *logStr);
    // void DoOP_REREAD(struct dwTransferData *dp, char *logStr);
    // void DoOP_READEX(struct dwTransferData *dp, char *logStr);
    // void DoOP_REREADEX(struct dwTransferData *dp, char *logStr);
    // void DoOP_WRITE(struct dwTransferData *dp, char *logStr);
    // void DoOP_REWRITE(struct dwTransferData *dp, char *logStr);
    // void DoOP_GETSTAT(struct dwTransferData *dp);
    // void DoOP_SETSTAT(struct dwTransferData *dp);
    // void DoOP_TERM(struct dwTransferData *dp);
    // void DoOP_TIME(struct dwTransferData *dp);
    // void DoOP_PRINT(struct dwTransferData *dp);
    // void DoOP_PRINTFLUSH(struct dwTransferData *dp);
    // void DoOP_VPORT_READ(struct dwTransferData *dp);
    // char *getStatCode(int statcode);
    // void WinInit(void);
    // void WinSetup(WINDOW *window);
    // void WinUpdate(WINDOW *window, struct dwTransferData *dp);
    // void WinTerm(void);
    // uint16_t computeChecksum(u_char *data, int numbytes);
    // uint16_t computeCRC(u_char *data, int numbytes);
    // int comOpen(struct dwTransferData *dp, const char *device);
    // void comRaw(struct dwTransferData *dp);
    // int comRead(struct dwTransferData *dp, void *data, int numbytes);
    // int comWrite(struct dwTransferData *dp, void *data, int numbytes);
    // int comClose(struct dwTransferData *dp);
    // unsigned int int4(u_char *a);
    // unsigned int int3(u_char *a);
    // unsigned int int2(u_char *a);
    // unsigned int int1(u_char *a);
    // void _int2(uint16_t a, u_char *b);
    // int loadPreferences(struct dwTransferData *datapack);
    // int savePreferences(struct dwTransferData *datapack);
    // void openDSK(struct dwTransferData *dp, int which);
    // void closeDSK(struct dwTransferData *dp, int which);
    // void *DriveWireProcessor(void *dp);
    // void prtOpen(struct dwTransferData *dp);
    // void prtClose(struct dwTransferData *dp);
    // void logOpen(void);
    // void logClose(void);
    // void logHeader(void);
    // void setCoCo(struct dwTransferData* datapack, int cocoType);

public:
    void setup();
    void service();
    void shutdown();

    int numDevices();
    void addDevice(virtualDevice *pDevice, int device_id);
    void remDevice(virtualDevice *pDevice);
    virtualDevice *deviceById(int device_id);
    void changeDeviceId(virtualDevice *pDevice, int device_id);

    int getBaudrate();                                          // Gets current DRIVEWIRE baud rate setting
    void setBaudrate(int baud);                                 // Sets DRIVEWIRE to specific baud rate
    void toggleBaudrate();                                      // Toggle between standard and high speed DRIVEWIRE baud rate

    bool shuttingDown = false;                                  // TRUE if we are in shutdown process
    bool getShuttingDown() { return shuttingDown; };

    drivewireCassette *getCassette() { return _cassetteDev; }
    drivewirePrinter *getPrinter() { return _printerdev; }
    drivewireCPM *getCPM() { return _cpmDev; }

    // I wish this codebase would make up its mind to use camel or snake casing.
    drivewireModem *get_modem() { return _modemDev; }

    QueueHandle_t qDrivewireMessages = nullptr;
};

extern systemBus DRIVEWIRE;

#endif // guard
