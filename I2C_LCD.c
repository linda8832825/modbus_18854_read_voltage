#include <xc.h>
#include "I2C_LCD.h"
unsigned char RS, i2c_add, BackLight_State = LCD_BACKLIGHT;
//---------------[ I2C Routines ]-------------------
//--------------------------------------------------
void I2C_Master_Wait()
{
  while ((SSP1STAT & 0x04) || (SSP1CON2 & 0x1F));
}
void I2C_Master_Start()
{
  I2C_Master_Wait();
  SSP1CON2bits.SEN = 1;
}
void I2C_Master_RepeatedStart()
{
  I2C_Master_Wait();
  SSP1CON2bits.RSEN = 1;
}
void I2C_Master_Stop()
{
  I2C_Master_Wait();
  SSP1CON2bits.PEN = 1;
}
void I2C_ACK(void)
{
  SSP1CON2bits.ACKDT = 0; // 0 -> ACK
  I2C_Master_Wait();
  SSP1CON2bits.ACKEN = 1; // Send ACK
}
void I2C_NACK(void)
{
  SSP1CON2bits.ACKDT = 1; // 1 -> NACK
  I2C_Master_Wait();
  SSP1CON2bits.ACKEN = 1; // Send NACK
}
unsigned char I2C_Master_Write(unsigned char data){
  I2C_Master_Wait();
  SSP1BUF = data;
  while(!PIR3bits.SSP1IF); // 等待傳輸
  PIR3bits.SSP1IF = 0; //傳輸完成之後會為1，要用軟體回歸0
  return SSP1CON2bits.ACKSTAT;//回傳有沒有收到確認，有的話是0
}
unsigned char I2C_Read_Byte(void)
{
  //---[ Receive & Return A Byte ]---
  I2C_Master_Wait();
  SSP1CON2bits.RCEN = 1; // Enable & Start Reception
  while(!PIR3bits.SSP1IF); // Wait Until Completion
  PIR3bits.SSP1IF = 0; // Clear The Interrupt Flag Bit
  I2C_Master_Wait();
  return SSP1BUF; // Return The Received Byte
}
//======================================================
//---------------[ LCD Routines ]----------------
//-----------------------------------------------
void LCD_Init(unsigned char I2C_Add)
{
  i2c_add = I2C_Add;
  IO_Expander_Write(0x00);
  Delay(3000);
  LCD_CMD(0x03);
  Delay(500);
  LCD_CMD(0x03);
  Delay(500);
  LCD_CMD(0x03);
  Delay(500);
  LCD_CMD(LCD_RETURN_HOME);
  Delay(500);
  LCD_CMD(0x20 | (LCD_TYPE << 2));
  Delay(5000);
  LCD_CMD(LCD_TURN_ON);
  Delay(5000);
  LCD_CMD(LCD_CLEAR);
  Delay(5000);
  LCD_CMD(LCD_ENTRY_MODE_SET | LCD_RETURN_HOME);
  Delay(5000);
}
void IO_Expander_Write(unsigned char Data)
{
  I2C_Master_Start();
  I2C_Master_Write(i2c_add);
  I2C_Master_Write(Data | BackLight_State);
  I2C_Master_Stop();
}
void LCD_Write_4Bit(unsigned char Nibble)
{
  // Get The RS Value To LSB OF Data
  Nibble |= RS;
  IO_Expander_Write(Nibble | 0x04);
  IO_Expander_Write(Nibble & 0xFB);
  Delay(50);
}
void LCD_CMD(unsigned char CMD)
{
  RS = 0; // Command Register Select
  LCD_Write_4Bit(CMD & 0xF0);
  LCD_Write_4Bit((CMD << 4) & 0xF0);
}
void LCD_Write_Char(char Data)
{
  RS = 1; // Data Register Select
  LCD_Write_4Bit(Data & 0xF0);
  LCD_Write_4Bit((Data << 4) & 0xF0);
}
void LCD_Write_String(char* Str)
{
  for(int i=0; Str[i]!='\0'; i++)
    LCD_Write_Char(Str[i]);
}
void LCD_Set_Cursor(unsigned char ROW, unsigned char COL)
{
  switch(ROW) 
  {
    case 2:
      LCD_CMD(0xC0 + COL-1);
      break;
    case 3:
      LCD_CMD(0x94 + COL-1);
      break;
    case 4:
      LCD_CMD(0xD4 + COL-1);
      break;
    // Case 1
    default:
      LCD_CMD(0x80 + COL-1);
  }
}
void Backlight()
{
  BackLight_State = LCD_BACKLIGHT;
  IO_Expander_Write(0);
}
void noBacklight()
{
  BackLight_State = LCD_NOBACKLIGHT;
  IO_Expander_Write(0);
}
void LCD_SL()
{
  LCD_CMD(0x18);
  Delay(40);
}
void LCD_SR()
{
  LCD_CMD(0x1C);
  Delay(40);
}
void LCD_Clear()
{
  LCD_CMD(0x01);
  Delay(40);
}
//char  wod1[8];
//void w_oneword(){
//    int i;
//
//     //****** word 1 ****
//     wod1[0]=0b00000100;
//     wod1[1]=0b00001110;
//     wod1[2]=0b00010101;
//     wod1[3]=0b00010101;
//     wod1[4]=0b00000100;
//     wod1[5]=0b00000100;
//     wod1[6]=0b00000100;
//     wod1[7]=0b00000000;
//    //  wcode(0x40);          //?定自定?字符的CGRAM地址//第1?~第8?CGRAM地址分??40H,48H,50H,58H,60H,68H,70H,78H
//      for(i=0;i<8;i++){     //循環8次，寫完1一個字
//          wdata(wod1[i]);   //寫入該行數據
//      }
//}
//void wcode(unsigned char t){
//  rs=0;           // 寫的是命令
//  rw=0;           // 寫狀態
//  e=1;            //使能
//  Dataport=t;     //寫入命令 
//  delay(20);      //等待寫入,如果時間太短，會導致液晶無法顯示
//  e=0;            //數據的鎖定
//}
//**************************************************************************************************
//向LCD寫一數據
//**************************************************************************************************
//void wdata(unsigned char t){
//  RS=1;          // 寫的是數據
//  RW=0;          // 寫狀態
//  e=1;           //使能
//  Dataport=t;    //寫入數據
//  delay(20);     //等待寫入,如果時間太短，會導致液晶無法顯示
//  e=0;           //數據的鎖定
//}

void Delay(unsigned int counter){
  while(counter>0) counter--;  
}  