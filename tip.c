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

#include <unistd.h>
#include <signal.h>

#include <stdio.h>

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <Xm/XmP.h>

#include <Xm/PushB.h>
#include <X11/ShellP.h>

#include "tip.h"

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

/* keep information about each widget we are keeping track of */
struct tip_context {
    Widget watched;		/* the widget we are watching */
    Window window;		/* Window of the object we are monitoring */
    TipWidget tw;		/* pointer back to the tip widget */
    Position abs_x, abs_y;
    Boolean active;		/* if False, tip is suppressed */
    char *text;			/* text to display */
    short size;			/* its size */
};

/* New fields for the LiteClue widget record */
typedef struct {
    /* resources */
    Pixel foreground;
    XFontSet fontset;		/* the font for text in box */
    int waitPeriod;		/* the delay resource - pointer must be
				   in watched widget this long before
				   help is poped - in millisecs
				 */
    int cwp;			/* after help is popped-down - normal
				   wait period is cancelled for this
				   period - in millisecs
				 */

    /* -------- private state --------- */
    struct tip_context twl[16];	/* list of widgets we are liteClue-ing */
    Cardinal nr_twl;		/* number of widgets we have attached */
    Dimension font_width;	/* width of '1' character */
    Dimension font_height;	/* height of font, rows are spaced using this */
    Dimension font_baseline;	/* relative displacement to baseline from top */
    GC text_GC;			/* for drawing text */
    XtIntervalId tid;		/* holds timer id */
    Widget isup;		/* the help popup is up on this widget */
    Time HelpPopDownTime;	/* the time at which help popup 
				   was popped down */
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

#define CheckWidgetClass(routine) \
	if (XtClass(w) != tipWidgetClass) \
		wrong_widget(routine)

static void initialize(Widget, Widget, ArgList, Cardinal *);
static Boolean set_values(Widget, Widget, Widget, ArgList, Cardinal *);
static void destroy(Widget);

/*
 * Widget resources: eg to set tip box background:
 * *Tip.background: yellow      
 */

#define offset(field) XtOffsetOf(TipRec, field)
static XtResource resources[] =
{
    {XtNforeground, XtCForeground,
     XtRPixel, sizeof(Pixel), offset(tip.foreground),
     XtRString, "black"},
    {XtNfontSet, XtCFontSet,
     XtRFontSet, sizeof(XFontSet), offset(tip.fontset),
     XtRString, "fixed"},
    {XmNwaitPeriod, XmCWaitPeriod,
     XtRInt, sizeof(int), offset(tip.waitPeriod),
     XtRImmediate, (XtPointer) 1000},
    {XmNcancelWaitPeriod, XmCCancelWaitPeriod,
     XtRInt, sizeof(int), offset(tip.cwp),
     XtRImmediate, (XtPointer) 250},
};
#undef offset

TipClassRec tipClassRec =
{
    {
 /* superclass           */ (WidgetClass) & overrideShellClassRec,
 /* class_name           */ "Tip",
 /* widget size          */ (Cardinal) sizeof(TipRec),
 /* class_init           */ NULL,
 /* class_part_init      */ (XtWidgetClassProc) NULL,
 /* class_inited         */ (XtEnum) FALSE,
 /* initialize           */ (XtInitProc) initialize,
 /* init_hook            */ (XtArgsProc) NULL,
 /* realize              */ XtInheritRealize,
 /* actions              */ (XtActionList) 0,
 /* num_actions          */ (Cardinal) 0,
 /* resources            */ (XtResourceList) resources,
 /* num_resources        */ (Cardinal) XtNumber(resources),
 /* xrm_class            */ NULLQUARK,
 /* compress_motion      */ TRUE,
 /* compress_exposur     */ (XtEnum) FALSE,
 /* compress enterleave  */ TRUE,
 /* visibility_interest  */ FALSE,
 /* destroy              */ destroy,
 /* resize               */ XtInheritResize,
 /* expose,              */ XtInheritExpose,
 /* set_values           */ (XtSetValuesFunc) set_values,
 /* set_values_hook      */ (XtArgsFunc) NULL,
 /* set_values_almost    */ XtInheritSetValuesAlmost,
 /* get_values_hook      */ (XtArgsProc) NULL,
 /* accept_focus         */ XtInheritAcceptFocus,
 /* version              */ XtVersion,
 /* callback_private     */ (XtPointer) NULL,
 /* translations         */ XtInheritTranslations,
 /* query_geometry       */ XtInheritQueryGeometry,
 /* display_accelerator  */ XtInheritDisplayAccelerator,
 /* extension            */ (XtPointer) 0,
    }
    ,
    /* composite part */
    {
/* geometry_manager     */ XtInheritGeometryManager,
/* change_managed       */ XtInheritChangeManaged,
/* insert_child         */ XtInheritInsertChild,
/* delete_child         */ XtInheritDeleteChild,
/* extension            */ NULL
    }
    ,
    /* Shell */
    {
	(XtPointer) NULL,
    }
    ,
    /* Override Shell */
    {
	0,
    }
    ,
    /* tip */
    {
	0,
    }
};

WidgetClass tipWidgetClass = (WidgetClass) & tipClassRec;

/*
 * The font_information is derived 
 */
static void compute_font_info(TipWidget cw)
{
    XRectangle ink;
    XRectangle logical;

    if (!cw->tip.fontset)
	return;
    XmbTextExtents(cw->tip.fontset, "1", 1, &ink, &logical);

    cw->tip.font_baseline = -logical.y;		/* y offset from top to baseline, 
						   don't know why this is returned as negative */
    cw->tip.font_width = logical.width;		/* the width and height of the object */
    cw->tip.font_height = logical.height;
}

/*
 * Creates the various graphic contexts we will need 
 */
static void create_GC(TipWidget cw)
{
    XtGCMask valuemask;
    XGCValues myXGCV;

    valuemask = GCForeground | GCBackground | GCFillStyle;
    myXGCV.foreground = cw->tip.foreground;
    myXGCV.background = cw->core.background_pixel;
    myXGCV.fill_style = FillSolid;

    if (cw->tip.text_GC)
	XtReleaseGC((Widget) cw, cw->tip.text_GC);
    cw->tip.text_GC = XtGetGC((Widget) cw, valuemask, &myXGCV);
}

/*
 * A routine to halt execution and force, a core dump for debugging
 * analysis when a public routine is called with the wrong class
 * of widget.
 */
static void wrong_widget(char *routine)
{
    int mypid = getpid();
    fprintf(stderr, "Wrong class of widget passed to %s\n", routine);
    fflush(stderr);
    kill(mypid, SIGABRT);
}

/*
 * Global list of shells for tips, we will use.
 */

static TipWidget *shells = NULL;
static int nr_shells = 0;

/*************************************************************************
 ** Widget Methods *******************************************************
 *************************************************************************/

static void initialize(Widget treq, Widget tnew, ArgList args, Cardinal * nargs)
{
    TipWidget tw = (TipWidget) tnew;

    tw->tip.text_GC = NULL;
    tw->tip.isup = NULL;
    tw->tip.HelpPopDownTime = 0;
    tw->tip.tid = (XtIntervalId) 0;
    tw->tip.nr_twl = 0;
    compute_font_info(tw);
    create_GC(tw);

    /*
     * Add to our list of tip shells
     */
    if (!shells)
	shells = (TipWidget *) XtMalloc(sizeof(TipWidget));
    else
	shells = (TipWidget *) XtRealloc((char *) shells,
				    sizeof(TipWidget) * (nr_shells + 1));

    shells[nr_shells++] = tw;
}

static Boolean set_values(Widget _current, Widget _request, Widget _new,
			  ArgList args, Cardinal * nargs)
{
    TipWidget cw_new = (TipWidget) _new;
    TipWidget cw_cur = (TipWidget) _current;

    /* values of cw_new->tip.cwp and
       cw_new->tip.waitPeriod are accepted without checking */

    if (cw_new->tip.foreground != cw_cur->tip.foreground
     || cw_new->core.background_pixel != cw_cur->core.background_pixel) {
	create_GC(cw_new);
    }
    return FALSE;
}

static void destroy(Widget w)
{
    TipWidget tw = (TipWidget) w;
    int i;
    Boolean copy = False;

    /*
     * Remove this tip shell from our global list.
     */
    for (i = 0; i < nr_shells; ++i) {
	if (shells[i] == tw) {
	    copy = True;
	    --nr_shells;
	}
	if (copy && nr_shells)
	    shells[i] = shells[i + 1];
    }
    if (!nr_shells) {
	XtFree((char *) shells);
	shells = NULL;
    }
}

/*************************************************************************
 ** Event handlers *******************************************************
 *************************************************************************/

/* At this point the help may be popup 
 */
static void timeout_event(XtPointer client_data, XtIntervalId * id)
{
#define BorderPix 2
    struct tip_context *obj = (struct tip_context *) client_data;
    TipWidget tw = obj->tw;
    Position abs_x, abs_y;

    XRectangle ink;
    XRectangle logical;
    Position w_height;
    Position w_width;
    Widget w;

    if (tw->tip.tid == (XtIntervalId) 0)
	return;			/* timeout was removed but callback happened
				   anyway */
    tw->tip.tid = (XtIntervalId) 0;
    if (obj->active == False)
	return;

    w = obj->watched;
    if (!XtIsManaged(w))
	return;
    XtVaGetValues(w, XtNheight, &w_height, XtNwidth, &w_width, NULL);
    /* position just below the widget */
    XtTranslateCoords(w, w_width / 2, w_height, &abs_x, &abs_y);

    XmbTextExtents(tw->tip.fontset, obj->text, obj->size, &ink, &logical);

    XtResizeWidget((Widget) tw, 2 * BorderPix + logical.width,
	     2 * BorderPix + tw->tip.font_height, tw->core.border_width);
    XtMoveWidget((Widget) tw, abs_x, abs_y + 6);
    XtPopup((Widget) tw, XtGrabNone);
    tw->tip.isup = obj->watched;

    XmbDrawImageString(XtDisplay((Widget) tw),
		       XtWindow((Widget) tw),
		       tw->tip.fontset,
		       tw->tip.text_GC,
		       BorderPix,
		       BorderPix + tw->tip.font_baseline,
		       obj->text, obj->size);
}

/*
 * Pointer enters watched widget, set a timer at which time it will
 * popup the help
 */
static void enter(struct tip_context *obj, XEvent * xevent, XtAppContext app)
{
    TipWidget tw = obj->tw;
    XEnterWindowEvent *event = &xevent->xcrossing;
    int current_waitPeriod;

    if (obj->active == False)
	return;

    /* check for two enters in a row - happens when widget is
       exposed under a pop-up */
    if (tw->tip.tid != (XtIntervalId) 0) {
	return;
    }
    if (event->mode != NotifyNormal)
	return;

    if ((event->time - tw->tip.HelpPopDownTime) >
	tw->tip.cwp)
	current_waitPeriod = tw->tip.waitPeriod, timeout_event;
    else
	current_waitPeriod = 0;
    tw->tip.tid = XtAppAddTimeOut(app,
				  current_waitPeriod, timeout_event,
				  (XtPointer) obj);
}

/*
 * Remove timer, if its pending. Then popdown help.
 */
static void leave(struct tip_context *obj,
		  XEvent * xevent)
{
    TipWidget tw = obj->tw;
    XEnterWindowEvent *event = &xevent->xcrossing;

    if (tw->tip.tid != (XtIntervalId) 0) {
	XtRemoveTimeOut(tw->tip.tid);
	tw->tip.tid = (XtIntervalId) 0;
    }
    if (obj->active == False)
	return;
    if (tw->tip.isup) {
	XtPopdown((Widget) tw);
	tw->tip.isup = NULL;
	tw->tip.HelpPopDownTime = event->time;
    }
}

/************************************************************************
 ** Public interface ****************************************************
 ************************************************************************/

void TipAppHandle(XtAppContext app, XEvent * xevent)
{
    int i;
    if (xevent->type == EnterNotify || xevent->type == LeaveNotify ||
	xevent->type == ButtonPress)
	for (i = 0; i < nr_shells; ++i) {
	    int j;

	    for (j = 0; j < shells[i]->tip.nr_twl; ++j) {
		if (xevent->xany.window == shells[i]->tip.twl[j].window) {

		    if (xevent->type == EnterNotify)
			enter(shells[i]->tip.twl + j, xevent, app);

		    if (xevent->type == LeaveNotify ||
			xevent->type == ButtonPress)
			leave(shells[i]->tip.twl + j, xevent);

		}
	    }
	}
}

void TipAppMainLoop(XtAppContext app)
{
    XEvent event;

    for (;;) {
	XtAppNextEvent(app, &event);
	TipAppHandle(app, &event);
	XtDispatchEvent(&event);
    }
}

/*
 * Add a widget to be watched tip will be given for this widget
 * 
 * This function must be called after the widget has been realized!
 * Further on please make sure that this function will not be called twice
 * for one button!
 *
 * w            - tip widget 
 * watch        - the widget to give tips for
 * text         - pointer to tip text. (May be NULL) 
 * size         - size of text. May be zero in which case a strlen
 *                will be done. 
 * option       - option mask, future use, zero for now.
 * 
 */
void TipAddWidget(Widget w, Widget watch, char *text, int size, int option)
{
#define ROUTINE "TipAddWidget"
    TipWidget tw = (TipWidget) w;
    int i;

    CheckWidgetClass(ROUTINE);	/* make sure we are called with a tip widget */

    for (i = 0; i < nr_shells; ++i)
	if (shells[i] == tw) {
	    struct tip_context *obj;

	    obj = tw->tip.twl + tw->tip.nr_twl;
	    obj->text = XtNewString(text);
	    obj->size = strlen(text);
	    obj->watched = watch;
	    obj->window = XtWindow(watch);
	    obj->active = True;
	    obj->tw = tw;
	    tw->tip.nr_twl++;
	}
#undef ROUTINE
}
