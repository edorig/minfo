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

/*
 * Access to arbitrary resources in the resource database
 */

#include <string.h>
#include <ctype.h>
#include <X11/StringDefs.h>

#include "resource.h"

char *XmLoadStringResource(Widget w, char *ResourceName,
			   char *ResourceClass,
			   char *Default)
{
    XtResource Resources[] =
    {
	{NULL, NULL, XtRString, sizeof(String), 0, XtRString, NULL}
    };
    String TheString;
    String TempClassName = NULL;

    Resources[0].resource_name = ResourceName;
    if (ResourceClass)
	Resources[0].resource_class = ResourceClass;
    else {
	TempClassName = XtNewString(ResourceName);
	TempClassName[0] = toupper(TempClassName[0]);
	Resources[0].resource_class = TempClassName;
    }
    XtGetApplicationResources(w, (XtPointer) & TheString,
			      Resources, XtNumber(Resources),
			      NULL, 0);
    if (ResourceClass == NULL)
	XtFree(TempClassName);
    return TheString ? TheString : Default;
}
