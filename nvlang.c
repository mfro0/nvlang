#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mint/osbind.h>
#include <gem.h>
#include "popup.h"

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




void do_dialog(void)
{
	OBJECT *nvselect;
	OBJECT *popup;
	struct NVM nvm;
	short exitobj;

	rsrc_gaddr(R_TREE, NVSELECT, &nvselect);
	rsrc_gaddr(R_TREE, POPUP, &popup);

	get_nvram(&nvm);

	if (!(nvm.language >= 0 && nvm.language < 20) ||
	   (!(nvm.keyboard >= 0 && nvm.keyboard < 20)))
	{
		/* NVRAM corrupt? */
		nvm.language = 0;		/* set to US English */
		nvm.keyboard = 0;
	}
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
		.mn_menu = PANEL,
		.mn_item = ENGLISH_US,
		.mn_scroll = 1,
		.mn_keystate = 0
	};
	
	MENU menu_kbd =
	{
		.mn_tree = popup,
		.mn_menu = PANEL,
		.mn_item = ENGLISH_US,
		.mn_scroll = 1,
		.mn_keystate = 0
	};
	
	while (exitobj != OK && exitobj != CANCEL)
	{
		short ind;

		if (exitobj == LANG)
		{
			short x, y;

			objc_offset(nvselect, LANG, &x, &y);
			
			ind = do_popup(&menu_lang, x, y);

			nvselect[LANG].ob_spec.free_string = popup[ind].ob_spec.free_string;
			nvselect[LANG].ob_state &= ~OS_SELECTED;
			objc_draw(nvselect, ROOT, MAX_DEPTH,
	 	              nvselect->ob_x, nvselect->ob_y,
					  nvselect->ob_width, nvselect->ob_height);

            nvm.language = ind - ENGLISH_US;
		}
		else if (exitobj == KBD_LANG)
		{
			short x, y;

			objc_offset(nvselect, KBD_LANG, &x, &y);
			
			ind = do_popup(&menu_kbd, x, y);
			
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
