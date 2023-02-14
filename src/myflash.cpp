#include "myflash.h"

wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

void read_data_in_batches(char *file_name)
{
    FILE *file = fopen(file_name, "rb");
    // open file
    if (file == NULL)
    {
        ESP_LOGI(ERROR_DEBUG, "Failed to open file for reading");
        return;
    }
    ESP_LOGI(INFO_DEBUG, "Opening file");

    if (Flash_read_flag)
    {
        ESP_LOGI(INFO_DEBUG, "Reading file");
        // check file size
        fseek(file, 0, SEEK_END);
        uint32_t file_size = ftell(file);
        rewind(file);

        // Allocate a buffer to store the data
        uint8_t *data = new uint8_t[CHUNK_SIZE];
        // Read the data in chunks of CHUNK_SIZE
        ESP_LOGI(INFO_DEBUG, "Read from file : ");
        for (uint32_t offset = 0; offset < file_size; offset += CHUNK_SIZE)
        {

            size_t bytes_to_read = (CHUNK_SIZE > (file_size - offset)) ? (file_size - offset) : CHUNK_SIZE;
            size_t bytes_read = fread(data, 1, bytes_to_read, file);

            // Use the read data as needed
            // ESP_LOGI(INFO_DEBUG, "%s", data);

            ESP_LOGI(INFO_DEBUG, "...... Flash read progress : %3d %% ......", (offset * 100) / file_size);
            Serial.printf("\r\n%s\r\n\r\n", data);

            // Check if there was an error reading the data
            if (bytes_read != bytes_to_read)
            {
                ESP_LOGE(ERROR_DEBUG, "Error reading the data");
                break;
            }
            // vTaskDelay(10 / portTICK_PERIOD_MS);
            memset(data, 0, CHUNK_SIZE); // Clean up

            ESP_LOGE(ERROR_DEBUG, "ERROR:%s", data);
        }
        delete[] data; // 动态数组销毁
        ESP_LOGI(INFO_DEBUG, "...... Flash read progress : 100 %% ......");
    }
    fclose(file);
    Flash_read_flag = 0; // 擦除标志位
}

void myflash_init(void)
{
    // Set up SPI bus and initialize the external SPI Flash chip
    // esp_log_level_set("example", ESP_LOG_INFO);
    esp_flash_t *flash = example_init_ext_flash();
    if (flash == NULL)
    {
        // ESP_LOGE(ERROR_DEBUG, "Can't find flash!");
        return;
    }

    // Add the entire external flash chip as a partition
    const char *partition_label = "storage";
    example_add_partition(flash, partition_label);

    // List the available partitions
    example_list_data_partitions();

    // Initialize FAT FS in the partition
    if (!example_mount_fatfs(partition_label))
    {
        return;
    }

    // Print FAT FS size information
    size_t bytes_total, bytes_free;
    example_get_fatfs_usage(&bytes_total, &bytes_free);
    ESP_LOGI(INFO_DEBUG, "FAT FS: %d kB total, %d kB free", bytes_total / 1024, bytes_free / 1024);

    // Open file for reading

    // uint8_t line[1 * 1024];
    // FILE *f = fopen("/extflash/hello.txt", "rb");
    // if (f == NULL)
    // {
    //     ESP_LOGE(TAG, "Failed to open file for reading");
    //     return;
    // }
    //
    // fgets(line, sizeof(line), f);
    // fclose(f);
    // // strip newline
    // char *pos = strchr(line, '\n');
    // if (pos)
    // {
    //     *pos = '\0';
    // }
    read_data_in_batches("/extflash/hello.txt");
}

