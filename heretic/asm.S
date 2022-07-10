// kgsws' Doom ACE
// this is an initialization code
//

.section .text
.global _start

.org	0

// store .bss size at the top of binary
_the_top:
	.4byte	_size_of_bss

//
// Entry point
// %eax - entry point address
// %esi - CODE segment base
// %edi - DATA segment base
_start:
	// loader already backed-up all registers

	// get ACE base // 0x0024E038
	sub	$0x1004,%eax	// compensate for VMA offset (set in linker script) and '_size_of_bss'

	// get CODE base
	mov	%ebx,doom_code_segment(%eax)

	// get DATA base
	mov	%ecx,doom_data_segment(%eax)

	// move CODE base
	mov	%ebx,%esi

	// relocate ACE
	// this takes data from ELF
	// this is only handles one type of relocation
	mov	%eax, %ecx
	lea	_reloc_start(%eax),%ebx
	lea	_reloc_end(%eax),%edx
rel_ace_loop:
	cmp	%edx,%ebx
	jge	rel_ace_end
	mov	(%ebx),%eax
	add	%ecx,(%ecx,%eax,1)
	add	$8,%ebx
	jmp	rel_ace_loop
rel_ace_end:
	// everything after this point was relocated
	// everything from ACE should be accessible directly

	// relocate 012 table
	mov	$__reloc_code_012,%eax
	mov	$__reloc_code_012_end,%edx
rel_012_loop:
	cmp	%edx,%eax
	jge	rel_012_end
	add	%esi,1(%eax)	// add CODE base to the address
	add	$5,%eax	// each entry is 5 bytes long
	sub	%eax,-4(%eax)
	jmp	rel_012_loop
rel_012_end:

	// relocate arg3 table
	mov	$__reloc_code_arg3,%eax
	mov	$__reloc_code_arg3_end,%edx
rel_arg3_loop:
	cmp	%edx,%eax
	jge	rel_arg3_end
	add	%esi,6(%eax)	// add CODE base to the address
	add	$10,%eax	// 10 bytes
	sub	%eax,-4(%eax)
	add	$2,%eax	// 2 bytes (12 total)
	jmp	rel_arg3_loop
rel_arg3_end:

	// relocate arg4 table
	mov	$__reloc_code_arg4,%eax
	mov	$__reloc_code_arg4_end,%edx
rel_arg4_loop:
	cmp	%edx,%eax
	jge	rel_arg4_end
	add	%esi,11(%eax)	// add CODE base to the address
	add	$15,%eax	// 15 bytes
	sub	%eax,-4(%eax)
	add	$3,%eax	// 3 bytes (18 total)
	jmp	rel_arg4_loop
rel_arg4_end:

	// relocate arg5 table
	mov	$__reloc_code_arg5,%eax
	mov	$__reloc_code_arg5_end,%edx
rel_arg5_loop:
	cmp	%edx,%eax
	jge	rel_arg5_end
	add	%esi,16(%eax)	// add CODE base to the address
	add	$20,%eax	// 20 bytes
	sub	%eax,-4(%eax)
	add	$3,%eax	// 3 bytes (23 total)
	jmp	rel_arg5_loop
rel_arg5_end:

	// clear .bss
	mov	$_bss_start,%ebx
	mov	$_bss_end,%edx
	xor	%eax,%eax
bss_loop:
	cmp	%edx,%ebx
	jge	bss_end
	mov	%eax,(%ebx)
	add	$4, %ebx
	jmp	bss_loop
bss_end:

	// run custom code
	call	ace_main

	// continue the game
	jmp	__return_to_game

	// exit
.global dos_exit
dos_exit:
	and	$0xff,%eax
	mov	$0x4c,%ah
	int	$0x21

.global I_Error
I_Error:
	movl	$exit_I_Error,(%esp)
	call	doom_printf

.global doom_printf
doom_printf:
	mov	(%esp),%eax
	mov	%eax,gfx_call_bkup
	movl	$gfx_print_buffer,(%esp)
	call	doom_sprintf
	mov	gfx_call_bkup,%eax
	movl	%eax,(%esp)
	jmp	update_gfx_text

