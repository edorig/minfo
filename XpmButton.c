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
 * Convenience routines to add pixmaps to buttons.
 * The following color names will be treat specially in the pixmaps we are
 * using:
 * 
 * "none", "shade0", "shade1", "shade2" are simbolic names for colors which
 * will be substitued with corresponding motif runtime color values.
 */

#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>

#include <X11/xpm.h>

#include "XpmButton.h"
#include "resource.h"


#define NOPIXMAP 0

/* To prevent overload of the X11-server with pixmap duplications we are
 * establishing an corresponding cache here. It isn't that fast but 
 * compleatly sufficient for most of the applications.
 */
typedef struct _CacheEntry CacheEntry;
struct _CacheEntry {
    CacheEntry *Next;
    Display *Dsp;		/* the display where the pixmap will 
				   appear */
    int Depth;			/* depth of the pixmap */
    Pixel Background;		/* background color */
    char **XpmData;		/* data in xpm format */
    int Type;			/* type of cached value */
    Pixmap Pixels;		/* X11 server ident of the pixmap */
    int RefCount;		/* number of refferences */
};

/* Definitions for contents of the Type field in CacheEntry
 * TYPE_ORIGINAL: This entry was generated from the given Pixmap or through
 *                the resource mechanism. In the second case the bit 
 *                TYPE_FROMRESOURCE is set too.
 * TYPE_ARMED:    The pixmap came from an loaded image.
 * TYPE_INSENSITIVE: dito....
 */
#define TYPE_ORIGINAL		0
#define TYPE_ARMED		1
#define TYPE_INSENSITIVE	2
#define TYPE_FROMRESOURCE	0x80

static CacheEntry *PixmapCache = NULL;	/* main cache anchor */


/*
 * Lookup the corresponding pixmap in the cache. If it's allready there, the
 * existing pixmap code from the server will be returned. Otherwise we will
 * create it and return the corresponding new id back.
 *
 *   Dsp                display for the pixmap
 *   Scr                corresponing screen
 *   Depth              necessary depth of the pixmap
 *   XpmData            pixmap data in the portable xpm format
 *   XpmAttr            attributes for creation
 *   ResourceXpmData    data from resource
 *   Type               makes it possible to pass allready existent
 *                      Pixmap data. But int this case there must be an
 *                      valid XpmData too, to prevent duplication.
 *   Default            Fallback pixmap value
 *   JustLookup         Don't add to cache it set.
 */
static Pixmap AddXpmToCache(Display * Dsp, Screen * Scr,
			    int Depth, Pixel Background,
			    char **XpmData, XpmAttributes * XpmAttr,
			    char *ResourceXpmData,
			    int Type, Pixmap Default,
			    Boolean JustLookup)
{
    CacheEntry *pCache = PixmapCache;

    if (ResourceXpmData)
	Type = Type | TYPE_FROMRESOURCE;
    /* 
     * Lookup cache
     */
    while ((pCache != NULL) &&
	   ((pCache->Dsp != Dsp) ||
	    (pCache->Depth != Depth) ||
	    (pCache->Background != Background) ||
	    (pCache->XpmData != XpmData) ||
	    (pCache->Type != Type)
	   ))
	pCache = pCache->Next;
    /*
     * Create and add to cache if neccessary
     */
    if (pCache == NULL) {
	int Error;
	Pixmap Pixels;

	if (JustLookup)
	    return NOPIXMAP;
	/*
	 * If no default specified, create the pixmap or just return the
	 * default otherwise.
	 */
	if (Default == NOPIXMAP) {
	    if (ResourceXpmData)
		Error = XpmCreatePixmapFromBuffer(Dsp, RootWindowOfScreen(Scr),
				ResourceXpmData, &Pixels, NULL, XpmAttr);
	    else
		Error = XpmCreatePixmapFromData(Dsp, RootWindowOfScreen(Scr),
					XpmData, &Pixels, NULL, XpmAttr);
	    if (Error != XpmSuccess)
		return NOPIXMAP;
	    XpmFreeAttributes(XpmAttr);
	} else
	    Pixels = Default;

	pCache = (CacheEntry *) XtMalloc(sizeof(CacheEntry));
	if (pCache == NULL) {
	    XFreePixmap(Dsp, Pixels);
	    return NOPIXMAP;
	}
	pCache->Dsp = Dsp;
	pCache->Depth = Depth;
	pCache->Background = Background;
	pCache->XpmData = XpmData;
	pCache->Type = Type;
	pCache->Pixels = Pixels;
	pCache->RefCount = 1;

	pCache->Next = PixmapCache;
	PixmapCache = pCache;

	return Pixels;
    } else {
	/* return the allready cached value */
	++(pCache->RefCount);
	return pCache->Pixels;
    }
}

