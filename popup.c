#include <gem.h>
#include "popup.h"

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

static short max(const short a, const short b)
{
    return (a > b ? a : b);
}

static short min(const short a, const short b)
{
	return (a < b ? a : b);
}

static MN_SET mn_set =
{
	0,			/* submenu display delay */
	0,			/* submenu drag display */
	0,			/* single click scroll delay */
	0,			/* continuous scroll delay */
	7			/* number of displayed items */
};

short do_popup(MENU *pm, short x, short y)
{
	short button, state;
	short exit_obj;
	OBJECT *popup = pm->mn_tree;

	const short max_items = mn_set.height;
    short num_items = obj_num_children(popup, ROOT);
    short dsp_items = min(max_items, num_items);


	char upstr[] = " \x01 ";
	char dnstr[] = " \x02 ";

	wind_update(BEG_UPDATE);
	
	popup->ob_x = x;
	popup->ob_y = y;

	short first;
	short last;
	char *first_str;
	char *last_str;
	short mn_item_adjust = 0;

	first = popup->ob_head;
	last = popup->ob_tail;

	short extra_item;
	do
	{
		pm->mn_item += mn_item_adjust;
		extra_item = mn_item_adjust = 0;

		if (pm->mn_item != first)
		{
			/*
			 * pm->mn_item is not the first item to be displayed, thus
			 * we need to add an up arrow menu. We save the menu text
			 * of the menu item immediately before pm->mn_item and temporarily
			 * replace it with our up arrow string
			 */

			first_str = popup[pm->mn_item - 1].ob_spec.free_string;
			popup[pm->mn_item - 1].ob_spec.free_string = upstr;
			popup->ob_head = pm->mn_item - 1;
			extra_item = 1;						/* remember that we added an item */
		}
		else
		{
			popup->ob_head = pm->mn_item;				
		}
		

		if (pm->mn_item + dsp_items - 1 - extra_item < last)
		{
			/*
			 * now take care about the lower end of the popup. If the last item displayed is not
			 * the last item of the menu, we temporarily replace its menu text with our down arrow
			 * text to enable menu scrolling
			 */
			last_str = popup[pm->mn_item + dsp_items - 1 - extra_item].ob_spec.free_string;
			popup[pm->mn_item + dsp_items - 1 - extra_item].ob_spec.free_string = dnstr;
			
			popup->ob_tail = pm->mn_item + dsp_items - 1 - extra_item;
			popup[popup->ob_tail].ob_flags |= OF_LASTOB;

			popup[pm->mn_item + dsp_items - 1 - extra_item].ob_next = ROOT;
			
			popup->ob_height = popup[first].ob_height * dsp_items;
		}
		else
		{
			popup->ob_tail = last;
		}
		
		short ob_y = 0;
		
		for (int i = popup->ob_head; i <= popup->ob_tail; i++)
		{
			popup[i].ob_y = ob_y;
			ob_y += popup[i].ob_height;
		}
		objc_draw(popup, ROOT, MAX_DEPTH,
				  popup->ob_x, popup->ob_y,
				  popup->ob_width, popup->ob_height);		
		
		exit_obj = form_do(popup, ROOT) & 0x7fff;

		popup[exit_obj].ob_state &= ~OS_SELECTED;
		
		/*
		* undo object tree mods we did above
		*/
		if (pm->mn_item != first)
		{
			popup[pm->mn_item - 1].ob_spec.free_string = first_str;		/* restore saved menu text */
			popup->ob_head = first;

			if (exit_obj == pm->mn_item - 1)
			{
				/* up arrow selected */
				mn_item_adjust -= 1;
			}
		}

		if (pm->mn_item + dsp_items - 1 < last)
		{
			short downarrow = pm->mn_item + dsp_items - 1 - extra_item;
			/*
			 * restore saved menu text of down arrow object
			 */
			popup[downarrow].ob_spec.free_string = last_str;
			popup->ob_tail = last;

			if (downarrow != last)
			{
				popup[downarrow].ob_next = pm->mn_item + dsp_items;
				popup[downarrow].ob_flags &= ~OF_LASTOB;
			}
			
			if (exit_obj == downarrow)
			{
				mn_item_adjust += 1;
			}
			if (!extra_item)
				mn_item_adjust += 1;
		}
		
		printf("exit_obj=%d, check=(%d,%d) extra_item=%d, pm->mn_item=%d\r\n",
		       exit_obj, pm->mn_item - 1, pm->mn_item + dsp_items - 1 - extra_item,
			   extra_item, pm->mn_item);
	} while (exit_obj == pm->mn_item - 1 || exit_obj == pm->mn_item + dsp_items - 1 - extra_item);

	wind_update(END_UPDATE);

	
	return exit_obj;
}
