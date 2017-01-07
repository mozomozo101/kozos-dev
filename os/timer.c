#include "defines.h"
#include "lib.h"
#include "timer.h"
#include "interrupt.h"

#define TIMER_REG01 ((volatile struct timer_reg *)0xffff80)


struct timer_reg {
	volatile uint8 tcr0;
	volatile uint8 tcr1;
	volatile uint8 tcsr0;
	volatile uint8 tcsr1;
	volatile uint8 tcora0;
	volatile uint8 tcora1;
	volatile uint8 tcorb0;
	volatile uint8 tcorb1;
	volatile uint8 tcnt0;
	volatile uint8 tcnt1;
};


// タイマ終了
static void timer_cancel(void)
{
	volatile struct timer_reg *timer = TIMER_REG01;
	timer->tcsr0 = 0;	// CMFA（コンペアマッチ）フラグを落とす
	timer->tcr0 = 0;	// クロック入力を禁止に
	timer->tcr1 = 0;	// クロック入力を禁止に
	timer->tcsr0 = 0;
	timer->tcsr1 = 0;
}


// タイマーが切れたら
void timer_intr(softvec_type_t type, unsigned long sp)
{
	puts("timer expired!\n");
	timer_cancel();
}


void timer_start(int second) {

	// 設定の順番によってはうまく動作しなくなるかも・・・
	volatile struct timer_reg *timer = TIMER_REG01;
	timer->tcnt0 = 0;
	timer->tcnt1 = 0;
	timer->tcsr0 = 0;
	timer->tcsr1 = 0;

	// tcnt0の1sあたりのカウント数： (20 * 10^6 / 8192 /256) ≒ 10
	timer->tcora0 = 10 * second;
	
	// tcnt0:
	// 		クロック入力は、tcnt1のオーバーフロー信号
	// 		tcnt0, 1 は、tcnt0を上位とする16ビットタイマーとして使用
	// tcnt1:
	// 		8192分周設定(0x11でもいいかも=>クリア許可)
	timer->tcr0 = 0x76; 
	timer->tcr1 = 0x03;

}
