#include "defines.h"
#include "interrupt.h"
#include "serial.h"
#include "elf.h"
#include "xmodem.h"
#include "lib.h"


static int init(void)
{
	/* �ʲ��ϥ�󥫡�������ץȤ�������Ƥ��륷��ܥ� */
	extern int erodata, data_start, edata, bss_start, ebss;

	/*
	 * �ǡ����ΰ��BSS�ΰ���������롥���ν����ʹߤǤʤ��ȡ�
	 * �����Х��ѿ������������Ƥ��ʤ��Τ���ա�
	 */
	memcpy(&data_start, &erodata, (long)&edata - (long)&data_start);
	memset(&bss_start, 0, (long)&ebss - (long)&bss_start);

	/* ���եȥ������������ߥ٥������������� */
	//softvec_init();

	/* ���ꥢ��ν���� */
	serial_init(SERIAL_DEFAULT_DEVICE);

	return 0;
}

/* �����16�ʥ���׽��� */
static int dump(char *const buf, long size)
{
	long i;

	if (size < 0) {
		puts("no data.\n");
		return -1;
	}
	for (i = 0; i < size; i++) {
		if ((i & 0xf) == 0) {
			putxval((long)buf + i, 8);
			puts(" | ");
		}
		putxval(buf[i], 2);
		puts(" ");
		if ((i & 0xf) == 7) puts(" ");
		if ((i & 0xf) == 15) puts("\n");
	}
	puts("\n");

	return 0;
}

static void wait()
{
	volatile long i;
	for (i = 0; i < 300000; i++)
		;
}

int main(void)
{
	static char buf[16];
	static long size = -1;
	char *entry_point = NULL;
	void (*f)(void);
	extern char buffer_start; /* ��󥫡�������ץȤ��������Ƥ���Хåե� */
	char* const loadbuf = &buffer_start;
	long e_hdr_p = 0;


	/* �ǥХå��� */
	struct debug_struct debug_s;
	memset(&debug_s, 0, sizeof(struct debug_struct));
	/* :::::::::: */

	//INTR_DISABLE; /* �����̵���ˤ��� */

	init();

	puts("kzload (kozos boot loader) started.\n");
	puts("\ndebus_s:");
	putxval((long)&debug_s, 8);
	puts("\n");

	while (1) {
		puts("kzload> "); /* �ץ��ץ�ɽ�� */
		gets(buf); /* ���ꥢ�뤫��Υ��ޥ�ɼ��� */

		if (!strcmp(buf, "load")) { /* XMODEM�ǤΥե�����Υ�������� */
			//size = xmodem_recv(loadbuf);
			e_hdr_p = xmodem_recv(loadbuf, &debug_s);	//�ǥХå���
			wait(); /* ž�����ץ꤬��λ��ü�����ץ�����椬���ޤ��Ԥ���碌�� */

			if (e_hdr_p < 0){
				puts("\nXMODEM_RECEIVE_ERROR\n");
			}else{
				puts("\nXMODEM_RECEIVE_SUCCEEDED\n");
			}
		} else if (!strcmp(buf, "dump")) { /* �����16�ʥ���׽��� */
			puts("\ndebus_s:");
			putxval((long)&debug_s, 8);
			puts("\n");

			puts("\nbuffer_start:");
			putxval((long)loadbuf, 8);
			puts("\n");


			puts("\n**** d_st ****\n");
			puts("\nehdr:");
			putxval((long)debug_s.target_ehdr, 8);
			puts("\nphdr:");
			putxval((long)debug_s.target_phdr, 8);
			puts("\nsegs1:");
			putxval((long)debug_s.target_segs1 ,8);
			puts("\nsegs2:");
			putxval((long)debug_s.target_segs2 ,8);
			puts("\nshdr:");
			putxval((long)debug_s.target_shdr, 8);
			puts("\nphdr_size1:");
			putxval((long)debug_s.ph[0].file_size, 8);
			puts("\nphdr_size2:");
			putxval((long)debug_s.ph[1].file_size, 8);
			puts("\n**** d_st ****\n");

			puts("\n******target_elf_header******\n");
			dump(debug_s.target_ehdr, sizeof(struct elf_header));
			puts("\n***************************\n");

			puts("\n******target_program_header******\n");
			dump(debug_s.target_phdr, sizeof(struct program_header)*2);
			puts("\n***************************\n");

			puts("\n******segment1******\n");
			puts("start:");
			putxval(debug_s.ph[0].v_addr, 6);
			puts("\n");
			puts("file size:");
			putxval(debug_s.ph[0].file_size, 6);
			puts("\n");
			dump((char*)debug_s.ph[0].p_addr, debug_s.ph[0].file_size);
			puts("\n***************************\n");

			puts("\n******segment2******\n");
			puts("start:");
			putxval(debug_s.ph[1].v_addr, 6);
			puts("\n");
			puts("file size:");
			putxval(debug_s.ph[1].file_size, 6);
			dump((char*)debug_s.ph[1].p_addr, debug_s.ph[1].file_size);
			puts("\n***************************\n");

			puts("\n******fourth_if******\n");
			puts(debug_s.fourth_if);
			puts("\n***************************\n");

			puts("\n******current_size******\n");
			putxval(debug_s.current_size, 8);
			puts("\n***************************\n");

			puts("\n******p_hdr[0].offset + p_hdr[0].file_size******\n");
			puts("offset:");
			putxval(debug_s.ph[0].offset, 8);
			puts("\n");
			puts("file_size:");
			putxval(debug_s.ph[0].file_size, 8);
			puts("\n***************************\n");

			puts("\n******p_hdr[1].offset + p_hdr[1].file_size******\n");
			puts("offset:");
			putxval(debug_s.ph[1].offset, 8);
			puts("\n");
			puts("file_size:");
			putxval(debug_s.ph[1].file_size, 8);
			puts("\n***************************\n");

			dump(loadbuf, 128);
			//dump(loadbuf, size);
		} else if (!strcmp(buf, "run")) { /* ELF�����ե�����μ¹� */
			if (elf_check(e_hdr_p) < 0) {
				puts("run error!\n");
			} else {
				entry_point = (char*)(((struct elf_header*)e_hdr_p)->entry_point);
				puts("starting from entry point: ");
				putxval((unsigned long)entry_point, 8);
				puts("\n");
				f = (void (*)(void))entry_point;
				f(); /* �����ǡ����ɤ����ץ����˽������Ϥ� */
				/* �����ˤ��֤äƤ��ʤ� */
			}
		} else {
			puts("unknown.\n");
		}
	}

	return 0;
}