static void FreeXpmFromCache(Display * Dsp, Pixmap Pixels)
{
    CacheEntry *pCache = PixmapCache;
    CacheEntry *pPrev = NULL;

    while ((pCache != NULL) &&
	   ((pCache->Dsp != Dsp) ||
	    (pCache->Pixels != Pixels)
	   )) {
	pPrev = pCache;
	pCache = pCache->Next;
    }
    if (pCache == NULL)
	XtWarningMsg("PixmapCache", "pixmapcache", "UnknownPixmap",
		     "Pixmap to be freed not found in pixmap cache",
		     NULL, 0);
    else {
	if (--(pCache->RefCount) == 0) {
	    /* remove the pixmap from the X11 server */
	    XFreePixmap(Dsp, pCache->Pixels);
	    if (pPrev == NULL)
		/* it's the first cache entry */
		PixmapCache = pCache->Next;
	    else
		pPrev->Next = pCache->Next;
	    XtFree((char *) pCache);
	}
    }
}


/* 
 * This will trigger the garbage collecting for our cache.
 */
static void DestroyCallback(Widget w, XtPointer ClientData,
			    XtPointer CallData)
{
    Display *Dsp;
    Pixmap NormalPixmap, ArmedPixmap, InsensitivePixmap;
    Boolean IsButton;

    /* shouldn't happen */
    if (ClientData == NULL)
	return;

    IsButton = XmIsPushButton(w) || XmIsPushButtonGadget(w);
    NormalPixmap = ((Pixmap *) ClientData)[0];
    InsensitivePixmap = ((Pixmap *) ClientData)[1];
    Dsp = XtDisplay(w);
    FreeXpmFromCache(Dsp, NormalPixmap);
    FreeXpmFromCache(Dsp, InsensitivePixmap);
    if (IsButton) {
	ArmedPixmap = ((Pixmap *) ClientData)[2];
	FreeXpmFromCache(Dsp, ArmedPixmap);
    }
    XtFree((char *) ClientData);
}


