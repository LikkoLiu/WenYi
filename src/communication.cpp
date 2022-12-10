#include <communication.h>


uint8_t table_data[9]; // 这是提前定义一个数组存放接收到的数据
uint8_t table_cp[9];   // 这是额外定义一个数组，将接收到的数据复制到这里面
uint16_t count = 0;    // 接收数据计数

void serialEvent() // 关键的来了。串口中断部分来了。多注意，多百度。
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

    for (i = 0; i < 9; i++)
    {
      // USART_SendData(USART1, table_cp[i]);
      Serial.printf("%c", table_cp[i]);
    }

    count = 0;
  }
}
