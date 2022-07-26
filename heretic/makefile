program = chocoheretic
OBJ = asm.o main.o utils.o mlook.o stbar.o map.o poly.o script.o deh_io.o d_mode.o v_video.o deh_main.o i_cdmus.o m_misc.o w_file.o deh_mapping.o i_input.o m_argv.o w_file_stdc.o deh_str.o i_sound.o m_bbox.o deh_text.o i_system.o m_cheat.o w_merge.o d_event.o i_timer.o m_config.o tables.o w_wad.o d_iwad.o i_video.o m_controls.o d_loop.o i_winmusic.o m_fixed.o z_zone.o am_map.o deh_weapon.o m_random.o p_maputl.o p_switch.o r_plane.o ct_chat.o d_main.o p_ceilng.o p_mobj.o p_telept.o r_segs.o deh_ammo.o d_net.o p_doors.o p_plats.o p_tick.o r_things.o deh_frame.o f_finale.o p_enemy.o p_pspr.o p_user.o sb_bar.o deh_htext.o g_game.o p_floor.o p_saveg.o r_bsp.o sounds.o deh_htic.o info.o p_inter.o p_setup.o r_data.o s_sound.o deh_sound.o in_lude.o p_lights.o p_sight.o r_draw.o deh_thing.o mn_menu.o p_map.o p_spec.o r_main.o
CC = gcc -m32 -march=i386
OPT = -O2 -g0
BOTH = -fno-pie
CFLAGS = ${BOTH} ${OPT} -mgeneral-regs-only -fdata-sections -ffunction-sections -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables

build: ${program}

clean:
	rm -f *.o ${program} ${program}.bin

${program}: ${OBJ}
	${CC} ${OBJ} ${LIBS} ${BOTH} -o ${program} -nostdlib -Wl,--gc-sections -Wl,--no-ld-generated-unwind-info -Wl,--omagic -Wl,--no-dynamic-linker -Wl,-T linker.ld
	objcopy -O binary --only-section=.text --only-section=.rel.dyn --only-section=.hooks --only-section=.rodata --only-section=.data ${program} ${program}.bin
