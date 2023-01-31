#ifndef _MYFLASH_H
#define _MYFLASH_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "esp_partition.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"
#include "soc/spi_pins.h"
#include "esp32-hal-log.h"

// h4 and c2 will not support external flash
#define EXAMPLE_FLASH_FREQ_MHZ      40

const char TAG[] = "example";

// Pin mapping
// ESP32 (VSPI)
#ifdef CONFIG_IDF_TARGET_ESP32
#define HOST_ID  SPI3_HOST
#define PIN_MOSI SPI3_IOMUX_PIN_NUM_MOSI
#define PIN_MISO SPI3_IOMUX_PIN_NUM_MISO
#define PIN_CLK  SPI3_IOMUX_PIN_NUM_CLK
#define PIN_CS   SPI3_IOMUX_PIN_NUM_CS
#define PIN_WP   SPI3_IOMUX_PIN_NUM_WP
#define PIN_HD   SPI3_IOMUX_PIN_NUM_HD
#define SPI_DMA_CHAN SPI_DMA_CH_AUTO
#else // Other chips (SPI2/HSPI)
#define HOST_ID  SPI2_HOST
#define PIN_MOSI SPI2_IOMUX_PIN_NUM_MOSI
#define PIN_MISO SPI2_IOMUX_PIN_NUM_MISO
#define PIN_CLK  SPI2_IOMUX_PIN_NUM_CLK
#define PIN_CS   SPI2_IOMUX_PIN_NUM_CS
#define PIN_WP   SPI2_IOMUX_PIN_NUM_WP
#define PIN_HD   SPI2_IOMUX_PIN_NUM_HD
#define SPI_DMA_CHAN SPI_DMA_CH_AUTO
#endif

// Handle of the wear levelling library instance
// int32_t s_wl_handle = WL_INVALID_HANDLE;

// Mount path for the partition
const char base_path[] = "/extflash";

void myflash_init(void);
esp_flash_t* example_init_ext_flash(void);
const esp_partition_t* example_add_partition(esp_flash_t* ext_flash, const char* partition_label);
void example_list_data_partitions(void);
bool example_mount_fatfs(const char* partition_label);
void example_get_fatfs_usage(size_t* out_total_bytes, size_t* out_free_bytes);



#endif

