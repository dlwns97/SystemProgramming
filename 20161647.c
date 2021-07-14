#include "20161647.h"

/*shell print*/
void print_prompt(void)
{
	fprintf(stderr, "sicsim> ");
}


/*shell command refine*/

/***************************************************/
/* remove unnecessary blanks
	and makes the command string refined
												   */
/****************************************************/
char* Remove_Blanks(char* str1)
{
	char* str2 = (char*)malloc(sizeof(char) * 100);
	char* temp;
	int len;

	temp = strtok(str1, " ");
	strcpy(str2, temp);
	len = strlen(temp);

	if ((strcmp(str2, "dump") != 0) && (strcmp(str2, "du") != 0) &&
		(strcmp(str2, "edit") != 0) && (strcmp(str2, "e") != 0) &&
		(strcmp(str2, "fill") != 0) && (strcmp(str2, "f") != 0)) {
		while (temp != NULL) {
			temp = strtok(NULL, " ");
			if (temp != NULL) {
				if (strcmp(temp, ",") != 0) {
					strcat(str2, " ");
					len += 1;
				}
				strcat(str2, temp);
				len += (strlen(temp));
			}
		}
	}
	// execption in dump, edit, fill
	else {
		temp = strtok(NULL, " ,");
		if (temp != NULL && strcmp(str2, "fill") != 0)
		{
			strcat(str2, " ");
			strcat(str2, temp);
			len += (strlen(temp) + 1);

			while (1) {
				temp = strtok(NULL, " ");
				if (temp != NULL) {
					strcat(str2, ", ");
					strcat(str2, temp);
					len += (strlen(temp) + 2);
				}
				else
					break;
			}
		}
		else if (temp != NULL && strcmp(str2, "fill") == 0)
		{
			strcat(str2, " ");
			strcat(str2, temp);
			len += (strlen(temp) + 1);

			temp = strtok(NULL, " ,");
			if (temp != NULL) {
				strcat(str2, ", ");
				strcat(str2, temp);
				len += (strlen(temp) + 2);
				temp = strtok(NULL, " ");
				if (temp != NULL) {
					strcat(str2, ", ");
					strcat(str2, temp);
					len += (strlen(temp) + 2);
				}
			}
		}
		else {
			str2[len] = '\0';
			return str2;
		}
	}
	str2[len - 1] = '\0';
	return str2;
}

/*command list input function*/

/* If empty push in head.
   If not empty push pack */

void func_list(NODE* head, char* command)
{
	NODE* NewNode = (NODE*)malloc(sizeof(NODE));
	strcpy(NewNode->str, command);
	NewNode->link = NULL;

	/*empty list*/
	if (head->link == NULL) {
		head->link = NewNode;
	}
	/*not empty*/
	else {
		NODE* tmp = head;
		while (tmp->link != NULL) {
			tmp = tmp->link;
		}
		tmp->link = NewNode;
	}
}

/*Initialization for program*/

/* command list
   virtual memory
   hash table memory
					 */
void func_alloc()
{
	head = (NODE*)malloc(sizeof(NODE));
	head->link = NULL;

	for (int i = 0; i < 65536; i++) {
		for (int j = 0; j < 16; j++) {
			mem[i][j] = 0;
		}
	}

	for (int i = 0; i < HASH_LEN; i++)
	{
		hash[i].link = NULL;
	}
	func_hashmake();

	// initialize break point array
	for (int i = 0; i < 20; i++) {
		bp[i] = -1;
	}
}

/*allocated memory free*/
void func_free()
{
	/*free linked list*/
	NODE* tmp;
	do {
		tmp = head;
		head = head->link;
		free(tmp);
	} while (head != NULL);

	/*free hash table*/
	HASH* tmp2, * tmp3;
	for (int i = 0; i < HASH_LEN; i++) {
		tmp2 = hash[i].link;
		if (tmp2 != NULL) {
			while (tmp2->link != NULL) {
				tmp3 = tmp2;
				tmp2 = tmp2->link;
				free(tmp3);
			}
			free(tmp2);
		}
	}

	func_SYMTABFREE(sym);
	func_SYMTABFREE(sym_tmp);
}

/*print error in program*/

void func_error(char* err) {
	printf("%s\n", err);
}

/*help command*/

/* print command */
void func_help()
{
	for (int i = 0; i < INS_LEN; i++)
	{
		printf("%s\n", ins[i]);
	}
}

/*dir command*/

/* open directory

   put '/' behind the directory file
   put '*' behind the execution file
									*/
/*
int func_dir()
{
	DIR* dir = opendir(".");
	struct dirent* de = NULL;
	struct stat status;

	if (dir == NULL)
	{
		printf("failed to open directory\n");
		return -1;
	}

	// read all files in dir
	while ((de = readdir(dir)) != NULL)
	{
		printf("%s", de->d_name);

		if (stat(de->d_name, &status) == -1)
		{
			perror("stat");
			return 1;
		}

		//directory
		if ((status.st_mode & S_IFMT) == S_IFDIR)
		{
			printf("/");
		}
		//execute file
		else if ((status.st_mode & S_IRWXU) == S_IRWXU || (status.st_mode & S_IRWXU) == S_IXUSR)
		{
			printf("*");
		}
		printf("\n");

	}
	closedir(dir);

	return 0;
}
*/
/* history command*/

/* read the command list
										*/
void func_history(NODE* head)
{
	NODE* tmp = head->link;
	int index = 1;

	while (tmp != NULL)
	{
		printf("%d %s\n", index, tmp->str);
		tmp = tmp->link; index++;
	}
}

/* dump command*/

/*

   ex) 00000 00 00 00 00 ...  00 ; ..................
														  */

void func_dump(int s_pos, int e_pos, int* pos)
{

	int s_pos_i = s_pos / 16, s_pos_j = s_pos % 16;
	int code_len = e_pos - s_pos + 1;
	int mem_address = s_pos - s_pos_j;

	if (e_pos + 1 < 1048575)
		*pos = e_pos + 1;

	int i = 0, count = 0;


	while (count < code_len) {

		// first line address
		if (s_pos_i == s_pos / 16 && i == 0) {
			printf("%05X ", mem_address);
		}
		// first memory address in next line
		if (s_pos_i != s_pos / 16 && i % 16 == 0)
			printf("%05X ", mem_address + (s_pos_i - s_pos / 16) * 16);
		// first line memory not in range
		if (i < s_pos_j && count == 0) {
			printf("   ");
			i++;
		}
		// moemory in range
		if (s_pos_i == s_pos / 16 && i >= s_pos_j && i < 15) {
			printf("%02X ", mem[s_pos_i][i]);
			i++; count++;
		}
		// memory in range
		if (s_pos_i != s_pos / 16 && i % 16 >= 0 && i % 16 < 15) {
			printf("%02X ", mem[s_pos_i][i % 16]);
			i++; count++;
		}
		// print the value of memory
		if (i % 16 == 15) {
			if (count == code_len)
				break;
			printf("%02X ", mem[s_pos_i][15]);
			i++; count++;
			printf("; ");
			for (int j = 0; j < 16; j++) {
				if (mem[s_pos_i][j] >= 32 && mem[s_pos_i][j] <= 126) {
					printf("%c", mem[s_pos_i][j]);
				}
				else printf(".");
			}
			printf("\n");
			s_pos_i++;
		}

	}
	// last line memory not in range
	if (16 - (e_pos % 16 + 1) > 0) {
		for (i = 0; i < 16 - (e_pos % 16 + 1); i++)
			printf("   ");
		printf("; ");
		for (i = 0; i < 16; i++) {
			if (mem[s_pos_i + count / 16][i] >= 32 && mem[s_pos_i + count / 16][i] <= 126) {
				printf("%c", mem[s_pos_i + count / 16][i]);
			}
			else printf(".");
		}
		printf("\n");
	}
}

/*get the range of dump command*/

/* if exception happens
   return correct error message and value
											 */
int func_range(char* command, int* range)
{
	char* tmp;
	char* copy;
	int count = 0;

	if (command[0] != '\0' && command[0] != ' ') {
		func_error("Wrong instruction");
		return -2;
	}

	copy = (char*)malloc(sizeof(char) * strlen(command));
	strcpy(copy, command);

	tmp = strtok(copy, "' ',");
	if (tmp == NULL)
		return 0;
	else if (tmp != NULL)
	{
		range[0] = func_hextodec(tmp);
		tmp = strtok(NULL, "' '");
		count++;
		if (range[0] > 1048575 || range[0] < 0)
			return -1;
		if (tmp != NULL) {
			range[1] = func_hextodec(tmp);
			count++;
			if (range[1] > 1048575) range[1] = 1048575;
			if (range[0] > range[1]) return -1;
		}
	}
	return count;
}

/*edit command*/

/* if there is error in range.
   print the error message and return values

   change the value of dump memory
											*/

int func_edit(char* command)
{
	char* tmp;
	char* copy;

	int address, value;


	if (command[0] != ' ') {
		func_error("Wrong instruction");
		return -2;
	}

	copy = (char*)malloc(sizeof(char) * strlen(command));
	strcpy(copy, command);

	tmp = strtok(copy, " ,");
	if (tmp == NULL) {
		func_error("No address");
		return -1;
	}
	else if (tmp != NULL)
	{
		address = func_hextodec(tmp);
		if (address > 1048575 || address < 0) {
			func_error("Address range error");
			return -1;
		}
		else {
			tmp = strtok(NULL, "' '");
			if (tmp == NULL) {
				func_error("No Value");
				return -1;
			}
			value = func_hextodec(tmp);
			if (value >= 32 && value <= 126) {
				mem[address / 16][address % 16] = value;
			}
			else {
				func_error("Value range error");
				mem[address / 16][address % 16] = value;
			}
		}
	}
	return 0;
}

/*reset command*/

/* reset all virtual memories to 0 */
void func_reset()
{
	for (int i = 0; i < 65536; i++) {
		for (int j = 0; j < 16; j++) {
			mem[i][j] = 0;
		}
	}
}

/*fill command*/

/* assign the virtual memory to value at range given
												   */
