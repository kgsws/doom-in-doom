// kgsws' Doom ACE
// Various utility functions.
//
#include "engine.h"
#include "utils.h"

// this is '.hooks' section
extern const hook_t _hooks_start;
extern const hook_t _hooks_end;

//
// API

void utils_init()
{
	// this will install all hooks in '.hooks' section
	// don't call this function if you don't use this section
	utils_install_hooks(&_hooks_start, ((void*)&_hooks_end - (void*)&_hooks_start) / sizeof(hook_t));
}

void utils_install_hooks(const hook_t *table, uint32_t count)
{
	do
	{
		uint32_t addr = table->addr;

		if(table->type & DATA_HOOK)
			addr += doom_data_segment;
		else
		if(table->type & CODE_HOOK)
			addr += doom_code_segment;

		switch(table->type & 0xFFFF)
		{
			// these modify Doom memory
			case HOOK_CALL_ACE:
				*((uint8_t*)addr) = 0xE8;
				addr++;
			goto reladdr_ace;
			case HOOK_JMP_ACE:
				*((uint8_t*)addr) = 0xE9;
				addr++; // fall trough
			case HOOK_RELADDR_ACE:
reladdr_ace:
				*((uint32_t*)(addr)) = table->value - (addr + 4);
			break;
			case HOOK_CALL_DOOM:
				*((uint8_t*)addr) = 0xE8;
				addr++;
			goto reladdr_doom;
			case HOOK_JMP_DOOM:
				*((uint8_t*)addr) = 0xE9;
				addr++; // fall trough
			case HOOK_RELADDR_DOOM:
reladdr_doom:
				*((uint32_t*)(addr)) = (table->value + doom_code_segment) - (addr + 4);
			break;
			case HOOK_UINT8:
				*((uint8_t*)addr) = table->value;
			break;
			case HOOK_UINT16:
				*((uint16_t*)addr) = table->value;
			break;
			case HOOK_UINT32:
				*((uint32_t*)addr) = table->value;
			break;
			case HOOK_CSTR_ACE:
				strcpy((char*)addr, (char*)table->value);
			break;
			case HOOK_CSTR_DOOM:
				strcpy((char*)addr, (char*)(table->value + doom_data_segment));
			break;
			case HOOK_BUF8_ACE:
				memcpy((char*)addr, (char*)(table->value + 1), *((uint8_t*)table->value));
			break;
			case HOOK_ABSADDR_CODE:
				*((uint32_t*)addr) = table->value + doom_code_segment;
			break;
			case HOOK_ABSADDR_DATA:
				*((uint32_t*)addr) = table->value + doom_data_segment;
			break;
			case HOOK_MOVE_OFFSET:
			{
				uint16_t size = table->value >> 16;
				int16_t offset = table->value & 0xFFFF;

				if(offset > 0)
				{
					// must go from back
					uint8_t *src = (uint8_t*)addr;
					uint8_t *dst = (uint8_t*)addr + offset;
					for(int32_t i = size-1; i >= 0; i--)
						dst[i] = src[i];
				} else
					// just copy
					memcpy((void*)addr + offset, (void*)addr, size);
			}
			break;
			case HOOK_SET_NOPS:
				memset((void*)addr, 0x90, table->value);
			break;
			// these modify ACE memory
			case HOOK_IMPORT:
				*((uint32_t*)(table->value)) = addr;
			break;
			case HOOK_READ8:
				*((uint8_t*)(table->value)) = *((uint8_t*)addr);
			break;
			case HOOK_READ16:
				*((uint16_t*)(table->value)) = *((uint16_t*)addr);
			break;
			case HOOK_READ32:
				*((uint32_t*)(table->value)) = *((uint32_t*)addr);
			break;
		}

		table++;
		count--;
	} while(count && table->addr);
}

char *stpcpy(char *__restrict __dest, const char *__restrict __src)
{
	while(1)
	{
		*__dest++ = *__src;
		if(!*__src)
			break;
		__src++;
	}
	return __dest;
}

char *strcpy(char *__restrict __dest, const char *__restrict __src)
{
	char *dst = __dest;
	while(1)
	{
		*dst++ = *__src;
		if(!*__src)
			break;
		__src++;
	}
	return __dest;
}

char *strncpy(char *__restrict __dest, const char *__restrict __src, size_t count)
{
	char *dst = __dest;
	while(count--)
	{
		*dst++ = *__src;
		if(!*__src)
		{
			while(count--)
				*dst++ = 0;
			break;
		}
		__src++;
	}
	return __dest;
}

void *memset(void *dst, int value, size_t len)
{
	void *ret = dst;
	while(len--)
	{
		*(uint8_t*)dst = value;
		dst++;
	}
	return ret;
}

int strcmp(const char *s1, const char *s2)
{
	while(1)
	{
		int diff = (uint8_t)*s1 - (uint8_t)*s2;
		if(diff)
			return diff;
		if(!*s1)
			return 0;
		s1++;
		s2++;
	}
}

int strncmp(const char *s1, const char *s2, uint32_t len)
{
	while(len--)
	{
		int diff = (uint8_t)*s1 - (uint8_t)*s2;
		if(diff)
			return diff;
		if(!*s1)
			return 0;
		s1++;
		s2++;
	}
	return 0;
}

int strcasecmp(const char *s1, const char *s2)
{
	while(1)
	{
		uint8_t i1 = *s1++;
		uint8_t i2 = *s2++;
		int diff;

		if(i1 >= 'a' && i1 <= 'z')
			i1 &= 0xDF;
		if(i2 >= 'a' && i2 <= 'z')
			i2 &= 0xDF;

		diff = i1 - i2;
		if(diff)
			return diff;
		if(!i1)
			return 0;
	}
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
	while(n--)
	{
		uint8_t i1 = *s1++;
		uint8_t i2 = *s2++;
		int diff;

		if(i1 >= 'a' && i1 <= 'z')
			i1 &= 0xDF;
		if(i2 >= 'a' && i2 <= 'z')
			i2 &= 0xDF;

		diff = i1 - i2;
		if(diff)
			return diff;
		if(!i1)
			return 0;
	}
	return 0;
}

char *strchr(const char *__s, int __c)
{
	while(*__s)
	{
		if(*__s == __c)
			return __s;
		__s++;
	}
	return NULL;
}

char *strrchr(const char *__s, int __c)
{
	char *ret = NULL;
	while(*__s)
	{
		if(*__s == __c)
			ret = __s;
		__s++;
	}
	return ret;
}

void *memmove(void *destination, const void *source, size_t num)
{
	void *temp;

	temp = doom_malloc(num);
	memcpy(temp, source, num);
	memcpy(destination, temp, num);
	doom_free(temp);

	return destination;
}

char *strdup(const char *str1)
{
	char *ret;

	ret = doom_malloc(strlen(str1) + 1);
	if(!ret)
		return ret;

	strcpy(ret, str1);

	return ret;
}

char *strstr(const char *str1, const char *str2)
{
	while(*str1)
	{
		if(!strcmp(str1, str2))
			return str1;
		str1++;
	}
	return NULL;
}

