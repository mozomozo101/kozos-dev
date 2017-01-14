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

static void debug_func(void)
{
	volatile struct timer_reg *timer = TIMER_REG01;
	
	puts("tcr0:\t\t");
	putxval(timer->tcr0, 6);
	puts("\n");

	puts("tcr1:\t\t");
	putxval(timer->tcr1, 6);
	puts("\n");

	puts("tcsr0:\t\t");
	putxval(timer->tcsr0, 6);
	puts("\n");

	puts("tcsr1:\t\t");
	putxval(timer->tcsr1, 6);
	puts("\n");

	puts("tcora0:\t\t");
	putxval(timer->tcora0, 6);
	puts("\n");

	puts("tcora1:\t\t");
	putxval(timer->tcora1, 6);
	puts("\n");

	puts("tcorb0:\t\t");
	putxval(timer->tcorb0, 6);
	puts("\n");

	puts("tcorb1:\t\t");
	putxval(timer->tcorb1, 6);
	puts("\n");

	puts("tcnt0:\t\t");
	putxval(timer->tcnt0, 6);
	puts("\n");

	puts("tcnt1:\t\t");
	putxval(timer->tcnt1, 6);
	puts("\n");

}

// タイマ終了
static void timer_cancel(void)
{
	volatile struct timer_reg *timer = TIMER_REG01;

	puts("****** register value - before cancel ******\n");
	debug_func();

	timer->tcsr0 = 0;	// CMFA（コンペアマッチ）フラグを落とす
	timer->tcr0 = 0;	// クロック入力を禁止に
	timer->tcr1 = 0;	// クロック入力を禁止に
	timer->tcsr0 = 0;
	timer->tcsr1 = 0;

	puts("****** current register value - after_cancel ******\n");
	debug_func();

}


// タイマーが切れたら
void timer_intr(softvec_type_t type, unsigned long sp)
{
	puts("timer expired!\n");

	// thread_send(kz_msgbox_id_t id, int size, char *p)
	//
	// send(timers->top, 
	


	timer_cancel();
}


void timer_start(int second) {

	// 設定の順番によってはうまく動作しなくなるかも・・・
	volatile struct timer_reg *timer = TIMER_REG01;
	
	// tcnt0:
	// 		クロック入力は、tcnt1のオーバーフロー信号
	// 		tcnt0, 1 は、tcnt0を上位とする16ビットタイマーとして使用
	// tcnt1:
	// 		8192分周設定(0x11でもいいかも=>クリア許可)
	timer->tcr0 = 0x4c; 
	timer->tcr1 = 0x03;
	
	timer->tcsr0 = 0;
	timer->tcsr1 = 0;
	timer->tcnt0 = 0;
	timer->tcnt1 = 0;

	// tcnt0の1sあたりのカウント数： (20 * 10^6 / 8192 /256) ≒ 10
	timer->tcora0 = 10 * second;

	puts("****** current register value - on start\n");
	debug_func();

}
