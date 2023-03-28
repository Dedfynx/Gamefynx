#pragma once

#include <string>
#include <fstream>
#include <map>
#include <cstdint>

#define CART_SIZE 0x200000

namespace DedOs
{
    typedef struct
    {
        uint8_t entry[4];
        uint8_t logo[0x30];

        char title[16];
        uint16_t new_lic_code;
        uint8_t sgb_flag;
        uint8_t type;
        uint8_t rom_size;
        uint8_t ram_size;
        uint8_t dest_code;
        uint8_t lic_code;
        uint8_t version;
        uint8_t checksum;
        uint16_t global_checksum;
    } romHeader;

    class Cartouche
    {
    public:
        Cartouche();
        ~Cartouche();

        bool load(std::string filePath);
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t value);
        uint8_t* data;

    private:
        std::string nameLic();
        std::string nameType();
        int size = 0;
        romHeader* header;


        std::map<uint8_t, std::string> NEW_LIC_NAMES{
            {0x00, "None"},
            {0x01, "Nintendo R&D1"},
            {0x08, "Capcom"},
            {0x13, "Electronic Arts"},
            {0x18, "Hudson Soft"},
            {0x19, "b-ai"},
            {0x20, "kss"},
            {0x22, "pow"},
            {0x24, "PCM Complete"},
            {0x25, "san-x"},
            {0x28, "Kemco Japan"},
            {0x29, "seta"},
            {0x30, "Viacom"},
            {0x31, "Nintendo"},
            {0x32, "Bandai"},
            {0x33, "Ocean/Acclaim"},
            {0x34, "Konami"},
            {0x35, "Hector"},
            {0x37, "Taito"},
            {0x38, "Hudson"},
            {0x39, "Banpresto"},
            {0x41, "Ubi Soft"},
            {0x42, "Atlus"},
            {0x44, "Malibu"},
            {0x46, "angel"},
            {0x47, "Bullet-Proof"},
            {0x49, "irem"},
            {0x50, "Absolute"},
            {0x51, "Acclaim"},
            {0x52, "Activision"},
            {0x53, "American sammy"},
            {0x54, "Konami"},
            {0x55, "Hi tech entertainment"},
            {0x56, "LJN"},
            {0x57, "Matchbox"},
            {0x58, "Mattel"},
            {0x59, "Milton Bradley"},
            {0x60, "Titus"},
            {0x61, "Virgin"},
            {0x64, "LucasArts"},
            {0x67, "Ocean"},
            {0x69, "Electronic Arts"},
            {0x70, "Infogrames"},
            {0x71, "Interplay"},
            {0x72, "Broderbund"},
            {0x73, "sculptured"},
            {0x75, "sci"},
            {0x78, "THQ"},
            {0x79, "Accolade"},
            {0x80, "misawa"},
            {0x83, "lozc"},
            {0x86, "Tokuma Shoten Intermedia"},
            {0x87, "Tsukuda Original"},
            {0x91, "Chunsoft"},
            {0x92, "Video system"},
            {0x93, "Ocean/Acclaim"},
            {0x95, "Varie}"},
            {0x96, "Yonezawa/s�pal"},
            {0x97, "Kaneko"},
            {0x99, "Pack in soft"},
            {0xA4, "Konami (Yu-Gi-Oh!)"},
        };
        
        std::map<uint8_t, std::string> LIC_NAMES{
            {0x00, "None"},
            {0x01, "Nintendo"},
            {0x08, "Capcom"},
            {0x09, "Hot-b"},
            {0x0A, "Jaleco"},
            {0x0B, "Coconuts"},
            {0x0C, "Elite System"},
            {0x13, "Electronic Arts"},
            {0x18, "Hudson Soft"},
            {0x19, "b-ai"},
            {0x20, "kss"},
            {0x22, "pow"},
            {0x24, "PCM Complete"},
            {0x25, "san-x"},
            {0x28, "Kemco Japan"},
            {0x29, "seta"},
            {0x30, "Viacom"},
            {0x31, "Nintendo"},
            {0x32, "Bandai"},
            {0x33, "Ocean/Acclaim"},
            {0x34, "Konami"},
            {0x35, "Hector"},
            {0x37, "Taito"},
            {0x38, "Hudson"},
            {0x39, "Banpresto"},
            {0x41, "Ubi Soft"},
            {0x42, "Atlus"},
            {0x44, "Malibu"},
            {0x46, "angel"},
            {0x47, "Bullet-Proof"},
            {0x49, "irem"},
            {0x50, "Absolute"},
            {0x51, "Acclaim"},
            {0x52, "Activision"},
            {0x53, "American sammy"},
            {0x54, "Konami"},
            {0x55, "Hi tech entertainment"},
            {0x56, "LJN"},
            {0x57, "Matchbox"},
            {0x58, "Mattel"},
            {0x59, "Milton Bradley"},
            {0x60, "Titus"},
            {0x61, "Virgin"},
            {0x64, "LucasArts"},
            {0x67, "Ocean"},
            {0x69, "Electronic Arts"},
            {0x70, "Infogrames"},
            {0x71, "Interplay"},
            {0x72, "Broderbund"},
            {0x73, "sculptured"},
            {0x75, "sci"},
            {0x78, "THQ"},
            {0x79, "Accolade"},
            {0x80, "misawa"},
            {0x83, "lozc"},
            {0x86, "Tokuma Shoten Intermedia"},
            {0x87, "Tsukuda Original"},
            {0x91, "Chunsoft"},
            {0x92, "Video system"},
            {0x93, "Ocean/Acclaim"},
            {0x95, "Varie}"},
            {0x96, "Yonezawa/s�pal"},
            {0x97, "Kaneko"},
            {0x99, "Pack in soft"},
            {0xA4, "Konami (Yu-Gi-Oh!)"},
        };
        std::map<uint8_t, std::string> ROM_TYPES{
            {0x00,"ROM ONLY"},
            {0x01,"MBC1"},
            {0x02,"MBC1+RAM"},
            {0x03,"MBC1+RAM+BATTERY"},
            {0x05,"MBC2"},
            {0x06,"MBC2+BATTERY"},
            {0x08,"ROM+RAM 1"},
            {0x09,"ROM+RAM+BATTERY 1"},
            {0x0B,"MMM01"},
            {0x0C,"MMM01+RAM"},
            {0x0D,"MMM01+RAM+BATTERY"},
            {0x0F,"MBC3+TIMER+BATTERY"},
            {0x10,"MBC3+TIMER+RAM+BATTERY 2"},
            {0x11,"MBC3"},
            {0x12,"MBC3+RAM 2"},
            {0x13,"MBC3+RAM+BATTERY 2"},
            {0x19,"MBC5"},
            {0x1A,"MBC5+RAM"},
            {0x1B,"MBC5+RAM+BATTERY"},
            {0x1C,"MBC5+RUMBLE"},
            {0x1D,"MBC5+RUMBLE+RAM"},
            {0x1E,"MBC5+RUMBLE+RAM+BATTERY"},
            {0x20,"MBC6"},
            {0x22,"MBC7+SENSOR+RUMBLE+RAM+BATTERY"},
            {0xFC,"POCKET CAMERA"},
            {0xFD,"BANDAI TAMA5"},
            {0xFE,"HuC3"},
            {0xFF,"HuC1+RAM+BATTERY"}
        };
    };
}