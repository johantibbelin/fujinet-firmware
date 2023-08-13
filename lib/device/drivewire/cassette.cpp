#ifdef BUILD_COCO

#include "cassette.h"

#include <cstring>

#include "../../include/debug.h"

#include "fnSystem.h"
#include "fnUART.h"
#include "fnFsSD.h"

#include "led.h"

/** thinking about state machine
 * boolean states:
 *      file mounted or not
 *      motor activated or not 
 *      (play/record button?)
 * state variables:
 *      baud rate
 *      file position (offset)
 * */

//#define CASSETTE_FILE "/test.cas" // zaxxon
#define CASSETTE_FILE "/csave" // basic program

// copied from fuUART.cpp - figure out better way
#define UART2_RX 33
#define ESP_INTR_FLAG_DEFAULT 0
#define BOXLEN 5

unsigned long last = 0;
unsigned long delta = 0;
unsigned long boxcar[BOXLEN];
uint8_t boxidx = 0;

static void IRAM_ATTR cas_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    if (gpio_num == UART2_RX)
    {
        unsigned long now = fnSystem.micros();
        boxcar[boxidx++] = now - last; // interval between current and last ISR call
        if (boxidx > BOXLEN)
            boxidx = 0; // circular buffer action
        delta = 0; // accumulator for boxcar filter
        for (uint8_t i = 0; i < BOXLEN; i++)
        {
            delta += boxcar[i]; // accumulate internvals for averaging
        }
        delta /= BOXLEN; // normalize accumulator to make mean
        last = now; // remember when this was (maybe move up to right before if statement?)
    }
}

softUART casUART;

uint8_t softUART::available()
{
    return index_in - index_out;
}

void softUART::set_baud(uint16_t b)
{
    baud = b;
    period = 1000000 / baud;
};

uint8_t softUART::read()
{
    return buffer[index_out++];
}

int8_t softUART::service(uint8_t b)
{
    unsigned long t = fnSystem.micros();
    if (state_counter == STARTBIT)
    {
        if (b == 1)
        { // found start bit - sync up clock
            state_counter++;
            received_byte = 0; // clear data
            baud_clock = t;    // approx beginning of start bit
#ifdef DEBUG
//            Debug_println("Start bit received!");
#endif
        }
    }
    else if (t > baud_clock + period * state_counter + period / 4)
    {
        if (t < baud_clock + period * state_counter + 9 * period / 4)
        {
            if (state_counter == STOPBIT)
            {
                buffer[index_in++] = received_byte;
                state_counter = STARTBIT;
#ifdef DEBUG
//                Debug_printf("received %02X\n", received_byte);
#endif
                if (b != 0)
                {
#ifdef DEBUG
                    Debug_println("Stop bit invalid!");
#endif
                    return -1; // frame sync error
                }
            }
            else
            {
                uint8_t bb = (b == 1) ? 0 : 1;
                received_byte |= (bb << (state_counter - 1));
                state_counter++;
#ifdef DEBUG
//                Debug_printf("bit %u ", state_counter - 1);
//                Debug_printf("%u\n ", b);
#endif
            }
        }
        else
        {
#ifdef DEBUG
            Debug_println("Bit slip error!");
#endif
            state_counter = STARTBIT;
            return -1; // frame sync error
        }
    }
    return 0;
}


//************************************************************************************************************
// ***** nerd at work! ******

void drivewireCassette::close_cassette_file()
{
    // for closing files used for writing
    if (_file != nullptr)
    {
        fclose(_file);
#ifdef DEBUG
        Debug_println("CAS file closed.");
#endif
    }
}

