// kgsws' Doom ACE
// Various utility stuff.
//

enum
{
	// hooks that modify Doom memory
	HOOK_RELADDR_ACE,	// replace 32bit 'call' or 'jmp' destination to ACE function
	HOOK_RELADDR_DOOM,	// replace 32bit 'call' or 'jmp' destination to DOOM function
	HOOK_JMP_ACE,	// same as 'RELADDR' but also writes 'jmp' opcode
	HOOK_JMP_DOOM,	// same as 'RELADDR' but also writes 'jmp' opcode
	HOOK_CALL_ACE,	// same as 'RELADDR' but also writes 'call' opcode
	HOOK_CALL_DOOM,	// same as 'RELADDR' but also writes 'call' opcode
	HOOK_UINT32,	// replace 32bit value
	HOOK_UINT16,	// replace 16bit value
	HOOK_UINT8,	// replace 8bit value
	HOOK_CSTR_ACE,	// replace cstring from ACE segment
	HOOK_CSTR_DOOM,	// replace cstring from Doom DATA segment
	HOOK_BUF8_ACE,	// replace buffer from ACE segment
	HOOK_ABSADDR_CODE, // replace 32bit pointer destination to Doom CODE segment
	HOOK_ABSADDR_DATA, // replace 32bit pointer destination to Doom DATA segment
	HOOK_MOVE_OFFSET, // move block by offset
	HOOK_SET_NOPS, // memset with 0x90
	// 'hooks' that modify ACE memory
	HOOK_IMPORT,	// get address of any variable
	HOOK_READ8,	// get value of 8bit variable
	HOOK_READ16,	// get value of 16bit variable
	HOOK_READ32,	// get value of 32bit variable
	// address is referenced to
	CODE_HOOK = 0x40000000,
	DATA_HOOK = 0x80000000,
};

#define HOOK_MOVE_VAL(size,offset)	(((size) << 16) | ((offset) & 0xFFFF))

typedef struct
{
	uint32_t addr;
	uint32_t type;
	uint32_t value;
} hook_t;

#define relocate_addr_code(x)	((uint32_t)(x)+doom_code_segment)
#define relocate_addr_data(x)	((uint32_t)(x)+doom_data_segment)
#define relocate_ptr_code(x)	((void*)((uint32_t)(x)+doom_code_segment))
#define relocate_ptr_data(x)	((void*)((uint32_t)(x)+doom_data_segment))

extern uint32_t doom_code_segment;
extern uint32_t doom_data_segment;

void utils_init();
void utils_install_hooks(const hook_t *table, uint32_t count);

