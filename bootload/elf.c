#include "defines.h"
#include "elf.h"
#include "lib.h"

/*
   struct elf_header {
   struct {
   unsigned char magic[4];
   unsigned char class;
   unsigned char format;
   unsigned char version;
   unsigned char abi;
   unsigned char abi_version;
   unsigned char reserve[7];
   } id;
   short type;
   short arch;
   long version;
   long entry_point;
   long program_header_offset;
   long section_header_offset;
   long flags;
   short header_size;
   short program_header_size;
   short program_header_num;
   short section_header_size;
   short section_header_num;
   short section_name_index;
   };

   struct elf_program_header {
   long type;
   long offset;
   long virtual_addr;
   long physical_addr;
   long file_size;
   long memory_size;
   long flags;
   long align;
   };
   */
/* ELFヘッダのチェック */
long elf_check(long e_hdr_p)
{
	struct elf_header *header = (struct elf_header*)e_hdr_p;

	if (memcmp(header->id.magic, "\x7f" "ELF", 4))
		return -1;

	if (header->id.bit   != 1) return -2; /* ELF32 */
	if (header->id.data  != 2) return -3; /* Big endian */
	if (header->id.version != 1) return -4; /* version 1 */
	if (header->type       != 2) return -5; /* Executable file */
	if (header->version    != 1) return -6; /* version 1 */

	/* Hitachi H8/300 or H8/300H */
	if ((header->machine != 46) && (header->machine != 47)) return -7;

	return header->entry_point;
}


/*
   char *elf_load(char *buf)
   {
   struct elf_header *header = (struct elf_header *)buf;

   if (elf_check(header) < 0) //ELFヘッダのチェック
   return NULL;


// if (elf_load_program(header) < 0) // セグメント単位でのロード
// return NULL;

return (char *)header->entry_point;
}
*/

