#include "pin_manager.h"
void PIN_MANAGER_Initialize(void)
{
    /**
    LATx registers
    */
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;

    /**
    TRISx registers
    */
    TRISA = 0xC0; //RA3=放電，RA2=充電，RA1=LED，RA0=啟動鈕，RA4=下一頁紐，RA5=蜂鳴器
    TRISB = 0xFF;
    TRISC = 0xBF;

    /**
    ANSELx registers
    */
    ANSELC = 0x27;//0 = Digital I/O(6&7腳) txrx  = Digital I/O(3&4腳) sclsda
    ANSELB = 0xFF;
    ANSELA = 0xF1;//led燈腳和充放電的固態繼電器

    /**
    WPUx registers
    */
    WPUE = 0x00;
    WPUB = 0x00;
    WPUA = 0x00;
    WPUC = 0x00;

    /**
    ODx registers
    */
    ODCONA = 0x00;
    ODCONB = 0x00;
    ODCONC = 0x00;

    /**
    SLRCONx registers
    */
    SLRCONA = 0xFF;
    SLRCONB = 0xFF;
    SLRCONC = 0xFF;

    /**
    INLVLx registers
    */
    INLVLA = 0xFF;
    INLVLB = 0xFF;
    INLVLC = 0xFF;
    INLVLE = 0x08;

    RXPPS = 0x17;   //RC7->EUSART:RX;    把rx功能設定腳位記憶體位置
    RC6PPS = 0x10;   //RC6->EUSART:TX;    把腳位設定到功能記憶體位置，不太確定為何不把tx功能設定到腳位記憶體位置
    SSP1CLKPPS = 0x13;   //RC3->MSSP1:SCL1;    
    RC3PPS = 0x14;   //RC3->MSSP1:SCL1;    
    RC4PPS = 0x15;   //RC4->MSSP1:SDA1;    
    SSP1DATPPS = 0x14;   //RC4->MSSP1:SDA1;  
}
  
void PIN_MANAGER_IOC(void)
{   
}



/**
 End of File
*/