//**************************************************************************/
//《单片机技术》基础实验
//文件名称：tick.h
//功能概要：1ms时基组件头文件
//开发人员：zyc
//开发日期：2026-5-17
/***************************************************************************/
#ifndef  __TICK_H    //防止重复定义(开头)
#define  __TICK_H

//1)   包含头文件
#include "main.h"

//2) 对外接口数据定义   
union time_flag_t
{
	unsigned char byte;
	struct 
	{	
		unsigned char time_1ms_flag			:	1;
		unsigned char time_2ms_flag			:	1;
		unsigned char time_5ms_flag			:	1;
		unsigned char time_10ms_flag		:	1;
		unsigned char time_50ms_flag		:	1;
		unsigned char time_100ms_flag		:	1;			
		unsigned char time_500ms_flag		:	1;
		unsigned char time_1000ms_flag		:	1;
	}bit_field;
};
extern union time_flag_t	time_flag;

//3) 对外接口函数声明    
extern void tick_init(void); 

#endif  //防止重复定义(结尾)
