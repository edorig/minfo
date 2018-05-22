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

#ifndef TIPP_h
#define TIPP_h

#include <X11/ShellP.h>

#include "Tip.h"

/* Doubly Linked List Processing */
struct list_thread_str {
    struct list_thread_str *forw;	/* next pointer */
    struct list_thread_str *back;	/* prev pointer */
};
typedef struct list_thread_str ListThread;


typedef struct {
    int nothing;		/* place holder */
} TipClassPart;

/* Full class record declaration */
typedef struct _TipClassRec {
    CoreClassPart core_class;
    CompositeClassPart composite_class;
    ShellClassPart shell_class;
    OverrideShellClassPart override_shell_class;
    TipClassPart tip_class;
} TipClassRec;

extern TipClassRec xmTipClassRec;

/* New fields for the LiteClue widget record */
typedef struct {
    /* resources */
    Pixel foreground;
    XFontSet fontset;		/* the font for text in box */
    int waitPeriod;		/* the delay resource - pointer must be
				   in watched widget this long before
				   help is poped - in millisecs
				 */
    int cancelWaitPeriod;	/* after help is popped-down - normal
				   wait period is cancelled for this
				   period - in millisecs
				 */

    /* -------- private state --------- */
    ListThread widget_list;	/* list of widgets we are liteClue-ing */
    Dimension font_width;	/* width of '1' character */
    Dimension font_height;	/* height of font, rows are spaced using this */
    Dimension font_baseline;	/* relative displacement to baseline from top */
    GC text_GC;			/* for drawing text */
    XtIntervalId tid;		/* holds timer id */
    XtIntervalId pid;		/* holds pooler id for insensitive widgets */
    Widget parent;
    Widget isup;		/* the help popup is up on this widget */
    Time HelpPopDownTime;	/* the time at which help popup was popped down */
} TipPart;


/*
 * Full instance record declaration
 */
typedef struct _TipRec {
    CorePart core;
    CompositePart composite;
    ShellPart shell;
    OverrideShellPart override;
    TipPart tip;
} TipRec;


#endif
