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
    TRISA = 0xC0; //RA3=��q�ARA2=�R�q�ARA1=LED�ARA0=�Ұʶs�ARA4=�U�@���áARA5=���ﾹ
    TRISB = 0xFF;
    TRISC = 0xBF;

    /**
    ANSELx registers
    */
    ANSELC = 0x27;//0 = Digital I/O(6&7�}) txrx  = Digital I/O(3&4�}) sclsda
    ANSELB = 0xFF;
    ANSELA = 0xF1;//led�O�}�M�R��q���T�A�~�q��

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

    RXPPS = 0x17;   //RC7->EUSART:RX;    ��rx�\��]�w�}��O�����m
    RC6PPS = 0x10;   //RC6->EUSART:TX;    ��}��]�w��\��O�����m�A���ӽT�w���󤣧�tx�\��]�w��}��O�����m
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