int func_fill(char* command)
{
	char* tmp;
	char* copy;
	int adr1, adr2;
	int value;


	if (command[0] != ' ') {
		func_error("Wrong instruction");
		return -2;
	}

	copy = (char*)malloc(sizeof(char) * strlen(command));
	strcpy(copy, command);

	tmp = strtok(copy, " ,");
	if (tmp == NULL) {
		func_error("No address");
		return -1;
	}
	adr1 = func_hextodec(tmp);
	if (adr1 > 1048575 || adr1 < 0) {
		func_error("Address range error");
		return -1;
	}
	tmp = strtok(NULL, " ,");
	if (tmp == NULL) {
		func_error("No address");
		return -1;
	}
	adr2 = func_hextodec(tmp);
	if (adr2 > 1048575) {
		adr2 = 1048575;
	}
	if (adr1 > adr2) {
		func_error("Address error");
		return -1;
	}
	tmp = strtok(NULL, " ");
	if (tmp == NULL) {
		func_error("No Value");
		return -1;
	}
	value = func_hextodec(tmp);
	if (value >= 32 && value <= 126) {
		for (int i = adr1; i <= adr2; i++) {
			mem[i / 16][i % 16] = value;
		}
	}
	else {
		func_error("Value range error");
		for (int i = adr1; i <= adr2; i++) {
			mem[i / 16][i % 16] = value;
		}
	}
	return 0;

}

/* change hexdecimal number to decimal number*/

/* read from the back of string

	multiply 16 to show where is the digit
										*/
int func_hextodec(char* command)
{
	int len = strlen(command);
	int i = 1, h = 1;
	int dec = 0, tmp;

	while (i <= len) {
		if (48 <= command[len - i] && command[len - i] <= 57)
			tmp = command[len - i] - 48;
		else if (65 <= command[len - i] && command[len - i] <= 70)
			tmp = command[len - i] - 55;
		else
			tmp = command[len - i] - 87;
		dec += (tmp * h);
		h *= 16;
		i++;
	}
	return dec;
}

/* hash function */

/* get the has value of command that given
											*/
int func_hash(char* mne)
{
	int mne_len;
	int hash_value = 0;
	mne_len = strlen(mne);

	for (int i = 0; i < mne_len; i++)
	{
		hash_value += mne[i];
		hash_value += (mne[i] / HASH_LEN);
	}
	return hash_value % 20;
}

/* function make hash table */

/* read opcode.txt

   assign 'op, name, format' in this format
											*/
void func_hashmake()
{
	FILE* hfp;
	HASH* NewNode;
	HASH* tmp;
	int h_index, op;
	char name[10], format[10];

	hfp = fopen("opcode.txt", "r");

	if (hfp == NULL) {
		func_error("No text file");
		exit(0);
	}
	else
	{
		while (fscanf(hfp, "%x%s%s", &op, name, format) != EOF)
		{
			tmp = (HASH*)malloc(sizeof(HASH));
			NewNode = (HASH*)malloc(sizeof(HASH));
			NewNode->op = op;
			strcpy(NewNode->name, name);
			strcpy(NewNode->format, format);
			h_index = func_hash(name);

			tmp = hash[h_index].link;
			NewNode->link = tmp;
			hash[h_index].link = NewNode;
		}
	}
	fclose(hfp);
}

/* opcodelist command */

/* print hash table */
void func_opcodelist()
{
	HASH* tmp;
	for (int i = 0; i < 20; i++)
	{
		printf("%02d : ", i);
		tmp = hash[i].link;
		while (tmp->link != NULL) {
			printf("[%s,%02X] -> ", tmp->name, tmp->op);
			tmp = tmp->link;
		}
		printf("[%s,%02X]\n", tmp->name, tmp->op);
	}
}

/* opcode mnemonic command */

/* get hash value of input command

   and search in opcode table if that command exists

   if exists then print the value
												   */

int func_mnemonic(char* command)
{
	HASH* tmp;
	int hash_value;
	char* copy = (char*)malloc(sizeof(char) * strlen(command));
	char* mne;

	strcpy(copy, command);
	mne = strtok(copy, " ");
	if (mne == NULL) {
		func_error("No mnemmonic");
		return -1;
	}

	hash_value = func_hash(command);
	tmp = hash[hash_value].link;
	while (tmp != NULL) {
		if (strcmp(tmp->name, command) == 0) {
			printf("opcode is %02X\n", tmp->op);
			return 0;
		}
		tmp = tmp->link;
	}
	func_error("Wrong mnemonic");
	return -1;
}

/* type filename command
   print the contents of file
								*/

// directory input X
int func_type(char* filename) {
	FILE* f = fopen(filename, "r");
	char code[100];

	if (f == NULL) {
		func_error("No such file in dir");
		return -1;
	}
	else {
		fgets(code, sizeof(code), f);
		while (feof(f) == 0) {
			printf("%s", code);
			fgets(code, sizeof(code), f);
		}
	}
	fclose(f);
	return 0;
}
/* push symbol in symboltable

   firstly find the location to input

   then push it in table
									*/
void func_SYMTABPUSH(SYMTAB* tab, char* str, int loc)
{
	SYMTAB* newsym, * tmpsym;
	int idx;

	newsym = (SYMTAB*)malloc(sizeof(SYMTAB));
	strcpy(newsym->symbol, str);
	newsym->loc = loc;
	newsym->link = NULL;


	idx = str[0] - 'A'; // alphabet - 'A'

	/*if list is empty*/
	if (tab[idx].link == NULL) {
		tab[idx].link = newsym;
	}
	/*fine the location to input symbol */
	else {
		tmpsym = tab[idx].link;
		/*push front*/
		if (strcmp(tmpsym->symbol, str) >= 1)
		{
			newsym->link = tmpsym;
			tab[idx].link = newsym;
		}
		/*push in middle*/
		else {
			while (tmpsym->link != NULL) {
				if (strcmp(tmpsym->link->symbol, str) >= 1)
				{
					newsym->link = tmpsym->link;
					tmpsym->link = newsym;
					return;
				}
				tmpsym = tmpsym->link;
			}
			/*push back*/
			tmpsym->link = newsym;
		}
	}
}
/* copy the temp table to real symbol table */

// if assemble failed then don't copy tmp to real
void func_SYMTABCOPY()
{
	SYMTAB* tmp;

	func_SYMTABFREE(sym);

	for (int i = 0; i < SYMTAB_LEN; i++)
	{
		tmp = sym_tmp[i].link;
		if (tmp != NULL) {
			func_SYMTABPUSH(sym, tmp->symbol, tmp->loc);
			while (tmp->link != NULL) {
				tmp = tmp->link;
				func_SYMTABPUSH(sym, tmp->symbol, tmp->loc);
			}
		}
	}
	func_SYMTABFREE(sym_tmp);
}
/*free the symbol table memory*/

void func_SYMTABFREE(SYMTAB* tab)
{
	SYMTAB* tmp, * free_tmp;

	for (int i = 0; i < SYMTAB_LEN; i++) {
		tmp = tab[i].link;
		if (tmp != NULL) {
			while (tmp->link != NULL) {
				free_tmp = tmp;
				tmp = tmp->link;
				free(free_tmp);
			}
			free(tmp);
		}
		tab[i].link = NULL;
	}
}
/* search the symbol in sybmol table

	if it exitst return the locctr of symbol
												*/

/* pass1, pass2 get the locctr of symbol   	 */

int func_SYMTABSEARCH(SYMTAB* tab, char* cmd)
{
	SYMTAB* tmp;
	int idx;

	idx = cmd[0] - 'A'; // alpha - 'A'

	tmp = tab[idx].link;

	while (tmp != NULL) {
		//symbol exists
		if (strcmp(cmd, tmp->symbol) == 0) {
			return tmp->loc;
		}
		tmp = tmp->link;
	}
	//symbol not exists
	return -1;
}

/* symbol command
   print table if exitst if not return -1 */
int func_SYMTABPRINT()
{
	SYMTAB* tmp;
	int flag = 0;

	for (int i = 0; i < SYMTAB_LEN; i++)
	{
		tmp = sym[i].link;
		while (tmp != NULL) {
			printf("\t%s\t%04X\n", tmp->symbol, tmp->loc);
			flag = 1;
			tmp = tmp->link;
		}
	}
	if (flag != 1) {
		func_error("There is no symbol table");
		return 0;
	}
	return 0;
}

/*assemble filename*/

/* assemble file command */

// pass1 read the file and make symbol table
// call pass2 function

