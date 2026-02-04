#ifndef __SERVO_H__
#define __SERVO_H__


//#define SERVO_YAW_KP 1
//#define SERVO_YAW_KI 0
//#define SERVO_YAW_KD 0

#define TEST_MODE 0

void servo_ini();
void servo_limit();
void servo_control();

void servo_count();				

#endif
