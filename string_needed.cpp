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

//64はelf形式が6464版か6464版かに対応させる

//dynamicセクションとstrtabセクションを取得し、d_tagのNEEDEDが指す文字列を表示する
static int elf_get_needed(char* head) {
	
	Elf64_Ehdr *ehdr;
	Elf64_Shdr *shdr, *shstr, *str, *sym, *dyn, *rel;
	Elf64_Phdr *phdr;
	Elf64_Sym *symp;
	Elf64_Rel *relp;
	int i, j, size;
	char *sname;

	ehdr = (Elf64_Ehdr *)head;

	//セクション名格納用セクション(.shstrtab)の取得
	shstr = (Elf64_Shdr *)(head + ehdr->e_shoff + 
							ehdr->e_shentsize * ehdr->e_shstrndx);

	//セクション名一覧を表示
	::printf("Sections:\n");
	for (i = 0; i < ehdr->e_shnum; i++) {//  セクションヘッダテーブルを検索
		shdr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * i);//  当該セクションヘッダの先頭へのポインタ
		sname = (char *)(head + shstr->sh_offset + shdr->sh_name);// 当該セクションヘッダのセクション名の先頭へのポインタ
		::printf("\t[%d]\t%s\n", i, sname);
		if (!::strcmp(sname, ".dynstr")) str = shdr;//  セクション名が"dynstr"（シンボル名を格納するセクション）に一致していればstrに当該セクションヘッダを代入する
		if (!::strcmp(sname, ".dynamic")) dyn = shdr, printf("Get dynamic section!\n");// dynamicセクションを取得
	}
	
	//dynamicセクションの各Dyn構造体のd_tagの値がDT_NEEDEDであるかを調べる
	//そして、そのNEEDEDが指す(d_ptrが指す)strtabの文字列を表示する
	for (i = 0; i < dyn->sh_size / dyn->sh_entsize; i++) {
		Elf64_Dyn* dyn_strc = NULL;
		dyn_strc = (Elf64_Dyn *)(head + dyn->sh_offset// dynamicセクションまでの距離
									+ dyn->sh_entsize * i);// dyn_strc = Dyn構造体
		
		if (dyn_strc->d_tag == DT_NEEDED) {// d_tagがNEEDEDの時,
			//::printf("%x\n", dyn->d_tag);
			

			#if DEBUG == 0// 普通にNEEDEDの指す文字列を表示
			::printf("(NEEDED)  %s\n", szNeeded);
			#endif

			#if DEBUG == 1// d_ptrの値をずらして表示
			dyn_strc->d_un.d_ptr++;
			char* szNeeded = (char *)(head + str->sh_offset + dyn_strc->d_un.d_ptr);// d_ptrが指す文字列
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

	hFile = ::CreateFile(argv[1], GENERIC_READ/*読み取りアクセス*/, 0, NULL, 
				OPEN_EXISTING/*ファイルをオープン*/, 0, NULL);//  argv[1]へのアクセスに必要なハンドルをhFileに渡す
	iSize = ::GetFileSize(hFile, NULL);//  現在オープンしているファイルのサイズを取得する
	
	pData = ::VirtualAlloc(NULL/*システムがメモリを確保する領域を自動的に決定*/, iSize + iInsPageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);// メモリをiSize + iInsPageSizeバイト分確保 読みとりと書き込みを許可
	DWORD iSized;
	bRet = ::ReadFile(hFile/*読み取り先のファイルハンドルを指定*/, pData/*pDataが指すアドレスを基点にして、メモリ上にファイルをロード*/, iSize/*読み取るバイト数*/, &iSized/*読みとったバイト数を格納するアドレス*/, NULL); //  読みとりに成功すると1を返す ファイルをメモリに読みだす

	if (bRet) ::printf("Yes, load\n");

	char* head = NULL;
	head = (char *)pData;
	elf_get_needed(head);
	

	::CloseHandle(hFile);
}

