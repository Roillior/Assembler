#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "structs.h"
#include "defines.h"
#include "usedMethods.h"

int conv_index = 0;/*will use as an index to the recursive conveert function*/

int symbolCheck(char *str){/* function check if srt is a valid symbol*/
	if(strlen(str) < 2)/* symbol have to be longer then 2 char (each one finish with ":")*/
		return UNKNOWN;
	if(strlen(str) > MAX_ARGUMENT_LEN ){
		printf("ERROR -In %s symbol name can be at most 30 char \n", str);
		return ERROR;
	}
	if(str[strlen(str) - 1 ] == ':'){/*check that the label finish with ":" */
		if(checkLabelName(str , strlen(str) - 1) == NO){ /*check if the label name is ok*/
			printf("ERROR -In %s , Label name is not legal \n", str);
			return ERROR;
		}
		return SYMBOL;
	}
	return UNKNOWN;
	
}

int isImmediateAdd(char *str , int *n){/* function that checks if the addressing number 0*/
	if(strlen(str) < 2){
		return NO;
	}
	if(str[0] != '#'){
		return NO;
	}
	if(sscanf(str + 1, "%d", n) == 1){
		return YES;
	}
	return NO;
}

int checkRegister(char *str, register_names *rname, int *index){/* function that checks register name if it is update the index*/
	int ind;
	for(ind = 0; ind < NUM_REG; ind++){
		if(!strcmp(str , rname[ind].name)){
			*index = ind;
			return YES;
		}
	}
	return NO;
}

int isRandAdd(char *str , int *n ,int *numOfStar ){/* function that checks if its addressing number 2 - (first operand with stars)*/
	int ind;
	time_t t;/*for using random numbers*/
	int num;
	char s[5];
	srand((unsigned) time(&t));
	if(strlen(str) > 3){
		return NO;
	}
	if(str[0] != '*'){
		return NO;
	}
	ind = strncmp(str, "**",3 ); /*if ind is negative there is 1 star if positive 3 stars if its zero 2 stars*/
	if(ind < 0){
		num = rand() % NUM_REG; /*get random number betwin 0 - 7*/
		sprintf(s,"%d",num);
		sscanf(s, "%d", n);
		sscanf("1", "%d", numOfStar);
		return YES;
	}
	else if(ind == 0){
		num = rand() % MAX_INT;/*get random number betwin 0 to 8191 */
		if(num > MAX_POS_INT){/*if num is bigger then 4095 (max int num) then make it negativ number betwin -1 to -4096*/
			num = (MAX_POS_INT - num);
			sprintf(s,"%d",num);
			sscanf(s, "%d", n);
			sscanf("2", "%d", numOfStar);
		}
		else{
			sprintf(s,"%d",num);
			sscanf(s, "%d", n);
			sscanf("2", "%d", numOfStar);
		}
		return YES;
	}
	else{
		sscanf("3", "%d", numOfStar);
		return YES;
	}
	
}

int checkLabelName(char *str, int len){/*function that checks the label name syntax is good*/
	int ind;
	if(len > MAX_ARGUMENT_LEN) /*if label name to long return */
		return NO;
	for(ind = 0; ind < len; ind++){/*check that this is a legal label name */
		if((str[ind] >= 'A' && str[ind] <= 'Z') || (str[ind] >= 'a' && str[ind] <= 'z') || (ind > 0 && str[ind] >= '0' && str[ind] <= '9')){
			continue;
		}
		return NO;
	}
	return YES;
}

int isVirtual(char *str, unsigned char *type){/*function that checks if its .string or .data*/
	if(!strcmp(str, ".string")){
		*type = VIRTUAL_STRING;
		return VIRTUAL;
	}
	if(!strcmp(str, ".data")){
		*type = VIRTUAL_DATA;
		return VIRTUAL;
	}
	return UNKNOWN;
}

int isEntry(char *str){/*function that checks if its .entry*/
	if(!strcmp(str, ".entry"))
		return ENTRY;
	return UNKNOWN;
}

int isExtern(char *str){/*function that checks if its extern*/
	if(!strcmp(str, ".extern")){
		return EXTERN;}
	return UNKNOWN;
}

int isCommand(char *str, int *index, cmd_names *commands){/*function that checks for valid command*/
	int ind;
	if(!strncmp(str,"stop",4)){
		*index = 15;
		return COMMAND;
	}
	for(ind = 0; ind < (NUM_CMD - 1) ;ind++){
		if(!strncmp(commands[ind].name,str, 3)){
			*index = ind;
			return COMMAND;
		}
	}
	return UNKNOWN;
}

