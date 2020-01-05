#include <wiringPi.h>
#include <softPwm.h>

#define PIN			1
#define RANGE	   	200 //1 means 100 us , 200 means 20 ms 1等于100微妙，200等于20毫秒 

void initSG90()
{
	wiringPiSetup();  //wiringpi初始化
	softPwmCreate(PIN, 10, RANGE);  //创建一个使舵机转到90的pwm输出信号
}

void hit()
{
	softPwmWrite(PIN, 5);   //将pwm输出复写为使舵机转到0
	delay(1000);
	softPwmWrite(PIN, 10);//再次复写pwm输出
}