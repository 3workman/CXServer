#pragma once
#ifdef GLOBAL_VAR
	#define GLOBAL_EXTERN
#else
	#define GLOBAL_EXTERN extern
#endif

//这里定义系统中的所有全局变量、函数
GLOBAL_EXTERN int g_system_state;

/*【cpp中，包含头文件】
	#define GLOBAL_VAR
	#include "global.h"
	#undef GLOBAL_VAR
*/