#include <wiringPi.h>
#include <softPwm.h>

#define PIN			1
#define RANGE	   	200 //1 means 100 us , 200 means 20 ms 1等于100微妙，200等于20毫秒 

void initSG90()
{
	wiringPiSetup();  //wiringpi初始化
}

void hit()
{
	softPwmCreate(PIN, 5, RANGE);  //创建一个使舵机转到90的pwm输出信号
	delay(1000);
	softPwmWrite(PIN, 10);//再次复写pwm输出
	delay(1000);
	pinMode(PIN, INPUT);// 停止 PWM 信号输出, 不然会时不时抖动一下
}