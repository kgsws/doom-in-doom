
void script_draw();
void script_tick();
void script_spawn();
uint32_t script_execute(uint8_t idx, uint8_t map, uint8_t arg0, uint8_t arg1, uint8_t arg2);

//
void kg_Explosion0(doom_mobj_t *mo) __attribute((regparm(2),no_caller_saved_registers));
void kg_Explosion1(doom_mobj_t *mo) __attribute((regparm(2),no_caller_saved_registers));

void center_message(const char *text, uint32_t time);

extern uint_fast8_t redirect_keys;
extern uint_fast8_t is_running;