int func_pass1(char* filename)
{
	char label[30], directive[30], operand[30];
	char code[100];
	int line_num = 5, locctr, modloc = 0, locvar;
	int format;
	int opcode;

	FILE* fp;
	fp = fopen(filename, "r");


	func_SYMTABFREE(sym_tmp);

	if (fp == NULL) {
		func_error("No assembly file");
		return -1;
	}


	fgets(code, sizeof(code), fp);
	func_SEPERATEWORDS(code, label, directive, operand);

	if (strcmp(directive, "START") == 0) {
		locctr = atoi(operand);
		line_num += 5;
	}
	else {
		fseek(fp, 0L, SEEK_SET);
		locctr = 0;
	}
	while (!feof(fp))
	{
		fgets(code, sizeof(code), fp);
		func_SEPERATEWORDS(code, label, directive, operand);

		if (operand[0] != '\0') operand[strlen(operand) - 1] = '\0';

		//comment
		if (code[0] == '.') {
			line_num += 5;
			continue;
		}

		/*error*/
		if (label[0] != '0' && func_SYMTABSEARCH(sym_tmp, label) != -1) {
			func_error("Symbol already exists");
			printf("Error in line : %d\n", line_num);
			return -1;
		}
		/*format 4*/
		else if (directive[0] == '+') {
			if (func_isOPCODE(directive + 1, &format, &opcode)) {
				if (label[0] != '\0')
					func_SYMTABPUSH(sym_tmp, label, locctr);
				locctr += 4;
				line_num += 5;
				modloc++;
			}
			else {
				func_error("Wrong code");
				printf("Error in line : %d\n", line_num);
				return -1;
			}
		}
		//  opcode, format 3 format 2 format 1
		else if (func_isOPCODE(directive, &format, &opcode)) {
			if (label[0] != '\0') {
				func_SYMTABPUSH(sym_tmp, label, locctr);
			}
			if (format == 3) locctr += 3;
			else if (format == 2) locctr += 2;
			else if (format == 1) locctr += 1;
			line_num += 5;
		}
		// Directive 

		// Directive have different locctr function
		// call func_directive to get variance of locctr

		// assign BASE register
		else if (strcmp(directive, "BASE") == 0) {
			if (label[0] != '\0') {
				func_SYMTABPUSH(sym_tmp, label, locctr);
			}
			line_num += 5;

		}
		// Directive WORD
		else if (strcmp(directive, "WORD") == 0) {
			if (label[0] != '\0') {
				func_SYMTABPUSH(sym_tmp, label, locctr);
			}
			func_directive(directive, &locctr, operand, &locvar);
			line_num += 5;
		}
		// Directive RESW
		else if (strcmp(directive, "RESW") == 0) {
			if (label[0] != '\0') {
				func_SYMTABPUSH(sym_tmp, label, locctr);
			}
			func_directive(directive, &locctr, operand, &locvar);
			line_num += 5;
		}
		// Directvie RESB
		else if (strcmp(directive, "RESB") == 0) {
			if (label[0] != '\0') {
				func_SYMTABPUSH(sym_tmp, label, locctr);
			}
			func_directive(directive, &locctr, operand, &locvar);
			line_num += 5;
		}
		// Directive BYTE
		else if (strcmp(directive, "BYTE") == 0) {
			if (label[0] != '\0') {
				func_SYMTABPUSH(sym_tmp, label, locctr);
			}
			func_directive(directive, &locctr, operand, &locvar);
			line_num += 5;
		}
		// END
		else if (strcmp(directive, "END") == 0) {
			if (label[0] != '\0') {
				func_SYMTABPUSH(sym_tmp, label, locctr);
			}
			break;
		}
		// ERROR CASE
		else {
			func_error("Invalid operation code");
			printf("Error in line: %d\n", line_num);
			return -1;
		}
	}

	fclose(fp);
	func_SYMTABCOPY();
	func_pass2(filename, locctr, modloc); // pass2
	return 1;
}
//  seperate assembly code into label, directive(opcode), operand
void func_SEPERATEWORDS(char* code, char* label, char* directive, char* operand)
{
	char* tmp;
	char* copy;

	copy = (char*)malloc(sizeof(char) * (strlen(code) + 1));
	strcpy(copy, code);

	// if there is no symbol(label)
	if (copy[0] == ' ' || copy[0] == '\t') {
		label[0] = '\0';
		tmp = strtok(copy, " ");
		strcpy(directive, tmp);
	}
	// comment
	else if (copy[0] == '.') {
		return;
	}
	else {
		tmp = strtok(copy, " ");
		strcpy(label, tmp);
		tmp = strtok(NULL, " ");
		strcpy(directive, tmp);
	}
	tmp = strtok(NULL, ", ");
	// if there is no operand
	if (tmp != NULL) {
		strcpy(operand, tmp);
		tmp = strtok(NULL, ", ");
		// if there is more than one
		while (tmp != NULL) {
			strcat(operand, ", ");
			strcat(operand, tmp);
			tmp = strtok(NULL, ", ");
		}
		operand[strlen(operand)] = '\0';
	}
	else {
		directive[strlen(directive) - 1] = '\0';
		operand[0] = '\0';
	}
}
// get value, format  of opcode  from opcodelist(table)
int func_isOPCODE(char* cmd, int* format, int* opcode)
{
	int hash_value;
	HASH* tmp;

	hash_value = func_hash(cmd);
	tmp = hash[hash_value].link;
	// find the opcode in opcode list
	while (tmp != NULL) {
		if (strcmp(tmp->name, cmd) == 0) {
			*format = tmp->format[0] - '0';
			*opcode = tmp->op;
			return 1;
		}
		tmp = tmp->link;
	}
	return 0;
}

