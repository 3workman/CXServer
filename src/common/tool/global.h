#pragma once
#ifdef GLOBAL_VAR
	#define GLOBAL_EXTERN
#else
	#define GLOBAL_EXTERN extern
#endif

//���ﶨ��ϵͳ�е�����ȫ�ֱ���������
GLOBAL_EXTERN int g_system_state;

/*��cpp�У�����ͷ�ļ���
	#define GLOBAL_VAR
	#include "global.h"
	#undef GLOBAL_VAR
*/