#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "defines.h"
#include "assembler.h"
#include "usedMethods.h"

int dc ; /* data counter */
int ic ; /* instruction counter */
int passNum; /*will mark the pass we are in one or two */
int opFlag = OFF;/* use as a flag to check if both operands are registers*/
int arg_type;
int arg_pos;/*argument position in the reading line */
int s_counter ; /* will use as counter for symboles */
int e_counter ; /* will use as counter for enteries */
int external_counter ;/* will use to count the external totaly*/
int ext_counter ; /* will use as counter for external in the array */
int errorExist ; /* will mark if there is an error */
int cur_ic; /* will mark the current position of the instruction counter */
char checkLine[MAXLINESIZE];
char *ptr; /* will point to the current arrgument in the line */
char exterArr[MAX_MEMORY][MAX_ARGUMENT_LEN];
long memo[MAX_MEMORY] = {0};/*initialize / reset the memory to all 0*/



register_names registers[NUM_REG] = { /* initialize the register structure */
	{"r0", 0},
	{"r1", 1},
	{"r2", 2},
	{"r3", 3},
	{"r4", 4},
	{"r5", 5},
	{"r6", 6},
	{"r7", 7}
};

cmd_names commands[NUM_CMD] = { /* initialize the commands structure first comand name then the command index and last a pointer for function that will parse this command */
	{"mov", 0, &twoOperands},
	{"cmp", 1, &twoOperands},
	{"add", 2, &twoOperands},
	{"sub", 3, &twoOperands},
	{"not", 4, &oneOperand},
	{"clr", 5, &oneOperand},
	{"lea", 6, &twoOperands},
	{"inc", 7, &oneOperand},
	{"dec", 8, &oneOperand},
	{"jmp", 9, &oneOperand},
	{"bne", 10, &oneOperand},
	{"red", 11, &oneOperand},
	{"prn", 12, &oneOperand},
	{"jsr", 13, &oneOperand},
	{"rts", 14, &nonOperand},
	{"stop", 15, &nonOperand}
};

externals_struct extern_table[MAX_MEMORY];/* define the externals table */

entries_struct entry_table[MAX_MEMORY];/* define the entries table */

obj_struct object_table[MAX_MEMORY];/* define the object table */

symbols_struct symbol_table[MAX_MEMORY];/* define symbols table */