void makeEntryFile(char *name, entries_struct *e_table , int counter ){/*function that will prepare the output entry file*/
	int ind;
	FILE *file;
	char str[MAXLINESIZE];
	char temp[MAXLINESIZE] = "";
	conv_index = 0;/*reset the convert index to zero*/
	if(counter < 1){
		return;
	}
	file = fopen(name,"w");
	for(ind = 0; ind < counter; ind++){
		convert(e_table[ind].address , temp);/*convert the address number to base 32*/
		sprintf(str, "%s	%s\n", e_table[ind].name, temp);
		fputs(str, file);
	}
	fclose(file);
}

void makeExternalFile(char *name, externals_struct *ex_table, int counter){/*function that will prepare the output extern file*/
	int ind;
	FILE *file;
	char str[MAXLINESIZE];
	char temp[MAXLINESIZE] = "";
	conv_index = 0;/*reset the convert index to zero*/
	if(counter < 1){
		return;
	}
	file = fopen(name, "w");
	for(ind = 0; ind < counter; ind++){
		convert(ex_table[ind].address ,temp);/*convert the address number to base 32*/
		sprintf(str, "%s	%s\n", ex_table[ind].name, temp);
		fputs(str, file);
	}
	fclose(file);
}

void makeObjFile(char *name,obj_struct *o_table,long *memory ,int i_count ,int d_count ){/*function that will prepare the output object file*/

	FILE *file;
 	file = fopen(name, "w");
	printHeader(name, i_count, d_count, file);/*print the header of the object file*/
	printInstructions(o_table, i_count, file);/*print the instructions addresses and code to the object file*/
	printData(memory, i_count, d_count, file);/*print the data addresses and code to the object file*/
	fclose(file);
}


void printHeader(char *name,int i_count ,int d_count, FILE *file) {/*function that will print the header of the object file*/

  	char str[MAXLINESIZE];
	char temp1[MAXLINESIZE] = "";
	char temp2[MAXLINESIZE] = "";
  	
	sprintf(str, "Base 32 Address	Base 32 machine code\n");
	fputs(str, file);
  	conv_index = 0;/*reset the convert index to zero*/
	convert(i_count, temp1);/*convert the number to base 32*/
	conv_index = 0;/*reset the convert index to zero*/
	convert(d_count, temp2);/*convert the number to base 32*/
	conv_index = 0;/*reset the convert index to zero*/
	sprintf(str, "	%s	%s\n", temp1, temp2);
	fputs(str, file);
}

void printInstructions(obj_struct *o_table ,int i_count , FILE *file) {/*function print the instructions address and code*/
  	
	int ind;
	for(ind = 0; ind < i_count; ind++){
              	char str[MAXLINESIZE];
		char temp1[MAXLINESIZE] = "";
		char temp2[MAXLINESIZE] = "";
		convert(o_table[ind].start_address, temp1);/*convert the number to base 32*/
		conv_index = 0;/*reset the convert index to zero*/
		convert(o_table[ind].bin_code, temp2);/*convert the number to base 32*/
		conv_index = 0;/*reset the convert index to zero*/
		sprintf(str, "%s	%s\n",temp1, temp2);
		fputs(str, file);

	}
}

void printData(long *memory ,int i_count ,int d_count , FILE *file) {/*function print the data address and code*/
 	
	int ind;
  	for(ind = 0; ind < d_count; ind++){
    	  	char str[MAXLINESIZE];
 	  	char temp1[MAXLINESIZE] = "";
 	   	char temp2[MAXLINESIZE] = "";
		convert(START_ADDRESS + i_count + ind, temp1);/*convert the number to base 32*/
		conv_index = 0;/*reset the convert index to zero*/
		convert(memory[ind], temp2);/*convert the number to base 32*/
		conv_index = 0;/*reset the convert index to zero*/
		sprintf(str, "%s	%s\n", temp1, temp2);
		fputs(str, file);

	}
}

void convert(unsigned n, char *str){/*recursive function that convert from base 10 to base 32*/
    	n &= MASK; /* fix the bits for negativ numbers (take only the most right 15 bits)*/
	int dig;
	conv_index = 0;/*reset the convert index to zero*/
	static char *conv = "0123456789ABCDEFGHIJKLMNOPQRSTUV"; /*all the "digits" of base 32*/
	if((dig = (n / 32))){/*if n bigger then 32 dig is bigger then 0 and go inside the if other wise not go inside*/
		convert(dig , str);/*use recursive call to convert*/
	}
	str[conv_index++] = conv[n % 32];
}