// calculate the directive's memory by its rule
// and assign locvar how much locctr change
void func_directive(char* directive, int* locctr, char* operand, int* locvar)
{
	int num;

	num = atoi(operand);

	if (strcmp(directive, "WORD") == 0) {
		*locctr += 3;
		*locvar = 3;
	}
	else if (strcmp(directive, "RESW") == 0) {
		*locctr += 3 * num;
		*locvar = 3 * num;
	}
	else if (strcmp(directive, "RESB") == 0) {
		*locctr += num;
		*locvar = num;
	}
	else if (strcmp(directive, "BYTE") == 0) {
		// character
		if (operand[0] == 'C') {
			*locctr += strlen(operand) - 3;
			*locvar = strlen(operand) - 3;
		}
		// hexadecimal
		else if (operand[0] == 'X') {
			*locctr += 1;
			*locvar = 1;
		}
	}
	else {
		func_error("Wrong directive");
	}

}
// get register's value 
int func_RegisterNum(char* reg) {

	if (strcmp(reg, "A") == 0) {
		return 0;
	}
	else if (strcmp(reg, "X") == 0) {
		return 1;
	}
	else if (strcmp(reg, "L") == 0) {
		return 2;
	}
	else if (strcmp(reg, "PC") == 0) {
		return 8;
	}
	else if (strcmp(reg, "SW") == 0) {
		return 9;
	}
	else if (strcmp(reg, "B") == 0) {
		return 3;
	}
	else if (strcmp(reg, "S") == 0) {
		return 4;
	}
	else if (strcmp(reg, "T") == 0) {
		return 5;
	}
	else if (strcmp(reg, "F") == 0) {
		return 6;
	}
	// if not register
	return -1;
}
// secondly read assembly file
// make listing file and object file
// open "name".lst, "name".obj
int func_pass2(char* filename, int end_location, int modloc)
{
	FILE* fp1;
	FILE* fp2;
	FILE* fp3;

	char lst[10], obj[10]; // name of file
	char label[10], directive[10], operand[30], tmp_operand[30], code[50];
	int cmdlen, locctr, program_line, opcode, locvar, format, temp = 0, line_num = 5, trecord_count = 0; // locvar = variance locctr, trecord_count = assigned text record from object code
	int base = 0, count = 0; //count str until ','
	unsigned char obj_code[4], t_record[30]; //maximum 4 bytes, extended format
	int* mod_code; // modification location array

	fp1 = fopen(filename, "r");


	cmdlen = strlen(filename);

	// make file name
	strcpy(lst, filename);
	strcpy(obj, filename);
	strncpy(&lst[cmdlen - 3], "lst", 3);
	strncpy(&obj[cmdlen - 3], "obj", 3);

	lst[cmdlen] = '\0';
	obj[cmdlen] = '\0';

	fp1 = fopen(filename, "r");
	fp2 = fopen(lst, "w");
	fp3 = fopen(obj, "w");

	// allocate momory of modification location
	mod_code = (int*)malloc(sizeof(int) * modloc);
	modloc = 0;


	if (fp1 == NULL) {
		func_error("No assembly file");
		return -1;
	}

	fseek(fp1, 0L, SEEK_SET);

	fgets(code, 20, fp1);
	func_SEPERATEWORDS(code, label, directive, operand);


	/*HEADER RECORD*/

	if (strcmp(directive, "START") == 0) {
		locctr = atoi(operand);
		program_line = end_location - locctr;
		fprintf(fp2, "%-5d  %04X\t%s", line_num, locctr, code);
		if (label[0] == '\0') { //No file name
			fprintf(fp3, "H      %06X%06X\n", locctr, program_line);
		}
		else //file name exits
			fprintf(fp3, "H%-6s%06X%06X\n", label, locctr, program_line);
		line_num += 5;
	}
	else {
		// move file pointer to start position and initialize locctr 0
		fseek(fp1, 0L, SEEK_SET);
		locctr = 0;
		program_line = end_location - locctr;
		fprintf(fp3, "H      %06X%06X\n", locctr, program_line);
	}

	//TEXT RECORD

	while (!feof(fp1)) {

		fgets(code, sizeof(code), fp1);
		func_SEPERATEWORDS(code, label, directive, operand);


		if (operand[0] != '\0') operand[strlen(operand) - 1] = '\0';

		//BASE
		if (strcmp(directive, "BASE") == 0) {
			fprintf(fp2, "%-5d      \t%-30s\n", line_num, code);
			base = func_SYMTABSEARCH(sym, operand);
			line_num += 5;
			continue;
		}
		//comment
		if (code[0] == '.') {
			fprintf(fp2, "%-5d      \t%-30s\n", line_num, code);
			line_num += 5;
			continue;
		}
		code[strlen(code) - 1] = '\0';

		// print listing line except object code
		if (locctr != end_location) {
			fprintf(fp2, "%-5d  %04X\t%-30s\t", line_num, locctr, code);
		}
		//RESW/
		//No object code
		if (strcmp(directive, "RESW") == 0) {
			func_directive(directive, &locctr, operand, &locvar);
			line_num += 5;
			fprintf(fp2, "\n");
			//Text record exists
			if (trecord_count != 0) {
				fprintf(fp3, "%06X%02X", locctr - trecord_count - locvar, trecord_count);
				//text record
				for (int i = 0; i < trecord_count; i++) {
					fprintf(fp3, "%02X", t_record[i]);
				}
				fprintf(fp3, "\n");
				trecord_count = 0;
			}
		}
		//RESB
		//No object code
		else if (strcmp(directive, "RESB") == 0) {
			func_directive(directive, &locctr, operand, &locvar);
			line_num += 5;
			fprintf(fp2, "\n");
			//Text record exists
			if (trecord_count != 0) {
				fprintf(fp3, "%06X%02X", locctr - trecord_count - locvar, trecord_count);
				//text record
				for (int i = 0; i < trecord_count; i++) {
					fprintf(fp3, "%02X", t_record[i]);
				}
				fprintf(fp3, "\n");
				trecord_count = 0;
			}
		}
		//WORD
		//Decimal Number->Hexa Number
		else if (strcmp(directive, "WORD") == 0) {

			func_directive(directive, &locctr, operand, &locvar);

			//listing file add object code
			temp = func_hextodec(operand);
			fprintf(fp2, "%06X\n", temp);
			line_num += 5;

			//object file

			//make objcode to add in text_record
			obj_code[0] = temp / 65536; // first two col
			obj_code[1] = temp / 256 - obj_code[0] * 256; // second two col
			obj_code[2] = temp % 256; // last two col

			//if ad object code then text record overflow
			if (trecord_count > 26) { // WORD = 3bytes
				fprintf(fp3, "%06X%02X", locctr - trecord_count - locvar, trecord_count);
				//text record
				for (int i = 0; i < trecord_count; i++) {
					fprintf(fp3, "%02X", t_record[i]);
				}
				fprintf(fp3, "\n");
				trecord_count = 3;
				t_record[0] = obj_code[0];
				t_record[1] = obj_code[1];
				t_record[2] = obj_code[2];
			}
			// add object code to text code
			else {
				t_record[trecord_count++] = obj_code[0];
				t_record[trecord_count++] = obj_code[1];
				t_record[trecord_count++] = obj_code[2];
			}

		}
		//BYTE
		//Char or Hexa
		else if (strcmp(directive, "BYTE") == 0) {
			func_directive(directive, &locctr, operand, &locvar);
			line_num += 5;
			//Char//
			if (operand[0] == 'C') {

				//listing file
				for (int i = 0; i < locvar; i++) {
					obj_code[i] = operand[2 + i];
					fprintf(fp2, "%02X", obj_code[i]);
				}
				fprintf(fp2, "\n");

				//object file
				// if overflow
				if (trecord_count + locvar > 29) {
					fprintf(fp3, "%06X%02X", locctr - trecord_count - locvar, trecord_count);
					//text record
					for (int i = 0; i < trecord_count; i++) {
						fprintf(fp3, "%02X", t_record[i]);
					}
					fprintf(fp3, "\n");
					trecord_count = locvar;
					for (int i = 0; i < locvar; i++) {
						t_record[i] = obj_code[i];
					}
				}
				// add object code to text record
				else {
					for (int i = 0; i < locvar; i++) {
						t_record[trecord_count + i] = obj_code[i];
					}
					trecord_count += locvar;
				}

			}
			//Hexa
			else if (operand[0] == 'X') {
				for (int i = 0; i < locvar; i++) {
					obj_code[i] = operand[2 + i];
					fprintf(fp2, "%02X", obj_code[i]);
				}
				fprintf(fp2, "\n");

				//object file
				if (trecord_count + locvar > 29) {
					fprintf(fp3, "%06X%02X", locctr - trecord_count - locvar, trecord_count);
					//text record
					for (int i = 0; i < trecord_count; i++) {
						fprintf(fp3, "%02X", t_record[i]);
					}
					fprintf(fp3, "\n");
					trecord_count = locvar;
					for (int i = 0; i < locvar; i++) {
						t_record[i] = obj_code[i];
					}
				}
				// add object code to text cord
				else {
					for (int i = 0; i < locvar; i++) {
						t_record[trecord_count + i] = obj_code[i];
					}
					trecord_count += locvar;
				}
			}
		}
		//format 4

		else if (directive[0] == '+')
		{

			func_isOPCODE(directive + 1, &format, &opcode);
			locctr += 4;
			line_num += 5;

			//immediate addressing, n=0, i=1 -> +1
			if (operand[0] == '#') {
				obj_code[0] = opcode + 1; // first two col
				//integer operand (relocation X)
				if (operand[1] >= '0' && operand[1] <= '9') {
					temp = func_hextodec(operand + 1);
				}
				else {
					temp = func_SYMTABSEARCH(sym, operand + 1);
					mod_code[modloc++] = (locctr - 4 + 1); // extended format?first col?¿¿
				}
			}
			//indirect addressing, n=1, i=0 -> +2
			else if (operand[0] == '@') {
				obj_code[0] = opcode + 2; // first two col
				temp = func_SYMTABSEARCH(sym, operand + 1);
				mod_code[modloc++] = locctr - 3;
			}
			//simple addressing n=1 i=1 -> +3
			else {
				obj_code[0] = opcode + 3; // first two col
				temp = func_SYMTABSEARCH(sym, operand);
				mod_code[modloc++] = locctr - 3;
			}
			//extended object code
			obj_code[1] = 16 + (temp / 65536); // default '10' extended format, xbpe=0001
			obj_code[2] = (temp / 256) - (obj_code[1] % 16) * 256;
			obj_code[3] = temp % 256;

			//listing file
			fprintf(fp2, "%02X%02X%02X%02X\n", obj_code[0], obj_code[1], obj_code[2], obj_code[3]);

			//object file
			if (trecord_count > 25) { // extended format, 4bytes
				fprintf(fp3, "%06X%02X", locctr - trecord_count - 4, trecord_count);
				//text record
				for (int i = 0; i < trecord_count; i++) {
					fprintf(fp3, "%02X", t_record[i]);
				}
				fprintf(fp3, "\n");
				trecord_count = 4;
				//allocate object code to text record
				for (int i = 0; i < 4; i++) {
					t_record[i] = obj_code[i];
				}
			}
			// add object code to text record
			else {
				t_record[trecord_count++] = obj_code[0];
				t_record[trecord_count++] = obj_code[1];
				t_record[trecord_count++] = obj_code[2];
				t_record[trecord_count++] = obj_code[3];
			}

		}
		//opcode
		else if (func_isOPCODE(directive, &format, &opcode)) {
			line_num += 5;
			//format 3
			if (format == 3) {
				obj_code[0] = opcode;
				locctr += 3;

				while (1) {
					if (operand[count] == ',' || operand[count] == '\0')
						break;
					count++;
				}
				strncpy(tmp_operand, operand, count);
				tmp_operand[count] = '\0';

				//immediate addressing '01'
				if (operand[0] == '#') {
					obj_code[0] += 1;
					if (operand[1] >= '0' && operand[1] <= '9') {
						temp = func_hextodec(operand + 1);
						obj_code[1] = temp / 256;
						obj_code[2] = temp % 256;
					}
					else {
						temp = func_SYMTABSEARCH(sym, operand + 1);
						//Base relative, temp - locctr(pc's locctr) range out of -2048~2047, use integer variable base
						if ((temp - locctr) < -2048 || (temp - locctr) > 2047) {
							temp -= base;
							obj_code[1] = 4 * 16; // xbpe 0100
						}
						//pc realative, positive
						else if (temp >= locctr) {
							temp -= locctr;
							obj_code[1] = 2 * 16; // xbpe 0010
						}
						//pc relative, negative
						else {
							temp = 4096 - (locctr - temp);
							obj_code[1] = 2 * 16; // xbpe 0010
						}
						obj_code[2] = temp % 256;
						//if temp > 256
						obj_code[1] += temp / 256;
					}
				}
				//indirect addressing '10'
				else if (operand[0] == '@') {
					obj_code[0] += 2;
					temp = func_SYMTABSEARCH(sym, operand + 1);
					//Base relative
					if ((temp - locctr) < -2048 || (temp - locctr) > 2047) {
						temp -= base;
						obj_code[1] = 4 * 16; // xbpe 0100
					}
					// pc relatvie, positive
					else if (temp >= locctr) {
						temp -= locctr;
						obj_code[1] = 2 * 16; // xbpe 0010
					}
					else {
						temp = 4096 - (locctr - temp);
						obj_code[1] = 2 * 16; // xbpe 0010
					}
					obj_code[2] = temp % 256;
					//if temp > 256
					obj_code[1] += temp / 256;
				}
				//simple addressing '11'
				else {
					obj_code[0] += 3;

					// RSUB, exception
					if (strcmp(directive, "RSUB") == 0) {
						obj_code[1] = obj_code[2] = 0;
					}
					else {
						// check if X buffer in operand
						if (operand[strlen(operand) - 1] == 'X' && operand[strlen(operand) - 2] == ' ') {
							temp = func_SYMTABSEARCH(sym, tmp_operand);
							obj_code[1] = 8 * 16; // at least xbpe 1000
						}
						else {
							temp = func_SYMTABSEARCH(sym, operand);
							obj_code[1] = 0; // initialize 0
						}
						// Base relative
						if ((temp - locctr) < -2048 || (temp - locctr) > 2047) {
							temp -= base;
							obj_code[1] += 4 * 16; // xbpe ?100
						}
						// pc relative positive
						else if (temp - locctr >= 0) {
							temp -= locctr;
							obj_code[1] += 2 * 16; // xbpe ?010
						}
						// pc relative negative
						else {
							temp = 4096 - (locctr - temp);
							obj_code[1] += 2 * 16; // xbpe ?010
						}
						obj_code[2] = temp % 256;
						//if temp > 256
						obj_code[1] += temp / 256;
					}
				}
				//listing file
				fprintf(fp2, "%02X%02X%02X\n", obj_code[0], obj_code[1], obj_code[2]);

				//object file
				if (trecord_count > 26) { // format 3, 3bytes
					fprintf(fp3, "%06X%02X", locctr - trecord_count - 3, trecord_count);
					//text record
					for (int i = 0; i < trecord_count; i++) {
						fprintf(fp3, "%02X", t_record[i]);
					}
					fprintf(fp3, "\n");
					trecord_count = 3;
					for (int i = 0; i < 3; i++) {
						t_record[i] = obj_code[i];
					}
				}
				// add object code to text record
				else {
					for (int i = 0; i < 3; i++) {
						t_record[trecord_count++] = obj_code[i];
					}
				}
			}
			//foramt 2
			else if (format == 2) {
				count = 0;
				locctr += 2;
				while (1) {
					if (operand[count] == ',' || operand[count] == '\0')
						break;
					count++;
				}
				strncpy(tmp_operand, operand, count);
				tmp_operand[count] = '\0';



				obj_code[0] = opcode; // no additional format

				// check number of register

				// one register
				if (strlen(operand) == 1) {
					temp = func_RegisterNum(operand);
					obj_code[1] = 16 * temp;
				}
				//two register
				else {
					temp = func_RegisterNum(tmp_operand);
					obj_code[1] = 16 * temp;
					temp = func_RegisterNum(operand + 3);
					obj_code[1] += temp;
				}
				// listing file
				fprintf(fp2, "%02X%02X\n", obj_code[0], obj_code[1]);

				//object file
				if (trecord_count > 27) // format 2, 2bytes
				{
					fprintf(fp3, "%06X%02X", locctr - trecord_count - 2, trecord_count);
					for (int i = 0; i < trecord_count; i++) {
						fprintf(fp3, "%02X", t_record[i]);
					}
					fprintf(fp3, "\n");
					trecord_count = 2;
					for (int i = 0; i < 2; i++) {
						t_record[i] = obj_code[i];
					}
				}
				// add object code to text record
				else {
					for (int i = 0; i < 2; i++) {
						t_record[trecord_count++] = obj_code[i];
					}
				}
			}
			// format 1 only opcode, 1bytes
			else if (format == 1) {
				locctr += 1;
				obj_code[0] = opcode;

				//listing file
				fprintf(fp2, "%02X\n", obj_code[0]);

				//object file
				if (trecord_count > 28) {
					fprintf(fp3, "%06X%02X", locctr - trecord_count - 1, trecord_count);
					for (int i = 0; i < trecord_count; i++) {
						fprintf(fp3, "%02X", t_record[i]);
					}
					fprintf(fp3, "\n");
					trecord_count = 1;
					t_record[0] = obj_code[0];
				}
				// add object code to text record
				else {
					t_record[trecord_count++] = obj_code[0];
				}
			}

		}
		//END
		else if (strcmp(directive, "END") == 0) {
			//print modification record
			//print last line


			//listing file
			fprintf(fp2, "%-5d      \t%-30s\n", line_num, code);

			//object file

			//last line
			if (trecord_count != 0) {
				fprintf(fp3, "%06X%02X", locctr - trecord_count, trecord_count);
				for (int i = 0; i < trecord_count; i++) {
					fprintf(fp3, "%02X", t_record[i]);
				}
				fprintf(fp3, "\n");
				trecord_count = 0;
			}

			//modification record
			for (int i = 0; i < modloc; i++) {
				fprintf(fp3, "M%06X05\n", mod_code[i]); // 05 = half bytes
			}
			// print endline (start location) in object file
			fprintf(fp3, "E%06X\n", locctr - program_line);
			free(mod_code);
			break;
		}
	}

	printf("[%s], [%s]\n", lst, obj);

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);

	return 0;
}

