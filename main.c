//######################################################//
//         項目 : 永磁 UPS _ Convertet 輸出調整
//         日期 : 2021.02.18
//         版本 : v1.0
//         作者 : linda
//######################################################//


#include "mcc_generated_files/mcc.h"

#define VoltageSet_H          3500          //充電最高點
#define VoltageSet_L          2500          //充電最低點
#define Voltage_difference    150          //單電芯壓差容忍值
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
//    unsigned int Current;  //庫倫計回的值
} Coulomb_Data_struct_define;//存庫輪計回的值

Coulomb_Data_struct_define Coulomb_Data ; //用一個struct
void Get_Voltage(int);  //函數宣告
void BadBetteryFilter();  //函數宣告
void quickSort(int left,int right);  //函數宣告
unsigned int Voltage_sort[17]={0};
unsigned int badBettery[17]={0};
unsigned int mid=0;
void main(void)
{
    SYSTEM_Initialize();  //mcc出來的東西，用來初始化記憶體設定
    INTERRUPT_GlobalInterruptEnable();//INTCONbits.GIE = 1       //讓中斷的功能打開
    INTERRUPT_PeripheralInterruptEnable();//INTCONbits.PEIE = 1   //讓中斷的功能打開

    unsigned char out[8]={0x01,0x03,0x00,0x03,0x00,0x11,0x9F,0x9C};  
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
        if(eusartRxCount == 39){     //如果接收到9個值，就進來讀值 //原本是9，因為只有讀電壓
            unsigned int  *index;    //指標index
            index=&Coulomb_Data.ID;  //指標index放(struct的id位置)
            while(eusartRxCount != 0){
                *index = EUSART_Read(); //把讀到的餵給指標index的值
                H_L_counter++;  //放好一個值加一
                if((H_L_counter<=3)||(H_L_counter>=0x27))   index++;   //輪下一個位置
                else{
                    if((H_L_counter%2)==0)  index+=0x11;               //輪下一個位置
                    else index-=0x10;                //輪下一個位置
                }
            }//在eusart裡會把資料丟進格式裡，那邊會每丟一個就減一，當eusartRxCount到0時跳出來
            for(j=0;j<=17;j++) {//把電芯電壓的高低位元組起來並且做一個一樣的陣列
                Get_Voltage(j);
                Voltage_sort[j] = Coulomb_Data.Voltage[j];
            }
            for(j=0;j<=17;j++){
                if(Coulomb_Data.Voltage[j]>=VoltageSet_H) PORTAbits.RA0=1;  //停止充電
                if(Coulomb_Data.Voltage[j]<=VoltageSet_L) PORTAbits.RA1=1;  //停止放電
            }
            BadBetteryFilter();
            //顯示誰是壞電池 和最高電池的電壓多少和壞電池壓差多少
            ////////////////////////////////////////////////////////
        }
    }
}
void Get_Voltage(int k) {  //為了精密所以取到小數點第一位
    Coulomb_Data.Voltage[k] = (Coulomb_Data.Voltage_H[k] << 8) + Coulomb_Data.Voltage_L[k];//為了把高位跟低位元合起來，也可以把高位乘256再加低位
//    if(Coulomb_Data.Voltage_Point==4)   Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/10000;	 		//電壓   
//	else if(Coulomb_Data.Voltage_Point==3)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/1000;	 	//電壓  
//	else if(Coulomb_Data.Voltage_Point==2)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/100;     	//電壓    
//    else if(Coulomb_Data.Voltage_Point==1)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/10;     	//電壓    
//	else if(Coulomb_Data.Voltage_Point==0)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/1 ;         //電壓    
}
void quickSort(int left,int right){ //輸入資料，和從兩邊開始的位置
    int i = left;                      //左邊的代理人
    int j = right;                     //右邊的代理人
    int temp = 0;                      //暫存位
    int key = Voltage_sort[left] ;                //基準點
    while(i != j){                  
        while((Voltage_sort[j] > key) && (i < j))  j -= 1; //從右邊開始找，找比基準點小的值
        while((Voltage_sort[i] <= key) && (i < j)) i += 1; //從左邊開始找，找比基準點大的值
        if(i < j) {//當左右代理人沒有相遇時，互換值
            temp = Voltage_sort[i];
            Voltage_sort[i] = Voltage_sort[j];
            Voltage_sort[j] = temp;                 
        }
    }        
    //將基準點歸換至代理人相遇點
    Voltage_sort[left] = Voltage_sort[i]; 
    Voltage_sort[i] = key;
    mid=i;
}
void BadBetteryFilter(){
    quickSort(0,17);
    quickSort(0, mid-1);   //繼續處理較小部分的子循環
    quickSort(mid+1, 17);  //繼續處理較大部分的子循環
    for(int k=0;k<=17;k++){ //挑出最低點
        if((Voltage_sort[16]-Coulomb_Data.Voltage[k])>=Voltage_difference)   badBettery[k]=1;
    }
}