void counterSetup(){/*setup all the counters to zero*/
	dc = 0; 
	ic = 0;
	cur_ic = 0;
	s_counter = 0;
	external_counter = 0;
	e_counter = 0;
	ext_counter = 0;
	errorExist = NO;
	passNum = PASSONE;
	
}
void firstPass(char line[]){ /* first pass algoritem */
	int symbolFlag = NO_SYMBOL;
	int c_index = -1; /* mark a command index (valid command is 0 - 15 )*/
	unsigned char virtual_type = 0; /*will mark if its .data or .string */
	char labelName[MAX_ARGUMENT_LEN];
	
	arg_pos = 1; 
	arg_type = UNKNOWN;
	strncpy(checkLine , line , MAXLINESIZE);
	ptr = strtok(checkLine, "  \t\n");
	if(ptr == NULL || strlen(ptr) < 1 || ptr[0] == ';')/*check if it is a comment or empty line*/
		return;
	
	while(ptr != NULL){
		arg_type = symbolCheck(ptr);/*check if first word in line is symbol*/
		if(arg_type == UNKNOWN){
			arg_type = isVirtual(ptr , &virtual_type); }/*check if its virtual instruction (.data or .string) */
		if(arg_type == UNKNOWN){
			arg_type = isEntry(ptr); }/* check if its entry instruction */
		if(arg_type == UNKNOWN){
			arg_type = isExtern(ptr); }/* check if its extern instruction */
		if(arg_type == UNKNOWN){
			arg_type = isCommand(ptr, &c_index, commands); }/* check if its valid command */
		
		if(arg_type == SYMBOL ){
			if(arg_pos > 1){ /* symbol can be only as first argument */
				printf("ERROR -In symbol %s, symbols definition is allowed only in the beggining of the line - as 1st argument \n", ptr);
				errorExist = YES;
				return;
			}
			memset(labelName, 0, MAX_ARGUMENT_LEN);/* reset label name to all 0 */
			strncpy(labelName, ptr, strlen(ptr) - 1); /* copy the label name without the : in its end */
			symbolFlag = WITH_SYMBOL; /* update the symbol flag */
			ptr = strtok(NULL," \t\n"); /*get the next argument in the line */
			if(ptr != NULL){/* update the argument position */
				arg_pos++; 
			}
		}
		else if(arg_type == VIRTUAL){
			if(symbolFlag == WITH_SYMBOL){
				strcpy(symbol_table[s_counter].name , labelName);
				symbol_table[s_counter].address = 0 ;/* will get the address only in the 2nd pass */
				strcpy(symbol_table[s_counter].type , DATA_LABEL);
				symbol_table[s_counter].value = dc;
				s_counter++;
			}
			if(dataParse(virtual_type) == NO){ /* put the data in the memory if failed do so print error*/
				printf("ERROR -In %s , data could not be saved in the memory \n", ptr);
				errorExist = YES;
			}
			return;
		}
		else if(arg_type == ENTRY){
			ptr = strtok(NULL," \t\n"); /*get the next argument in the line */
			strcpy(entry_table[e_counter].name, ptr);
			entry_table[e_counter].address = 0 ;/* will get the address only in the 2nd pass */
			e_counter++;
			return;
		}
		else if(arg_type == EXTERN){
			ptr = strtok(NULL," \t\n"); /*get the next argument in the line */
			strcpy(exterArr[ext_counter++], ptr); 
			return;
		}
		else if(arg_type == COMMAND){
			if(symbolFlag == WITH_SYMBOL){/* will add the new label to the symbol tables */
				strcpy(symbol_table[s_counter].name , labelName);
				symbol_table[s_counter].address = START_ADDRESS + ic;
				strcpy(symbol_table[s_counter].type , CODE_LABEL);
				symbol_table[s_counter].value = ic;
				s_counter++;
			}
			if(encodeCommand(c_index) == NO){
				printf("ERROR -In %s , The command is not vallid \n", ptr);
				errorExist = YES;	
			}
			return;
		}
		else{
			printf("ERROR -In %s , The argument type in this line is not a vallid \n", ptr);
			errorExist = YES;
			return;
		} 	
	}	
}
void countZero(){/*prepare to pass two*/
	ic = 0;	
	passNum = PASSTWO;
}

void secondPass(char line[]){
	int c_index = -1; /* mark a command index (valid command is 0 - 15 )*/
	unsigned char virtual_type = 0; /*will mark if its .data or .string */
	arg_type = UNKNOWN;
	arg_pos = 1; 
	strncpy(checkLine , line , MAXLINESIZE);
	ptr = strtok(checkLine, " \t\n");
	if(ptr == NULL || strlen(ptr) < 1 || ptr[0] == ';'){/*check if it is a comment or empty line*/
		return;
	}
	while(ptr != NULL){
		arg_type = symbolCheck(ptr);/*check if first word in line is symbol*/
		if(arg_type == UNKNOWN){
			arg_type = isVirtual(ptr , &virtual_type); }/*check if its virtual instruction (.data or .string) */
		if(arg_type == UNKNOWN){
			arg_type = isEntry(ptr); }/* check if its entry instruction */
		if(arg_type == UNKNOWN){
			arg_type = isExtern(ptr); }/* check if its extern instruction */
		if(arg_type == UNKNOWN){
			arg_type = isCommand(ptr, &c_index, commands); }/* check if its valid command */
		
		if(arg_type == SYMBOL ){
			ptr = strtok(NULL, " \t\n");
			if(ptr != NULL){
				arg_pos++;
			}
		}
		else if(arg_type == VIRTUAL){
			return; /* read the next line */
		}
		else if(arg_type == ENTRY){
			ptr = strtok(NULL, " \t\n");
			updateAddOfEntry(ptr);
			return;/* read the next line */
		}
		else if(arg_type == EXTERN){
			ptr = strtok(NULL, " \t\n");
			return;/* read the next line */
		}
		else if(arg_type == COMMAND){
			if(encodeCommand(c_index) == NO){
				printf("ERROR -In %s , The command is not vallid \n", ptr);
				errorExist = YES;
			}
			return;/* read the next line */
		}
		else{
			printf("ERROR - In %s , The argument type in this line is not vallid \n", ptr);
			errorExist = YES;
			return;
		}
	}
}