gfx_call_bkup:
	.4byte	0

//
// hook trampolines
.global hook_CrossSpecialLine
hook_CrossSpecialLine:
	push	%ebx
	call	map_CrossSpecialLine
	add	$4,%esp
	ret

.global hook_UseSpecialLine
hook_UseSpecialLine:
	push	%ebx
	call	map_UseSpecialLine
	add	$4,%esp
	ret

.global hook_BlockLinesIterator
hook_BlockLinesIterator:
	push	%ebx
	call	poly_BlockLinesIterator
	add	$4,%esp
	ret

.global hook_polyobj
hook_polyobj:
	call	doom_R_AddSprites
	mov	28(%esp),%eax
	call	poly_render
	ret

.global hook_subsector_ret
hook_subsector_ret:
	pop	%ebp
	pop	%edi
	pop	%esi
	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%eax
	ret    

//
// Relocation table for functions with 0, 1 or 2 arguments.
// Every entry is 'jmp 0x........' opcode. Offsets are relocated to "CODE segment".
.section .text.__reloc_code_012
__reloc_code_012:
////////////////////////////////// special hook
__return_to_game:
	.byte	0xE9
	.4byte	0x00020846
////////////////////////////////// doom_printf_xx
.section .text.doom_printf_xx
.global doom_printf_xx
doom_printf_xx:
	.byte	0xE9
	.4byte	0x0003FE40
////////////////////////////////// doom_I_Error
.section .text.doom_I_Error
.global doom_I_Error
doom_I_Error:
	.byte	0xE9
	.4byte	0x0001AB10
////////////////////////////////// doom_sprintf
.section .text.doom_sprintf
.global doom_sprintf
doom_sprintf:
	.byte	0xE9
	.4byte	0x0004101C
////////////////////////////////// doom_sscanf
.section .text.doom_sscanf
.global doom_sscanf
doom_sscanf:
	.byte	0xE9
	.4byte	0x00040AD4
////////////////////////////////// doom_free
.section .text.doom_free
.global doom_free
doom_free:
	.byte	0xE9
	.4byte	0x0004066D
////////////////////////////////// doom_malloc
.section .text.doom_malloc
.global doom_malloc
doom_malloc:
	.byte	0xE9
	.4byte	0x00040602
////////////////////////////////// doom_realloc
.section .text.doom_realloc
.global doom_realloc
doom_realloc:
	.byte	0xE9
	.4byte	0x00040EE3
////////////////////////////////// doom_calloc
.section .text.doom_calloc
.global doom_calloc
doom_calloc:
	.byte	0xE9
	.4byte	0x00040F68
////////////////////////////////// doom_close
.section .text.doom_close
.global doom_close
doom_close:
	.byte	0xE9
	.4byte	0x00040902
////////////////////////////////// doom_fstat
.section .text.doom_fstat
.global doom_fstat
doom_fstat:
	.byte	0xE9
	.4byte	0x0004176E
////////////////////////////////// doom_atoi
.section .text.doom_atoi
.global doom_atoi
doom_atoi:
	.byte	0xE9
	.4byte	0x00040AF6
////////////////////////////////// doom_fopen
.section .text.doom_fopen
.global doom_fopen
doom_fopen:
	.byte	0xE9
	.4byte	0x00040217
////////////////////////////////// doom_fclose
.section .text.doom_fclose
.global doom_fclose
doom_fclose:
	.byte	0xE9
	.4byte	0x0004043A
////////////////////////////////// doom_ftell
.section .text.doom_ftell
.global doom_ftell
doom_ftell:
	.byte	0xE9
	.4byte	0x00041262
////////////////////////////////// doom_fgetc
.section .text.doom_fgetc
.global doom_fgetc
doom_fgetc:
	.byte	0xE9
	.4byte	0x0004140E
////////////////////////////////// doom_fputc
.section .text.doom_fputc
.global doom_fputc
doom_fputc:
	.byte	0xE9
	.4byte	0x00044276
////////////////////////////////// doom_ungetc
.section .text.doom_ungetc
.global doom_ungetc
doom_ungetc:
	.byte	0xE9
	.4byte	0x0004420B