void drivewireCassette::open_cassette_file(FileSystem *_FS)
{
    // to open files for writing
    char fn[32];
    char mm[21];
    strcpy(fn, CASSETTE_FILE);
    if (cassetteMode == cassette_mode_t::record)
    {
        sprintf(mm, "%020lu", fnSystem.millis());
        strcat(fn, mm);
    }
    strcat(fn, ".cas");

    close_cassette_file();
    _file = _FS->file_open(fn, "w+"); // use "w+" for CSAVE test
    if (!_file)
    {
        _mounted = false;
        Debug_print("Could not open CAS file :( ");
        Debug_println(fn);
        return;
    }
#ifdef DEBUG
    Debug_printf("%s - ", fn);
    Debug_println("CAS file opened succesfully!");
#endif
}


//************************************************************************************************************


void drivewireCassette::umount_cassette_file()
{
#ifdef DEBUG
        Debug_println("CAS file closed.");
#endif
        _mounted = false;
}

void drivewireCassette::mount_cassette_file(FILE *f, size_t fz)
{

    tape_offset = 0;
    if (cassetteMode == cassette_mode_t::playback)
    {
        Debug_printf("Cassette image filesize = %u\n", fz);
        _file = f;
        filesize = fz;
        check_for_file();
    }
    else
    {
        // CONFIG does not mount a CAS file for writing - only read only.
        // disk mount (mediatype_t drivewireDisk::mount(FILE *f, const char *filename, uint32_t disksize, mediatype_t disk_type))
        // mounts a CAS file by calling this function.
        // There is no facility to specify an output file for writing to C: or CSAVE
        // so instead of using the file mounted in slot 8 by CONFIG, create an output file with some serial number
        // files are created with the cassette is enabled.
 
    }

    _mounted = true;
}

void drivewireCassette::drivewire_enable_cassette()
{
    cassetteActive = true;

    if (cassetteMode == cassette_mode_t::playback)
        fnUartBUS.set_baudrate(CASSETTE_BAUDRATE);

    if (cassetteMode == cassette_mode_t::record && tape_offset == 0)
    {
        open_cassette_file(&fnSDFAT); // hardcode SD card?
        fnUartBUS.end();
        fnSystem.set_pin_mode(UART2_RX, gpio_mode_t::GPIO_MODE_INPUT, SystemManager::pull_updown_t::PULL_NONE, GPIO_INTR_ANYEDGE);

        // hook isr handler for specific gpio pin
        if (gpio_isr_handler_add((gpio_num_t)UART2_RX, cas_isr_handler, (void *)UART2_RX) != ESP_OK)
            {
                Debug_println("error attaching cassette data reading interrupt");
                return;
            }
        // TODO: do i need to unhook isr handler when cassette is disabled?

#ifdef DEBUG
        Debug_println("stopped hardware UART");
        int a = fnSystem.digital_read(UART2_RX);
        Debug_printf("set pin to input. Value is %d\n", a);
        Debug_println("Writing FUJI File HEADERS");
#endif
        fprintf(_file, "FUJI");
        fputc(16, _file);
        fputc(0, _file);
        fputc(0, _file);
        fputc(0, _file);
        fprintf(_file, "FujiNet CAS File");

        fprintf(_file, "baud");
        fputc(0, _file);
        fputc(0, _file);
        fputc(0x58, _file);
        fputc(0x02, _file);

        fflush(_file);
        tape_offset = ftell(_file);
        block++;
    }

#ifdef DEBUG
    Debug_println("Cassette Mode enabled");
#endif
}

void drivewireCassette::drivewire_disable_cassette()
{
    if (cassetteActive)
    {
        cassetteActive = false;
        if (cassetteMode == cassette_mode_t::playback)
            fnUartBUS.set_baudrate(DRIVEWIRE_BAUDRATE);
        else
        {
            close_cassette_file();
            //TODO: gpio_isr_handler_remove((gpio_num_t)UART2_RX);
            fnUartBUS.begin(DRIVEWIRE_BAUDRATE);
        }
#ifdef DEBUG
        Debug_println("Cassette Mode disabled");
#endif
    }
}

