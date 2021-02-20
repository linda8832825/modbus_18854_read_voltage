#include "mcc.h"
#include "pic16f18854.h"


void SYSTEM_Initialize(void)
{
    PMD_Initialize();
    PIN_MANAGER_Initialize();
    OSCILLATOR_Initialize();
    TMR0_Initialize();
    EUSART_Initialize();
    master_init();
}

void OSCILLATOR_Initialize(void)
{
    // NOSC HFINTOSC; NDIV 4; 
    OSCCON1 = 0x62;//110 HFINTOSC、0010 4(clock divider)
    // CSWHOLD may proceed; SOSCPWR Low power; 
    OSCCON3 = 0x00;
    // MFOEN disabled; LFOEN disabled; ADOEN disabled; SOSCEN disabled; EXTOEN disabled; HFOEN disabled; 
    OSCEN = 0x00;
    // HFFRQ 4_MHz; 
    OSCFRQ = 0x02;
    // HFTUN 0; 
    OSCTUNE = 0x00;
}

void PMD_Initialize(void)
{
    // CLKRMD CLKR enabled; SYSCMD SYSCLK enabled; SCANMD SCANNER enabled; FVRMD FVR enabled; IOCMD IOC enabled; CRCMD CRC enabled; NVMMD NVM enabled; 
    PMD0 = 0x00;
    // TMR0MD TMR0 enabled; TMR1MD TMR1 enabled; TMR4MD TMR4 enabled; TMR5MD TMR5 enabled; TMR2MD TMR2 enabled; TMR3MD TMR3 enabled; NCOMD DDS(NCO) enabled; TMR6MD TMR6 enabled; 
    PMD1 = 0x00;
    // ZCDMD ZCD enabled; DACMD DAC enabled; CMP1MD CMP1 enabled; ADCMD ADC enabled; CMP2MD CMP2 enabled; 
    PMD2 = 0x00;
    // CCP2MD CCP2 enabled; CCP1MD CCP1 enabled; CCP4MD CCP4 enabled; CCP3MD CCP3 enabled; CCP5MD CCP5 enabled; PWM6MD PWM6 enabled; PWM7MD PWM7 enabled; 
    PMD3 = 0x00;
    // CWG3MD CWG3 enabled; CWG2MD CWG2 enabled; CWG1MD CWG1 enabled; MSSP1MD MSSP1 enabled; UART1MD EUSART enabled; MSSP2MD MSSP2 enabled; 
    PMD4 = 0x00;
    // DSMMD DSM enabled; CLC3MD CLC3 enabled; CLC4MD CLC4 enabled; SMT1MD SMT1 enabled; SMT2MD SMT2 enabled; CLC1MD CLC1 enabled; CLC2MD CLC2 enabled; 
    PMD5 = 0x00;
}
void master_init(void){
    SSP1CON1=0x28;
    SSP1CON2=0x00;
    SSP1STATbits.SMP=1; //1 =在標準速度模式（100 kHz和1 MHz）下禁用slew rate control
    SSP1STATbits.CKE=1; //1 =智能輸入邏輯，以使閾值符合SMBus規範
    SSP1ADD=0x06;//100k Hz
    SSP1MSK=0xFF;
}


/**
 End of File
*/
