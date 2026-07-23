//**************************************************************************/
//����������ϵͳ��ơ�����ʵ��
//�ļ����ƣ�tick.c
//���ܸ�Ҫ��1msʱ�����Դ�ļ�
//������Ա��zyc
//�������ڣ�2026-5-17
/***************************************************************************/
//1.���������ͷ�ļ�
#include "tick.h"
//#include "tim.h"

extern TIM_HandleTypeDef htim3;

//2) ����ӿ����ݶ���   
union time_flag_t	time_flag;

static unsigned char t1ms_cnt;	//1ms��ʱ����
static unsigned char t2ms_cnt;
static unsigned char t5ms_cnt;
static unsigned char t10ms_cnt;
static unsigned char t50ms_cnt;
static unsigned char t100ms_cnt;
static unsigned char t200ms_cnt;
static unsigned short t500ms_cnt;
static unsigned short t1000ms_cnt;	//1s��ʱ����


//3) ����ӿں�������
void tick_init(void)
{
	HAL_TIM_Base_Start_IT(&htim3);
}

/* ��ʱ������ж��жϻص����� */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
			t1ms_cnt++;
		if(t1ms_cnt >= 1)	
		{
			t1ms_cnt = 0;
			time_flag.bit_field.time_1ms_flag = 1;
		}
		t2ms_cnt++;
		if(t2ms_cnt >= 2)	
		{
			t2ms_cnt = 0;
			time_flag.bit_field.time_2ms_flag = 1;
		}
		t5ms_cnt++;
		if(t5ms_cnt >= 5)	
		{
			t5ms_cnt = 0;
			time_flag.bit_field.time_5ms_flag = 1;
		}
		t10ms_cnt++;
		if(t10ms_cnt >= 10)	
		{
			t10ms_cnt = 0;
			time_flag.bit_field.time_10ms_flag = 1;
		}
		t50ms_cnt++;
		if(t50ms_cnt >= 50)	
		{
			t50ms_cnt = 0;
			time_flag.bit_field.time_50ms_flag = 1;
		}
		t100ms_cnt++;
		if(t100ms_cnt >= 100)
		{
			t100ms_cnt = 0;
			time_flag.bit_field.time_100ms_flag = 1;
		}
		t200ms_cnt++;
		if(t200ms_cnt >= 200)
		{
			t200ms_cnt = 0;
			time_flag.bit_field.time_200ms_flag = 1;
		}
		t500ms_cnt++;
		if(t500ms_cnt >= 500)	
		{
			t500ms_cnt = 0;
			time_flag.bit_field.time_500ms_flag = 1;
		}
		t1000ms_cnt++;
		if(t1000ms_cnt >= 1000)	
		{
			t1000ms_cnt = 0;
			time_flag.bit_field.time_1000ms_flag = 1;
		}
    }
}
