#include <myflash.h>

/* Norflash spi init */
void norflash_spi_init()
{
    // gpio init
    // pinMode(NORFLASH_HOLD_PIN, OUTPUT);
    // pinMode(NORFLASH_WP_PIN, OUTPUT);
    // digitalWrite(NORFLASH_HOLD_PIN, HIGH);
    // digitalWrite(NORFLASH_WP_PIN, HIGH);

    pinMode(NORFLASH_CS_PIN, OUTPUT);
    digitalWrite(NORFLASH_CS_PIN, HIGH);

#ifdef HARDWARE_SPI
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setFrequency(SPI_FREQUENCY);
#else
    pinMode(NORFLASH_CLK_PIN, OUTPUT);
    pinMode(NORFLASH_MOSI_PIN, OUTPUT);
    pinMode(NORFLASH_MISO_PIN, INPUT);
    digitalWrite(NORFLASH_CLK_PIN, LOW);
    delay(1);
#endif

    // check write enable status
    uint8_t data = 0;
    write_enable();
    data = read_status();
#ifdef NORFLASH_DEBUG_ENABLE
    Serial.printf("norflash write enable status:");
    Serial.println(data, BIN);
#endif

    // read device id
    uint16_t device_id = 0;
    device_id = read_norflash_id();
#ifdef NORFLASH_DEBUG_ENABLE
    Serial.printf("norflash device id: 0x%04X", device_id);
#endif
}

/* Norflash write one byte */
void write_byte(uint8_t data)
{
#if HARDWARE_SPI
    // SPI.transfer(data);
    SPI.write(data);
#endif
#if 0
    for (uint8_t i = 0; i < 8; i++)
    {
        uint8_t status;
        status = data & (0x80 >> i);
        digitalWrite(NORFLASH_MOSI_PIN, status);
        digitalWrite(NORFLASH_CLK_PIN, LOW);
        digitalWrite(NORFLASH_CLK_PIN, HIGH);
    }
#endif
}

/* Norflash read one byte */
uint8_t read_byte(uint8_t tx_data)
{
#if HARDWARE_SPI
    uint8_t data = 0;
    data = SPI.transfer(tx_data);
    return data;
#endif
#if 0
    uint8_t i = 0, data = 0;
    for (i = 0; i < 8; i++)
    {
        digitalWrite(NORFLASH_CLK_PIN, HIGH);
        digitalWrite(NORFLASH_CLK_PIN, LOW);
        if (digitalRead(NORFLASH_MISO_PIN))
        {
            data |= 0x80 >> i;
        }
    }
    return data;
#endif
}

/* Norflash write enable */
void write_enable()
{
    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(WRITE_ENABLE_CMD);
    digitalWrite(NORFLASH_CS_PIN, HIGH);
}

/* Norflash write disable */
void write_disable()
{
    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(WRITE_DISABLE_CMD);
    digitalWrite(NORFLASH_CS_PIN, HIGH);
}

/* Read norflash status */
uint8_t read_status()
{
    uint8_t status = 0;
    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(READ_STATU_REGISTER_1);
    delay(5);
    status = read_byte(0x00);
    digitalWrite(NORFLASH_CS_PIN, HIGH);
    return status;
}

/* check norflash busy status */
void check_busy(char *str)
{
    while (read_status() & 0x01)
    {
#ifdef NORFLASH_DEBUG_ENABLE
        Serial.printf("status = 0, flash is busy of %s\n", str);
#endif
    }
}

/* Write less than oneblock(256 byte) data */
void write_one_block_data(uint32_t addr, uint8_t *pbuf, uint16_t len)
{
    uint16_t i;
    check_busy("write_one_block_data");
    write_enable();
    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(PAGE_PROGRAM_CMD);
    write_byte((uint8_t)(addr >> 16));
    write_byte((uint8_t)(addr >> 8));
    write_byte((uint8_t)addr);
    for (i = 0; i < len; i++)
    {
        write_byte(*pbuf++);
    }
    digitalWrite(NORFLASH_CS_PIN, HIGH);
    check_busy("write_one_block_data");
}