int dataParse(unsigned char type){
	int ind = 0;
	int pos;
	int data;
	
	if(type == VIRTUAL_DATA){/* the data that entered is .data*/
		ptr = strtok(NULL, ","); /* take each number alone - the data is apart by comma (,) */
		if(ptr == NULL){/* no data found */
			printf("ERROR - There is no argument after .data \n");
			return FAIL;	
		}
		while(ptr != NULL){
			ind = sscanf(ptr , "%d", &data);
			if(ind == 1){
				memo[dc++] = data & MASK;/*save the data in the memory array*/
			}
			else{/* data is not legal*/
				printf("ERROR -In %s , The data that entered is not legal \n", ptr);
				return FAIL;
			}
			ptr = strtok(NULL,",");
		}
	}
	else if(type == VIRTUAL_STRING){/* the data that entered is .string */
		pos = ptr - checkLine + 8; /* check the pos that the string start the "+8" is because .string + 1 gap is taking 8 spaces*/
		for(;pos < MAXLINESIZE && checkLine[pos] == ' ';pos++); /*count the gaps */
		if(pos >= MAXLINESIZE){
			printf("ERROR - There is missing argument after .string \n");
			return FAIL;
		}
		if(checkLine[pos] != '"'){/* check string that start with " */
			printf("ERROR -In %s , string that entered is no ligal \n", ptr);
			return FAIL;
		}
		pos += 1; /* set the pos one char after the " sign*/
		for(;pos < MAXLINESIZE && checkLine[pos] != '"';pos++){
			if(checkLine[pos] < 32 || checkLine[pos] > 127){ /* check that all char entered are legal 32-127 in asci table */
				printf("ERROR -In %s, charters entered are not legal \n", ptr);
				return FAIL;
			}
			memo[dc + ind++] = checkLine[pos];/* update the memory array  */
		}
		if(pos >= MAXLINESIZE){/* check that the line is not over max line size */
			printf("ERROR -In %s , line is to long \n",ptr);
			return FAIL;
		}
		memo[dc + ind++] = '\0'; /* add \0 in end of the string in the memory array and add 1 to the data counter*/
		dc += ind;	
	}
	return SUCCESS;
}

int encodeCommand(int index){
	int res;
	char com_name[MAX_ARGUMENT_LEN];
	char *str;
	cur_ic = ic;
	
	strcpy(com_name , ptr); /*get the command name from the current line */
	str = strtok(com_name , " ");
	if(str == NULL){
		printf("ERROR - Unknown command \n");
		return FAIL;
	}
	if(index == 0 || index == 1 || index == 2 || index == 3 || index == 6){/* command that get 2 operands */
		object_table[ic].bin_code = (object_table[ic].bin_code | (2 << 10)); /* update the group bits to 2*/
	}
	else if(index == 4 || index == 5 || index == 7 || index == 8 || index == 9 || index == 10 || index == 11 || index == 12 || index == 13){/* command that get 1 operands */
		object_table[ic].bin_code = (object_table[ic].bin_code | (1 << 10));/* update the group bits to 1*/
	}
	else{/* command that get 0 operands */
		object_table[ic].bin_code = (object_table[ic].bin_code & MASK_ZERO_GROUP_BITS);/* update the group bits to 0*/
	}
	object_table[ic].bin_code = object_table[ic].bin_code | (index << 6); /* update the bits with the operation code */
	object_table[ic].start_address = (START_ADDRESS + ic);
	res = commands[index].p_cmd();
	if(res == FAIL){
		return FAIL;
	}
	ic++; 
	return SUCCESS;	
}

int nonOperand(){/* function to check commands that get zero operands */
	return SUCCESS;
}

int oneOperand(){/* function to check commands that get one operands */
	ptr = strtok(checkLine + (ptr - checkLine) + strlen(ptr) + 1, " \t\n");/*get the operand */
	if(ptr == NULL){
		printf("ERROR - There is need to be one operand \n");
		return FAIL;
	}
	return encodeOperands(DEST);
}

