#include <stdio.h>
#include <stdint.h>

#define CRC32_POLY 0xEDB88320

uint32_t crc32_table[256];
uint32_t crc32_reverse[256];

void init_custom_crc32()
{
    if(crc32_table[1]) return;

    uint32_t i, j;
    for(i=0; i<256; i++)
    {
        uint32_t crc = i;
        uint32_t rev = i << 24;
        for(j=0; j<8; j++)
        {
            if (crc & 1)
                crc = ( crc >> 1 ) ^ CRC32_POLY;
            else
                crc = crc >> 1 ;

            if (rev & 0x80000000)
                rev = ( ( rev ^ CRC32_POLY ) << 1 ) | 1;
            else
                rev = rev << 1;
        }

        crc32_table[i] = crc;
        crc32_reverse[i] = rev;
    }
}

uint32_t custom_crc32(uint32_t crc32, const void *front, uint32_t front_length, const void *behind, uint32_t behind_length)
{
    int i = 0;
    const uint8_t *front_ptr = (const uint8_t *)front;
    const uint8_t *behind_ptr = (const uint8_t *)behind;
    init_custom_crc32();

    uint32_t front_crc32 = 0xFFFFFFFF;
    for(i=0; i<front_length; i++)
    {
        front_crc32 = ( front_crc32 >> 8 ) ^ crc32_table[ front_crc32 & 0xFF ^ front_ptr[i] ];
    }

    uint32_t behind_crc32 = crc32 ^ 0xFFFFFFFF;
    for(i=0; i<behind_length; i++)
    {
        behind_crc32 = ( behind_crc32 << 8 ) ^ crc32_reverse[ behind_crc32 >> 24 ];
        behind_crc32 = behind_crc32 ^ behind_ptr[behind_length - 1 - i];
    }

    for(i=0; i<4; i++)
    {
        const uint8_t *crc = (const uint8_t *)&front_crc32;

        behind_crc32 = ( behind_crc32 << 8 ) ^ crc32_reverse[ behind_crc32 >> 24 ];
        behind_crc32 = behind_crc32 ^ crc[4 - 1 - i];
    }

    return behind_crc32;
}

int main()
{
    printf("%08X", custom_crc32(0x12345678, "123", 3, "456", 3));
}