void drivewireCassette::drivewire_handle_cassette()
{
    if (cassetteMode == cassette_mode_t::playback)
    {
        if (tape_flags.FUJI)
            tape_offset = send_tape_block(tape_offset);
        else
            tape_offset = send_tape_block(tape_offset);

        // if after trying to send data, still at the start, then turn off tape
        if (tape_offset == 0 || !cassetteActive)
        {
            drivewire_disable_cassette();
        }
    }
    else if (cassetteMode == cassette_mode_t::record)
    {
        tape_offset = receive_tape_block(tape_offset);
    }
}

void drivewireCassette::rewind()
{
    // Is this all that's needed? -tschak
    tape_offset = 0;
}

void drivewireCassette::set_buttons(bool play_record)
{
    if (!play_record)
        cassetteMode = cassette_mode_t::playback;
    else
        cassetteMode = cassette_mode_t::record;
}

bool drivewireCassette::get_buttons()
{
    return (cassetteMode == cassette_mode_t::playback);
}

void drivewireCassette::set_pulldown(bool resistor)
{
            pulldown = resistor;
}

void drivewireCassette::clear_sector_buffer(uint16_t len)
{
    //Maze sector_buffer
    unsigned char *ptr;
    ptr = sector_buffer;
    do
    {
        *ptr++ = 0;
        len--;
    } while (len);
}

size_t drivewireCassette::send_tape_block(size_t offset)
{
    unsigned char *p = sector_buffer + BLOCK_LEN - 1;
    unsigned char i, r;

    // if (offset < FileInfo.vDisk->size) {	//data record
    if (offset < filesize)
    { //data record
#ifdef DEBUG
        //print_str(35,132,2,Yellow,window_bg, (char*) sector_buffer);
        //sprintf_P((char*)sector_buffer,PSTR("Block %u / %u "),offset/BLOCK_LEN+1,(FileInfo.vDisk->size-1)/BLOCK_LEN+1);
        Debug_printf("Block %u of %u \r\n", offset / BLOCK_LEN + 1, filesize / BLOCK_LEN + 1);
#endif
        //read block
        //r = faccess_offset(FILE_ACCESS_READ, offset, BLOCK_LEN);
        fseek(_file, offset, SEEK_SET);
        r = fread(sector_buffer, 1, BLOCK_LEN, _file);

        //shift buffer 3 bytes right
        for (i = 0; i < BLOCK_LEN; i++)
        {
            *(p + 3) = *p;
            p--;
        }
        if (r < BLOCK_LEN)
        {                                  //no full record?
            sector_buffer[2] = 0xfa; //mark partial record
            sector_buffer[130] = r;  //set size in last byte
        }
        else
            sector_buffer[2] = 0xfc; //mark full record

        offset += r;
    }
    else
    { //this is the last/end record
#ifdef DEBUG
        //print_str_P(35, 132, 2, Yellow, window_bg, PSTR("End  "));
        Debug_println("CASSETTE END");
#endif
        clear_sector_buffer(BLOCK_LEN + 3);
        sector_buffer[2] = 0xfe; //mark end record
        offset = 0;
    }
    sector_buffer[0] = 0x55; //sync marker
    sector_buffer[1] = 0x55;
    // USART_Send_Buffer(sector_buffer, BLOCK_LEN + 3);
    fnUartBUS.write(sector_buffer, BLOCK_LEN + 3);
    //USART_Transmit_Byte(get_checksum(sector_buffer, BLOCK_LEN + 3));
    //fnUartBUS.write(drivewire_checksum(sector_buffer, BLOCK_LEN + 3));
    fnUartBUS.flush(); // wait for all data to be sent just like a tape
    // _delay_ms(300); //PRG(0-N) + PRWT(0.25s) delay
    fnSystem.delay(300);
    return (offset);
}