/* Write less than one sector(4096 byte) length data  */
void write_one_sector_data(uint32_t addr, uint8_t *pbuf, uint16_t len)
{
    uint16_t free_space, head, page, remain;
    free_space = ONE_PAGE_SIZE - addr % ONE_PAGE_SIZE;
    if (len <= free_space)
    {
        head = len;
        page = 0;
        remain = 0;
    }
    if (len > free_space)
    {
        head = free_space;
        page = (len - free_space) / ONE_PAGE_SIZE;
        remain = (len - free_space) % ONE_PAGE_SIZE;
    }

    if (head != 0)
    {
#ifdef NORFLASH_DEBUG_ENABLE
        Serial.print("head:");
        Serial.println(head);
#endif
        write_one_block_data(addr, pbuf, head);
        pbuf = pbuf + head;
        addr = addr + head;
    }
    if (page != 0)
    {
#ifdef NORFLASH_DEBUG_ENABLE
        Serial.print("page:");
        Serial.println(page);
#endif
        for (uint16_t i = 0; i < page; i++)
        {
            write_one_block_data(addr, pbuf, ONE_PAGE_SIZE);
            pbuf = pbuf + ONE_PAGE_SIZE;
            addr = addr + ONE_PAGE_SIZE;
        }
    }
    if (remain != 0)
    {
#ifdef NORFLASH_DEBUG_ENABLE
        Serial.print("remain:");
        Serial.println(remain);
#endif
        write_one_block_data(addr, pbuf, remain);
    }
}

/* Write arbitrary length data */
void write_arbitrary_data(uint32_t addr, uint8_t *pbuf, uint32_t len)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t *write_buf = pbuf;
    uint8_t save_buffer[4096]; // save sector original data and add new data
    secpos = addr / 4096;      // sector number
    secoff = addr % 4096;      // sector offset
    secremain = 4096 - secoff; // sector remaining space

    if (len <= secremain)
    {
        secremain = len; // sector remaining space less than 4096
    }
    while (1)
    {
        read_data(secpos * 4096, save_buffer, 4096); // read sector data
        for (i = 0; i < secremain; i++)
        { // check data, if all data is 0xFF no need erase sector
            if (save_buffer[secoff + i] != 0XFF)
            { // need erase sector
                break;
            }
        }
        if (i < secremain)
        { // erase sector and write data
            sector_erase(secpos);
            for (i = 0; i < secremain; i++)
            {
                save_buffer[i + secoff] = write_buf[i]; // add new data
            }
            write_one_sector_data(secpos * 4096, save_buffer, 4096); // write sector
        }
        else
        { // no need erase sector
            write_one_sector_data(addr, write_buf, secremain);
        }
        if (len == secremain)
        { // write done
            break;
        }
        else
        {               // continue write
            secpos++;   // sector number + 1
            secoff = 0; // sector offset = 0

            write_buf += secremain; // write buff offset
            addr += secremain;      // addr offset
            len -= secremain;       // remaining data len
            if (len > 4096)
            { // remaining data more than one sector(4096 byte)
                secremain = 4096;
            }
            else
            { // remaining data less than one sector(4096 byte)
                secremain = len;
            }
        }
    }
}

/* Read arbitrary length data */
void read_data(uint32_t addr24, uint8_t *pbuf, uint32_t len)
{
    check_busy("read_data");
    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(READ_DATA_CMD);
    write_byte((uint8_t)(addr24 >> 16));
    write_byte((uint8_t)(addr24 >> 8));
    write_byte((uint8_t)addr24);
    for (uint32_t i = 0; i < len; i++)
    {
        *pbuf = read_byte(0xFF);
        pbuf++;
    }
    digitalWrite(NORFLASH_CS_PIN, HIGH);
}

/* Erase sector */
void sector_erase(uint32_t addr24)
{
    addr24 *= 4096;
    check_busy("sector_erase");
    write_enable();

    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(SECTOR_ERASE_CMD);
    write_byte((uint8_t)(addr24 >> 16));
    write_byte((uint8_t)(addr24 >> 8));
    write_byte((uint8_t)addr24);

    digitalWrite(NORFLASH_CS_PIN, HIGH);
    check_busy("sector_erase");
}

/* Read norflash id */
uint16_t read_norflash_id()
{
    uint8_t data = 0;
    uint16_t device_id = 0;

    digitalWrite(NORFLASH_CS_PIN, LOW);
    write_byte(ManufactDeviceID_CMD);
    write_byte(0x00);
    write_byte(0x00);
    write_byte(0x00);
    data = read_byte(0);
    device_id |= data; // low byte
    data = read_byte(0);
    device_id |= (data << 8); // high byte
    digitalWrite(NORFLASH_CS_PIN, HIGH);

    return device_id;
}
