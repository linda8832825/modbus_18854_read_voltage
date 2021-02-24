//######################################################//
//         項目 : 永磁 UPS _ Convertet 輸出調整
//         日期 : 2021.02.18
//         版本 : v1.0
//         作者 : linda
//######################################################//


#include "mcc_generated_files/mcc.h"
#include <stdlib.h>
#include "I2C_LCD.h"

#define VoltageSet_H          3500          //充電最高點
#define VoltageSet_L          2500          //充電最低點
#define Voltage_difference    150           //單電芯壓差容忍值
#define SLAVE_ADD 0x4E                      //螢幕的ID

typedef struct tagCoulomb_Data_struct {
    unsigned int   ID  ;  //庫倫計回的值
    unsigned int   Features ; //功能  庫倫計回的值
    unsigned int   Quantity ; //數量   庫倫計回的值的數量
    unsigned int Voltage_H[17] 	; //庫倫計回的值
	unsigned int Voltage_L[17] 	;	//庫倫計回的值		
    unsigned int CRC_H;  //庫倫計回的值
    unsigned int CRC_L;  //庫倫計回的值
    union {
		unsigned int _Voltage;
		struct {
			unsigned Voltage_Point	:4;		//電壓後小數點  
			unsigned Voltage_statue	:4;		//		
		};
	};
    union {
		unsigned int _Current;
		struct {
			unsigned Current_Point	:4;		//電流後小數點  
			unsigned Current_statue	:4;		//看電流是正還負
		};
	}; //聯合主要用來表示同概念但不同資料類型的實體
    unsigned int Voltage[17];  //庫倫計回的值
} Coulomb_Data_struct_define;//存庫輪計回的值

