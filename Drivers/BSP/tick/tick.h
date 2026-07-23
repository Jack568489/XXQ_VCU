//**************************************************************************/
//����Ƭ������������ʵ��
//�ļ����ƣ�tick.h
//���ܸ�Ҫ��1msʱ�����ͷ�ļ�
//������Ա��zyc
//�������ڣ�2026-5-17
/***************************************************************************/
#ifndef  __TICK_H    //��ֹ�ظ�����(��ͷ)
#define  __TICK_H

//1)   ����ͷ�ļ�
#include "main.h"

//2) ����ӿ����ݶ���
union time_flag_t
{
	unsigned short byte;
	struct
	{
		unsigned char time_1ms_flag			:	1;
		unsigned char time_2ms_flag			:	1;
		unsigned char time_5ms_flag			:	1;
		unsigned char time_10ms_flag		:	1;
		unsigned char time_50ms_flag		:	1;
		unsigned char time_100ms_flag		:	1;
		unsigned char time_200ms_flag		:	1;
		unsigned char time_500ms_flag		:	1;
		unsigned char time_1000ms_flag		:	1;
	}bit_field;
};
extern union time_flag_t	time_flag;

//3) ����ӿں�������    
extern void tick_init(void); 

#endif  //��ֹ�ظ�����(��β)
