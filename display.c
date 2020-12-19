/*
 * (C) 1996 by Marcin Dalecki <dalecki@math.uni-goettingen.de>
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

#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <Xm/MainW.h>
#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/Text.h>
#include <Xm/TextP.h>
#include <Xm/TextOutP.h>
#include <Xm/TextF.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>

#include <stdio.h>
#include <string.h>

#include "parse.h"
#include "display.h"
#include "fops.h"

Widget node_text;
Widget location_text;
Widget back_button;
Widget fwrd_button;
Widget prev_button;
Widget top_button;
Widget next_button;
Widget home_button;
Widget search_button;
Widget top_menu_button;
Widget next_menu_button;
Widget prev_menu_button;
Widget home_menu_button;
Widget menu_menu_pullright;
Widget menu_item_button;
Widget menu_node_button;
Widget xref_menu_pullright;
Widget xref_item_button;
Widget xref_node_button;
Widget popup_next;
Widget popup_prev;
Widget popup_top;

/*
 * This isn't quite the right place for the definition...
 * Perhaps we should move the whole handling of bookmark's to this
 * file.
 */
Widget bookmark_menu;

struct page *current_page;
struct page_stack *page_stack;
struct page_stack *fwrd_stack;

/*
 * Used by the drawing routines for special elements
 */

Pixel trough_pixel;
Pixel background_pixel;
Pixel top_pixel;
Pixel bottom_pixel;

void display_page(struct page *page)
{
    char label[512];
    char *s[10];
    int i, n;
    Boolean set;
    Widget w;

    /* 
     * Update the sesitivity of the all the navigation buttons.
     */
    set = (page->Prev != NULL);
    XtSetSensitive(prev_button, set);
    XtSetSensitive(prev_menu_button, set);
    XtSetSensitive(popup_prev, set);

    set = (page->Up != NULL);
    XtSetSensitive(top_button, set);
    XtSetSensitive(top_menu_button, set);
    XtSetSensitive(popup_top, set);

    set = (page->Next != NULL);
    XtSetSensitive(next_button, set);
    XtSetSensitive(next_menu_button, set);
    XtSetSensitive(popup_next, set);

    set = (page->menu != NULL);
    XtSetSensitive(menu_item_button, set);
    XtSetSensitive(menu_node_button, set);
    XtSetSensitive(menu_menu_pullright, set);

    set = (page->note != NULL);
    XtSetSensitive(xref_item_button, set);
    XtSetSensitive(xref_node_button, set);
    XtSetSensitive(xref_menu_pullright, set);

    /*
     * Inform the user where we are currently
     */

    label[0] = '\0';
    n = 0;
    if (page->File) {
	s[n++] = strdup("File ");
	s[n++] = strdup(page->File);
    }
    if (page->Node) {
	s[n++] = strdup(", Node ");
	s[n++] = strdup(page->Node);
    }
    if (page->Next) {
	s[n++] = strdup(", Next ");
	s[n++] = strdup(page->Next);
    }
    if (page->Prev) {
	s[n++] = strdup(", Prev ");
	s[n++] = strdup(page->Prev);
    }
    if (page->Up) {
	s[n++] = strdup(", Up ");
	s[n++] = strdup(page->Up);
    }
    for (i = 0; i < n; ++i) {
	strcat(label, s[i]);
	free(s[i]);
    }

    XmTextFieldSetString(location_text, label);

    XtVaGetValues(XtParent(node_text), XmNverticalScrollBar, &w, NULL);
    /*
     * preserve current position vales
     */
    if (current_page) {
	current_page->saved_pos = XmTextGetInsertionPosition(node_text);

	if (w)
	    XtVaGetValues(w, XmNvalue, &current_page->saved_sbar, NULL);
    }
    XmTextClearSelection(node_text,
			 XtLastTimestampProcessed(XtDisplay(top_level)));
    /* 
     * Make sure it doesn't try anything cute until we're ready 
     */
    XmTextDisableRedisplay(node_text);

    XmTextSetString(node_text, page->text);


    /* 
     * Make sure we return to the previous position if we had 
     * already visited this page.
     */
    XmTextSetInsertionPosition(node_text, page->saved_pos);
    XmTextScroll(node_text, page->saved_sbar);

    /* 
     * Update the page stack.
     */
    if (current_page) {
	struct page_stack *stack;

	stack = (struct page_stack *) malloc(sizeof(struct page_stack));
	stack->page = current_page;
	stack->prev = page_stack;
	page_stack = stack;
	XtSetSensitive(back_button, True);
    }
    current_page = page;

    /* Go for redisplay */

    XmTextEnableRedisplay(node_text);
}


/*
 * Customized drawing routines for our widget
 */

/*
 * Graphis context setup used to draw normal text
 */
static void set_normal_gc(XmTextWidget tw, GC gc)
{
    XGCValues gcv;

    gcv.background = tw->core.background_pixel;
    gcv.foreground = tw->primitive.foreground;

    XChangeGC(XtDisplay(tw), gc, GCForeground | GCBackground, &gcv);
}

/*
 * Graphics context used to fill out normal text areas.
 */
