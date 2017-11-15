#include "defines.h"

typedef struct{
	char name[MAX_CMD_LEN];/*command name */
	unsigned char num; /* index */
	int (*p_cmd)(); /* pointer to a function that will parse the wanted command */
}cmd_names;

typedef struct{
	char name[MAX_REGISTER_LEN]; /*register name */
	unsigned char num; /* index */
}register_names;

typedef	struct{
	char name[MAX_ARGUMENT_LEN];
	char type[LABEL_TYPE_LEN];
	int address;
	int value;
}symbols_struct;

typedef struct{
	char name[MAX_ARGUMENT_LEN];
	int address;
}entries_struct;

typedef struct{
	char name[MAX_ARGUMENT_LEN];
	int address;
}externals_struct;

typedef struct{
	long bin_code;
	int start_address;
}obj_struct;
