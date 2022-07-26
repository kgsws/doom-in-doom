program = chocodoom
OBJ = asm.o main.o utils.o mlook.o stbar.o map.o poly.o script.o am_map.o deh_thing.o g_game.o m_argv.o p_lights.o r_bsp.o deh_ammo.o deh_weapon.o hu_lib.o m_bbox.o p_map.o r_data.o deh_bexstr.o d_event.o hu_stuff.o m_cheat.o p_maputl.o r_draw.o deh_cheat.o d_items.o i_cdmus.o m_config.o p_mobj.o r_main.o deh_doom.o d_iwad.o i_input.o m_controls.o p_plats.o r_plane.o deh_frame.o d_loop.o m_fixed.o p_pspr.o r_segs.o deh_io.o d_main.o info.o m_menu.o p_saveg.o r_sky.o deh_main.o d_mode.o i_sound.o m_misc.o p_setup.o r_things.o deh_mapping.o d_net.o i_system.o m_random.o p_sight.o sounds.o deh_misc.o doomdef.o i_timer.o p_ceilng.o p_spec.o st_lib.o deh_ptr.o doomstat.o i_video.o p_doors.o p_switch.o st_stuff.o deh_sound.o dstrings.o p_enemy.o p_telept.o deh_str.o f_finale.o i_winmusic.o p_floor.o p_tick.o wi_stuff.o deh_text.o f_wipe.o p_inter.o p_user.o z_zone.o w_wad.o w_file.o w_file_stdc.o v_video.o s_sound.o tables.o
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
