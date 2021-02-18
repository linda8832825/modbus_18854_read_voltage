//######################################################//
//         項目 : 永磁 UPS _ Convertet 輸出調整
//         日期 : 2021.02.18
//         版本 : v1.0
//         作者 : linda
//######################################################//


#include "mcc_generated_files/mcc.h"

//#define Converter_Out_Voltage    304          // =3.5*87=304 輸出設定值//開會討論出來的值
//#define Out_Voltage_difference    1           // =3 因為不能直接讓輸出的值調到完全跟預設的值一樣，所以給他一個誤差值
//#define Converter_Out_RE         270          // =3.1*87=270 要將輸出調高了
//#define Converter_Out_Current    6            // 當充電電流小於這個值就要再繼續充電

typedef struct tagCoulomb_Data_struct {
    unsigned int   ID  ;  //庫倫計回的值
    unsigned int   Features ; //功能  庫倫計回的值
    unsigned int   Quantity ; //數量   庫倫計回的值的數量
    unsigned int Voltage_H[32] 	; //庫倫計回的值
	unsigned int Voltage_L[32] 	;	//庫倫計回的值			
//    unsigned int Current_H	; //庫倫計回的值
//    unsigned int Current_L	;	//庫倫計回的值	
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
    unsigned int Voltage[32];  //庫倫計回的值
//    unsigned int Current;  //庫倫計回的值
} Coulomb_Data_struct_define;//存庫輪計回的值

Coulomb_Data_struct_define Coulomb_Data ; //用一個struct
unsigned int Get_Voltage();  //函數宣告
//unsigned int Get_Current();  //函數宣告

void main(void)
{
    // initialize the device
    SYSTEM_Initialize();  //mcc出來的東西，用來初始化記憶體設定
//    DAC1CON0 = 0xA0; //設定dac
//    DAC1CON1 = 1;    //dac調0~31的讓他初始化為0，之後會改////////////////////////////////////////////////////////////////

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();//INTCONbits.GIE = 1       //讓中斷的功能打開

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();//INTCONbits.PEIE = 1   //讓中斷的功能打開

    
    unsigned char out[8]={0x01,0x03,0x00,0x03,0x00,0x20,0xB4,0x12};  
    int i,j,a=0;//a=1時會先a=0 把電壓拉低再升高
              //a=0時會先把變壓拉高  
    int H_L_counter=0;
    
    while (1){
        if(TMR0_ReadTimer()==0){  //計時用的，時間到了就中斷，間隔一秒發送一次
            for(i=0;i<8;i++){   // 一開始寫出那行值，要給庫輪計的
                EUSART_Write(out[i]);
                NOP();
            }
        }
        if(eusartRxCount == 69){     //如果接收到9個值，就進來讀值 //原本是9，因為只有讀電壓
            unsigned int  *index;    //指標index
            index=&Coulomb_Data.ID;  //指標index放(struct的id位置)
            while(eusartRxCount != 0){
                *index = EUSART_Read(); //把讀到的餵給指標index的值
                H_L_counter++;  //放好一個值加一
                if((H_L_counter<=3)||(H_L_counter>=0x43)){
                    index++;                //輪下一個位置
                }
                else{
                    if((H_L_counter%2)==0)  index+=0x20;                //輪下一個位置
                    else index-=0x1F;                //輪下一個位置
                }
                
            }//在eusart裡會把資料丟進格式裡，那邊會每丟一個就減一，當eusartRxCount到0時跳出來
            for(j=0;j<=31;j++){
                Get_Voltage(j);
            }
            int v=Coulomb_Data.Voltage[0];
//            Get_Current();
//            if((Converter_Out_RE > Coulomb_Data.Voltage )&& a == 1){//(1) //214>得到的
//                a = 0;
//            }//out小於214且正在穩壓
//            else if((Converter_Out_Voltage-Out_Voltage_difference) > Coulomb_Data.Voltage && a == 0){//(2) //219>得到的
//                if(DAC1CON1 < 0x1F) //原0x1F
//                    DAC1CON1 += 1;
//            }//如果輸出的電壓小於設定值，就把電壓往上升
//            
//            else if((Converter_Out_Voltage) <= Coulomb_Data.Voltage){//(3) //220<=得到的 且充電電流小於設定值
////            else if(((Converter_Out_Voltage) <= Coulomb_Data.Voltage)){//(3) //220<=得到的 且充電電流小於設定值
////                if(Coulomb_Data.Current < Converter_Out_Current){
//                    if(DAC1CON1 > 1)
//                        DAC1CON1 = 0;
////                }
//            }//如果輸出的電壓大於等於設定值，就把電壓往下降
//            else {//(4) //輸出到了214
//                if(!a){
//                    DAC1CON1 = 1;
//                    a = 1;
//                }
//            }
        }
    }
}
//#################################################################//
//
// 判斷式&&現在的電壓區間   || a=1 |||  a=0 |||     |||  
// 做哪一個if               || (1) |||  (2) ||| (4) ||| (3)
//-----------------------------------------------------------
// if裡做的事               || a=0 ||| 壓升 ||| a=1 ||| 壓降
// 電壓區間                 ||     214      219     220 
//
//#################################################################//


unsigned int Get_Voltage(int k) {  //為了精密所以取到小數點第一位
    Coulomb_Data.Voltage[k] = (Coulomb_Data.Voltage_H[k] << 8) + Coulomb_Data.Voltage_L[k];//為了把高位跟低位元合起來，也可以把高位乘256再加低位
    if(Coulomb_Data.Voltage_Point==4)   Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/10000;	 		//電壓   
	else if(Coulomb_Data.Voltage_Point==3)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/1000;	 	//電壓  
	else if(Coulomb_Data.Voltage_Point==2)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/100;     	//電壓    
    else if(Coulomb_Data.Voltage_Point==1)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/10;     	//電壓    
	else if(Coulomb_Data.Voltage_Point==0)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/1 ;         //電壓    
}

//unsigned int Get_Current() {//為了精密所以取到小數點第一位
//    Coulomb_Data.Current = (Coulomb_Data.Current_H << 8) + Coulomb_Data.Current_L;//為了把高位跟低位元合起來，也可以把高位乘256再加低位
//    if(Coulomb_Data.Current_Point==4)   Coulomb_Data.Current = Coulomb_Data.Current/10000;	 		//電流   
//	else if(Coulomb_Data.Current_Point==3)  Coulomb_Data.Current = Coulomb_Data.Current/1000;	 	//電流  
//	else if(Coulomb_Data.Current_Point==2)  Coulomb_Data.Current = Coulomb_Data.Current/100;     	//電流    
//    else if(Coulomb_Data.Current_Point==1)  Coulomb_Data.Current = Coulomb_Data.Current/10;     	//電流    
//	else if(Coulomb_Data.Current_Point==0)  Coulomb_Data.Current = Coulomb_Data.Current/1 ;         //電流    
//}