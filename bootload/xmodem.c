#include "defines.h"
#include "serial.h"
#include "lib.h"
#include "elf.h"
#include "xmodem.h"

#define XMODEM_SOH 0x01
#define XMODEM_STX 0x02
#define XMODEM_EOT 0x04
#define XMODEM_ACK 0x06
#define XMODEM_NAK 0x15
#define XMODEM_CAN 0x18
#define XMODEM_EOF 0x1a /* Ctrl-Z */

#define XMODEM_BLOCK_SIZE 128

/*
   struct elf_header {
   struct {
   char magic[4];
   char class;
   char data;
   char version;
   char os_abi;
   char abi_version;
   char reserved[7];
   } id;
   short type;
   short machine;
   long version;
   long entry_point;
   long program_header;
   long section_header;
   long flag;
   short elf_header_size;
   short program_header_size;
   short program_header_count;
   short section_header_size;
   short section_header_count;
   short section_table_index;
   };

   struct program_header {
   long type;
   long offset;
   long v_addr;
   long p_addr;
   long file_size;
   long mem_size;
   long flag;
   long aign;
   };
   */

// extern char buffer_start;
// static char *buffer = &buffer_start;

/* 受信開始されるまで送信要求を出す */
static int xmodem_wait(void)
{
	long cnt = 0;

	while (!serial_is_recv_enable(SERIAL_DEFAULT_DEVICE)) {
		if (++cnt >= 2000000) {
			cnt = 0;
			serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_NAK);
		}
	}

	return 0;
}

/* ブロック単位での受信 */
static int xmodem_read_block(unsigned char block_number, char *const loadbuf)
{
	unsigned char c, block_num, check_sum;
	int i;
	char *buf = loadbuf;

	block_num = serial_recv_byte(SERIAL_DEFAULT_DEVICE);
	if (block_num != block_number)
		return -1;

	block_num ^= serial_recv_byte(SERIAL_DEFAULT_DEVICE);
	if (block_num != 0xff)
		return -1;

	check_sum = 0;
	for (i = 0; i < XMODEM_BLOCK_SIZE; i++) {
		c = serial_recv_byte(SERIAL_DEFAULT_DEVICE);
		*(buf++) = c;
		check_sum += c;
	}

	check_sum ^= serial_recv_byte(SERIAL_DEFAULT_DEVICE);
	if (check_sum)
		return -1;

	return i;
}

static struct elf_header *load_data(char *const loadbuf, long total_size, struct debug_struct *d_st)
{
	static struct elf_header e_hdr;
	static struct program_header p_hdr[5]; // mallocが使えないので動的な確保は難しい
	static int program_header_size = 1;
	static long current_size = 0;
	static int p_hdr_index = 0;
	static char *target;

	long buffer_offset = 0;

	while(buffer_offset < XMODEM_BLOCK_SIZE){
		if (current_size == 0) {

			target = (char*)&e_hdr;
			d_st->target_ehdr = target; //debug
		}
		else if (current_size == sizeof(struct elf_header)){
			program_header_size = sizeof(struct program_header) * e_hdr.program_header_count;
			target = (char*)p_hdr;
			d_st->target_phdr = target; //debug
			d_st->phdr_size = program_header_size; //debug
			memset(e_hdr.id.reserved, 'A', 7); //debug
			memcpy(&(d_st->eh), &e_hdr, sizeof(struct elf_header));
		}
		else if (current_size == sizeof(struct elf_header) + program_header_size ){
			target = (char*)p_hdr[0].p_addr + sizeof(struct elf_header) + program_header_size;
			d_st->target_segs1 = target; //debug
			memcpy(&(d_st->ph[0]), &p_hdr[0], sizeof(struct program_header)); //debug
			memcpy(&(d_st->ph[1]), &p_hdr[1], sizeof(struct program_header)); //debug
		}
		//else if (current_size == sizeof(struct elf_header) + program_header_size + p_hdr[p_hdr_index].file_size){
		else if (current_size == p_hdr[p_hdr_index].offset + p_hdr[p_hdr_index].file_size){

			d_st->fourth_if = "OK";

			// メモリ上で新たにセットされる部分は、０で埋めておく

			memset(
					(char*)(p_hdr[p_hdr_index].p_addr + p_hdr[p_hdr_index].file_size + 1),
					0,
					p_hdr[p_hdr_index].mem_size - p_hdr[p_hdr_index].file_size
				  );

			current_size += (p_hdr[p_hdr_index].mem_size - p_hdr[p_hdr_index].file_size);

			if (p_hdr_index < e_hdr.program_header_count - 1) {
				target = (char*)(p_hdr[++p_hdr_index].p_addr);
				d_st->target_segs2 = target;
			} else {
				target = (char*)e_hdr.section_header;
				d_st->target_shdr = target;	 //debug
			}
		}
		*(target++) = *(loadbuf + (buffer_offset++));
		current_size++;
		d_st->current_size = current_size;
	}
	return &e_hdr;
	}




	//long xmodem_recv(char* const loadbuf)
	long xmodem_recv(char* const loadbuf, struct debug_struct *d_st)	//デバッグ用
	{
		// r: 受信したデータ部分のサイズ（通常128）
		// receiving: 受信中かどうか
		int r, receiving = 0;

		//これまでに受信したデータ量
		long size = 0;

		//block_number: 受信したブロックに書かれている通し番号
		unsigned char c, block_number = 1;

		// elfヘッダ
		struct elf_header *e_hdr;

		while (1) {
			if (!receiving)
				xmodem_wait(); /* 受信開始されるまで送信要求を出す */

			c = serial_recv_byte(SERIAL_DEFAULT_DEVICE);

			if (c == XMODEM_EOT) { /* 受信終了 */
				serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_ACK);
				break;
			} else if (c == XMODEM_CAN) { /* 受信中断 */
				return -1;
			} else if (c == XMODEM_SOH) { /* 受信開始 */
				receiving++;
				r = xmodem_read_block(block_number, loadbuf); /* ブロック単位での受信 */
				if (r < 0) { /* 受信エラー */
					serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_NAK);
				} else { /* 正常受信 */
					size += r;
					e_hdr = load_data(loadbuf, size, d_st);
					block_number++;
					serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_ACK);
				}
			} else {
				if (receiving)
					return -1;
			}
		}

		return (long)e_hdr;
	}



