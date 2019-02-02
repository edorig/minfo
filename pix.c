
/*
 * (C) 1997 by Marcin Dalecki <dalecki@math.uni-goettingen.de>
 *
 * This file is part of the Motif Editor.
 *
 * The GNU Info widget is free software; you can redistribute it and/or
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

/*
 * Pixmap handling code.
 *
 * Thank's to the famous game xbill we got the technology, how to
 * create pixmaps behaving well on pseudo/virtual visuals :-)
 */

#include <X11/Intrinsic.h>
#include <X11/xpm.h>

#include <Xm/Xm.h>


/*#include "xpm.h" */ 

/*
 * Setup the icon of the given application
 */

void setup_icon(Widget w, char *icon[])
{
    Display *dsp;
    Screen *scr;
    XpmAttributes attr;
    Colormap cmap;
    int depth;
    Pixmap shape;
    Pixmap image;
    Window win;

    dsp = XtDisplay(w);
    scr = XtScreen(w);
    cmap = DefaultColormap(dsp, DefaultScreen(dsp));
    depth = DefaultDepthOfScreen(scr);

    win = XRootWindowOfScreen(scr);

    XtVaSetValues(w, XtNcolormap, cmap, NULL);

    attr.valuemask = 0L;
    attr.valuemask |= XpmCloseness;
    attr.valuemask |= XpmReturnPixels;
    attr.valuemask |= XpmColormap;
    attr.valuemask |= XpmDepth;
    attr.closeness = 65535;	/* accuracy isn't crucial */
    attr.colormap = cmap;
    attr.depth = depth;

    XpmCreatePixmapFromData(dsp, win, icon, &image, &shape, &attr);
    XtVaSetValues(w, XmNiconPixmap, image, XmNiconMask, shape, NULL);
}