static void set_inverse_gc(XmTextWidget tw, GC gc)
{
    XGCValues gcv;

    gcv.foreground = tw->core.background_pixel;
    gcv.background = tw->primitive.foreground;

    XChangeGC(XtDisplay(tw), gc, GCForeground | GCBackground, &gcv);
}

/*
 * Graphics context used to fill out areas in selections.
 */
static void set_selection_gc(XmTextWidget tw, GC gc)
{
    XGCValues gcv;

    gcv.foreground = app_data.hbgpixel;
    gcv.background = app_data.hfgpixel;

    XChangeGC(XtDisplay(tw), gc, GCForeground | GCBackground, &gcv);
}

/*
 * Graphics context used to draw text in selections and non visited links.
 */
static void set_highlight_gc(XmTextWidget tw, GC gc)
{
    XGCValues gcv;

    gcv.foreground = app_data.hfgpixel;
    gcv.background = app_data.hbgpixel;

    XChangeGC(XtDisplay(tw), gc, GCForeground | GCBackground, &gcv);
}

/*
 * Graphics context setup for links which had been already visited
 * during this session.
 */
static void set_visited_gc(XmTextWidget tw, GC gc)
{
    XGCValues gcv;

    gcv.foreground = app_data.vfgpixel;
    gcv.background = app_data.hbgpixel;

    XChangeGC(XtDisplay(tw), gc, GCForeground | GCBackground, &gcv);
}

static void set_margin_clip(XmTextWidget tw, GC gc)
{
    XRectangle rect;
    Dimension mwidth = tw->text.margin_width +
    tw->primitive.shadow_thickness +
    tw->primitive.highlight_thickness;
    Dimension mheight = tw->text.margin_height +
    tw->primitive.shadow_thickness +
    tw->primitive.highlight_thickness;

    if (mwidth < tw->core.width)
	rect.x = mwidth;
    else
	rect.x = tw->core.width;

    if (mheight < tw->core.height)
	rect.y = mheight;
    else
	rect.y = tw->core.height;

    if (2 * mwidth < tw->core.width)
	rect.width = tw->core.width - 2 * mwidth;
    else
	rect.width = 0;

    if (2 * mheight < tw->core.height)
	rect.height = tw->core.height - 2 * mheight;
    else
	rect.height = 0;

    XSetClipRectangles(XtDisplay(tw), gc, 0, 0, &rect, 1, Unsorted);
}

static void set_full_clip(XmTextWidget tw, GC gc)
{
    XRectangle rect;
    int shade = tw->primitive.highlight_thickness
    + tw->primitive.shadow_thickness;

    rect.y = rect.x = shade;
    rect.width = tw->core.width - 2 * shade;
    rect.height = tw->core.height - 2 * shade;

    XSetClipRectangles(XtDisplay(tw), gc, 0, 0, &rect, 1, Unsorted);
}

/*
 * Routines used to draw non texttrual elements.
 */
static void draw_dot(Display * d, Window w, GC gc,
		     int x, int y,
		     int width)
{
    XFillArc(d, w, gc, x, y, width, width, 0, 23040);
    XDrawArc(d, w, gc, x, y, width, width, 0, 23040);
}

static void draw_odot(Display * d, Window w, GC gc,
		      int x, int y,
		      int width)
{
    XDrawArc(d, w, gc, x, y, width, width, 0, 23040);
}

static void draw_shade_line(Display * d, Window w, GC gc,
			    int x, int y, int width)
{
    XGCValues gcv;

    gcv.foreground = background_pixel;
    XChangeGC(d, gc, GCForeground, &gcv);
    XDrawLine(d, w, gc, x, y, x + width, y);

    gcv.foreground = top_pixel;
    XChangeGC(d, gc, GCForeground, &gcv);
    XDrawLine(d, w, gc, x, y - 1, x + width, y - 1);
    XDrawPoint(d, w, gc, x, y);
    XDrawPoint(d, w, gc, x, y + 1);

    gcv.foreground = bottom_pixel;
    XChangeGC(d, gc, GCForeground, &gcv);
    XDrawLine(d, w, gc, x + 1, y + 1, x + width, y + 1);
    XDrawPoint(d, w, gc, x + width - 1, y);

}

static void draw_arrow(Display * d, Window w, GC gc,
		       int x, int y,
		       int width, int height)
{
    XGCValues gcv;
    XPoint segs[8];

    segs[0].x = x;
    segs[0].y = y + height / 4;

    segs[1].x = x;
    segs[1].y = y - height / 4;

    segs[2].x = x + width - height / 2 - 1;
    segs[2].y = y - height / 4;

    segs[3].x = x + width - height / 2 - 1;
    segs[3].y = y - height / 2;

    segs[4].x = x + width - 1;
    segs[4].y = y;

    segs[5].x = x + width - height / 2 - 1;
    segs[5].y = y + height / 2;

    segs[6].x = x + width - height / 2 - 1;
    segs[6].y = y + height / 4;	/* remove +1 if using XDrawLines */

    segs[7].x = x;
    segs[7].y = y + height / 4;	/* remove +1 if using XDrawLines */

    gcv.foreground = background_pixel;
    XChangeGC(d, gc, GCForeground, &gcv);
    XFillPolygon(d, w, gc, segs, 8, Complex, CoordModeOrigin);

    gcv.foreground = top_pixel;
    XChangeGC(d, gc, GCForeground, &gcv);
    XDrawLines(d, w, gc, segs, 5, CoordModeOrigin);
    XDrawPoints(d, w, gc, segs, 5, CoordModeOrigin);

    gcv.foreground = bottom_pixel;
    XChangeGC(d, gc, GCForeground, &gcv);
    XDrawLines(d, w, gc, segs + 4, 4, CoordModeOrigin);
    XDrawPoints(d, w, gc, segs + 4, 4, CoordModeOrigin);
}