// linking loader

// push extdef in estab
void func_ESTABPUSH(char* sym, int loc, int sec_num, int length) {
	ESTAB* tmp, * newnode;

	newnode = (ESTAB*)malloc(sizeof(ESTAB));
	newnode->cslth = length;
	newnode->loc = loc;
	newnode->link = NULL;
	strcpy(newnode->symbol, sym);

	if (est[sec_num].link == NULL) {
		est[sec_num].link = newnode;
	}
	else {
		tmp = est[sec_num].link;
		while (tmp->link != NULL) {
			tmp = tmp->link;
		}
		tmp->link = newnode;
	}
}
// search if extdef exists
// if exists return loc, means error
int func_ESTABSEARCH(char* sym)
{
	ESTAB* tmp;

	for (int i = 0; i < csec; i++) {
		tmp = est[i].link;
		while (tmp != NULL) {
			if (strcmp(tmp->symbol, sym) == 0)
				return tmp->loc;
			else
				tmp = tmp->link;
		}
	}
	return -1;
}
// free estab if error occurs
void func_ESTABFREE() {
	ESTAB* tmp, * free_tmp;

	for (int i = 0; i < csec; i++) {
		tmp = est[i].link;
		while (tmp != NULL) {
			free_tmp = tmp;
			tmp = tmp->link;
			free(free_tmp);
		}
		est[i].link = NULL;
	}

}
// print load map after loading
void func_PRINTLOADMAP()
{
	ESTAB* tmp;
	int total_len = 0;

	printf("control symbol address length\nsection name\n");
	printf("-----------------------------\n");

	for (int i = 0; i < csec; i++) {
		tmp = est[i].link;
		printf("%-6s          %04X    %04X  \n", tmp->symbol, tmp->loc, tmp->cslth);
		total_len += tmp->cslth;
		tmp = tmp->link;
		while (tmp != NULL) {
			printf("        %-6s  %04X\n", tmp->symbol, tmp->loc);
			tmp = tmp->link;
		}
	}
	printf("------------------------------\n");
	printf("           total length %04X\n", total_len);

	program_len = total_len;
}

// get filename and section number
// seperate section by blank
int func_CSECNUM(char* cmd, char* obj1, char* obj2, char* obj3) {
	char* tmp, * copy;
	int len = strlen(cmd);
	int num = 0;
	copy = (char*)malloc(sizeof(char) * (len + 1));
	strcpy(copy, cmd);

	tmp = strtok(copy, " ");
	tmp = strtok(NULL, " ");
	if (tmp == NULL) {
		return -1;
	}
	num++;
	strcpy(obj1, tmp);

	tmp = strtok(NULL, " ");
	if (tmp == NULL) {
		return num;
	}
	num++;
	strcpy(obj2, tmp);

	tmp = strtok(NULL, " ");
	if (tmp == NULL) {
		return num;
	}
	num++;
	strcpy(obj3, tmp);

	return num;
}
// Linking Loader pass1
// Read only H and D record
int func_LOADpass1(char* filename, int csaddr, int sec_num) {
	FILE* fp = fopen(filename, "r");
	char* tmp;
	char symbol[7], data[7], text[50];

	int count, cslth, value;

	printf("%s\n", filename);

	if (fp == NULL) {
		func_error("No file");
		func_ESTABFREE();
		return -1;
	}

	fgets(text, 50, fp);

	// read header record
	if (text[0] == 'H') {
		// section name
		strncpy(symbol, text + 1, 6);
		symbol[6] = '\0';
		tmp = strtok(symbol, " ");
		strcpy(symbol, tmp);
		// section name already exists
		if (func_ESTABSEARCH(symbol) != -1) {
			func_error("Duplicate symbol");
			fclose(fp);
			func_ESTABFREE();
			return -1;
		}
		// get section length
		strncpy(data, text + 13, 6);
		data[6] = '\0';
		cslth = func_hextodec(data);
		func_ESTABPUSH(symbol, csaddr, sec_num, cslth);

		fgets(text, 50, fp);
		// Read EXTDEF record
		if (text[0] == 'D') {
			//EXTDEF
			count = strlen(text) / 12; // number of EXTDEF
			for (int i = 0; i < count; i++) {
				//symbol
				strncpy(symbol, text + 12 * i + 1, 6);
				symbol[6] = '\0';
				tmp = strtok(symbol, " ");
				strcpy(symbol, tmp);
				// check symbol exists
				if (func_ESTABSEARCH(symbol) != -1) {
					func_error("Duplicate symbol");
					fclose(fp);
					func_ESTABFREE();
					return -1;
				}
				// symbol's location
				strncpy(data, text + 12 * i + 7, 6);
				data[6] = '\0';
				value = func_hextodec(data) + csaddr;
				func_ESTABPUSH(symbol, value, sec_num, 0);
			}
		}
		fclose(fp);
		return csaddr + cslth;
	}
	else {
		func_error("Head record don't exists");
		fclose(fp);
		func_ESTABFREE();
		return -1;
	}
}
// Linking Loader pass2
// Read H, R, T, M, E record
int func_LOADpass2(char* filename, int csaddr, int sec_num) {

	FILE* fp = fopen(filename, "r");
	char text[100], value[7], refidx[3];
	char* tmp;
	// refnum : reference array
	int cslth, startadr, * refnum, refcount, datanum, dataidx, idx; 
	int len;
	unsigned char data;

	refnum = (int*)malloc(sizeof(int) * 2); // check if no reference record

	//head record
	fgets(text, 100, fp);
	strncpy(value, text + 13, 6);
	value[6] = '\0';
	cslth = func_hextodec(value);

	// read record except D record
	while (1) {
		fgets(text, 100, fp);
		// store EXTREF by using reference number
		if (text[0] == 'R') {
			free(refnum);
			text[strlen(text)] = '\0';
			refcount = (strlen(text)) / 8 + 1;
			refnum = (int*)malloc(sizeof(int) * (refcount + 1)); // add 1 for section name 

			refnum[0] = csaddr; //section name
			len = strlen(text);
			for (int i = len - 2; i < len; i++) {
				if (text[i] == '\n') {
					text[i] = '\0';
				}
			}

			// other EXTREF
			for (int i = 1; i < refcount; i++) {
				strncpy(value, text + 8 * i - 5, 6);
				value[6] = '\0';
				tmp = strtok(value, " ");
				strcpy(value, tmp);
				refnum[i] = func_ESTABSEARCH(value); // find location of symbol

				if (refnum[i] == -1) { // func_ESTABSEARCH return -1 means no symbol
					func_error("No EXTREF in table");
					fclose(fp);
					return -1;
				}
			}
		}
		// read Text record
		else if (text[0] == 'T') {
			strncpy(value, text + 1, 6);
			value[6] = '\0';
			startadr = func_hextodec(value) + csaddr; // Text record start addr+csaddr

			strncpy(value, text + 7, 2);
			value[2] = '\0';
			datanum = func_hextodec(value);

			dataidx = 9; // T+location+len = 9
			for (int i = 0; i < datanum; i++) {
				strncpy(value, text + dataidx, 2);
				value[2] = '\0';
				data = func_hextodec(value);
				mem[(startadr + i) / 16][(startadr + i) % 16] = data;
				dataidx += 2; // read two characters
			}
		}
		// read Modification record
		else if (text[0] == 'M') {
			//if NO R record except section name
			//need to save section name
			refnum[0] = csaddr;

			strncpy(value, text + 1, 6);
			value[6] = '\0';
			dataidx = func_hextodec(value) + csaddr;

			strncpy(refidx, text + 10, 2);
			refidx[2] = '\0';
			idx = func_hextodec(refidx);

			// idx-1 in array ex) 02->1
			// idx out of range
			if (idx - 1 > refcount) {
				func_error("There is no reference");
				fclose(fp);
				return -1;
			}
			// read 3bytes value
			datanum = mem[dataidx / 16][dataidx % 16] * 65536;
			datanum += mem[(dataidx + 1) / 16][(dataidx + 1) % 16] * 256;
			datanum += mem[(dataidx + 2) / 16][(dataidx + 2) % 16];

			if (text[9] == '+') {
				datanum += refnum[idx-1];
			}
			else {
				datanum -= refnum[idx-1];
			}

			mem[dataidx / 16][dataidx % 16] = datanum / 65536;
			mem[(dataidx + 1) / 16][(dataidx + 1) % 16] = (datanum % 65536) / 256;
			mem[(dataidx + 2) / 16][(dataidx + 2) % 16] = datanum % 256;
		}
		// read End record
		else if (text[0] == 'E') {
			// only first control section has thing
			if (sec_num == 0) {
				strncpy(value, text + 1, 6);
				value[6] = '\0';
				execaddr = func_hextodec(value);
			}
			fclose(fp);
			return csaddr + cslth;
		}
		// Do not need to read EXTDEF
		else if (text[0] == 'D') {
			continue;
		}
	}

}