static void CreateButtonPixmaps(Widget PushButton,
				char **Normal, char **Armed,
				char **Insensitive)
{
    Display *Dsp;
    Screen *Scr;
    int Depth;
    Pixel bg;			/* normal background */
    Pixel abg;			/* armed background */
    Pixel bsc;			/* bottom shadow color */
    Pixel tsc;			/* top shadow color */

    int PixmapX, PixmapY;
    unsigned int PixmapHeight, PixmapWidth, PixmapBorder, PixmapDepth;
    Window RootWindow;
    Pixmap NormalPixmap, ArmedPixmap, InsensitivePixmap = 0;
    XpmAttributes XpmAttr;
    Boolean RecomputeSize;
    char *ResourceData;
    Boolean HasResourceData;
    Pixmap *PixmapReminder;
    /*
     * Predefined color names which will adjust to the usage context
     */
    XpmColorSymbol color[4] =
    {
	{"mask", NULL, 0},
	{"shade0", NULL, 0},
	{"shade1", NULL, 0},
	{"shade2", NULL, 0}
    };


    /*
     * We need to be carefull here, since in case of gadgets, there is
     * no way to get the background color directly from the widget itself.
     * In such cases we get it from The Core part of his parent instead.
     */
    Dsp = XtDisplayOfObject(PushButton);
    Scr = XtScreenOfObject(PushButton);
    XtVaGetValues(XtIsSubclass(PushButton, coreWidgetClass) ?
		  PushButton : XtParent(PushButton),
		  XmNdepth, &Depth,
		  XmNbackground, &bg,
		  XmNarmColor, &abg,
		  XmNbottomShadowColor, &bsc,
		  XmNtopShadowColor, &tsc,
		  NULL);

    color[1].pixel = tsc;
    color[2].pixel = bg;
    color[3].pixel = bsc;
    /* First we create the pixmap for the normal , active atate of the button.
     */
    ResourceData = XmLoadStringResource(PushButton, "normalFace",
					NULL, NULL);
    HasResourceData = ResourceData != NULL;
    color[0].pixel = bg;
    XpmAttr.valuemask = XpmColorSymbols | XpmCloseness | XpmDepth;
    XpmAttr.colorsymbols = color;
    XpmAttr.numsymbols = 4;
    XpmAttr.closeness = 65535;
    XpmAttr.depth = Depth;
    NormalPixmap = AddXpmToCache(Dsp, Scr, Depth, bg,
				 Normal, &XpmAttr, ResourceData,
				 TYPE_ORIGINAL, NOPIXMAP, False);
    if (NormalPixmap == NOPIXMAP)
	return;

    /* now for the pressed state... */
    ResourceData = XmLoadStringResource(PushButton, "armedFace",
					NULL, NULL);
    if ((Armed && !HasResourceData) ||
	(ResourceData && HasResourceData)) {
	color[0].pixel = abg;
	XpmAttr.valuemask = XpmColorSymbols | XpmCloseness | XpmDepth;
	XpmAttr.colorsymbols = color;
	XpmAttr.numsymbols = 4;
	XpmAttr.closeness = 65535;
	XpmAttr.depth = Depth;
	ArmedPixmap = AddXpmToCache(Dsp, Scr, Depth, abg,
				    Armed, &XpmAttr, ResourceData,
				    TYPE_ORIGINAL, NOPIXMAP, False);
	if (ArmedPixmap == NOPIXMAP) {
	    FreeXpmFromCache(Dsp, NormalPixmap);
	    return;
	}
    } else {
	/* If there wasn't any pixmap specified for the state, we will 
	 * create our own one here. For this we move the image on the normal 
	 * pixmap by the shadw size top down.
	 */
	GC gc;
	XGCValues GCValues;
	Dimension ShadowThickness;

	ArmedPixmap = AddXpmToCache(Dsp, Scr, Depth, abg,
				    Normal, &XpmAttr, NULL,
				    TYPE_ARMED, NOPIXMAP, True);
	if (ArmedPixmap == NOPIXMAP) {
	    XtVaGetValues(PushButton,
			  XmNshadowThickness, &ShadowThickness, NULL);
	    GCValues.foreground = abg;
	    gc = XtGetGC(PushButton, GCForeground, &GCValues);

	    XGetGeometry(Dsp, NormalPixmap, &RootWindow,
			 &PixmapX, &PixmapY, &PixmapWidth, &PixmapHeight,
			 &PixmapBorder, &PixmapDepth);
	    ArmedPixmap = XCreatePixmap(Dsp, RootWindowOfScreen(Scr),
					PixmapWidth, PixmapHeight,
					PixmapDepth);
	    XCopyArea(Dsp, NormalPixmap, ArmedPixmap, gc,
		      0, 0,
		      PixmapWidth - ShadowThickness,
		      PixmapHeight - ShadowThickness,
		      ShadowThickness, ShadowThickness);
	    XFillRectangle(Dsp, ArmedPixmap, gc, 0, 0,
			   PixmapWidth, ShadowThickness);
	    XFillRectangle(Dsp, ArmedPixmap, gc, 0, 0,
			   ShadowThickness, PixmapHeight);

	    XtReleaseGC(PushButton, gc);
	    if (AddXpmToCache(Dsp, Scr, Depth, bg, Normal,
			      &XpmAttr, NULL, TYPE_ARMED, ArmedPixmap,
			      False) == NOPIXMAP) {
		FreeXpmFromCache(Dsp, NormalPixmap);
		XFreePixmap(Dsp, ArmedPixmap);
		return;
	    }
	}
    }

    /* And now for the insensitive state.
     */
    ResourceData = XmLoadStringResource(PushButton, "insensitiveFace",
					NULL, NULL);
    if ((Insensitive && !HasResourceData) ||
	(ResourceData && HasResourceData)) {
	color[0].pixel = bg;
	XpmAttr.valuemask = XpmColorSymbols | XpmCloseness | XpmDepth;
	XpmAttr.colorsymbols = color;
	XpmAttr.numsymbols = 4;
	XpmAttr.closeness = 65535;
	XpmAttr.depth = Depth;
	InsensitivePixmap = AddXpmToCache(Dsp, Scr, Depth, bg,
					  Insensitive, &XpmAttr,
					  ResourceData,
					  TYPE_ORIGINAL, NOPIXMAP, False);
	if (InsensitivePixmap == NOPIXMAP) {
	    FreeXpmFromCache(Dsp, NormalPixmap);
	    FreeXpmFromCache(Dsp, ArmedPixmap);
	    return;
	}
    } else {
	/* Once again we will create our own one by stippling the original
	 * pixmap.
	 */
	GC gc;
	XGCValues GCValues;
	Pixmap Stipple;
	static char StippleBitmap[8] =
	{0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

	InsensitivePixmap = AddXpmToCache(Dsp, Scr, Depth, bg,
					  Normal, &XpmAttr, NULL,
					  TYPE_INSENSITIVE,
					  InsensitivePixmap, True);
	if (InsensitivePixmap == NOPIXMAP) {
	    Stipple = XCreateBitmapFromData(Dsp, RootWindowOfScreen(Scr),
					    StippleBitmap, 8, 8);
	    GCValues.foreground = bg;
	    GCValues.fill_style = FillStippled;
	    GCValues.stipple = Stipple;
	    gc = XtGetGC(PushButton,
			 GCForeground | GCFillStyle | GCStipple,
			 &GCValues);
	    XGetGeometry(Dsp, NormalPixmap, &RootWindow,
			 &PixmapX, &PixmapY, &PixmapWidth, &PixmapHeight,
			 &PixmapBorder, &PixmapDepth);
	    InsensitivePixmap = XCreatePixmap(Dsp, RootWindowOfScreen(Scr),
					      PixmapWidth, PixmapHeight,
					      PixmapDepth);
	    XCopyArea(Dsp, NormalPixmap, InsensitivePixmap, gc,
		      0, 0, PixmapWidth, PixmapHeight, 0, 0);
	    XFillRectangle(Dsp, InsensitivePixmap, gc,
			   0, 0, PixmapWidth, PixmapHeight);

	    XtReleaseGC(PushButton, gc);
	    XFreePixmap(Dsp, Stipple);
	    if (AddXpmToCache(Dsp, Scr, Depth, bg, Normal,
			      &XpmAttr, NULL, TYPE_INSENSITIVE,
			      InsensitivePixmap, False) == NOPIXMAP) {
		FreeXpmFromCache(Dsp, NormalPixmap);
		FreeXpmFromCache(Dsp, ArmedPixmap);
		XFreePixmap(Dsp, InsensitivePixmap);
		return;
	    }
	}
    }

    /* If anything went well, we can pass all the data to our button.
     */
    XtVaGetValues(PushButton,
		  XmNrecomputeSize, &RecomputeSize, NULL);
    XtVaSetValues(PushButton, XmNrecomputeSize, True, NULL);
    XtVaSetValues(PushButton,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, NormalPixmap,
		  XmNarmPixmap, ArmedPixmap,
		  XmNlabelInsensitivePixmap, InsensitivePixmap,
		  NULL);
    XtVaSetValues(PushButton, XmNrecomputeSize, RecomputeSize, NULL);

    /* Here we install the garbage collecting callback. 
     */
    PixmapReminder = (Pixmap *) XtMalloc(sizeof(Pixmap) * 3);
    if (PixmapReminder != NULL) {
	PixmapReminder[0] = NormalPixmap;
	PixmapReminder[1] = InsensitivePixmap;
	PixmapReminder[2] = ArmedPixmap;
    }
    XtAddCallback(PushButton, XmNdestroyCallback,
		  (XtCallbackProc) DestroyCallback,
		  (XtPointer) PixmapReminder);
}

Widget XmCreatePixmapPushButton(Widget Parent, String Name,
				char **Normal, char **Armed,
				char **Insensitive,
				ArgList Args, Cardinal ArgCount)
{
    Widget ButtonWidget;

    ButtonWidget = XmCreatePushButton(Parent, Name, Args, ArgCount);
    if (ButtonWidget == NULL)
	return NULL;
    CreateButtonPixmaps(ButtonWidget, Normal, Armed, Insensitive);
    return ButtonWidget;
}


Widget XmCreatePixmapPushButtonGadget(Widget Parent, String Name,
				      char **Normal, char **Armed,
				      char **Insensitive,
				      ArgList Args, Cardinal ArgCount)
{
    Widget ButtonWidget;

    ButtonWidget = XmCreatePushButtonGadget(Parent, Name, Args, ArgCount);
    if (ButtonWidget == NULL)
	return NULL;
    CreateButtonPixmaps(ButtonWidget, Normal, Armed, Insensitive);
    return ButtonWidget;
}


static void CreatePixmapLabel(Widget LabelWidget,
			      char **Normal, char **Insensitive)
{
    Display *Dsp;
    Screen *Scr;
    int Depth;
    int PixmapX, PixmapY;
    unsigned int PixmapHeight, PixmapWidth, PixmapBorder, PixmapDepth;
    Window RootWindow;
    Pixmap NormalPixmap, InsensitivePixmap = 0;
    XpmAttributes XpmAttr;
    Boolean RecomputeSize;
    Pixel bg, bsc, tsc;
    char *ResourceData;
    Boolean HasResourceData;
    Pixmap *PixmapReminder;
    XpmColorSymbol color[4] =
    {
	{"mask", NULL, 0},
	{"shade0", NULL, 0},
	{"shade1", NULL, 0},
	{"shade2", NULL, 0}
    };


    /*
     * We need to be carefull here, since in case of gadgets, there is
     * no way to get the background color directly from the widget itself.
     * In such cases we get it from The Core part of his parent instead.
     */
    Dsp = XtDisplayOfObject(LabelWidget);
    Scr = XtScreenOfObject(LabelWidget);
    XtVaGetValues(XtIsSubclass(LabelWidget, coreWidgetClass) ?
		  LabelWidget : XtParent(LabelWidget),
		  XmNdepth, &Depth,
		  XmNbackground, &bg,
		  XmNbottomShadowColor, &bsc,
		  XmNtopShadowColor, &tsc,
		  NULL);

    color[1].pixel = tsc;
    color[2].pixel = bg;
    color[3].pixel = bsc;

    ResourceData = XmLoadStringResource(LabelWidget, "normalFace",
					NULL, NULL);
    HasResourceData = ResourceData != NULL;
    color[0].pixel = bg;
    XpmAttr.valuemask = XpmColorSymbols | XpmCloseness | XpmDepth;
    XpmAttr.colorsymbols = color;
    XpmAttr.numsymbols = 4;
    XpmAttr.closeness = 65535;
    XpmAttr.depth = Depth;
    NormalPixmap = AddXpmToCache(Dsp, Scr, Depth, bg,
				 Normal, &XpmAttr, ResourceData,
				 TYPE_ORIGINAL, NOPIXMAP, False);
    if (NormalPixmap == NOPIXMAP)
	return;

    ResourceData = XmLoadStringResource(LabelWidget, "insensitiveFace",
					NULL, NULL);
    if ((Insensitive && !HasResourceData) ||
	(ResourceData && HasResourceData)) {
	XpmAttr.valuemask = XpmColorSymbols | XpmCloseness | XpmDepth;
	XpmAttr.closeness = 65535;
	XpmAttr.depth = Depth;
	InsensitivePixmap = AddXpmToCache(Dsp, Scr, Depth, bg,
					  Insensitive, &XpmAttr,
					  ResourceData,
					  TYPE_ORIGINAL, NOPIXMAP, False);
	if (InsensitivePixmap == NOPIXMAP) {
	    FreeXpmFromCache(Dsp, NormalPixmap);
	    return;
	}
    } else {
	GC gc;
	XGCValues GCValues;
	Pixmap Stipple;
	static char StippleBitmap[8] =
	{0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

	InsensitivePixmap = AddXpmToCache(Dsp, Scr, Depth, bg,
					  Normal, &XpmAttr, NULL,
					  TYPE_INSENSITIVE,
					  InsensitivePixmap, True);
	if (InsensitivePixmap == NOPIXMAP) {
	    Stipple = XCreateBitmapFromData(Dsp, RootWindowOfScreen(Scr),
					    StippleBitmap, 8, 8);
	    GCValues.foreground = bg;
	    GCValues.fill_style = FillStippled;
	    GCValues.stipple = Stipple;
	    gc = XtGetGC(LabelWidget,
			 GCForeground | GCFillStyle | GCStipple,
			 &GCValues);
	    XGetGeometry(Dsp, NormalPixmap, &RootWindow,
			 &PixmapX, &PixmapY, &PixmapWidth, &PixmapHeight,
			 &PixmapBorder, &PixmapDepth);
	    InsensitivePixmap = XCreatePixmap(Dsp, RootWindowOfScreen(Scr),
					      PixmapWidth, PixmapHeight,
					      PixmapDepth);
	    XCopyArea(Dsp, NormalPixmap, InsensitivePixmap, gc,
		      0, 0, PixmapWidth, PixmapHeight, 0, 0);
	    XFillRectangle(Dsp, InsensitivePixmap, gc,
			   0, 0, PixmapWidth, PixmapHeight);

	    XtReleaseGC(LabelWidget, gc);
	    XFreePixmap(Dsp, Stipple);
	    if (AddXpmToCache(Dsp, Scr, Depth, bg, Normal,
			      &XpmAttr, NULL, TYPE_INSENSITIVE,
			      InsensitivePixmap, False) == NOPIXMAP) {
		FreeXpmFromCache(Dsp, NormalPixmap);
		XFreePixmap(Dsp, InsensitivePixmap);
		return;
	    }
	}
    }

    XtVaGetValues(LabelWidget,
		  XmNrecomputeSize, &RecomputeSize, NULL);
    XtVaSetValues(LabelWidget, XmNrecomputeSize, True, NULL);
    XtVaSetValues(LabelWidget,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, NormalPixmap,
		  XmNlabelInsensitivePixmap, InsensitivePixmap,
		  NULL);
    XtVaSetValues(LabelWidget, XmNrecomputeSize, RecomputeSize, NULL);

    PixmapReminder = (Pixmap *) XtMalloc(sizeof(Pixmap) * 2);
    if (PixmapReminder != NULL) {
	PixmapReminder[0] = NormalPixmap;
	PixmapReminder[1] = InsensitivePixmap;
    }
    XtAddCallback(LabelWidget, XmNdestroyCallback,
		  (XtCallbackProc) DestroyCallback,
		  (XtPointer) PixmapReminder);
}

Widget XmCreatePixmapLabel(Widget Parent, String Name,
			   char **Normal, char **Insensitive,
			   ArgList Args, Cardinal ArgCount)
{
    Widget LabelWidget;

    LabelWidget = XmCreateLabel(Parent, Name, Args, ArgCount);
    if (LabelWidget == NULL)
	return NULL;
    CreatePixmapLabel(LabelWidget, Normal, Insensitive);
    return LabelWidget;
}

Widget XmCreatePixmapLabelGadget(Widget Parent, String Name,
				 char **Normal, char **Insensitive,
				 ArgList Args, Cardinal ArgCount)
{
    Widget LabelWidget;

    LabelWidget = XmCreateLabelGadget(Parent, Name, Args, ArgCount);
    if (LabelWidget == NULL)
	return NULL;
    CreatePixmapLabel(LabelWidget, Normal, Insensitive);
    return LabelWidget;
}