/*
 * Determine the displaying width of a particular character
 */
static int character_width(XmTextWidget tw, XFontStruct * font,
			   int x,
			   XmTextBlock block,
			   int from, int to)
{
    OutputData data = tw->text.output->data;
    char *ptr;
    unsigned char c;
    int i;
    int result = 0;


    for (i = from, ptr = block->ptr + from; i < to; i++, ptr++) {
	c = (unsigned char) *ptr;
	if (c == '\t')
	    result += (data->tabwidth -
		     ((x + result - data->leftmargin) % data->tabwidth));
	else {
	    if (font->per_char) {
		if (c >= font->min_char_or_byte2 && c <= font->max_char_or_byte2)
		    result += font->per_char[c - font->min_char_or_byte2].width;
		else if (font->default_char >= font->min_char_or_byte2 &&
			 font->default_char <= font->max_char_or_byte2)
		    result += font->per_char[font->default_char -
					  font->min_char_or_byte2].width;
		else
		    result += font->min_bounds.width;
	    } else
		result += font->min_bounds.width;
	}
    }

    return result;
}

/*
 * Check if we are inside the refference chain initiated by xref.
 * Return the refference we are in or NULL otherwise.
 *
 * This routine is used in the resolving of click's too.
 */
struct menu *check_in_xref(struct menu *xref, int start, int stop)
{
    while (xref) {
	if ((xref->start >= start && xref->start <= stop)
	    || (xref->stop >= start && xref->stop <= stop)
	    || (start >= xref->start && stop <= xref->stop))
	    return xref;
	xref = xref->next;
    }

    return NULL;
}

/*
 * Check if we are inside the underlined part of a refference chain
 * initiated by xref. Return the refference we are in or NULL otherwise.
 */
static struct menu *check_in_uline(struct menu *xref, int start, int stop)
{
    while (xref) {
	if ((xref->ustart >= start && xref->ustart <= stop)
	    || (xref->ustop >= start && xref->ustop <= stop)
	    || (start >= xref->ustart && stop <= xref->ustop))
	    return xref;
	xref = xref->next;
    }

    return NULL;
}

/*
 * Check if we are inside of some special drawing area.
 */
static enum chunk_kind check_in_chunk(struct page *page, int start, int stop, struct chunk **chunk)
{
    struct chunk *c;

    if (!page)
	return 0;

    for (c = page->chunk; c; c = c->next)
	if (start >= c->start && stop <= c->stop) {
	    *chunk = c;
	    return c->kind;
	}
    *chunk = NULL;
    return NO_CHUNK;
}

/*
 * Get the text from the previous line above the index positions
 * between start and stop.
 * The is needed to fixup compleatly the redrawing of underlines in
 * headers. Otherise, we would get some blank areas.
 */

static int get_above(char *text, int start)
{
    int i;
    int delta;

    i = start - 1;
    while (i >= 0 && text[i] != '\n')
	--i;
    if (i < 0)
	return -1;
    delta = start - i;
    do
	--i;
    while (i >= 0 && text[i] != '\n');

    return i + delta;
}


/*
 * This is the main drawing routine, which will be substitued 
 * as the drawing engine of the motif Text widget, which we are
 * using to display our text.
 * 
 * Pleas don't get the impression that I'm always writing such
 * messy code. This here isn't jsut going to control any aeroplain,
 * so I did it in a hurry of one day. Just coding ahead to make
 * it produce somehow reasonable result.
 *
 * In fact currently I'm already quite pleased by the result's.
 */

/* XDrawString -> Xutf8DrawString, but now we need a fontset. 
We may need to do something like 

XFontSet fontSet = XCreateFontSet(display, "-*-courier-medium-r-*-*-*-120-75-75-*-*", 
                                               &missingCharsetList, &missingCharsetCount, 
                                               &defaultString);
    
    printf("fontSet %p, missing: %p(%d), default: %p\n", fontSet, missingCharsetList,
                                                                  missingCharsetCount, defaultString[0]&0xff);
    
    
    XFontStruct* fontStruct = XLoadQueryFont(display, 
                                             //"-*-courier-medium-r-*-*-*-120-75-75-*-*-iso10646-1"
                                             "-*-courier-medium-r-*-*-*-120-75-75-*-*-iso8859-15"
                                            );
    Font font = fontStruct->fid;
XSetFont(display, gc, font);

*/
 