// initialize data of register
void func_initializeregister() {
	for (int i = 0; i < 10; i++) {
		reg[i] = 0;
	}
}

// bp command
int func_breakpoint(char* oper) {

	int len;
	int addr;
	char* copy;

	len = strlen(oper);
	copy = (char*)malloc(sizeof(char) * (len - 1));
	if (len != 2) {
		strcpy(copy, oper + 3);
	}

	//bp command
	if (len == 2) {
		printf("\t breakpoint\n");
		printf("\t ----------\n");
		for (int i = 0; i < bp_len; i++) {
			printf("\t %X\n", bp[i]);
		}
		return 0;
	}
	//bp clear command
	else if (strcmp(copy, "clear") == 0) {
		for (int i = 0; i < bp_len; i++) {
			bp[i] = -1;
			current_bp = 0;
		}
		return 0;
	}
	//bp set command
	else {
		addr = func_hextodec(copy);
		if (addr<0 || addr>program_len) {
			return -1;
		}
		else {
			bp[bp_len++] = progaddr+addr;
			printf("\t[ok] create breakpoint %s\n", copy);
			return 0;
		}
	}
}

// run command
// check object code in copy.obj
// make code for mnemonic in copy.obj
int func_run(int startaddr) {

	unsigned char object_code[4];
	int opcode, data;

	// set pc register
	reg[8] = startaddr;

	// run untill end program or break point
	while (1) {
		object_code[0] = mem[reg[8] / 16][reg[8] % 16]; // get opcode
		reg[8]++; // read next byte
		opcode = object_code[0] / 16; // classify opcode by its first number

		// process of format2, register operation
		// In copy.obj CLEAR(B4), COMPR(A0), TIXR(B8)
		if (opcode == 10 || opcode == 11) {
			object_code[1] = mem[reg[8] / 16][reg[8] % 16];
			reg[8]++;
			if (opcode == 10) { //COMPR R1 R2
				if (reg[object_code[1] / 16] > reg[object_code[1] % 16])
					reg[9] = -1; // R1>R2, then SW = -1
				else if (reg[object_code[1] / 16] == reg[object_code[1] % 16])
					reg[9] = 0; // R1==R2, then SW = 0
				else
					reg[9] = 1; // R1<R2, then SW = 1
			}
			else if (opcode == 11 && object_code[0] % 16 == 4) { // CLEAR R1
				reg[object_code[1] / 16] = 0; // Set R1 register 0
			}
			else if (opcode == 11 && object_code[0] % 16 == 8) { // TIXR R1

				reg[1]++;
				if (reg[1] < reg[object_code[1] / 16]) {
					reg[9] = -1; // X register < R1, then SW = -1
				}
				else if (reg[1] == reg[object_code[1] / 16]) {
					reg[9] = 0; // X register == R1, then SW = 0
				}
				else {
					reg[9] = 1; // X register > R1, then SW = 1
				}
			}
		}
		else {
			object_code[1] = mem[reg[8] / 16][reg[8] % 16];
			reg[8]++;
			object_code[2] = mem[reg[8] / 16][reg[8] % 16];
			reg[8]++;

			if (opcode == 0) { // LDA, STA
				// check if format 4 or format 3
				// format 4 -> e = 1, format 3 -> e = 0
				if ((object_code[1] / 16) % 2 == 0) { // format 3, object_code[1]/16 -> xbpe
					// check simple, indirect, immediate, sic style
					// base relatvie, pc relatvie, indexed bit
					// get data of address
					data = func_getdataformat3(object_code);
					if (object_code[0] % 16 < 4) { // LDA : 00 // Load data in address to Register A
						if ((object_code[0] % 16) % 4 != 1) {
							object_code[0] = mem[data / 16][data % 16];
							object_code[1] = mem[(data + 1) / 16][(data + 1) % 16];
							object_code[2] = mem[(data + 2) / 16][(data + 2) % 16];

							data = object_code[0] * 65536 + object_code[1] * 256 + object_code[2];
							if ((object_code[0] / 16) > 7) { // if negative
								data = data - 16777216; // 65536 * 256, 00(<-1677216) 00 00 00
							}
						}
						else {
							data *= 1;
						}
						reg[0] = data;
					}
					else if ((object_code[0] % 16) > 11 && (object_code[0] % 16) <= 15) { // STA : 0C // Store data in Register A to address
						mem[data / 16][data % 16] = reg[0] / 65536;
						mem[(data + 1) / 16][(data + 1) % 16] = (reg[0] % 65536) / 256;
						mem[(data + 2) / 16][(data + 2) % 16] = reg[0] % 256;
					}
				}
				else if ((object_code[1] / 16) % 2 == 1) { // format 4, e = 1
					// check simple, immediate, X bit
					object_code[3] = mem[reg[8] / 16][reg[8] % 16];
					reg[8]++;
					data = func_getdataformat4(object_code);
					if (object_code[0] % 16 < 4) { // LDA : 00 // Load data in address to Register A
						object_code[0] = mem[data / 16][data % 16];
						object_code[1] = mem[(data + 1) / 16][(data + 1) % 16];
						object_code[2] = mem[(data + 2) / 16][(data + 2) % 16];

						data = object_code[0] * 65536 + object_code[1] * 256 + object_code[2];
						if (object_code[0] / 16 > 7) { // if negative
							data = data - 16777216;
						}
						reg[0] = data;
					}
					else if (object_code[0] % 16 > 11 && object_code[0] % 16 <= 15) { // STA : 0C // Store data in Register A to address
						mem[data / 16][data % 16] = reg[0] / 65536;
						mem[(data + 1) / 16][(data + 1) % 16] = (reg[0] % 65536) / 256;
						mem[(data + 2) / 16][(data + 2) % 16] = reg[0] % 256;
					}
				}
			}
			else if (opcode == 1) { // STL, STX
				// check format
				if ((object_code[1] / 16) % 2 == 0) { //format 3
					// get target address
					data = func_getdataformat3(object_code);
					if ((object_code[0] % 16) > 4 && (object_code[0] % 16) <= 7) { // STL : 14~17
						mem[data / 16][data % 16] = reg[2] / 65536;
						mem[(data + 1) / 16][(data + 1) % 16] = (reg[2] % 65536) / 256;
						mem[(data + 2) / 16][(data + 2) % 16] = reg[2] % 256;
					}
					else if ((object_code[0] % 16) < 4) { // STX : 10~13
						mem[data / 16][data % 16] = reg[1] / 65536;
						mem[(data + 1) / 16][(data + 1) % 16] = (reg[1] % 65536) / 256;
						mem[(data + 2) / 16][(data + 2) % 16] = reg[1] % 256;
					}
				}
				else if ((object_code[1] / 16) % 2 == 1) { // format 4
					// get target address
					object_code[3] = mem[reg[8] / 16][reg[8] % 16];
					reg[8]++;
					data = func_getdataformat4(object_code);
					if ((object_code[0] % 16) > 4 && (object_code[0] % 16) <= 7) { // STL : 14~17
						mem[data / 16][data % 16] = reg[2] / 65536;
						mem[(data + 1) / 16][(data + 1) % 16] = (reg[2] % 65536) / 256;
						mem[(data + 2) / 16][(data + 2) % 16] = reg[2] % 256;
					}
					else if ((object_code[0] % 16) < 4) { // STX : 10~13
						mem[data / 16][data % 16] = reg[1] / 65536;
						mem[(data + 1) / 16][(data + 1) % 16] = (reg[1] % 65536) / 256;
						mem[(data + 2) / 16][(data + 2) % 16] = reg[1] % 256;
					}
				}
			}
			else if (opcode == 2) { // COMP 
				//check format
				if ((object_code[1] / 16) % 2 == 0) { // format 3
					// get compare address
					data = func_getdataformat3(object_code);
					if ((object_code[0] % 16) > 7 && (object_code[0] % 16) <= 11) { // COMP : 28~2B
						if ((object_code[0] % 16) % 4 != 1) { // not immediate
							object_code[0] = mem[data / 16][data % 16];
							object_code[1] = mem[(data + 1) / 16][(data + 1) % 16];
							object_code[2] = mem[(data + 2) / 16][(data + 2) % 16];

							data = object_code[0] * 65536 + object_code[1] * 256 + object_code[2];
							if (object_code[0] / 16 > 7) { // if negative
								data = data - 16777216;
							}
						}
						// if immediate, func_getdataformat3 function return data
						if (reg[0] > data) {
							reg[9] = -1; // if A register > data, then SW = -1
						}
						else if (reg[0] == data) {
							reg[9] = 0; // if A register == data, then SW = 0
						}
						else {
							reg[9] = 1; // if A register < data, then SW = 1
						}
					}
				}
				else if ((object_code[1] / 16) % 2 == 1) { // format 4
					object_code[3] = mem[reg[8] / 16][reg[8] % 16];
					reg[8]++;
					// get compare address
					data = func_getdataformat4(object_code);
					if ((object_code[1] % 16) > 7 && (object_code[1] % 16) <= 11) { // COMP : 28~2B
						if ((object_code[0] % 16) % 4 != 1) { // not immediate
							object_code[0] = mem[data / 16][data % 16];
							object_code[1] = mem[(data + 1) / 16][(data + 1) % 16];
							object_code[2] = mem[(data + 2) / 16][(data + 2) % 16];

							data = object_code[0] * 65536 + object_code[1] * 256 + object_code[2];
							if (object_code[0] / 16 > 7) { // if negative
								data = data - 16777216;
							}
						}
						// if immediate, func_getdataformat3 function return data
						if (reg[0] > data) {
							reg[9] = -1; // if A register > data, then SW = -1
						}
						else if (reg[0] == data) {
							reg[9] = 0; // if A register == data, then SW = 0
						}
						else {
							reg[9] = 1; // if A register < data, then SW = 1
						}
					}
				}
			}
			else if (opcode == 3) { // JEQ, J, JLT
				//check format
				if ((object_code[1] / 16) % 2 == 0) { // format 3
					// get address to jump
					data = func_getdataformat3(object_code);
					if ((object_code[0] % 16) < 4) { // JEQ : 30~33
						if (reg[9] == 0) {
							reg[8] = data;
						}
					}
					else if ((object_code[0] % 16) > 11 && (object_code[0] % 16) <= 15) { // J : 3C~3F
						reg[8] = data;
					}
					else if ((object_code[0] % 16) > 7 && (object_code[0] % 16) <= 11) { // JLT : 38~3B
						if (reg[9] == -1) {
							reg[8] = data;
						}
					}
				}
				else if ((object_code[1] / 16) % 2 == 1) {
					object_code[3] = mem[reg[8] / 16][reg[8] % 16];
					reg[8]++;
					// get address to jump
					data = func_getdataformat4(object_code);
					if ((object_code[0] % 16) < 4) { // JEQ : 30~33
						if (reg[9] == 0) {
							reg[8] = data;
						}
					}
					else if ((object_code[0] % 16) > 11 && (object_code[0] % 16) <= 15) { // J : 3C~3F
						reg[8] = data;
					}
					else if ((object_code[0] % 16) > 7 && (object_code[0] % 16) <= 11) { // JLT : 38~3B
						if (reg[9] == -1) {
							reg[8] = data;
						}
					}
				}
			}
			else if (opcode == 4) { // JSUB, RSUB
				// check format
				if ((object_code[1] / 16) % 2 == 0) { // format 3
					data = func_getdataformat3(object_code);
					if ((object_code[0] % 16) > 7 && (object_code[0] % 16) <= 11) { // JSUB : 48~4B
						reg[2] = reg[8];
						reg[8] = data;
					}
					else if ((object_code[0] % 16) > 11 && (object_code[0] % 16) <= 15) { // RSUB : 4C~4F
						reg[8] = reg[2];
					}
				}
				else if ((object_code[1] / 16) % 2 == 1) { // format 4
					object_code[3] = mem[reg[8] / 16][reg[8] % 16];
					reg[8]++;
					data = func_getdataformat4(object_code);
					if ((object_code[0] % 16) > 7 && (object_code[0] % 16) <= 11) { // JSUB : 48~4B
						reg[2] = reg[8];
						reg[8] = data;
					}
					else if ((object_code[0] % 16) > 11 && (object_code[0] % 16) <= 15) { // RSUB : 4C~4F
						reg[8] = reg[2];
					}
				}
			}
			else if (opcode == 5) { // STCH, LDCH
				//check format
				if ((object_code[1] / 16) % 2 == 0) { //format 3
					data = func_getdataformat3(object_code);

					if ((object_code[0] % 16) > 3 && (object_code[0] % 16) <= 7) { // STCH : 54~57
						mem[data / 16][data % 16] = reg[0] % 256; // A's rightmost byte -> 0000'00'
					}
					else if ((object_code[0] % 16) < 4) { // LDCH : 50~53
						data = mem[data / 16][data % 16];
						if ((object_code[0] % 16) % 4 != 1) { // not immediate
							if (data / 16 > 7) { // negative
								data = data - 256;
							}
						}
						else { // immediate
							if (data < 0) { // negative
								data = (data * (-1) % 256) * (-1);
							}
							else {
								data = data % 256;
							}
						}
						reg[0] = data;
					}
				}
				else if ((object_code[1] / 16) % 2 == 1) { // format 4
					object_code[3] = mem[reg[8] / 16][reg[8] % 16];
					reg[8]++;
					data = func_getdataformat3(object_code);

					if ((object_code[0] % 16) > 3 && (object_code[0] % 16) <= 7) { // STCH : 54~57
						mem[data / 16][data % 16] = reg[0] % 256; // A's rightmost byte -> 0000'00'
					}
					else if ((object_code[0] % 16) < 4) { // LDCH : 50~53
						if ((object_code[0] % 16) % 4 != 1) { // not immediate
							data = mem[data / 16][data % 16];
							if (data / 16 > 7) { // negative
								data = data - 256;
							}
						}
						else { // immediate
							if (data < 0) { // negative
								data = (data * (-1) % 256) * (-1);
							}
							else {
								data = data % 256;
							}
						}
						reg[0] = data;
					}
				}
			}
			else if (opcode == 6) { // LDB
				//check format
				if ((object_code[1] / 16) % 2 == 0) { // format 3
					data = func_getdataformat3(object_code);
					if ((object_code[0] % 16) > 7 && (object_code[0] % 16) <= 11) { // LDB : 68~6B
						reg[3] = data; // LDB #LENGTH
					}
				}
				else if ((object_code[1] / 16) % 2 == 1) { // format 4
					object_code[3] = mem[reg[8] / 16][reg[8] % 16];
					reg[8]++;
					data = func_getdataformat4(object_code);
					if ((object_code[0] % 16) % 4 != 1) { // not immediate
						object_code[0] = mem[data / 16][data % 16];
						object_code[1] = mem[(data + 1) / 16][(data + 1) % 16];
						object_code[2] = mem[(data + 2) / 16][(data + 2) % 16];

						data = object_code[0] * 65536 + object_code[1] * 256 + object_code[2];
						if (object_code[0] / 16 > 7) {
							data = data - 16777216;
						}
					}
					reg[3] = data;
				}
			}
			else if (opcode == 7) { // LDT
				// check format
				if ((object_code[1] / 16) % 2 == 0) { // format 3
					data = func_getdataformat3(object_code);
					if ((object_code[0] % 16) > 3 && (object_code[0] % 16) <= 7) { // LDT : 74~77
						if ((object_code[0] % 16) % 4 != 1) { // not immediate
							object_code[0] = mem[data / 16][data % 16];
							object_code[1] = mem[(data + 1) / 16][(data + 1) % 16];
							object_code[2] = mem[(data + 2) / 16][(data + 2) % 16];

							data = object_code[0] * 65536 + object_code[1] * 256 + object_code[2];
							if (object_code[0] / 16 > 7) {
								data = data - 16777216;
							}
						}
						reg[5] = data;
					}
				}
				else if ((object_code[1] / 16) % 2 == 1) { // format 4
					object_code[3] = mem[reg[8] / 16][reg[8] % 16];
					reg[8]++;
					data = func_getdataformat4(object_code);
					if ((object_code[0] % 16) > 3 && (object_code[0] % 16) <= 7) { // LDT : 74~77
						if ((object_code[0] % 16) % 4 != 1) { // not immediate
							object_code[0] = mem[data / 16][data % 16];
							object_code[1] = mem[(data + 1) / 16][(data + 1) % 16];
							object_code[2] = mem[(data + 2) / 16][(data + 2) % 16];

							data = object_code[0] * 65536 + object_code[1] * 256 + object_code[2];
							if (object_code[0] / 16 > 7) {
								data = data - 16777216;
							}
						}
						reg[5] = data;
					}
				}
			}
			else if (opcode == 13) { // WD, RD
				if ((object_code[0] % 16) > 7 && (object_code[0] % 16) <= 11) { // RD : D8~DB
					reg[0] = 0;
				}
				else if ((object_code[0] % 16) > 11 && (object_code[0] % 16) <= 15) {
					// do nothing
				}
			}
			else if (opcode == 14) { // TD
				if ((object_code[0] % 16) < 4) { // TD : E0~E3
					reg[9] = -1; /// set condition code '<'
				}
			}
		}
		if (reg[8] == (progaddr + program_len)) { // END program
			is_bp = 0;
			break;
		}
		else { // check break point
			current_bp = 0;
			while (current_bp<bp_len) {
				if (bp[current_bp] == reg[8]) {
					is_bp = 1;
					return 1;
				}
				current_bp++;
			}
		}
	}
	return 0;
}