////////////////////////////////// doom_fscanf
.section .text.doom_fscanf
.global doom_fscanf
doom_fscanf:
	.byte	0xE9
	.4byte	0x000418B6
////////////////////////////////// I_GetTime
.section .text.I_GetTime
.global I_GetTime
I_GetTime:
	.byte	0xE9
	.4byte	0x00019C10
////////////////////////////////// FixedDiv
.section .text.FixedDiv
.global FixedDiv
FixedDiv:
	.byte	0xE9
	.4byte	0x0001D180
////////////////////////////////// doom_open
.section .text.doom_open
.global doom_open
doom_open:
	.byte	0xE9
	.4byte	0x00040691
////////////////////////////////// doom_Z_Free
.section .text.doom_Z_Free
.global doom_Z_Free
doom_Z_Free:
	.byte	0xE9
	.4byte	0x00039600
////////////////////////////////// doom_I_ZoneBase
.section .text.doom_I_ZoneBase
.global doom_I_ZoneBase
doom_I_ZoneBase:
	.byte	0xE9
	.4byte	0x0001AC20
////////////////////////////////// doom_I_FinishUpdate
.section .text.doom_I_FinishUpdate
.global doom_I_FinishUpdate
doom_I_FinishUpdate:
	.byte	0xE9
	.4byte	0x00019F60
////////////////////////////////// doom_W_CacheLumpName
.section .text.doom_W_CacheLumpName
.global doom_W_CacheLumpName
doom_W_CacheLumpName:
	.byte	0xE9
	.4byte	0x00038CF0
////////////////////////////////// doom_W_CacheLumpNum
.section .text.doom_W_CacheLumpNum
.global doom_W_CacheLumpNum
doom_W_CacheLumpNum:
	.byte	0xE9
	.4byte	0x00038D00
////////////////////////////////// doom_W_ReadLump
.section .text.doom_W_ReadLump
.global doom_W_ReadLump
doom_W_ReadLump:
	.byte	0xE9
	.4byte	0x00038BF0
////////////////////////////////// doom_W_GetNumForName
.section .text.doom_W_GetNumForName
.global doom_W_GetNumForName
doom_W_GetNumForName:
	.byte	0xE9
	.4byte	0x00038B80
////////////////////////////////// doom_R_TextureNumForName
.section .text.doom_R_TextureNumForName
.global doom_R_TextureNumForName
doom_R_TextureNumForName:
	.byte	0xE9
	.4byte	0x00034750
////////////////////////////////// doom_G_Ticker
.section .text.doom_G_Ticker
.global doom_G_Ticker
doom_G_Ticker:
	.byte	0xE9
	.4byte	0x00020450
////////////////////////////////// doom_D_Display
.section .text.doom_D_Display
.global doom_D_Display
doom_D_Display:
	.byte	0xE9
	.4byte	0x0001D1E0
////////////////////////////////// doom_M_Responder
.section .text.doom_M_Responder
.global doom_M_Responder
doom_M_Responder:
	.byte	0xE9
	.4byte	0x00023010
////////////////////////////////// doom_S_StartSound
.section .text.doom_S_StartSound
.global doom_S_StartSound
doom_S_StartSound:
	.byte	0xE9
	.4byte	0x0003F370
////////////////////////////////// doom_M_StringWidth
.section .text.doom_M_StringWidth
.global doom_M_StringWidth
doom_M_StringWidth:
	.byte	0xE9
	.4byte	0x00022F00
////////////////////////////////// doom_G_BuildTiccmd
.section .text.doom_G_BuildTiccmd
.global doom_G_BuildTiccmd
doom_G_BuildTiccmd:
	.byte	0xE9
	.4byte	0x0001FB10
////////////////////////////////// doom_R_SetupFrame
.section .text.doom_R_SetupFrame
.global doom_R_SetupFrame
doom_R_SetupFrame:
	.byte	0xE9
	.4byte	0x00035E80
////////////////////////////////// doom_R_DrawPlayerSprites
.section .text.doom_R_DrawPlayerSprites
.global doom_R_DrawPlayerSprites
doom_R_DrawPlayerSprites:
	.byte	0xE9
	.4byte	0x00038210
