#pragma once


#define PSP_PAD_VALUE 2048

// MEGAIMAG.WAD - Max Payne PS2

struct wad_mega_mp_ps2 {
	char              name[16];
	int               size;
};



// The Warriors 
// PSP - zlibbed
// PS2 - raw


struct header_wad_main_tw {
	int               files;
	char              pad[12] = {};
};


struct wad_main_tw_psp {
	int                       offset;
	int                       rawSize;
	int                       size;
	unsigned int              pad;
};

struct wad_main_tw_ps2 {
	int               offset;
	int               size;
	int               pad;
};



// if i != 0 then ignore some other num?
// experimental, mostly for texture extraction

struct pak_main_tw_ps2_entry {
	int               size;
	char              pad[8];
};