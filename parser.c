#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <elf.h>

/*
 Steps: open a file and map in to the memory
*/

int check_if_elf(uint8_t *mem);


int main(int argc, char *argv[])
{
	int file_descriptor, file_info;
	struct stat st;
	
	uint8_t *mem; // it is a pointer to the beggining of the mapped area
	
	//elf header specific declarations
	Elf64_Ehdr *elf_header;
	
	if(argc != 2)
	{
		printf("Use like this: %s <nameOfExecutable>\n", argv[0]);
		exit(0);
	}
	
	//open file that will be parsed:
	file_descriptor = open(argv[1], O_RDONLY);
	
	if(file_descriptor < 0)
	{
		perror("open");
		exit(-1);
	}
	else
	{
		puts("Open success\n");
	}
	
	//get ... of the file
	file_info = fstat(file_descriptor, &st);
	
	if(file_info < 0)
	{
		perror("fstat");
		exit(-1);
	}
	else
	{
		puts("fstat success\n");
	}
	
	//map the file into memory, with read-only access and mapping private
	mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
	
	if(mem == MAP_FAILED)
	{
		perror("mmap");
		exit(-1);
	}
	else
	{
		puts("mmap success\n");
	}
	
	
	elf_header = (Elf64_Ehdr *)mem;
	
	if(!check_if_elf(mem))
	{
		printf("The file %s is not elf\n", argv[1]);
		exit(-1);
	}
	
	printf("The file has %d program segments\n", elf_header->e_phnum);
	
	Elf64_Phdr *program_header;
	program_header = (Elf64_Phdr *)&mem[elf_header->e_phoff];
	
	uint8_t i;
	for(i = 0; i < elf_header->e_phnum; i++)
	{
		switch(program_header->p_type)
		{
			case PT_PHDR:
			{
				puts("Program header table\n");
				printf("Offset is 0x%lX\n", program_header->p_offset);
				printf("Virtual address is 0x%lX\n", program_header->p_vaddr);
				printf("Physical address is 0x%lX\n", program_header->p_paddr);
				printf("Filesize address is 0x%lX\n", program_header->p_filesz);
				printf("Memory size address is 0x%lX\n", program_header->p_memsz);
				break;
			}
			
			case PT_LOAD:
			{
				puts("\nLoad segment\n");
				printf("Offset is 0x%lX\n", program_header->p_offset);
				printf("Virtual address is 0x%lX\n", program_header->p_vaddr);
				printf("Physical address is 0x%lX\n", program_header->p_paddr);
				printf("Filesize address is 0x%lX\n", program_header->p_filesz);
				printf("Memory size address is 0x%lX\n", program_header->p_memsz);
				break;
			}
			
			//default:
				//puts("Other segment\n");
		}
		program_header++;
	}
	
	Elf64_Shdr *section_header; 
	//section_header = (Elf64_Shdr *)&mem[elf_header->e_shoff];//it is an array
	section_header = (Elf64_Shdr *)(mem + elf_header->e_shoff);
	
	
	//the executable has not a string table
	if(elf_header->e_shstrndx == SHN_UNDEF)
	{
		puts("No string table found\n");
		exit(-1);
	}

	char *string_table;
	//beging of the file + offset()
	string_table = (char *)(mem + section_header[elf_header->e_shstrndx].sh_offset);
	
	//print the section header's names:
	for(i = 1; i < elf_header->e_shnum; i++)
	{
		printf("%s\n", (string_table + section_header->sh_name));
		printf("Address 0x%lX\n", section_header->sh_addr);
		section_header++;
	}
	
	exit(0);
}

int check_if_elf(uint8_t *mem)
{
	if(mem[0] != 0x7F && strcmp(&mem[1], "ELF")!=0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

