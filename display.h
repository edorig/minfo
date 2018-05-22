
/*
 * (C) 1997 by Marcin Dalecki <dalecki@math.uni-goettingen.de>
 *
 * This file is part of the Motif Info browser.
 *
 * The MInfo program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>


typedef struct {
    String path;
    String file;
    String node;
    Pixel hbgpixel;		/* highlight backgorund pixel */
    Pixel hfgpixel;		/* highlight foreground pixel */
    Pixel vfgpixel;		/* color for visited notes */
    XFontStruct *h1_font;	/* head 1 level font */
    XFontStruct *h2_font;	/* head 2 level font */
    XFontStruct *h3_font;	/* head 3 level font */
    XFontStruct *h4_font;	/* head 4 level font */
} app_data_t;

extern app_data_t app_data;

#include "parse.h"

struct page_stack {
    struct page_stack *prev;
    struct page *page;
};

extern struct page_stack *page_stack;
extern struct page_stack *fwrd_stack;

extern Widget node_text;
extern Widget back_button;
extern Widget fwrd_button;
extern Widget prev_button;
extern Widget top_button;
extern Widget next_button;
extern Widget popup_next;
extern Widget popup_prev;
extern Widget popup_top;
extern Widget home_button;
extern Widget search_button;

extern Widget prev_menu_button;
extern Widget top_menu_button;
extern Widget next_menu_button;
extern Widget home_menu_button;
extern Widget menu_menu_pullright;
extern Widget menu_item_button;
extern Widget menu_node_button;
extern Widget xref_menu_pullright;
extern Widget xref_item_button;
extern Widget xref_node_button;

extern Widget bookmark_menu;

extern Widget location_text;

extern struct page *current_page;

extern Pixel trough_pixel;
extern Pixel background_pixel;
extern Pixel top_pixel;
extern Pixel bottom_pixel;

extern void display_node(struct page *entry);
extern void display_page(struct page *page);
extern void do_draw(XmTextWidget, LineNum,
		    XmTextPosition, XmTextPosition, XmHighlightMode);

extern struct menu *check_in_xref(struct menu *, int, int);

#endif				/* DISPLAY_H */
