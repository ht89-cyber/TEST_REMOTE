#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "elf.h"
#include <windows.h>
#include <string.h>

#include <ctype.h>
#include <time.h>

#define	iInsPageSize	0x1000

#define COL_MAX 350
#define ROW_MAX 50000
#define DEBUG 1
// #define BIT 64

//64��elf�`����6464�ł�6464�ł��ɑΉ�������

//dynamic�Z�N�V������strtab�Z�N�V�������擾���Ad_tag��NEEDED���w���������\������
static int elf_get_needed(char* head) {
	
	Elf64_Ehdr *ehdr;
	Elf64_Shdr *shdr, *shstr, *str, *sym, *dyn, *rel;
	Elf64_Phdr *phdr;
	Elf64_Sym *symp;
	Elf64_Rel *relp;
	int i, j, size;
	char *sname;

	ehdr = (Elf64_Ehdr *)head;

	//�Z�N�V�������i�[�p�Z�N�V����(.shstrtab)�̎擾
	shstr = (Elf64_Shdr *)(head + ehdr->e_shoff + 
							ehdr->e_shentsize * ehdr->e_shstrndx);

	//�Z�N�V�������ꗗ��\��
	::printf("Sections:\n");
	for (i = 0; i < ehdr->e_shnum; i++) {//  �Z�N�V�����w�b�_�e�[�u��������
		shdr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * i);//  ���Y�Z�N�V�����w�b�_�̐擪�ւ̃|�C���^
		sname = (char *)(head + shstr->sh_offset + shdr->sh_name);// ���Y�Z�N�V�����w�b�_�̃Z�N�V�������̐擪�ւ̃|�C���^
		::printf("\t[%d]\t%s\n", i, sname);
		if (!::strcmp(sname, ".dynstr")) str = shdr;//  �Z�N�V��������"dynstr"�i�V���{�������i�[����Z�N�V�����j�Ɉ�v���Ă����str�ɓ��Y�Z�N�V�����w�b�_��������
		if (!::strcmp(sname, ".dynamic")) dyn = shdr, printf("Get dynamic section!\n");// dynamic�Z�N�V�������擾
	}
	
	//dynamic�Z�N�V�����̊eDyn�\���̂�d_tag�̒l��DT_NEEDED�ł��邩�𒲂ׂ�
	//�����āA����NEEDED���w��(d_ptr���w��)strtab�̕������\������
	for (i = 0; i < dyn->sh_size / dyn->sh_entsize; i++) {
		Elf64_Dyn* dyn_strc = NULL;
		dyn_strc = (Elf64_Dyn *)(head + dyn->sh_offset// dynamic�Z�N�V�����܂ł̋���
									+ dyn->sh_entsize * i);// dyn_strc = Dyn�\����
		
		if (dyn_strc->d_tag == DT_NEEDED) {// d_tag��NEEDED�̎�,
			//::printf("%x\n", dyn->d_tag);
			

			#if DEBUG == 0// ���ʂ�NEEDED�̎w���������\��
			::printf("(NEEDED)  %s\n", szNeeded);
			#endif

			#if DEBUG == 1// d_ptr�̒l�����炵�ĕ\��
			dyn_strc->d_un.d_ptr++;
			char* szNeeded = (char *)(head + str->sh_offset + dyn_strc->d_un.d_ptr);// d_ptr���w��������
			::printf("(NEEDED)  %s\n", szNeeded);
			#endif
		}
		

	}

	return 0;

}

int main(int argc, char* argv[]) {
	// first commit!
	PVOID pData = NULL;
	DWORD iSize = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BOOL bRet = TRUE;

	hFile = ::CreateFile(argv[1], GENERIC_READ/*�ǂݎ��A�N�Z�X*/, 0, NULL, 
				OPEN_EXISTING/*�t�@�C�����I�[�v��*/, 0, NULL);//  argv[1]�ւ̃A�N�Z�X�ɕK�v�ȃn���h����hFile�ɓn��
	iSize = ::GetFileSize(hFile, NULL);//  ���݃I�[�v�����Ă���t�@�C���̃T�C�Y���擾����
	
	pData = ::VirtualAlloc(NULL/*�V�X�e�������������m�ۂ���̈�������I�Ɍ���*/, iSize + iInsPageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);// ��������iSize + iInsPageSize�o�C�g���m�� �ǂ݂Ƃ�Ə������݂�����
	DWORD iSized;
	bRet = ::ReadFile(hFile/*�ǂݎ���̃t�@�C���n���h�����w��*/, pData/*pData���w���A�h���X����_�ɂ��āA��������Ƀt�@�C�������[�h*/, iSize/*�ǂݎ��o�C�g��*/, &iSized/*�ǂ݂Ƃ����o�C�g�����i�[����A�h���X*/, NULL); //  �ǂ݂Ƃ�ɐ��������1��Ԃ� �t�@�C�����������ɓǂ݂���

	if (bRet) ::printf("Yes, load\n");

	char* head = NULL;
	head = (char *)pData;
	elf_get_needed(head);
	

	::CloseHandle(hFile);
}

