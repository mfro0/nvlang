#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mint/osbind.h>
#include <gem.h>

#include "NVLANG.H"

struct NVM
{
	short bootpref;
	char reserved[4];
	unsigned char language;
	unsigned char keyboard;
	unsigned char datetime;
	char separator;
	unsigned char bootdelay;
	char reserved2[3];
	short vmode;
	unsigned char scsi;
};

#if 0
/* Structure for passing menu data */
typedef struct _menu
{
	OBJECT *mn_tree;		/* Object tree of the menu */
	WORD mn_menu;			/* Parent of the menu items*/
	WORD mn_item;			/* Starting menu item */
	WORD mn_scroll;			/* scroll flag for the menu*/
	WORD mn_keystate;		/* Key State */
} MENU;


/* Structure for the Menu Settings */
typedef struct _mn_set
{
	LONG Display;			/* The display delay */
	LONG Drag;				/* The drag delay */
	LONG Delay;				/* The Arrow Delay */
	LONG Speed;				/* The scroll speed delay */
	WORD Height;			/* The menu scroll height */
} MN_SET;
#endif

#define NVLANG_RSC "nvlang.rsc"

static void reset_nvram(struct NVM *buffer)
{
	(void) NVMaccess(2, 0, sizeof(buffer), buffer);
}

static void get_nvram(struct NVM *buffer)
{
	(void) NVMaccess(0, 0, sizeof(*buffer), buffer);
}

static void set_nvram(struct NVM *buffer)
{
	(void) NVMaccess(1, 0, sizeof(*buffer), buffer);
}

static short obj_num_children(OBJECT *tree, short obj)
{
	short head = tree[obj].ob_head;
	short next = head;
	short count = 0;

	while (next != obj)
	{
		next = tree[next].ob_next;
		count++;
	}

	return count;
}

static short do_popup(MENU *pm, OBJECT *dial, short originator)
{
	short x, y;
	short button, state;
	short exit_obj;
	OBJECT *popup = pm->mn_tree;
	OBJECT *o = &popup[pm->mn_menu];
	const int num_items = 5;

	char upstr[] = " \x01 ";
	char dnstr[] = " \x02 ";

	wind_update(BEG_UPDATE);
	objc_offset(dial, originator, &x, &y);


	o->ob_x = (x + dial[originator].ob_width / 2) - o->ob_width / 2;
	o->ob_y = (y + dial[originator].ob_height / 2) - o->ob_height / 2;

	short first;
	short last;
	char *first_str;
	char *last_str;

	first = o->ob_head;
	last = o->ob_tail;
	first_str = popup[first].ob_spec.free_string;
	last_str = popup[last].ob_spec.free_string;

	popup[first].ob_spec.free_string = upstr;
	popup[last].ob_spec.free_string = dnstr;
	
	form_dial(FMD_START, 0, 0, 0, 0,
              o->ob_x, o->ob_y,
			  o->ob_width, o->ob_height);

	objc_draw(popup, ROOT, MAX_DEPTH,
	          o->ob_x, o->ob_y,
			  o->ob_width, o->ob_height);

	exit_obj = form_do(popup, ROOT) & 0x7fff;
	popup[exit_obj].ob_state &= ~OS_SELECTED;

	form_dial(FMD_FINISH, 0, 0, 0, 0,
              o->ob_x, o->ob_y,
              o->ob_width, o->ob_height);

	wind_update(END_UPDATE);

	return exit_obj;
}