// data process of format 3 opcode
// get target address
int func_getdataformat3(unsigned char* objcode) {


	int value = objcode[1] % 16 * 256 + objcode[2];
	int tmp1, tmp2;
	if (objcode[1] % 16 > 7) { // negative number, larger than 0111
		value = value - 4096;
	}

	// immediate addressing n = 0, i= 1
	if ((objcode[0] % 16) % 4 == 1) {
		// pc relative
		if (objcode[1] / 16 == 2) {
			value += reg[8];
		}
		// pc relatvie with X bit
		else if (objcode[1] / 16 == 10) {
			value += reg[8];
			value += reg[1];
		}
		// base relative
		else if (objcode[1] / 16 == 4) {
			value += reg[3];
		}
		// base relative with X bit
		else if (objcode[1] / 16 == 12) {
			value += reg[3];
			value += reg[1];
		}
	}
	// indirect addressing n = 1, i = 0
	else if ((objcode[0] % 16) % 4 == 2) {
		// pc relative
		if (objcode[1] / 16 == 2) {
			value += reg[8];
		}
		// pc relatvie with X bit
		else if (objcode[1] / 16 == 10) {
			value += reg[8];
			value += reg[1];
		}
		// base relative
		else if (objcode[1] / 16 == 4) {
			value += reg[3];
		}
		// base relative with X bit
		else if (objcode[1] / 16 == 12) {
			value += reg[3];
			value += reg[1];
		}
		tmp1 = mem[value / 16][value % 16];
		tmp2 = tmp1 * 65536;
		tmp1 = mem[(value + 1) / 16][(value + 1) % 16];
		tmp2 += (tmp1 * 256);
		tmp1 = mem[(value + 2) / 16][(value + 2) % 16];
		tmp2 += tmp1;

		value = tmp2;
	}
	// simple addressing n = 1, i = 1
	else if ((objcode[0] % 16) % 4 == 3) {
		// pc relative
		if (objcode[1] / 16 == 2) {
			value += reg[8];
		}
		// pc relatvie with X bit
		else if (objcode[1] / 16 == 10) {
			value += reg[8];
			value += reg[1];
		}
		// base relative
		else if (objcode[1] / 16 == 4) {
			value += reg[3];
		}
		// base relative with X bit
		else if (objcode[1] / 16 == 12) {
			value += reg[3];
			value += reg[1];
		}
	}
	// standard SIC instruction n = 0, i = 0 
	// no relative addressing
	else {
		// 8bits 1bit 15bits Address -> X000 0000 / 0000 0000
		value = (objcode[1] / 16) * 256 + objcode[2];
		// if x bit set
		if ((objcode[1] / 16) > 7) {
			value += reg[1];
		}
	}
	return value;
}
// data process of format 4 opcode
// get target address
int func_getdataformat4(unsigned char* objcode) {

	int value = (objcode[1] % 16) * 65536 + objcode[2] * 256 + objcode[3];
	// simple or immediate addressing (commonly constant value)
	if ((objcode[0] % 16) % 4 == 3) { //simple
		// if X bit set
		if ((objcode[1] / 16) > 8) {
			value += reg[1];
		}
	}
	else if ((objcode[0] % 16) % 4 == 1) { //immediate (constant)
		// if negative number
		if ((objcode[1] % 16) > 7) {
			value = value - 1048576;
		}
	}
	return value;
}

