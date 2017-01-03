#ifndef _ELF_H_INCLUDED_
#define _ELF_H_INCLUDED_


struct elf_header {
	struct {
		char magic[4];
		char bit;
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
	long align;
};


struct debug_struct {
	char *target_ehdr;
	char *target_phdr;
	char *target_segs1;
	char *target_segs2;
	char *target_shdr;
	struct elf_header eh;
	struct program_header ph[2];
	int phdr_size;
	char *fourth_if;
	long current_size;
};

long elf_check(long e_hdr_p);
#endif