void do_dialog(void)
{
	OBJECT *nvselect;
	OBJECT *popup;
	struct NVM nvm;
	short exitobj;

	rsrc_gaddr(R_TREE, NVSELECT, &nvselect);
	rsrc_gaddr(R_TREE, POPUP, &popup);

	get_nvram(&nvm);

	nvselect[LANG].ob_spec.free_string = popup[nvm.language + ENGLISH_US].ob_spec.free_string;
	nvselect[KBD_LANG].ob_spec.free_string = popup[nvm.keyboard + ENGLISH_US].ob_spec.free_string;


    form_center(nvselect, &nvselect->ob_x, &nvselect->ob_y,
                &nvselect->ob_width, &nvselect->ob_height);
	
	wind_update(BEG_UPDATE);
	form_dial(FMD_START, 0, 0, 0, 0,
              nvselect->ob_x, nvselect->ob_y,
 			  nvselect->ob_width, nvselect->ob_height);
	form_dial(FMD_GROW, 0, 0, 2, 2,
	          nvselect->ob_x, nvselect->ob_y,
			  nvselect->ob_width, nvselect->ob_height);
				
	objc_draw(nvselect, ROOT, MAX_DEPTH, nvselect->ob_x, nvselect->ob_y,
	          nvselect->ob_width, nvselect->ob_height);

	exitobj = form_do(nvselect, ROOT) & 0x7fff;

	MENU menu_lang = 
	{
		.mn_tree = popup,
		.mn_menu = ROOT,
		.mn_item = ENGLISH_US,
		.mn_scroll = 1,
		.mn_keystate = 0
	};
	
	MENU menu_kbd =
	{
		.mn_tree = popup,
		.mn_menu = ROOT,
		.mn_item = ENGLISH_US,
		.mn_scroll = 1,
		.mn_keystate = 0
	};
	
	while (exitobj != OK && exitobj != CANCEL)
	{
		short ind;

		if (exitobj == LANG)
		{
			ind = do_popup(&menu_lang, nvselect, LANG);
			nvselect[LANG].ob_spec.free_string = popup[ind].ob_spec.free_string;
			nvselect[LANG].ob_state &= ~OS_SELECTED;
			objc_draw(nvselect, ROOT, MAX_DEPTH,
	 	              nvselect->ob_x, nvselect->ob_y,
					  nvselect->ob_width, nvselect->ob_height);

            nvm.language = ind - ENGLISH_US;
		}
		else if (exitobj == KBD_LANG)
		{
			ind = do_popup(&menu_kbd, nvselect, KBD_LANG);
			nvselect[KBD_LANG].ob_spec.free_string = popup[ind].ob_spec.free_string;
			nvselect[KBD_LANG].ob_state &= ~OS_SELECTED;
			objc_draw(nvselect, ROOT, MAX_DEPTH,
			          nvselect->ob_x, nvselect->ob_y,
					  nvselect->ob_width, nvselect->ob_height);

            nvm.keyboard = ind - ENGLISH_US;
		}

		exitobj = form_do(nvselect, ROOT) & 0x7fff;

        if (exitobj == OK)
            set_nvram(&nvm);
	}

	form_dial(FMD_FINISH, 0, 0, 0, 0,
	          nvselect->ob_x, nvselect->ob_y,
	 	      nvselect->ob_width, nvselect->ob_height);
	form_dial(FMD_SHRINK, nvselect->ob_x, nvselect->ob_y,
	          nvselect->ob_width, nvselect->ob_height,
			  0, 0, 2, 2);

	wind_update(END_UPDATE);

	nvselect[exitobj].ob_state &= ~OS_SELECTED;
}

int main(int argc, char *argv[])
{
	int apid;
	short msgbuf[8];
    extern short _app;


	apid = appl_init();

	if (!rsrc_load(NVLANG_RSC))
	{
		char norsc[] = "[1][Resource file " NVLANG_RSC "could not be loaded][OK]";
		
		form_alert(1, norsc);
		while (1) evnt_mesag(msgbuf);
	}

    if (!_app)		/* when started as accessory, we must wait to be called from the menu */
    {
	    menu_register(apid, "  NVLANG");

	    while (1)
	    {
		    evnt_mesag(msgbuf);
		    switch (msgbuf[0])
		    {
			    case AC_OPEN:
                    do_dialog();
				    break;	
				
			    case AC_CLOSE:
				    ;
		    }
	    }
    }
    else		/* just fire up straight away otherwise */
        do_dialog();	

	return 0;
}