void drivewireCassette::check_for_file()
{
    struct tape_hdr *hdr = (struct tape_hdr *)sector_buffer;
    uint8_t *p = hdr->chunk_type;

    // faccess_offset(FILE_ACCESS_READ, 0, sizeof(struct tape_hdr));
    fseek(_file, 0, SEEK_SET);
    fread(sector_buffer, 1, sizeof(struct tape_hdr), _file);
    if (p[0] == 'F' && //search for FUJI header
        p[1] == 'U' &&
        p[2] == 'J' &&
        p[3] == 'I')
    {
        tape_flags.FUJI = 1;
            Debug_println("FUJI File Found");
    }
    else
    {
        tape_flags.FUJI = 0;
          Debug_println("Not a FUJI File");
    }

    if (tape_flags.turbo) //set fix to
        baud = 1000;      //1000 baud
    else
        baud = 600;
    // TO DO support kbps turbo mode
    // set_tape_baud();

    block = 0;
    return;
}


size_t drivewireCassette::receive_tape_block(size_t offset)
{
    Debug_println("Start listening for tape block from Atari");
    clear_sector_buffer(BLOCK_LEN + 4);
    uint8_t idx = 0;

    // start counting the IRG
    uint64_t tic = fnSystem.millis();

    // write out data here to file
    offset += fprintf(_file, "data");
    offset += fputc(BLOCK_LEN + 4, _file); // 132 bytes
    offset += fputc(0, _file);

    while (!casUART.available()) // && motor_line()
        casUART.service(decode_fsk());
    uint16_t irg = fnSystem.millis() - tic - 10000 / casUART.get_baud(); // adjust for first byte
#ifdef DEBUG
    Debug_printf("irg %u\n", irg);
#endif
    offset += fwrite(&irg, 2, 1, _file);
    uint8_t b = casUART.read(); // should be 0x55
    sector_buffer[idx++] = b;
#ifdef DEBUG
    Debug_printf("marker 1: %02x\n", b);
#endif

    while (!casUART.available()) // && motor_line()
        casUART.service(decode_fsk());
    b = casUART.read(); // should be 0x55
    sector_buffer[idx++] = b;
#ifdef DEBUG
    Debug_printf("marker 2: %02x\n", b);
#endif

    while (!casUART.available()) // && motor_line()
        casUART.service(decode_fsk());
    b = casUART.read(); // control byte
    sector_buffer[idx++] = b;
#ifdef DEBUG
    Debug_printf("control byte: %02x\n", b);
#endif

    int i = 0;
    while (i < BLOCK_LEN)
    {
        while (!casUART.available()) // && motor_line()
            casUART.service(decode_fsk());
        b = casUART.read(); // data
        sector_buffer[idx++] = b;
#ifdef DEBUG
//        Debug_printf(" %02x", b);
#endif
        i++;
    }
#ifdef DEBUG
//    Debug_printf("\n");
#endif

    while (!casUART.available()) // && motor_line()
        casUART.service(decode_fsk());
    b = casUART.read(); // checksum
    sector_buffer[idx++] = b;
#ifdef DEBUG
    Debug_printf("checksum: %02x\n", b);
#endif

#ifdef DEBUG
    Debug_print("data: ");
    for (int i = 0; i < BLOCK_LEN + 4; i++)
        Debug_printf("%02x ", sector_buffer[i]);
    Debug_printf("\n");
#endif

    offset += fwrite(sector_buffer, 1, BLOCK_LEN + 4, _file);

#ifdef DEBUG
    Debug_printf("file offset: %d\n", offset);
#endif
    return offset;
}

uint8_t drivewireCassette::decode_fsk()
{
    // take "delta" set in the IRQ and set the demodulator output

    uint8_t out = last_output;

    if (delta > 0)
    {
        #ifdef DEBUG
           Debug_printf("%u ", delta);
        #endif
        if (delta > 90 && delta < 97)
            out = 0;
        if (delta > 119 && delta < 130)
            out = 1;
        last_output = out;
    }
    // #ifdef DEBUG
    //     Debug_printf("%lu, ", fnSystem.micros());
    //     Debug_printf("%u\n", out);
    // #endif
    return out;
}
#endif /* BUILD_COCO */