esp_flash_t *example_init_ext_flash(void)
{
    const spi_bus_config_t bus_config = {
        .mosi_io_num = VSPI_IOMUX_PIN_NUM_MOSI,
        .miso_io_num = VSPI_IOMUX_PIN_NUM_MISO,
        .sclk_io_num = VSPI_IOMUX_PIN_NUM_CLK,
        .quadwp_io_num = -1, // 硬件拉高
        .quadhd_io_num = -1, // 硬件拉高
    };

    const esp_flash_spi_device_config_t device_config = {
        .host_id = VSPI_HOST,
        .cs_io_num = VSPI_IOMUX_PIN_NUM_CS,
        .io_mode = SPI_FLASH_DIO,
        .speed = ESP_FLASH_5MHZ,
        .cs_id = 0,
    };

    ESP_LOGI(INFO_DEBUG, "Initializing external SPI Flash");
    ESP_LOGI(INFO_DEBUG, "Pin assignments :");
    ESP_LOGI(INFO_DEBUG, "MOSI: %2d   MISO: %2d   SCLK: %2d   CS: %2d",
             bus_config.mosi_io_num, bus_config.miso_io_num,
             bus_config.sclk_io_num, device_config.cs_io_num);

    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &bus_config, 1));

    // Add device to the SPI bus
    esp_flash_t *ext_flash;
    ESP_ERROR_CHECK(spi_bus_add_flash_device(&ext_flash, &device_config));

    // Probe the Flash chip and initialize it
    esp_err_t err = esp_flash_init(ext_flash);
    if (err != ESP_OK)
    {
        ESP_LOGE(ERROR_DEBUG, "Failed to initialize external Flash: %s (0x%x)", esp_err_to_name(err), err);
        return NULL;
    }

    // Print out the ID and size
    uint32_t id;
    ESP_ERROR_CHECK(esp_flash_read_id(ext_flash, &id));
    ESP_LOGI(INFO_DEBUG, "Initialized external Flash, size=%d KB, ID=0x%x", ext_flash->size / 1024, id);

    return ext_flash;
}

const esp_partition_t *example_add_partition(esp_flash_t *ext_flash, const char *partition_label)
{
    ESP_LOGI(INFO_DEBUG, "Adding external Flash as a partition, label=\"%s\", size=%d KB", partition_label, ext_flash->size / 1024);
    const esp_partition_t *fat_partition;
    ESP_ERROR_CHECK(esp_partition_register_external(ext_flash, 0, ext_flash->size, partition_label, ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, &fat_partition));
    return fat_partition;
}

void example_list_data_partitions(void)
{
    ESP_LOGI(INFO_DEBUG, "Listing data partitions:");
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);

    for (; it != NULL; it = esp_partition_next(it))
    {
        const esp_partition_t *part = esp_partition_get(it);
        ESP_LOGI(INFO_DEBUG, "- partition '%s', subtype %d, offset 0x%x, size %d kB",
                 part->label, part->subtype, part->address, part->size / 1024);
    }

    esp_partition_iterator_release(it);
}

bool example_mount_fatfs(const char *partition_label)
{
    ESP_LOGI(INFO_DEBUG, "Mounting FAT filesystem");
    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 4,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE};
    esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, partition_label, &mount_config, &s_wl_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(ERROR_DEBUG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return false;
    }
    return true;
}

void example_get_fatfs_usage(size_t *out_total_bytes, size_t *out_free_bytes)
{
    FATFS *fs;
    size_t free_clusters;
    int res = f_getfree("0:", &free_clusters, &fs);
    assert(res == FR_OK);
    size_t total_sectors = (fs->n_fatent - 2) * fs->csize;
    size_t free_sectors = free_clusters * fs->csize;

    // assuming the total size is < 4GiB, should be true for SPI Flash
    if (out_total_bytes != NULL)
    {
        *out_total_bytes = total_sectors * fs->ssize;
    }
    if (out_free_bytes != NULL)
    {
        *out_free_bytes = free_sectors * fs->ssize;
    }
}

void hex2string(char *hex, char *ascII, int len, int *newlen)
{
    int i = 0;
    char newchar[100] = {0};
    *newlen = len * 3;
    for (i = 0; i < len; i++)
    {
        sprintf(newchar, "%02X ", hex[i]);
        strcat(ascII, newchar);
    }
}