////////////////////////////////// doom_R_RenderPlayerView
.section .text.doom_R_RenderPlayerView
.global doom_R_RenderPlayerView
doom_R_RenderPlayerView:
	.byte	0xE9
	.4byte	0x00035FB0
////////////////////////////////// doom_P_SetThingPosition
.section .text.doom_P_SetThingPosition
.global doom_P_SetThingPosition
doom_P_SetThingPosition:
	.byte	0xE9
	.4byte	0x0002C460
////////////////////////////////// doom_WI_Start
.section .text.doom_WI_Start
.global doom_WI_Start
doom_WI_Start:
	.byte	0xE9
	.4byte	0x0003EB70
////////////////////////////////// doom_P_SpawnPlayer
.section .text.doom_P_SpawnPlayer
.global doom_P_SpawnPlayer
doom_P_SpawnPlayer:
	.byte	0xE9
	.4byte	0x000317F0
////////////////////////////////// doom_P_Random
.section .text.doom_P_Random
.global doom_P_Random
doom_P_Random:
	.byte	0xE9
	.4byte	0x00024160
////////////////////////////////// doom_R_AddSprites
.section .text.doom_R_AddSprites
.global doom_R_AddSprites
doom_R_AddSprites:
	.byte	0xE9
	.4byte	0x00037F90
////////////////////////////////// doom_R_AddLine
.section .text.doom_R_AddLine
.global doom_R_AddLine
doom_R_AddLine:
	.byte	0xE9
	.4byte	0x00033720
////////////////////////////////// doom_P_AddThinker
.section .text.doom_P_AddThinker
.global doom_P_AddThinker
doom_P_AddThinker:
	.byte	0xE9
	.4byte	0x00032ED0
////////////////////////////////// doom_P_RemoveThinker
.section .text.doom_P_RemoveThinker
.global doom_P_RemoveThinker
doom_P_RemoveThinker:
	.byte	0xE9
	.4byte	0x00032EF0
////////////////////////////////// doom_P_BoxOnLineSide
.section .text.doom_P_BoxOnLineSide
.global doom_P_BoxOnLineSide
doom_P_BoxOnLineSide:
	.byte	0xE9
	.4byte	0x0002C0D0
////////////////////////////////// doom_P_SpawnSpecials
.section .text.doom_P_SpawnSpecials
.global doom_P_SpawnSpecials
doom_P_SpawnSpecials:
	.byte	0xE9
	.4byte	0x0002FFF0
////////////////////////////////// doom_G_ExitLevel
.section .text.doom_G_ExitLevel
.global doom_G_ExitLevel
doom_G_ExitLevel:
	.byte	0xE9
	.4byte	0x00020D40
////////////////////////////////// end
.section .text.__reloc_code_012_end
__reloc_code_012_end:
////////////////////////////////////////////////////////

//
// Relocation table for functions with 3 arguments.
// Similar to '012' but there is some stack handling code. Jump is replaced with call.
.section .text.__reloc_code_arg3
__reloc_code_arg3:
////////////////////////////////// doom_read
.section .text.doom_read
.global doom_read
doom_read:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x00040935
	pop	%ebx
	ret
////////////////////////////////// doom_write
.section .text.doom_write
.global doom_write
doom_write:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x0004159a
	pop	%ebx
	ret
////////////////////////////////// doom_lseek
.section .text.doom_lseek
.global doom_lseek
doom_lseek:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x000408c0
	pop	%ebx
	ret
////////////////////////////////// doom_Z_Malloc
.section .text.doom_Z_Malloc
.global doom_Z_Malloc
doom_Z_Malloc:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x00039690
	pop	%ebx
	ret
////////////////////////////////// doom_fseek
.section .text.doom_fseek
.global doom_fseek
doom_fseek:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x000410BA
	pop	%ebx
	ret
////////////////////////////////// doom_M_WriteText
.section .text.doom_M_WriteText
.global doom_M_WriteText
doom_M_WriteText:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x00022FA0
	pop	%ebx
	ret
