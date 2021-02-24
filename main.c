//######################################################//
//         ���� : �ú� UPS _ Convertet ��X�վ�
//         ��� : 2021.02.18
//         ���� : v1.0
//         �@�� : linda
//######################################################//


#include "mcc_generated_files/mcc.h"
#include <stdlib.h>
#include "I2C_LCD.h"

#define VoltageSet_H          3500          //�R�q�̰��I
#define VoltageSet_L          2500          //�R�q�̧C�I
#define Voltage_difference    150           //��q�����t�e�ԭ�
#define SLAVE_ADD 0x4E                      //�ù���ID

typedef struct tagCoulomb_Data_struct {
    unsigned int   ID  ;  //�w�ۭp�^����
    unsigned int   Features ; //�\��  �w�ۭp�^����
    unsigned int   Quantity ; //�ƶq   �w�ۭp�^���Ȫ��ƶq
    unsigned int Voltage_H[17] 	; //�w�ۭp�^����
	unsigned int Voltage_L[17] 	;	//�w�ۭp�^����		
    unsigned int CRC_H;  //�w�ۭp�^����
    unsigned int CRC_L;  //�w�ۭp�^����
    union {
		unsigned int _Voltage;
		struct {
			unsigned Voltage_Point	:4;		//�q����p���I  
			unsigned Voltage_statue	:4;		//		
		};
	};
    union {
		unsigned int _Current;
		struct {
			unsigned Current_Point	:4;		//�q�y��p���I  
			unsigned Current_statue	:4;		//�ݹq�y�O���٭t
		};
	}; //�p�X�D�n�ΨӪ�ܦP���������P�������������
    unsigned int Voltage[17];  //�w�ۭp�^����
} Coulomb_Data_struct_define;//�s�w���p�^����

Coulomb_Data_struct_define Coulomb_Data ; //�Τ@��struct
void Get_Voltage(int);  //��ƫŧi
void BadBetteryFilter();  //��ƫŧi
void Sort();  //��ƫŧi
unsigned int Voltage_sort[17]={0};
unsigned int badBettery[17]={0};
unsigned int diffBettery[17]={0};
char ShowbadBettery[4]={0};
char no;
void main(void)
{
    SYSTEM_Initialize();  //mcc�X�Ӫ��F��A�ΨӪ�l�ưO����]�w
    INTERRUPT_GlobalInterruptEnable();//INTCONbits.GIE = 1       //�����_���\�ॴ�}
    INTERRUPT_PeripheralInterruptEnable();//INTCONbits.PEIE = 1   //�����_���\�ॴ�}

    unsigned char out[8]={0x01,0x03,0x00,0x03,0x00,0x11,0x75,0xC6};  //�q��3����ƨ�17���X��
    int i,j,H_L_counter=0;
    
    while (1){
        H_L_counter=0;
        if(TMR0_ReadTimer()==0){  //�p�ɥΪ��A�ɶ���F�N���_�A���j�@��o�e�@��
            for(i=0;i<8;i++){   // �@�}�l�g�X����ȡA�n���w���p��
                EUSART_Write(out[i]);
                NOP();
            }
        }
        if(eusartRxCount == 39){     //�p�G������9�ӭȡA�N�i��Ū�� //�쥻�O9�A�]���u��Ū�q��
            unsigned int  *index;    //����index
            unsigned int  *index_V_H;    //����index
            unsigned int  *index_V_L;    //����index
            index=&Coulomb_Data.ID;  //����index��(struct��id��m)
            index_V_H=&Coulomb_Data.Voltage_H[0];  //����index��(struct���q�����줸��m)
            index_V_L=&Coulomb_Data.Voltage_L[0];  //����index��(struct���q���C�줸��m)
            while(eusartRxCount != 0){
                *index = EUSART_Read(); //��Ū�쪺��������index����
                H_L_counter++;  //��n�@�ӭȥ[�@
                if((H_L_counter<=3)||(H_L_counter>=0x27))   index++;   //���U�@�Ӧ�m
                else{
                    if(H_L_counter==4)  index=index_V_L;
                    else if((H_L_counter%2)==0){
                        index_V_L++;               //���U�@�Ӧ�m
                        index=index_V_L;
                    }
                    else{
                        index_V_H++;                //���U�@�Ӧ�m
                        index=index_V_H;
                    } 
                }
            }//�beusart�̷|���ƥ�i�榡�̡A����|�C��@�ӴN��@�A��eusartRxCount��0�ɸ��X��
            for(j=0;j<=16;j++) {//��q��q�������C�줸�հ_�ӨåB���@�Ӥ@�˪��}�C
                Get_Voltage(j);
                Voltage_sort[j] = Coulomb_Data.Voltage[j];
            }
            for(j=0;j<=16;j++){
                if(Coulomb_Data.Voltage[j]>=VoltageSet_H) PORTAbits.RA2=1;  //����R�q
                if(Coulomb_Data.Voltage[j]<=VoltageSet_L) PORTAbits.RA3=1;  //�����q
            }
            BadBetteryFilter();  //��X�a�q��
            LCD_Init(SLAVE_ADD); // Initialize LCD module with I2C address = 01001110
            
//            sprintf(&ShowbadBettery, "%d", Coulomb_Data.Voltage[0]); //�����q�����q��
            for(j=1; j<=17 ;j++){  //��ܽ֬O�a�q�� �M�̰��q�����q���h�֩M�a�q�����t�h��
                LCD_Clear();
                LCD_Set_Cursor(1, 1);
                LCD_Write_String("No");
                LCD_Set_Cursor(1, 3);
                sprintf(&no, "%d", j); //�ĴX���q��
                LCD_Write_String(&no);
                LCD_Set_Cursor(1, 5);
                LCD_Write_String(":");
                LCD_Set_Cursor(1, 6);
                sprintf(ShowbadBettery, "%d", (Coulomb_Data.Voltage[j-1]/1000)); //�����q�����q��_�p���I�e
                LCD_Write_String(ShowbadBettery);
                LCD_Set_Cursor(1, 7);
                LCD_Write_String(".");
                LCD_Set_Cursor(1, 8);
                sprintf(ShowbadBettery, "%d", (Coulomb_Data.Voltage[j-1]%1000)); //�����q�����q��_�p���I��
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
                sprintf(ShowbadBettery, "%d", (diffBettery[j-1]/1000)); //�����q�����q���t_�p���I�e
                LCD_Write_String(ShowbadBettery);
                LCD_Set_Cursor(2, 6);
                LCD_Write_String(".");
                LCD_Set_Cursor(2, 7);
                if((diffBettery[j-1]/10!=0)&&(diffBettery[j-1]/100==0)&&(diffBettery[j-1]/1000==0))//�t0.01������
                    LCD_Write_String("0");
                
                else if((diffBettery[j-1]/10==0)&&(diffBettery[j-1]/100==0)&&(diffBettery[j-1]/1000==0))//�t0.001������
                    LCD_Write_String("00");
                
                sprintf(ShowbadBettery, "%d", (diffBettery[j-1]%1000)); //�����q�����q���t_�p���I�e
                LCD_Write_String(ShowbadBettery);
                
                LCD_Set_Cursor(2, 10);
                LCD_Write_String("v");
                Delay(25000);
            }
        }
    }
    
}
void Get_Voltage(int k) {  //���F��K�ҥH����p���I�Ĥ@��
    Coulomb_Data.Voltage[k] = (Coulomb_Data.Voltage_H[k] << 8) + Coulomb_Data.Voltage_L[k];//���F�Ⱚ���C�줸�X�_�ӡA�]�i�H�Ⱚ�쭼256�A�[�C��
    if(Coulomb_Data.Voltage_Point==4)   Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/10000;	 		//�q��   
	else if(Coulomb_Data.Voltage_Point==3)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/1000;	 	//�q��  
	else if(Coulomb_Data.Voltage_Point==2)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/100;     	//�q��    
    else if(Coulomb_Data.Voltage_Point==1)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/10;     	//�q��    
	else if(Coulomb_Data.Voltage_Point==0)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/1 ;         //�q��    
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
    for(int k=0;k<=16;k++){ //�D�X�̧C�I
        if((Voltage_sort[16]-Coulomb_Data.Voltage[k])>=Voltage_difference)   badBettery[k]=1;
        diffBettery[k]=Voltage_sort[16]-Coulomb_Data.Voltage[k];
    }
}