int twoOperands(){/* function to check commands that get two operands */
	int res;
	ptr = strtok(checkLine + (ptr - checkLine) + strlen(ptr) + 1, " ,");/*get the 1st operand */
	if(ptr == NULL){
		printf("ERROR - There is need to be two operands \n");
		return FAIL;
	}
	res = encodeOperands(SOURCE);
	if(res == FAIL)
		return FAIL;
	ptr = strtok(NULL, " ,\t\n"); /* get the 2nd operand */
	if(ptr == NULL){
		printf("ERROR - There is need to be two operands \n");
		return FAIL;
	}
	res = encodeOperands(DEST);
	opFlag = OFF;/* set off the flag of registers*/
	return res;
}

int encodeOperands(int type){ /* function that will encode the operands by bits*/
	int ind;
	int n = 0;
	int numOfStar = 0;
	int add;
	if(checkRegister(ptr, registers,&ind ) == YES){
		if(type == SOURCE){
			object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (3 << 4)); /*update the 4 and 5 bits to register addressing*/
			ic++;/* add one to the instruction counter*/
			object_table[ic].bin_code = (object_table[ic].bin_code | (ind << 8)); 
			object_table[ic].start_address = START_ADDRESS + ic;
			opFlag = ON;/* set on the flag of registers*/
		}
		else{
			if(opFlag == ON){/*check if the was also a register as 1st argument for sharing the extra word*/
				object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (3 << 2)); /*update the 2 and 3 bits to register addressing*/
				object_table[ic].bin_code = (object_table[ic].bin_code | (ind << 2));
			}
			else{
				object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (3 << 2)); /*update the 2 and 3 bits to register addressing*/
				ic++;/* add one to the instruction counter*/
				object_table[ic].bin_code = (object_table[ic].bin_code | (ind << 2));
				object_table[ic].start_address = START_ADDRESS + ic;
			}
		}
		return SUCCESS;
	}
	if(isImmediateAdd(ptr , &n) == YES){/* check if the first argument start with # then we need to use the 1st addressing option */
		if(type == SOURCE){
			object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code & MASK_ZERO_SOURCE_ADDRESSING);/*make bits 4 and 5 as zeros*/
		}
		else{/*operand is a target operand*/
			object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code & MASK_ZERO_TARGET_ADDRESSING);/*make bits 2 and 3 as zeros*/
		}
		ic++; /* add one to the instruction counter*/
		object_table[ic].bin_code = n;
		object_table[ic].bin_code =( object_table[ic].bin_code << 2);/* make the last two bits 00 */
		object_table[ic].start_address =( START_ADDRESS + ic);
		return SUCCESS;
	}
	if(checkLabelName(ptr, strlen(ptr)) == YES){
		if(type == SOURCE){
			object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (1 << 4));
		}
		else{
			object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (1 << 2));
		}
		ic++; /* add one to the instruction counter*/
		add = getLabelAdd(ptr);
		object_table[ic].bin_code = add;
		object_table[ic].start_address = (START_ADDRESS + ic);
		if(passNum == PASSTWO){
			if(add < 0){
				printf("ERROR -In %s , label is not exist \n", ptr);
				return FAIL;
			}
			if(add == 0){
				addExtern(ptr, START_ADDRESS + ic);
				object_table[ic].bin_code = 1;/* make the last two bits of external 01 (ARE bits)*/
			}
			else{
				object_table[ic].bin_code = ((object_table[ic].bin_code << 2) | (1 << 1));/*make the symbol take the left 13 bits and add ARE bits (10)*/
			}
		}
		return SUCCESS;
	}
	if(type == SOURCE){
		if(isRandAdd(ptr , &n ,&numOfStar ) == YES){
			switch(numOfStar){
				case 1:
					object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (1 << 12)); /*update the 12 and 13 bits*/
					object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (2 << 4));/*update the 4 and 5 bits*/
					ic++;/* add one to the instruction counter*/
					object_table[ic].bin_code = (object_table[ic].bin_code | (n << 8));
					object_table[ic].start_address = (START_ADDRESS + ic);
					opFlag = ON;/* set on the flag of registers*/
					return SUCCESS;
				case 2:
					object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (2 << 12));/*update the 12 and 13 bits*/
					object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (2 << 4));/*update the 4 and 5 bits*/
					ic++; /* add one to the instruction counter*/
					object_table[ic].bin_code = n;
					object_table[ic].bin_code = (object_table[ic].bin_code << 2);/* make the last two bits 00 */
					object_table[ic].start_address = (START_ADDRESS + ic);
					return SUCCESS;
				case 3:
					object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (3 << 12));/*update the 12 and 13 bits*/
					object_table[cur_ic].bin_code = (object_table[cur_ic].bin_code | (2 << 4));/*update the 4 and 5 bits*/
					ic++;
					if(s_counter == 0){
						printf("ERROR -In %s , There is no availbel label \n", ptr);
						return FAIL;
					}
					add = symbol_table[0].address;
					object_table[ic].bin_code = add;
					object_table[ic].bin_code = (object_table[ic].bin_code | (2));/*update the 0 and 1 bits*/
					object_table[ic].start_address = (START_ADDRESS + ic);
					return SUCCESS;
				default:
					printf("ERROR -In %s , ilegal operand \n", ptr);
					return FAIL;
			}
		}
	}
	
	printf("ERROR - In %s , The operand is not legal \n", ptr);
	return FAIL;
}