// print value of registers
void func_regvalue() {
	printf("A : %06X  X : %06X\n", reg[0], reg[1]);
	printf("L : %06X PC : %06X\n", reg[2], reg[8]);
	printf("B : %06X  S : %06X\n", reg[3], reg[4]);
	printf("T : %06X\n", reg[5]);
}

int main()
{
	func_alloc();
	int range[2]; int idx;
	int s_pos = 0; // dump memory start position

	char* cmd;

	char name1[20], name2[20], name3[20];
	int cslth = 0, secnum = 0;
	progaddr = 0; // default
	while (1)
	{
		print_prompt();
		cmd = (char*)malloc(sizeof(char) * MAX_SIZE);
		fgets(cmd, MAX_SIZE, stdin);

		cmd[strlen(cmd)] = '\0';

		if (!cmd)
		{
			exit(EXIT_SUCCESS);
		}
		if (cmd[0] == '\0' || strcmp(cmd, "\n") == 0)
		{
			free(cmd);
			continue;
		}

		//refine command
		cmd = Remove_Blanks(cmd);

		//instruction

		/*help*/
		if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0)
		{
			func_help();
			func_list(head, cmd);
		}
		/*dir*/
		/*
		else if (strcmp(cmd, "dir") == 0 || strcmp(cmd, "d") == 0)
		{
			func_dir();
			func_list(head, cmd);
		}
		*/
		/*quit*/
		else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0)
		{
			free(cmd);
			//free allocated memories
			func_free();
			break;
		}
		/*history*/
		else if (strcmp(cmd, "history") == 0 || strcmp(cmd, "hi") == 0)
		{
			//input command in list
			func_list(head, cmd);

			func_history(head);

		}
		/*dump, du*/
		else if (strncmp(cmd, "dump", 4) == 0 || strncmp(cmd, "du", 2) == 0)
		{
			/*dump or du */
			if (strncmp(cmd, "dump", 4) == 0) idx = func_range(cmd + 4, range);
			else idx = func_range(cmd + 2, range);


			if (idx == -1) {
				func_error("range input error!");
				free(cmd);
				continue;
			}
			else if (idx == -2) {
				free(cmd);
				continue;
			}
			else if (idx == 0)
				func_dump(s_pos, s_pos + 159, &s_pos);
			else if (idx == 1) {
				if (range[0] + 159 > 1048575)
					func_dump(range[0], 1048575, &s_pos);
				else
					func_dump(range[0], range[0] + 159, &s_pos);
			}
			else if (idx == 2)
				func_dump(range[0], range[1], &s_pos);
			func_list(head, cmd);
		}
		/*edit, e*/
		else if (strncmp(cmd, "edit", 4) == 0 || strncmp(cmd, "e", 1) == 0)
		{
			if (strncmp(cmd, "edit", 4) == 0) idx = func_edit(cmd + 4);
			else idx = func_edit(cmd + 1);
			if (idx == -1 || idx == -2) {
				free(cmd);
				continue;
			}
			else
				func_list(head, cmd);
		}
		/*fill, f*/
		else if (strncmp(cmd, "fill", 4) == 0 || strncmp(cmd, "f", 1) == 0)
		{
			if (strncmp(cmd, "fill", 4) == 0) idx = func_fill(cmd + 4);
			else idx = func_fill(cmd + 1);
			if (idx == -1 || idx == -2) {
				free(cmd);
				continue;
			}
			else
				func_list(head, cmd);
		}
		/*reset*/
		else if (strcmp(cmd, "reset") == 0)
		{
			func_reset();
			func_list(head, cmd);
		}
		/*opcodelist*/
		else if (strcmp(cmd, "opcodelist") == 0) {
			func_opcodelist();
			func_list(head, cmd);
		}
		/*opcode mnemonic*/
		else if (strncmp(cmd, "opcode", 6) == 0) {
			idx = func_mnemonic(cmd + 7);
			if (idx == -1) {
				free(cmd);
				continue;
			}
			else
				func_list(head, cmd);
		}
		/*type filename*/
		else if (strncmp(cmd, "type", 4) == 0) {
			idx = func_type(cmd + 5);
			if (idx == -1) {
				free(cmd);
				continue;
			}
			else
				func_list(head, cmd);
		}
		/*symbol*/
		else if (strcmp(cmd, "symbol") == 0) {
			idx = func_SYMTABPRINT();
			if (idx == -1) {
				free(cmd);
				continue;
			}
			else
				func_list(head, cmd);
		}
		/*assemble filename*/
		else if (strncmp(cmd, "assemble", 8) == 0) {
			idx = func_pass1(cmd + 9);
			if (idx == -1) {
				free(cmd);
				continue;
			}
			else {
				func_list(head, cmd);
			}
		}
		// change program address
		else if (strncmp(cmd, "progaddr", 8) == 0) {
			idx = func_hextodec(cmd + 9);
			if (idx == -1) {
				free(cmd);
				continue;
			}
			else {
				progaddr = idx;
				func_list(head, cmd);
			}
		}
		// linking loader
		else if (strncmp(cmd, "loader", 6) == 0) {
			secnum = func_CSECNUM(cmd, name1, name2, name3);
			if (secnum == -1) {
				func_error("Wrong Object file");
				free(cmd);
				continue;
			}
			else {
				// pass1
				csec = secnum;
				func_ESTABFREE();
				cslth = func_LOADpass1(name1, progaddr, 0);
				if (cslth == -1) {
					func_error("Error in obj file");
					free(cmd);
					continue;
				}
				if (secnum == 2) {
					cslth = func_LOADpass1(name2, cslth, 1);
				}
				else if (secnum == 3) {
					cslth = func_LOADpass1(name2, cslth, 1);
					if (cslth == -1) {
						func_error("Error in obj file");
						free(cmd);
						continue;
					}
					cslth = func_LOADpass1(name3, cslth, 2);
				}
				if (cslth == -1) {
					func_error("Error in obj file");
					free(cmd);
					continue;
				}

				// pass2
				cslth = func_LOADpass2(name1, progaddr, 0);
				if (cslth == -1) {
					func_error("Error in obj file");
					free(cmd);
					continue;
				}
				if (secnum == 2)
					cslth = func_LOADpass2(name2, cslth, 1);
				else if (secnum == 3) {
					cslth = func_LOADpass2(name2, cslth, 1);
					if (cslth == -1) {
						func_error("Error in obj file");
						free(cmd);
						continue;
					}
					cslth = func_LOADpass2(name3, cslth, 2);
				}
				if (cslth == -1) {
					func_error("Error in obj file");
					free(cmd);
					continue;
				}

				func_PRINTLOADMAP();
				func_initializeregister();
				func_list(head, cmd);
			}
		}
		// breakpoint
		else if (strncmp(cmd, "bp", 2) == 0) {
			idx = func_breakpoint(cmd);
			if (idx == -1) {
				func_error("Wrong Address");
				free(cmd);
				continue;
			}
			else {
				func_list(head, cmd);
			}
		}
		// run
		else if (strcmp(cmd, "run") == 0) {
			if (program_len != 0) {
				// no break points
				if (is_bp == 0) {
					reg[2] = progaddr + program_len; // set L register
					idx = func_run(progaddr + execaddr);
				}
				else {
					idx = func_run(bp[current_bp]);
				}
				func_regvalue();
				// end program
				if (idx == 0) {
					printf("\tEnd Program\n");
				}
				// break point
				else {
					printf("\tStop at checkpoint[%X]\n", bp[current_bp]);
				}
				func_list(head, cmd);
			}
			else {
				func_error("No file for execution");
				free(cmd);
				continue;
			}

		}
		/*wrong instruction*/
		else {
			func_error("Wrong instruction!");
			free(cmd);
			continue;
		}
		free(cmd);
	}

	return 0;
}