void Flash_Write_Data(const char *data_flag, uint16_t *val, uint8_t count, uint8_t gainval)
{
    uint8_t chval = 0;
    uint8_t hexSendBuff[4];
    FILE *f = fopen("/extflash/hello.txt", "a");
    if (f == NULL)
    {
        ESP_LOGE(ERROR_DEBUG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "#%s%d#", data_flag, gainval);
    for (size_t i = 0; i < count; i++)
        fprintf(f, "B%d", val[i]);
    fclose(f);
    ESP_LOGI(INFO_DEBUG, "File written");
}

void Flash_Write_float_Data(const char *data_flag, float val)
{
    FILE *f = fopen("/extflash/hello.txt", "a");
    if (f == NULL)
    {
        ESP_LOGE(ERROR_DEBUG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "%s%.2f", data_flag, val);
    fclose(f);
    ESP_LOGI(INFO_DEBUG, "File written");
}

void Flash_Write_RTC()
{
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    char strftime_buf[64];
    time(&now);

    setenv("TZ", "CST-8", 1); // 时区设置需位于localtime_r(&now, &timeinfo)前
    tzset();

    localtime_r(&now, &timeinfo);

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    FILE *f = fopen("/extflash/hello.txt", "a");
    if (f == NULL)
    {
        ESP_LOGE(ERROR_DEBUG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "@%s@", strftime_buf);
    fclose(f);
    ESP_LOGI(INFO_DEBUG, "The current date/time in HANGZHOU is: %s", strftime_buf);
    ESP_LOGI(INFO_DEBUG, "File written");
}

void Flash_Writeln()
{
    FILE *f = fopen("/extflash/hello.txt", "a");
    if (f == NULL)
    {
        ESP_LOGE(ERROR_DEBUG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "\r\n");
    fclose(f);
}

void Flash_Erase_FATfile(char *file_name)
{
    const char *partition_label = "storage";
    int res;
    FILE *file = fopen(file_name, "wb");
    fwrite(NULL, 0, 1, file);
    fclose(file);
    // // open file
    // if (file == NULL)
    // {
    //     ESP_LOGI(ERROR_DEBUG, "Failed to open file for Erase");
    //     return;
    // }

    res = remove(file_name);
    if (res == 0)
    {
        ESP_LOGI(INFO_DEBUG, "%s file deleted successfully.", file_name);
        fopen("/extflash/hello.txt", "wb");
        if (res == FR_OK)
            fclose(file);
        else
            ESP_LOGE(TAG, "Failed to open file for NEXT");
    }
    else
        Serial.printf("Unable to delete the file\n");
}

void read_data_config(char *file_name)
{
    FILE *file = fopen(file_name, "rb");
    // open file
    if (file == NULL)
    {
        ESP_LOGI(ERROR_DEBUG, "Failed to open config.file for reading");
        return;
    }

    ESP_LOGI(INFO_DEBUG, "Opening config.file");

    // Allocate a buffer to store the data
    uint8_t *data = new uint8_t[9];
    // Read the data in chunks of CHUNK_SIZE

    size_t bytes_read = fread(data, 1, 9, file);

    // Use the read data as needed
    HEX_Format_flag = data[6];
    TIME_TO_SLEEP = data[7];
    COMMNUI_CH_flag = data[5];
    if (data[2] == 0x30)
    {
        _STOPACAN();
    }

    else if (data[2] == 0x31)
    {
        _BMI160();
        SINGLE_flag = data[3];
    }

    else if (data[2] == 0x32)
    {
        _AS7341();
        SINGLE_flag = data[3];
        gainval = data[4];
    }
    else if (data[2] == 0x33)
    {
        _UV();
        SINGLE_flag = data[3];
    }
    else if (data[2] == 0x34)
    {
        _ALLCAN();
        SINGLE_flag = data[3];
        gainval = data[4];
    }

    ESP_LOGI(INFO_DEBUG, "Read from config.file :%c", data);

    // vTaskDelay(10 / portTICK_PERIOD_MS);
    // memset(data, 0, CHUNK_SIZE); // Clean up

    // ESP_LOGE(ERROR_DEBUG, "ERROR: 0\b%s", data);

    delete[] data; // 动态数组销毁

    fclose(file);
}

void Write_config_Data(uint8_t *val)
{
    uint8_t chval = 0;
    uint8_t hexSendBuff[4];
    FILE *f = fopen("/extflash/config.txt", "w");
    if (f == NULL)
    {
        ESP_LOGE(ERROR_DEBUG, "Failed to open config.file for writing");
        return;
    }
    for (size_t i = 0; i < 9; i++)
        fprintf(f, "%c", val[i]);
    fclose(f);
    ESP_LOGI(INFO_DEBUG, "config.file written");
}