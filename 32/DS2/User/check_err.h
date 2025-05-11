#ifndef _check_err_H_
#define _check_err_H_

#include <stm32f10x.h>

#define Check_OK	0
#define ERR_R1_Short	1
#define ERR_R2_Short	2
#define ERR_R1_Open		3
#define ERR_R2_Open		4
#define ERR_R3_Short	5
#define ERR_R3_Open		6
#define ERR_R4_Short	7
#define ERR_R4_Open		8
#define ERR_C1_Open		9
#define ERR_C2_Open		10
#define ERR_C3_Open		11
#define ERR_C1_Double	12
#define ERR_C2_Double	13
#define ERR_C3_Double	14

extern u8 check_err(void);
extern void display_err_info(u8 err);
extern u8 check_err1(void);
extern u8 check_err2(void);

#endif
