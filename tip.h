
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

#ifndef TIP_H
#define TIP_H
#include <X11/StringDefs.h>

/*
 * New resource names
 */

#define XmNcancelWaitPeriod	 "cancelWaitPeriod"
#define XmNwaitPeriod	 "waitPeriod"

/*
 * New resource classes
 */

#define XmCCancelWaitPeriod	"CancelWaitPeriod"
#define XmCWaitPeriod	"WaitPeriod"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

    extern WidgetClass tipWidgetClass;
    typedef struct _TipClassRec *TipWidgetClass;
    typedef struct _TipRec *TipWidget;

    extern void TipAddWidget(Widget, Widget, char *, int, int);
    extern void TipAppMainLoop(XtAppContext);
    extern void TipAppHandle(XtAppContext, XEvent *);

#if defined(__cplusplus) || defined(c_plusplus)
}

#endif
#endif
