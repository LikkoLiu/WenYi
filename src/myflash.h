#ifndef _MYFLASH_H
#define _MYFLASH_H
#include <Arduino.h>
#include <SPI.h>

// #define NORFLASH_DEBUG_ENABLE     // uart debug
#define HARDWARE_SPI 1 // use hardware spi
// #define SOFTWARE_SPI           // use software spi

#define NORFLASH_CS_PIN 5
#define NORFLASH_CLK_PIN 18
#define NORFLASH_MOSI_PIN 23
#define NORFLASH_MISO_PIN 19
// #define NORFLASH_HOLD_PIN       9   // hold pin no connect 3.3V
// #define NORFLASH_WP_PIN         14  // hold pin no connect 3.3V
// #define NORFLASH_HOLD_PIN       -1     // hold pin connect 3.3V
// #define NORFLASH_WP_PIN         -1     // wp pin connect 3.3V

#define ManufactDeviceID_CMD 0x90
#define READ_JEDEC_ID_CMD 0x9F
#define WRITE_STATUS 0x01
#define READ_STATU_REGISTER_1 0x05
#define READ_STATU_REGISTER_2 0x35
#define READ_DATA_CMD 0x03
#define WRITE_ENABLE_CMD 0x06
#define WRITE_DISABLE_CMD 0x04
#define SECTOR_ERASE_CMD 0x20
#define CHIP_ERASE_CMD 0xC7
#define PAGE_PROGRAM_CMD 0x02

#define ONE_PAGE_SIZE 256
#define SPI_FREQUENCY 100000 // 时钟频率太大则无时钟输出

#define FLASH_TEST_ENABLE

uint16_t read_norflash_id();
void sector_erase(uint32_t addr24);
void read_data(uint32_t addr24, uint8_t *pbuf, uint32_t len);
void write_arbitrary_data(uint32_t addr, uint8_t *pbuf, uint32_t len);
void write_one_sector_data(uint32_t addr, uint8_t *pbuf, uint16_t len);
void write_one_block_data(uint32_t addr, uint8_t *pbuf, uint16_t len);
void check_busy(char *str);
uint8_t read_status();
void write_disable();
void write_enable();
uint8_t read_byte(uint8_t tx_data);
void write_byte(uint8_t data);
void norflash_spi_init();

#endif
