#include "pin_manager.h"
void PIN_MANAGER_Initialize(void)
{
    PORTA = 0x00;
    /**
    LATx registers
    */
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;

    /**
    TRISx registers
    */
    TRISA = 0xF1;
    TRISB = 0xFF;
    TRISC = 0xBF;

    /**
    ANSELx registers
    */
    ANSELC = 0x3F;//0 = Digital I/O(6&7��)
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
}
  
void PIN_MANAGER_IOC(void)
{   
}

/**
 End of File
*/