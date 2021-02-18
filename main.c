//######################################################//
//         ���� : �ú� UPS _ Convertet ��X�վ�
//         ��� : 2021.02.18
//         ���� : v1.0
//         �@�� : linda
//######################################################//


#include "mcc_generated_files/mcc.h"

//#define Converter_Out_Voltage    304          // =3.5*87=304 ��X�]�w��//�}�|�Q�ץX�Ӫ���
//#define Out_Voltage_difference    1           // =3 �]�����ઽ������X���Ƚը짹����w�]���Ȥ@�ˡA�ҥH���L�@�ӻ~�t��
//#define Converter_Out_RE         270          // =3.1*87=270 �n�N��X�հ��F
//#define Converter_Out_Current    6            // ��R�q�q�y�p��o�ӭȴN�n�A�~��R�q

typedef struct tagCoulomb_Data_struct {
    unsigned int   ID  ;  //�w�ۭp�^����
    unsigned int   Features ; //�\��  �w�ۭp�^����
    unsigned int   Quantity ; //�ƶq   �w�ۭp�^���Ȫ��ƶq
    unsigned int Voltage_H[32] 	; //�w�ۭp�^����
	unsigned int Voltage_L[32] 	;	//�w�ۭp�^����			
//    unsigned int Current_H	; //�w�ۭp�^����
//    unsigned int Current_L	;	//�w�ۭp�^����	
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
    unsigned int Voltage[32];  //�w�ۭp�^����
//    unsigned int Current;  //�w�ۭp�^����
} Coulomb_Data_struct_define;//�s�w���p�^����

Coulomb_Data_struct_define Coulomb_Data ; //�Τ@��struct
unsigned int Get_Voltage();  //��ƫŧi
//unsigned int Get_Current();  //��ƫŧi

void main(void)
{
    // initialize the device
    SYSTEM_Initialize();  //mcc�X�Ӫ��F��A�ΨӪ�l�ưO����]�w
//    DAC1CON0 = 0xA0; //�]�wdac
//    DAC1CON1 = 1;    //dac��0~31�����L��l�Ƭ�0�A����|��////////////////////////////////////////////////////////////////

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();//INTCONbits.GIE = 1       //�����_���\�ॴ�}

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();//INTCONbits.PEIE = 1   //�����_���\�ॴ�}

    
    unsigned char out[8]={0x01,0x03,0x00,0x03,0x00,0x20,0xB4,0x12};  
    int i,j,a=0;//a=1�ɷ|��a=0 ��q���ԧC�A�ɰ�
              //a=0�ɷ|���������԰�  
    int H_L_counter=0;
    
    while (1){
        if(TMR0_ReadTimer()==0){  //�p�ɥΪ��A�ɶ���F�N���_�A���j�@��o�e�@��
            for(i=0;i<8;i++){   // �@�}�l�g�X����ȡA�n���w���p��
                EUSART_Write(out[i]);
                NOP();
            }
        }
        if(eusartRxCount == 69){     //�p�G������9�ӭȡA�N�i��Ū�� //�쥻�O9�A�]���u��Ū�q��
            unsigned int  *index;    //����index
            index=&Coulomb_Data.ID;  //����index��(struct��id��m)
            while(eusartRxCount != 0){
                *index = EUSART_Read(); //��Ū�쪺��������index����
                H_L_counter++;  //��n�@�ӭȥ[�@
                if((H_L_counter<=3)||(H_L_counter>=0x43)){
                    index++;                //���U�@�Ӧ�m
                }
                else{
                    if((H_L_counter%2)==0)  index+=0x20;                //���U�@�Ӧ�m
                    else index-=0x1F;                //���U�@�Ӧ�m
                }
                
            }//�beusart�̷|���ƥ�i�榡�̡A����|�C��@�ӴN��@�A��eusartRxCount��0�ɸ��X��
            for(j=0;j<=31;j++){
                Get_Voltage(j);
            }
            int v=Coulomb_Data.Voltage[0];
//            Get_Current();
//            if((Converter_Out_RE > Coulomb_Data.Voltage )&& a == 1){//(1) //214>�o�쪺
//                a = 0;
//            }//out�p��214�B���bí��
//            else if((Converter_Out_Voltage-Out_Voltage_difference) > Coulomb_Data.Voltage && a == 0){//(2) //219>�o�쪺
//                if(DAC1CON1 < 0x1F) //��0x1F
//                    DAC1CON1 += 1;
//            }//�p�G��X���q���p��]�w�ȡA�N��q�����W��
//            
//            else if((Converter_Out_Voltage) <= Coulomb_Data.Voltage){//(3) //220<=�o�쪺 �B�R�q�q�y�p��]�w��
////            else if(((Converter_Out_Voltage) <= Coulomb_Data.Voltage)){//(3) //220<=�o�쪺 �B�R�q�q�y�p��]�w��
////                if(Coulomb_Data.Current < Converter_Out_Current){
//                    if(DAC1CON1 > 1)
//                        DAC1CON1 = 0;
////                }
//            }//�p�G��X���q���j�󵥩�]�w�ȡA�N��q�����U��
//            else {//(4) //��X��F214
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
// �P�_��&&�{�b���q���϶�   || a=1 |||  a=0 |||     |||  
// �����@��if               || (1) |||  (2) ||| (4) ||| (3)
//-----------------------------------------------------------
// if�̰�����               || a=0 ||| ���� ||| a=1 ||| ����
// �q���϶�                 ||     214      219     220 
//
//#################################################################//


unsigned int Get_Voltage(int k) {  //���F��K�ҥH����p���I�Ĥ@��
    Coulomb_Data.Voltage[k] = (Coulomb_Data.Voltage_H[k] << 8) + Coulomb_Data.Voltage_L[k];//���F�Ⱚ���C�줸�X�_�ӡA�]�i�H�Ⱚ�쭼256�A�[�C��
    if(Coulomb_Data.Voltage_Point==4)   Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/10000;	 		//�q��   
	else if(Coulomb_Data.Voltage_Point==3)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/1000;	 	//�q��  
	else if(Coulomb_Data.Voltage_Point==2)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/100;     	//�q��    
    else if(Coulomb_Data.Voltage_Point==1)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/10;     	//�q��    
	else if(Coulomb_Data.Voltage_Point==0)  Coulomb_Data.Voltage[k] = Coulomb_Data.Voltage[k]/1 ;         //�q��    
}

//unsigned int Get_Current() {//���F��K�ҥH����p���I�Ĥ@��
//    Coulomb_Data.Current = (Coulomb_Data.Current_H << 8) + Coulomb_Data.Current_L;//���F�Ⱚ���C�줸�X�_�ӡA�]�i�H�Ⱚ�쭼256�A�[�C��
//    if(Coulomb_Data.Current_Point==4)   Coulomb_Data.Current = Coulomb_Data.Current/10000;	 		//�q�y   
//	else if(Coulomb_Data.Current_Point==3)  Coulomb_Data.Current = Coulomb_Data.Current/1000;	 	//�q�y  
//	else if(Coulomb_Data.Current_Point==2)  Coulomb_Data.Current = Coulomb_Data.Current/100;     	//�q�y    
//    else if(Coulomb_Data.Current_Point==1)  Coulomb_Data.Current = Coulomb_Data.Current/10;     	//�q�y    
//	else if(Coulomb_Data.Current_Point==0)  Coulomb_Data.Current = Coulomb_Data.Current/1 ;         //�q�y    
//}