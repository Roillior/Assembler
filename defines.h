#define MAX_CMD_LEN 5
#define MAX_FILE_NAME 80
#define MAX_REGISTER_LEN 3
#define MAX_ARGUMENT_LEN 30
#define MAX_FILE_NAME_LEN 100
#define MAXLINESIZE 80
#define MAX_INT 8192 /*will use for the random addressing return number*/
#define MAX_POS_INT 4095/*will use for the random addressing return number*/
#define NUM_CMD 16
#define NUM_REG 8
#define LABEL_TYPE_LEN 7 
#define DATA_LABEL "isdata"
#define CODE_LABEL "nodata"
#define MASK 0x7FFF
#define MASK_ZERO_TARGET_ADDRESSING ((0x7FFF)&(-13)) /*refresent 111111111110011*/
#define MASK_ZERO_SOURCE_ADDRESSING ((0x7FFF)&(-49)) /*refresent 111111111001111*/
#define MASK_ZERO_GROUP_BITS ((0x7FFF)&(-3073)) /*refresent 111001111111111*/
#define MAX_MEMORY 1000
#define SUCCESS 1
#define FAIL 0
#define YES 1
#define NO 0
#define NO_SYMBOL 0
#define WITH_SYMBOL 1
#define START_ADDRESS 100
#define ERROR -1
#define UNKNOWN 0
#define SYMBOL 1
#define VIRTUAL 2
#define ENTRY 3
#define EXTERN 4
#define COMMAND 5
#define VIRTUAL_DATA 1
#define VIRTUAL_STRING 2
#define DEST 2
#define SOURCE 1
#define PASSONE 1
#define PASSTWO 2
#define OFF 0
#define ON 1
#define ONE_STAR "1"
#define TWO_STARS "2"
#define THREE_STARS "3"
