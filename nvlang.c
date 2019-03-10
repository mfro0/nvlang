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

static const char rsrc_name[] = "nvlang.rsc";

static void print_nvram(struct NVM *buffer)
{
	printf( "  LANG: %s, KEYB: %s,\n  DATE: %s, CLOCK %s\n\n",
		lang[buffer->language],
		KEYB[buffer->keyboard]
		 );
	
	printf("\n");

}

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
	if (0 == rsrc_load(rsrc_name))
	{
		char noload[] = "[1][Resource file %s could not be loaded][OK]";
		char errstr[80];
		sprintf(errstr, noload, rsrc_name);
		form_alert(1, errstr);
		while (1) evnt_mesag(msgbuf);
	}

	menu_register(apid, "  NVLANG");
	rsrc_gaddr(R_TREE, NVSELECT, &nvselect);
	rsrc_gaddr(R_TREE, POPUP, &popup);


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

					exitobj = form_do(nvselect, ROOT) & 0x7fff;
				}

				form_dial(FMD_FINISH, 0, 0, 0, 0,
				                      r.g_x, r.g_y, r.g_w, r.g_h);
				form_dial(FMD_SHRINK, nvselect->ob_x, nvselect->ob_y,
				                      nvselect->ob_width, nvselect->ob_height,
									  0, 0, 2, 2);
				
				nvselect[exitobj].ob_state &= ~OS_SELECTED;
				break;	
				
			case AC_CLOSE:
				;
		}

	}
	get_nvram( &nvm );
	print_nvram(&nvm);

	return 0;
}