////////////////////////////////// doom_G_InitNew
.section .text.doom_G_InitNew
.global doom_G_InitNew
doom_G_InitNew:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x000215D0
	pop	%ebx
	ret
////////////////////////////////// doom_P_BlockLinesIterator
.section .text.doom_P_BlockLinesIterator
.global doom_P_BlockLinesIterator
doom_P_BlockLinesIterator:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x0002C520
	pop	%ebx
	ret
////////////////////////////////// doom_P_BlockThingsIterator
.section .text.doom_P_BlockThingsIterator
.global doom_P_BlockThingsIterator
doom_P_BlockThingsIterator:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x0002C5A0
	pop	%ebx
	ret
////////////////////////////////// doom_P_CheckPosition
.section .text.doom_P_CheckPosition
.global doom_P_CheckPosition
doom_P_CheckPosition:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x0002B060
	pop	%ebx
	ret
////////////////////////////////// doom_P_TeleportMove
.section .text.doom_P_TeleportMove
.global doom_P_TeleportMove
doom_P_TeleportMove:
	push	%ebx
	mov	8(%esp),%ebx
	.byte	0xE8
	.4byte	0x0002AC00
	pop	%ebx
	ret
////////////////////////////////// end
.section .text.__reloc_code_arg3_end
__reloc_code_arg3_end:
////////////////////////////////////////////////////////

//
// Relocation table for functions with 4 arguments.
// Similar to 'arg3'.
.section .text.__reloc_code_arg4
__reloc_code_arg4:
////////////////////////////////// doom_V_DrawPatchDirect
.section .text.doom_V_DrawPatchDirect
.global doom_V_DrawPatchDirect
doom_V_DrawPatchDirect:
	push	%ebx
	push	%ecx
	mov	12(%esp),%ebx
	mov	16(%esp),%ecx
	.byte	0xE8
	.4byte	0x000392A0
	pop	%ecx
	pop	%ebx
	ret
////////////////////////////////// doom_fread
.section .text.doom_fread
.global doom_fread
doom_fread:
	push	%ebx
	push	%ecx
	mov	12(%esp),%ebx
	mov	16(%esp),%ecx
	.byte	0xE8
	.4byte	0x0004128E
	pop	%ecx
	pop	%ebx
	ret
////////////////////////////////// doom_P_SpawnMobj
.section .text.doom_P_SpawnMobj
.global doom_P_SpawnMobj
doom_P_SpawnMobj:
	push	%ebx
	push	%ecx
	mov	12(%esp),%ebx
	mov	16(%esp),%ecx
	.byte	0xE8
	.4byte	0x00031540
	pop	%ecx
	pop	%ebx
	ret
////////////////////////////////// doom_P_DamageMobj
.section .text.doom_P_DamageMobj
.global doom_P_DamageMobj
doom_P_DamageMobj:
	push	%ebx
	push	%ecx
	mov	12(%esp),%ebx
	mov	16(%esp),%ecx
	.byte	0xE8
	.4byte	0x0002A460
	pop	%ecx
	pop	%ebx
	ret
////////////////////////////////// end
.section .text.__reloc_code_arg4_end
__reloc_code_arg4_end:
////////////////////////////////////////////////////////

//
// Relocation table for functions with 5 arguments.
// Similar to 'arg4' but 5th argument is placed at correct location on the stack.
.section .text.__reloc_code_arg5
__reloc_code_arg5:
////////////////////////////////// P_LineAttack_XX
.section .text.P_LineAttack_XX
.global P_LineAttack_XX
P_LineAttack:
	push	%ebx
	push	%ecx
	mov	20(%esp),%ecx
	push	%ecx
	mov	16(%esp),%ebx
	mov	20(%esp),%ecx
	.byte	0xE8
	.4byte	0x0002BC20
	pop	%ecx
	pop	%ebx
	ret
////////////////////////////////// end
.section .text.__reloc_code_arg5_end
__reloc_code_arg5_end:
////////////////////////////////////////////////////////

//
// Stored variables for later.
.global doom_code_segment
doom_code_segment:
	.4byte	0
.global doom_data_segment
doom_data_segment:
	.4byte	0

