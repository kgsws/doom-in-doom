
typedef struct
{
	struct polyobj_s *poly;
} extra_subsector_t;

extern doom_vertex_t **do_vertexes;
extern doom_line_t **do_lines;
extern uint32_t *do_numlines;
extern doom_side_t **do_sides;
extern doom_sector_t **do_sectors;
extern uint32_t *do_numsectors;
extern doom_seg_t **do_segs;
extern uint32_t *do_numsegs;
extern doom_subsector_t **do_subsectors;
extern extra_subsector_t *ex_subsector;
extern uint32_t *do_numsubsectors;

extern uint32_t *do_totalitems;
extern uint32_t *do_totalkills;
extern uint32_t *do_totalsecret;

extern doom_line_t *map_special_line;
extern doom_mobj_t *map_special_mobj;
extern uint_fast8_t map_special_side;

extern fixed_t *do_bmaporgx;
extern fixed_t *do_bmaporgy;