Coulomb_Data_struct_define Coulomb_Data ; //用一個struct
void Get_Voltage(int);  //函數宣告
void BadBetteryFilter();  //函數宣告
void Sort();  //函數宣告
unsigned int Voltage_sort[17]={0};
unsigned int badBettery[17]={0};
unsigned int diffBettery[17]={0};
char ShowbadBettery[4]={0};
char no;
void main(void)
{
    SYSTEM_Initialize();  //mcc出來的東西，用來初始化記憶體設定
    INTERRUPT_GlobalInterruptEnable();//INTCONbits.GIE = 1       //讓中斷的功能打開
    INTERRUPT_PeripheralInterruptEnable();//INTCONbits.PEIE = 1   //讓中斷的功能打開

    unsigned char out[8]={0x01,0x03,0x00,0x03,0x00,0x11,0x75,0xC6};  //從第3筆資料取17筆出來
    int i,j,H_L_counter=0;
    
    while (1){
        H_L_counter=0;
        if(TMR0_ReadTimer()==0){  //計時用的，時間到了就中斷，間隔一秒發送一次
            for(i=0;i<8;i++){   // 一開始寫出那行值，要給庫輪計的
                EUSART_Write(out[i]);
                NOP();
            }
        }
        if(eusartRxCount == 39){     //如果接收到9個值，就進來讀值 //原本是9，因為只有讀電壓
            unsigned int  *index;    //指標index
            unsigned int  *index_V_H;    //指標index
            unsigned int  *index_V_L;    //指標index
            index=&Coulomb_Data.ID;  //指標index放(struct的id位置)
            index_V_H=&Coulomb_Data.Voltage_H[0];  //指標index放(struct的電壓高位元位置)
            index_V_L=&Coulomb_Data.Voltage_L[0];  //指標index放(struct的電壓低位元位置)
            while(eusartRxCount != 0){
                *index = EUSART_Read(); //把讀到的餵給指標index的值
                H_L_counter++;  //放好一個值加一
                if((H_L_counter<=3)||(H_L_counter>=0x27))   index++;   //輪下一個位置
                else{
                    if(H_L_counter==4)  index=index_V_L;
                    else if((H_L_counter%2)==0){
                        index_V_L++;               //輪下一個位置
                        index=index_V_L;
                    }
                    else{
                        index_V_H++;                //輪下一個位置
                        index=index_V_H;
                    } 
                }
            }//在eusart裡會把資料丟進格式裡，那邊會每丟一個就減一，當eusartRxCount到0時跳出來
            for(j=0;j<=16;j++) {//把電芯電壓的高低位元組起來並且做一個一樣的陣列
                Get_Voltage(j);
                Voltage_sort[j] = Coulomb_Data.Voltage[j];
            }
            for(j=0;j<=16;j++){
                if(Coulomb_Data.Voltage[j]>=VoltageSet_H) PORTAbits.RA2=1;  //停止充電
                if(Coulomb_Data.Voltage[j]<=VoltageSet_L) PORTAbits.RA3=1;  //停止放電
            }
            BadBetteryFilter();  //找出壞電池
            LCD_Init(SLAVE_ADD); // Initialize LCD module with I2C address = 01001110
            
//            sprintf(&ShowbadBettery, "%d", Coulomb_Data.Voltage[0]); //那顆電池的電壓
            for(j=1; j<=17 ;j++){  //顯示誰是壞電池 和最高電池的電壓多少和壞電池壓差多少
                LCD_Clear();
                LCD_Set_Cursor(1, 1);
                LCD_Write_String("No");
                LCD_Set_Cursor(1, 3);
                sprintf(&no, "%d", j); //第幾顆電池
                LCD_Write_String(&no);
                LCD_Set_Cursor(1, 5);
                LCD_Write_String(":");
                LCD_Set_Cursor(1, 6);
                sprintf(ShowbadBettery, "%d", (Coulomb_Data.Voltage[j-1]/1000)); //那顆電池的電壓_小數點前
                LCD_Write_String(ShowbadBettery);
                LCD_Set_Cursor(1, 7);
                LCD_Write_String(".");
                LCD_Set_Cursor(1, 8);
                sprintf(ShowbadBettery, "%d", (Coulomb_Data.Voltage[j-1]%1000)); //那顆電池的電壓_小數點後
                LCD_Write_String(ShowbadBettery);
                LCD_Set_Cursor(1, 11);
                LCD_Write_String("v");
                
                LCD_Set_Cursor(2, 1);
                LCD_Write_String("^");
                LCD_Set_Cursor(2, 2);
                LCD_Write_String(&no);
                LCD_Set_Cursor(2, 4);
                LCD_Write_String(":");
                LCD_Set_Cursor(2, 5);
                sprintf(ShowbadBettery, "%d", (diffBettery[j-1]/1000)); //那顆電池的電壓差_小數點前
                LCD_Write_String(ShowbadBettery);
                LCD_Set_Cursor(2, 6);
                LCD_Write_String(".");
                LCD_Set_Cursor(2, 7);
                if((diffBettery[j-1]/10!=0)&&(diffBettery[j-1]/100==0)&&(diffBettery[j-1]/1000==0))//差0.01的等級
                    LCD_Write_String("0");
                
                else if((diffBettery[j-1]/10==0)&&(diffBettery[j-1]/100==0)&&(diffBettery[j-1]/1000==0))//差0.001的等級
                    LCD_Write_String("00");
                
                sprintf(ShowbadBettery, "%d", (diffBettery[j-1]%1000)); //那顆電池的電壓差_小數點前
                LCD_Write_String(ShowbadBettery);
                
                LCD_Set_Cursor(2, 10);
                LCD_Write_String("v");
                Delay(25000);
            }
        }
    }
    
}
void Get_Voltage(int k) {  //為了精密所以取到小數點第一位
    Coulomb_Data.Voltage[k] = (Coulomb_Data.Voltage_H[k] << 8) + Coulomb_Data.Voltage_L[k];//為了把高位跟低位元合起來，也可以把高位乘256再加低位
    if(Coulomb_Data.Voltage_Point==4)   Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/10000;	 		//電壓   
	else if(Coulomb_Data.Voltage_Point==3)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/1000;	 	//電壓  
	else if(Coulomb_Data.Voltage_Point==2)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/100;     	//電壓    
    else if(Coulomb_Data.Voltage_Point==1)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/10;     	//電壓    
	else if(Coulomb_Data.Voltage_Point==0)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/1 ;         //電壓    
}
void Sort(){
    int i, j, tmp;
    int n = 17;
    for(i = n-1; i > 0; i--){
        for(j = 0; j <= i-1; j++){
            if( Voltage_sort[j] > Voltage_sort[j+1]){
                tmp = Voltage_sort [j];
                Voltage_sort[j] = Voltage_sort[j+1];
                Voltage_sort[j+1] = tmp;
            }
        }
    }
}
void BadBetteryFilter(){
    Sort();
    for(int k=0;k<=16;k++){ //挑出最低點
        if((Voltage_sort[16]-Coulomb_Data.Voltage[k])>=Voltage_difference)   badBettery[k]=1;
        diffBettery[k]=Voltage_sort[16]-Coulomb_Data.Voltage[k];
    }
}