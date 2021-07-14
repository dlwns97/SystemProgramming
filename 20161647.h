#define _CRT_SECURE_NO_WARNINGS
#define MAX_SIZE 100 // Max input size
#define HASH_LEN 20 // Hasah table length
#define INS_LEN 19
#define SYMTAB_LEN 26 // length of SYMTAB (a~z, 26)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>


/********* instruction set **************/
char ins[INS_LEN][100] = { "h[elp]", "d[ir]", "q[uit]",
"hi[story]", "du[mp] [start, end]",
"e[dit] address, value", "f[ill] start, end, value",
"reset", "opcode mnemonic", "opcodelist",
"assemble filename", "type filename", "symbol",
"progaddr","loader objectfile","run","bp","bp address","bp clear"};
/************************************************/

/* command linked list structure, head*/
typedef struct _node
{
	char str[100];
	struct _node* link;
}NODE;

NODE* head;
/****************************************************/

unsigned char mem[65536][16];	// 1Mbyte memory


/*************opcode mnemonic, hash table***************/
typedef struct _hash
{
	int op;
	char name[10];
	char format[10];
	struct _hash* link;
}HASH;

typedef struct _table
{
	HASH* link;
}TABLE;
TABLE hash[HASH_LEN];

/*******************************************************/

typedef struct _symtab
{
	int loc;
	char symbol[10];
	struct _symtab* link;
}SYMTAB;

SYMTAB sym[SYMTAB_LEN];
SYMTAB sym_tmp[SYMTAB_LEN];

/*********************************************************/

// linking loader
int progaddr,execaddr,program_len;

// ESTAB
typedef struct _ESTAB {
	int loc;
	int cslth;
	char symbol[10];
	struct _ESTAB* link;
}ESTAB;

ESTAB est[3]; // ESTAB
int csec; // control section number

//break point
// is_bp -> if there is break point
int bp[20], bp_len = 0, current_bp=0, is_bp=0;
int reg[10]; // A(0) X(1) L(2) B(3) S(4) T(5) PC(8) SW(9)



/*********SHELL************/
void print_prompt(void);
char* Remove_Blanks(char* str1);


/***********command function***********/
void func_alloc();
void func_free();
void func_error(char* err);
//int func_dir();
void func_help();
void func_history(NODE* head);
void func_list(NODE* head, char* cmd);
void func_dump(int s_pos, int e_pos, int* pos);
int func_range(char* cmd, int* range);
int func_hextodec(char* cmd);
void func_reset();
int func_edit(char* cmd);
int func_fill(char* cmd);
int func_hash(char* mne);
void func_hashmake();
void func_opcodelist();
int func_mnemonic(char* cmd);
int func_type(char* cmd);
void func_SYMTABPUSH(SYMTAB* tab, char* cmd, int loc);
void func_SYMTABCOPY();
void func_SYMTABFREE(SYMTAB* tab);
int func_SYMTABPRINT();
int func_SYMTABSEARCH(SYMTAB* tab, char* cmd);
int func_pass1(char* cmd);
void func_SEPERATEWORDS(char* code, char* label, char* directive, char* operand);
int func_isOPCODE(char* cmd, int* format, int* opcode);
void func_directive(char* directive, int* loc, char* operand, int* locvar);
int func_pass2(char*filenmae, int locctr, int modloc);
void func_ESTABPUSH(char* sym, int loc, int sec_num, int length);
int func_ESTABSEARCH(char*sym);
void func_ESTABFREE();
void func_PRINTLOADMAP();
int func_CSECNUM(char* cmd, char* obj1, char* obj2, char* obj3);
int func_LOADpass1(char* filename, int csaddr, int sec_num);
int func_LOADpass2(char* filename, int csaddr, int sec_num);
int func_RegisterNum(char* reg);
int func_breakpoint(char* oper);
void func_initializeregister();
int func_run(int startaddr);
int func_getdataformat3(unsigned char* objcode);
int func_getdataformat4(unsigned char* objcode);
void func_regvalue();
