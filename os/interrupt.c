#include "defines.h"
#include "intr.h"
#include "interrupt.h"
#include "lib.h"

/* ���եȥ�����������ߥ٥����ν���� */
int softvec_init(void)
{
	int type;
	for (type = 0; type < SOFTVEC_TYPE_NUM; type++)
		softvec_setintr(type, NULL);
	return 0;
}

/* ���եȥ�����������ߥ٥��������� */
int softvec_setintr(softvec_type_t type, softvec_handler_t handler)
{
	SOFTVECS[type] = handler;
	puts("\n*** softvec_setintr ***\n");

	int i;
	for(i=0; i < SOFTVEC_TYPE_NUM; i++){
		putxval(i, 6);
		puts(":");
		putxval((long)SOFTVECS[i], 6);
		puts("\n");
	}

	return 0;
}

/*
 * ���̳���ߥϥ�ɥ顥
 * ���եȥ�����������ߥ٥����򸫤ơ��ƥϥ�ɥ��ʬ�����롥
 */
void interrupt(softvec_type_t type, unsigned long sp)
{
	softvec_handler_t handler = SOFTVECS[type];
	if (handler)
		handler(type, sp);
}
