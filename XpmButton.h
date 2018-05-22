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

#ifndef XPM_BUTTON_H
#define XPM_BUTTON_H

#include <Xm/PushB.h>

#ifdef __cplusplus
extern "C" {
#endif

    extern Widget XmCreatePixmapPushButton(Widget Parent, String Name,
					   char **Normal, char **Armed,
					   char **Insensitive,
					ArgList Args, Cardinal ArgCount);
    extern Widget XmCreatePixmapPushButtonGadget(Widget Parent, String Name,
					     char **Normal, char **Armed,
						 char **Insensitive,
					ArgList Args, Cardinal ArgCount);

    extern Widget XmCreatePixmapLabel(Widget Parent, String Name,
				      char **Normal, char **Insensitive,
				      ArgList Args, Cardinal ArgCount);
    extern Widget XmCreatePixmapLabelGadget(Widget Parent, String Name,
				       char **Normal, char **Insensitive,
					ArgList Args, Cardinal ArgCount);

#ifdef __cplusplus
}

#endif
#endif
