#ifdef BUILD_ATARI16BIT

#include "mediaType.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

const std::vector<MediaType::DiskImageDetails> supported_images =
{{
    // 8MB Hard Disk Drive
    { MEDIATYPE_IMG,           "MEDIATYPE_IMG", "IMG", (8 * 1024 * 1024)      },
 
    // 3.5" DS/DD Floppy Drive (720K)
    { MEDIATYPE_ST,        "MEDIATYPE_ST", "ST", (80 * 2 * 10 * 512)  },

    // MSA floppy image
    { MEDIATYPE_MSA, "MEDIATYPE_MSA", "MSA", (80 * 2 * 10 * 512) },

}};

MediaType::~MediaType()
{
    unmount();
}

bool MediaType::format(uint16_t *respopnsesize)
{
    return true;
}

bool MediaType::read(uint16_t sectornum, uint16_t *readcount)
{
    return true;
}

bool MediaType::write(uint16_t sectornum, bool verify)
{
    return true;
}

void MediaType::unmount()
{
    if (_media_fileh != nullptr)
    {
        fclose(_media_fileh);
        _media_fileh = nullptr;
    }
}

mediatype_t MediaType::discover_mediatype(const char *filename)
{
    // TODO: fix ST image

    int l = strlen(filename);
    if (l > 4 && filename[l - 4] == '.')
    {
        // Check the last 3 characters of the string
        const char *ext = filename + l - 3;
        if (strcasecmp(ext, "IMG") == 0)
        {
            return MEDIATYPE_IMG;
        }
        else if (strcasecmp(ext, "ST ") == 0)
        {
            return MEDIATYPE_ST;
        }

    }
    return MEDIATYPE_UNKNOWN;
}

uint16_t MediaType::sector_size(uint16_t sector)
{
    (void)sector; // variable sector lengths are evil!

    return 512; /* Sector size is always 512 on the ST */
}

#endif // ATARI16BIT