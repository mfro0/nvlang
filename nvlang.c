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

static const char *lang[] = { "en", "de", "fr", "XX", "es", "it", NULL };
static const char *KEYB[] = { "us", "de", "fr", "gb", "es", "it", "se", "ch-fr", "ch-de", NULL };

#define NVLANG_RSC "nvlang.rsc"

static void reset_nvram(struct NVM *buffer)
{
	NVMaccess(2, 0, sizeof(buffer), buffer);
}

static void get_nvram(struct NVM *buffer)
{
	NVMaccess(0, 0, sizeof(*buffer), buffer);
}

static void set_nvram(struct NVM *buffer)
{
	NVMaccess( 1, 0, sizeof(*buffer), buffer );
}

static GRECT ob_size(OBJECT *tree, short obj)
{
    short x, y;
    short width;
    short height;
    short framesize;

    x = tree[obj].ob_x;
    y = tree[obj].ob_y;
    width = tree[obj].ob_width;
    height = tree[obj].ob_height;
    framesize = tree[obj].ob_spec.obspec.framesize;

    x = framesize > 0 ? x - framesize : x;
    y = framesize > 0 ? y - framesize : y;
    width = framesize > 0 ? width + framesize * 2 : width;
    height = framesize > 0 ? height + framesize * 2 : height;

    GRECT rect = { x, y, width, height };

    return rect;
}

static short do_popup(OBJECT *popup, OBJECT *dial, short originator)
{
	short x, y;
	short button, state;
	short exit_obj;

	wind_update(BEG_UPDATE);
	objc_offset(dial, originator, &x, &y);
	popup[ROOT].ob_x = (x + dial[originator].ob_width / 2) - popup[ROOT].ob_width / 2;
	popup[ROOT].ob_y = (y + dial[originator].ob_height / 2) - popup[ROOT].ob_height / 2;

    GRECT sz = ob_size(popup, ROOT);

	form_dial(FMD_START, 0, 0, 0, 0,
              sz.g_x, sz.g_y, sz.g_w, sz.g_h);

    // printf("obj = { %d, %d, %d, %d }\r\n", popup->ob_x, popup->ob_y, popup->ob_width, popup->ob_height);
    // printf("sz  = { %d, %d, %d, %d }\r\n", sz.g_x, sz.g_y, sz.g_w, sz.g_h);

	objc_draw(popup, ROOT, MAX_DEPTH,
	          popup->ob_x, popup->ob_y,
			  popup->ob_width, popup->ob_height);

	exit_obj = form_do(popup, ROOT) & 0x7fff;
	popup[exit_obj].ob_state &= ~OS_SELECTED;

	form_dial(FMD_FINISH, 0, 0, 0, 0,
              sz.g_x, sz.g_y,
              sz.g_w, sz.g_h);

	wind_update(END_UPDATE);

	return exit_obj;
}

int main(int argc, char *argv[])
{
	struct NVM nvm;
	int apid;
	short msgbuf[8];
	short resource;
	short evnt;
	OBJECT *nvselect;
	OBJECT *popup;

	apid = appl_init();
	if (0 == rsrc_load(NVLANG_RSC))
	{
		char norsc[] = "[1][Resource file " NVLANG_RSC "could not be loaded][OK]";
		
		form_alert(1, norsc);
		while (1) evnt_mesag(msgbuf);
	}

	menu_register(apid, "  NVLANG");
	rsrc_gaddr(R_TREE, NVSELECT, &nvselect);
	rsrc_gaddr(R_TREE, POPUP, &popup);

	get_nvram(&nvm);

	nvselect[LANG].ob_spec.free_string = popup[nvm.language + ENGLISH_US].ob_spec.free_string;
	nvselect[KBD_LANG].ob_spec.free_string = popup[nvm.keyboard + ENGLISH_US].ob_spec.free_string;

	while (1)
	{
		evnt = evnt_mesag(msgbuf);
		switch (msgbuf[0])
		{
			short exitobj;

			case AC_OPEN:

				form_center(nvselect, &nvselect->ob_x, &nvselect->ob_y,
				                      &nvselect->ob_width, &nvselect->ob_height);
	
				GRECT r = { nvselect->ob_x - 2, nvselect->ob_y - 2,
	     		            nvselect->ob_width + 4, nvselect->ob_height + 4 };
		
				wind_update(BEG_UPDATE);
				form_dial(FMD_START, 0, 0, 0, 0,
				                     r.g_x, r.g_y, r.g_w, r.g_h);
				form_dial(FMD_GROW, 0, 0, 2, 2,
				                    nvselect->ob_x, nvselect->ob_y,
									nvselect->ob_width, nvselect->ob_height);
				
				objc_draw(nvselect, ROOT, MAX_DEPTH, nvselect->ob_x, nvselect->ob_y,
				                                     nvselect->ob_width, nvselect->ob_height);
				exitobj = form_do(nvselect, ROOT) & 0x7fff;
				while (exitobj != OK && exitobj != CANCEL)
				{
					short ind;

					if (exitobj == LANG)
					{
						ind = do_popup(popup, nvselect, LANG);
						nvselect[LANG].ob_spec.free_string = popup[ind].ob_spec.free_string;
						nvselect[LANG].ob_state &= ~OS_SELECTED;
						objc_draw(nvselect, ROOT, MAX_DEPTH,
						          nvselect->ob_x, nvselect->ob_y,
								  nvselect->ob_width, nvselect->ob_height);

                        nvm.language = ind - ENGLISH_US;
					}
					else if (exitobj == KBD_LANG)
					{
						ind = do_popup(popup, nvselect, KBD_LANG);
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
				                      r.g_x, r.g_y, r.g_w, r.g_h);
				form_dial(FMD_SHRINK, nvselect->ob_x, nvselect->ob_y,
				                      nvselect->ob_width, nvselect->ob_height,
									  0, 0, 2, 2);
				wind_update(END_UPDATE);
				nvselect[exitobj].ob_state &= ~OS_SELECTED;
				break;	
				
			case AC_CLOSE:
				;
		}
	}

	return 0;
}