int getLabelAdd(char *str){/* function that return the address of the wanted label*/
	int add = ERROR;
	int ind;
	for(ind = 0; ind < s_counter; ind++){/* try to find the label in the label tabel*/
		if(!strcmp(symbol_table[ind].name, str)){
			add = symbol_table[ind].address;
			return add;
		}
	}
	for(ind = 0; ind < ext_counter ; ind++){/*try to find the label in the external table */
		if(!strcmp(exterArr[ind], str)){
			return UNKNOWN;
		}
	}
	return add;
}

void addExtern(char *label, int add){/* function that will add externs to the extern table*/
	strcpy(extern_table[external_counter].name, label);
	extern_table[external_counter].address = add;
	external_counter++;
}

int isError(){/* function that return if was some error some where while reading the current file*/
	return errorExist;
}

void updateAddOfSymbols(){/* function that update the address of the symbols*/
	int ind;
	for(ind = 0; ind < s_counter; ind++){
		if(!strcmp(symbol_table[ind].type , DATA_LABEL)){
			symbol_table[ind].address = (START_ADDRESS + symbol_table[ind].value + ic);
		}
	}
}

int updateAddOfEntry(char *str){/* function that update the address of the entries*/
	int add = ERROR;
	int ind;
	for(ind = 0; ind < s_counter; ind++){
		if(!strcmp(symbol_table[ind].name, str)){
			add = symbol_table[ind].address;
			break;
		}
	}
	if(add == ERROR){
		printf("ERROR -In %s , That entry not exist \n", str);
		return FAIL;
	}
	for(ind = 0; ind < e_counter; ind++){
		if(!strcmp(entry_table[ind].name, str)){
			entry_table[ind].address = add;
			break;
		}
	}
	return SUCCESS;
}

void makeFiles(char *name){/* function that will prepare the output files if the was no error in program*/
	char fileName[MAX_FILE_NAME_LEN];
	char *fname;
	fname = strtok(name," \n");/* take name of file*/
	strcpy(fileName, fname);
	strcat(fileName, ".ent");
	makeEntryFile(fileName, entry_table , e_counter );
	strcpy(fileName, fname);
	strcat(fileName, ".ext");
	makeExternalFile(fileName, extern_table,external_counter );
	strcpy(fileName, fname);
	strcat(fileName, ".ob");
	makeObjFile(fileName, object_table, memo, ic, dc );
	
}

void clearGlobalStructs(){/*function that clean the data in the globel structs as prepare to read the next assembly file */
	clearExternTable();
	clearEntryTable();
	clearObjectTable();
}

void clearExternTable(){/*function that reset the global extern table */
	int ind;
	for(ind = 0; ind < external_counter; ind++){
		extern_table[ind].address = 0;
	}
}

void clearEntryTable(){/*function that reset the global entry table */
	int ind;
	for(ind = 0; ind < e_counter; ind++){
		entry_table[ind].address = 0;
	}
}

void clearObjectTable(){/*function that reset the global object table */
	int ind;
	for(ind = 0; ind < ic; ind++){
		object_table[ind].bin_code = 0;
		object_table[ind].start_address = 0;
	}
}

