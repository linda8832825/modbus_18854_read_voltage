//######################################################//
//         項目 : G5電芯測試製具
//         日期 : 2021.02.25
//         版本 : v1.0
//         作者 : linda
//######################################################//


#include "mcc_generated_files/mcc.h"
#include <stdlib.h>
#include "I2C_LCD.h"

#define VoltageSet_H          3500          //充電最高點
#define VoltageSet_L          2500          //充電最低點
#define Voltage_difference    200           //單電芯壓差容忍值
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
void Get_Voltage(int);  //組合讀進來的電壓
void Sort();  //排序所有讀進來的電壓
void BadBetteryFilter();  //放電過程中計算計算差值與差值顯示電壓
void ShowBetteryDiff();  //放電過程中顯示電壓與差值
void ShowAllBadBettery();  //放電完顯示所有壞電池
unsigned int Voltage_sort[17]={0}; //排序讀進來的電壓
unsigned int badBettery[17]={0};  //哪幾個是壞電池
unsigned int diffBettery[17]={0};  //電池壓差
unsigned int diffBadBettery[17]={0};  //壞電池的最壞壓差
unsigned int diffHighBettery[17]={0};  //出現壞電池的最壞壓差時的最高電壓
char ShowbadBettery[4]={0};  //顯示電壓的暫存的字串
char no;  //顯示電芯個數的字串
double hour=0.00; //計算放電了幾小時
double Ah=0.00;
void main(void)
{
    SYSTEM_Initialize();  //mcc出來的東西，用來初始化記憶體設定
    INTERRUPT_GlobalInterruptEnable();//INTCONbits.GIE = 1       //讓中斷的功能打開
    INTERRUPT_PeripheralInterruptEnable();//INTCONbits.PEIE = 1   //讓中斷的功能打開

    unsigned char out[8]={0x01,0x03,0x00,0x03,0x00,0x11,0x75,0xC6};  //從第3筆資料取17筆出來
    int i,j,H_L_counter=0;
    
    if(RA0==0){ //啟動鈕被按下，開始充電
        while (1){
            H_L_counter=0;
            if(TMR0_ReadTimer()==0){  //計時用的，時間到了就中斷，間隔一秒發送一次
                for(i=0;i<8;i++){  //跟庫倫計要資料
                    EUSART_Write(out[i]);
                    NOP();
                }
            }
            if(eusartRxCount == 39){     //3+17*2+2如果接收到9個值，就進來讀值 //原本是9，因為只有讀電壓
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
                for(j=0;j<=16;j++) { //把電芯電壓的高低位元組起來並且做一個一樣的陣列用來放排序的
                    Get_Voltage(j);
                    Voltage_sort[j] = Coulomb_Data.Voltage[j];
                }
                for(j=0;j<=16;j++){ //充放電控制
                    if(Coulomb_Data.Voltage[j]>=VoltageSet_H) {
                        LATA2=0;  //停止充電
                        Delay(25000);
                        LATA3=1;  //開始放電
                    }
                    if(Coulomb_Data.Voltage[j]<=VoltageSet_L) {
                        LATA3=0;  //停止放電
                        hour=sec/3600; //放電小時數
                        Ah=hour*2.5;  //放電安時數
                        LATA5 = 1; //蜂鳴器叫
                        ShowAllBadBettery(); //顯示所有壞電池
                    }
                }
                BadBetteryFilter();  //找出壞電池
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
        if((Voltage_sort[16]-Coulomb_Data.Voltage[k])>=Voltage_difference)   badBettery[k]=1; //紀錄有出現過問題的個數
        diffBettery[k] = Voltage_sort[16]-Coulomb_Data.Voltage[k]; //計算差多少
        if((LATA3==1)&&(diffBettery[k] >= Voltage_difference) && (diffBettery[k] > diffBadBettery[k])){ //在放電時紀錄有電壓差的最差狀況
            diffBadBettery[k] = diffBettery[k];
            diffHighBettery[k] = Voltage_sort[16];
        }
    }
    ShowBetteryDiff(); //放電過程中的最差值顯示
}
void ShowBetteryDiff(){ //放電過程中的最差值顯示
    int lineCount=1;
    LCD_Init(SLAVE_ADD); // Initialize LCD module with I2C address = 01001110
    
    for(int j=1; j<=17 ;j++){  //顯示誰是壞電池 和最高電池的電壓多少和壞電池壓差多少
        LCD_Clear();
        LCD_Set_Cursor(lineCount, 1);
        LCD_Write_String("No");
        LCD_Set_Cursor(lineCount, 3);
        sprintf(&no, "%d", j); //第幾顆電池
        LCD_Write_String(&no);
        LCD_Set_Cursor(lineCount, 5);
        LCD_Write_String(":");
        
        LCD_Set_Cursor(lineCount, 6);
        sprintf(ShowbadBettery, "%d", (Coulomb_Data.Voltage[j-1]/1000)); //那顆電池的電壓_小數點前
        LCD_Write_String(ShowbadBettery);
        LCD_Set_Cursor(lineCount, 7);
        LCD_Write_String(".");
        LCD_Set_Cursor(lineCount, 8);
        sprintf(ShowbadBettery, "%d", (Coulomb_Data.Voltage[j-1]%1000)); //那顆電池的電壓_小數點後
        LCD_Write_String(ShowbadBettery);
        LCD_Set_Cursor(lineCount, 11);
        LCD_Write_String("v");

        LCD_Set_Cursor(lineCount+1, 1);
        LCD_Write_String("^");
        LCD_Set_Cursor(lineCount+1, 2);
        LCD_Write_String(&no);
        LCD_Set_Cursor(lineCount+1, 4);
        LCD_Write_String(":");
        
        LCD_Set_Cursor(lineCount+1, 5);
        sprintf(ShowbadBettery, "%d", (diffBettery[j-1]/1000)); //那顆電池的電壓差_小數點前
        LCD_Write_String(ShowbadBettery);
        
        LCD_Set_Cursor(lineCount+1, 6);
        LCD_Write_String(".");
        LCD_Set_Cursor(lineCount+1, 7);
        if((diffBettery[j-1]/10!=0)&&(diffBettery[j-1]/100==0)&&(diffBettery[j-1]/1000==0))//差0.01的等級
            LCD_Write_String("0");

        else if((diffBettery[j-1]/10==0)&&(diffBettery[j-1]/100==0)&&(diffBettery[j-1]/1000==0))//差0.001的等級
            LCD_Write_String("00");

        sprintf(ShowbadBettery, "%d", (diffBettery[j-1]%1000)); //那顆電池的電壓差_小數點前
        LCD_Write_String(ShowbadBettery);

        LCD_Set_Cursor(lineCount+1, 10);
        LCD_Write_String("v");
        lineCount++;
        if(lineCount==4){
            lineCount=1;
            Delay(25000);
            LCD_Clear();
        }
        
    }
}
void ShowAllBadBettery(){ //放電完顯示所有壞電池
    int line_Count=1;
    int betteryCount=1;
    LCD_Init(SLAVE_ADD); // Initialize LCD module with I2C address = 01001110
    while(1){
        LCD_Set_Cursor(1, 1);
        LCD_Write_String("DONE!!");
        while(1){
            while(LATA4==0){ //下一頁鈕被按下
                LATA5 = 0; //蜂鳴器叫
                LCD_Clear();
                if(badBettery[betteryCount]){//有問題的電芯
                    LCD_Set_Cursor(line_Count, 1);
                    LCD_Write_String("^");
                    LCD_Set_Cursor(line_Count, 2);
                    sprintf(&no, "%d", betteryCount); //第幾顆電池
                    LCD_Write_String(&no);
                    LCD_Set_Cursor(line_Count, 4);
                    LCD_Write_String(":");

                    LCD_Set_Cursor(line_Count, 5);
                    sprintf(ShowbadBettery, "%d", (diffBadBettery[betteryCount-1]/1000)); //那顆電池的電壓差_小數點前
                    LCD_Write_String(ShowbadBettery);

                    LCD_Set_Cursor(line_Count, 6);
                    LCD_Write_String(".");

                    LCD_Set_Cursor(line_Count, 7);
                    sprintf(ShowbadBettery, "%d", (diffBadBettery[betteryCount-1]%1000)); //那顆電池的電壓差_小數點前
                    LCD_Write_String(ShowbadBettery);

                    LCD_Set_Cursor(line_Count, 11);
                    LCD_Write_String("(MaxV:");

                    LCD_Set_Cursor(line_Count, 17);
                    sprintf(ShowbadBettery, "%d", (diffBadBettery[betteryCount-1]/1000)); //那時候的最高電壓_小數點前
                    LCD_Write_String(ShowbadBettery);

                    LCD_Set_Cursor(line_Count, 18);
                    LCD_Write_String("(.");

                    LCD_Set_Cursor(line_Count, 19);
                    sprintf(ShowbadBettery, "%d", ((diffBadBettery[betteryCount-1]%1000)/10)); //那時候的最高電壓_小數點後
                    LCD_Write_String(ShowbadBettery);

                    line_Count++; //下一次顯示在下一行
                    betteryCount++; //有問題的計數
                }
                if(line_Count==4) {
                    line_Count=1;  //歸回第一行
                    if(LATA4==0) continue;//下一頁鈕被按下
                }
                if(betteryCount==17) {
                    while(1){
                        while(LATA4==0){
                            LCD_Clear();
                            LCD_Set_Cursor(1, 1);
                            sprintf(ShowbadBettery, "%1.2f", Ah); //安時數的
                            LCD_Write_String(ShowbadBettery);
                            while(1){
                                while(LATA4==0) break; //數到最後一顆了  
                                break;
                            }
                            break;
                        }
                        break;
                    }
                }
            }
            break;
        }
        continue;
    }
}