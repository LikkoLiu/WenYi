#include <communication.h>

RTC_DATA_ATTR uint8_t HEX_Format_flag = 1;
RTC_DATA_ATTR uint8_t LOW_POWER_flag = 0;
RTC_DATA_ATTR uint8_t BMI160_SCAN_flag = 0;
RTC_DATA_ATTR uint8_t AS7341_SCAN_flag = 0;
RTC_DATA_ATTR uint8_t UV_SCAN_flag = 0;
RTC_DATA_ATTR uint8_t SINGLE_flag = 2;
RTC_DATA_ATTR uint8_t COMMNUI_CH_flag = 0;
RTC_DATA_ATTR uint8_t Flash_read_flag = 0;

uint8_t table_data[9]; // 这是提前定义一个数组存放接收到的数据
uint8_t table_cp[9];   // 这是额外定义一个数组，将接收到的数据复制到这里面
uint16_t count = 0;    // 接收数据计数
uint8_t table_sum = 0;

void float_to_hex_printf(uint8_t signnum, float num)
{
  union float_int
  {
    float x;
    uint8_t s[4];
  } floattoint;

  floattoint.x = num;
  Serial.printf("%c", signnum);
  Serial.printf("%c%c%c%c", floattoint.s[0], floattoint.s[1], floattoint.s[2], floattoint.s[3]);
}

void uint16_to_hex_printf(uint16_t val, uint8_t chval, uint8_t gainval)
{
  uint8_t hexSendBuff[4];
  uint8_t i;
  hexSendBuff[0] = (0xff00 & val) >> 8;
  hexSendBuff[1] = (0xff & val);
  // hexSendBuff[2] = (0xff & chval);
  hexSendBuff[2] = chval << 4;
  hexSendBuff[2] += gainval;
  for (i = 0; i < 3; i++)
  {
    hexSendBuff[3] += hexSendBuff[i];
  }
  Serial.printf("%c", 0xbb);
  for (i = 0; i < 3; i++)
  {
    // USART_SendData(USART1, table_cp[i]);
    Serial.printf("%c", hexSendBuff[i]);
  }
}

void serialEvent()
{
  uint8_t Res, i;
  while (Serial.available() > 0) // 一直等待数据接收完成 用if的话loop函数执行一次接受1个字符
  {
    Res = Serial.read(); // 如果串口有数据就进到这一部分了，将串口中的数据读到变量里。

    if (count == 0) // 如果是接收的第一个数据
    {
      table_data[count] = Res;   // 将第一个数据存到数据中第一元素
      if (table_data[0] == 0x2c) // 判断接收的第一个数据是不是十六进制0X2C
        count++;                 // 如果第一个数据是0X2C则表示正确计数+1
    }
    else if (count == 1) // 第一个数据接收正确的情况下，判断第二个数据
    {
      if (Res == 0xe4) // 如果刚接收的数据是0xE4则表示数据正确
      {
        table_data[count] = Res; // 将数据储存到数组第二个元素位置
        count++;                 // 接收数据计数+1
      }
      else // 如果第二个字符不是0XE4则计数清零，重新接收
      {
        count = 0;
        memset(table_data, 0, 9);
      }
    }
    else if (count == 2 && Res == 0) // 如果前两个数据正确，接收的第三个数据是0，则清零计数，重新接收数据
    {
      count = 0;
      memset(table_data, 0, 9);
    }
    else if (count > 1 && count < 9) // 这是可以接收数据的范围，只要count在数据可接收数据范围内即可进行存入数据
    {
      table_data[count] = Res;
      count++;
    }
    else if (count >= 9) // 如果接收数据超过数组大小，则清零重新接收
    {
      count = 0;
      memset(table_data, 0, 9);
    }
  }
  if (count == 9)
  {
    // memset(table_cp, 0, sizeof(table_data));//在使用数组table_cp时清空
    for (i = 0; i < 9; i++) // 把接收到的数据复制到table_cp数组中
    {
      table_cp[i] = table_data[i];
    }

    if (table_cp[0] == 0x2c) // 如果数组第一个十六进制数据是0X2C则进行
    {
      // 原始数据（十六进制数据）是2C E4 04 00 00 AD 01 23 FC
      table_sum = 0;
      for (i = 0; i < 8; i++)
      {
        // USART_SendData(USART1, table_cp[i]);
        table_sum += table_cp[i];
      }
      if (table_sum == table_cp[8])
      {
        for (i = 0; i < 9; i++)
        {
          // USART_SendData(USART1, table_cp[i]);
          Serial.printf("%c", table_cp[i]);
        }
        getEventFlag();
        memset(table_cp, 0, 9); // 在使用数组table_cp时清空
        memset(table_data, 0, 9);
      }

      count = 0;
    }
  }
}

void getEventFlag()
{
  HEX_Format_flag = table_cp[6];
  TIME_TO_SLEEP = table_cp[7];
  COMMNUI_CH_flag = table_cp[5];
  if (table_cp[2] == 0x30)
  {
    _STOPACAN();
  }

  else if (table_cp[2] == 0x31)
  {
    _BMI160();
    SINGLE_flag = table_cp[3];
    // switch (table_cp[3])
    // {
    // case 0x31:

    //   break;
    // case 0x32:

    //   break;
    // case 0x33:

    //   break;
    // case 0x34:

    //   break;
    // default:
    //   break;
    // }
  }

  else if (table_cp[2] == 0x32)
  {
    _AS7341();
    SINGLE_flag = table_cp[3];
    gainval = table_cp[4];
    // switch (table_cp[5])
    // {
    // case 0x31:

    //   break;
    // case 0x32:

    //   break;
    // case 0x33:

    //   break;
    // case 0x34:

    //   break;
    // default:
    //   break;
    // }
  }
  else if (table_cp[2] == 0x33)
  {
    _UV();
    SINGLE_flag = table_cp[3];
    // switch (table_cp[3])
    // {
    // case 0x00:

    //   break;
    // case 0x01:

    //   break;
    // case 0x03:

    //   break;
    // default:
    //   break;
    // }
  }
  else if (table_cp[2] == 0x34)
  {
    _ALLCAN();
    SINGLE_flag = table_cp[3];
    gainval = table_cp[4];
    // switch (table_cp[3])
    // {
    // case 0x00:

    //   break;
    // case 0x01:

    //   break;
    // case 0x03:

    //   break;
    // default:
    //   break;
    // }
  }
}
