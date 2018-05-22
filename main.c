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

#include <stdlib.h>
#include <ctype.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <Xm/MainW.h>
#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/SeparatoG.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>
#include <Xm/TextP.h>
#include <Xm/TextOutP.h>
#include <Xm/SelectioB.h>
#include <Xm/TextF.h>

#include <X11/xpm.h>

#include <stdio.h>

#include "pix.h"

#include "fops.h"
#include "parse.h"
#include "display.h"
#include "tip.h"
#include "utils.h"
#include "XpmButton.h"

/*
 * Icons icons icons....
 */
#include "pixmaps/icon.xpm"
#include "pixmaps/back_arm.xpm"
#include "pixmaps/back_dis.xpm"
#include "pixmaps/fwrd_arm.xpm"
#include "pixmaps/fwrd_dis.xpm"
#include "pixmaps/prev_arm.xpm"
#include "pixmaps/prev_dis.xpm"
#include "pixmaps/next_arm.xpm"
#include "pixmaps/next_dis.xpm"
#include "pixmaps/top_arm.xpm"
#include "pixmaps/top_dis.xpm"
#include "pixmaps/home.xpm"
#include "pixmaps/search.xpm"
#include "pixmaps/vertical.xpm"
#include "pixmaps/horizontal.xpm"
#include "pixmaps/dalecki.xpm"

/*
 * We are overriding this since otherwise the Text widget would attempt to
 * first move the invissible cursor instead of the scroll bars. That's jet
 * another example of the typical X11 of not providing any reasonable
 * default behaviour. 
 */
static char *text_translations = "#override \n\
<Key>osfUp: scroll-one-line-down()\n\
<Key>osfDown: scroll-one-line-up()\n";

/*
 * Local application specific resources.
 *
 * Here is where we acutally record where we are.
 */

app_data_t app_data;

static String fallback_resources[] =
{
    "*enableEtchedInMenu: True",
    "*enableDefaultButton: True",
    "*enableThinThickness: True",
    "*detailShadowThickness: 1",
    "*menuBar.*.shadowThickness: 1",
    "*XmPushButton.shadowThickness: 1",
    "*menuBar.*.XmSeparatorGadget.shadowThickness: 2",
    "*highlightThickness: 1",
    "*pane.separatorOn: True",
    "*background: #aeb2c3",
    "*fontList: -adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*",
    "*XmTextField.fontList: -adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "*bookmarkButton.fontList: -adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "*XmText.fontList: -b&h-*-medium-r-*-*-12-*-*-*-m-*-iso8859-1",
    "*XmText.scrollLeftSide: True",
    "*XmList*fontList:-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*-*",
    "*head1Font: -b&h-*-bold-r-*-*-16-*-*-*-m-*-iso8859-1",
    "*head2Font: -b&h-*-bold-r-*-*-14-*-*-*-m-*-iso8859-1",
    "*head3Font: -b&h-*-bold-r-*-*-14-*-*-*-m-*-iso8859-1",
    "*head4Font: -b&h-*-bold-r-*-*-12-*-*-*-m-*-iso8859-1",
    "*XmText.foreground: black",
    "*XmText.background: gainsboro",
    "*XmTextField.foreground: Black",
    "*XmTextField.background: gainsboro",
    "*XmList.foreground: Black",
    "*XmList.background: gainsboro",
    "*tipShell.background: lemon chiffon",
  "*tipShell.fontSet: -*-helvetica-medium-r-*-*-*-120-*-*-*-*-iso8859-*",
    "*buttonLine.?.shadowThickness: 1",
    "*buttonLine.?.highlightThickness: 0",
    "*buttonLine.?.highlightOnEnter: False",
    "*buttonLine.switchButton.marginWidth: 0",
    "*buttonLine.marginWidth: 0",
    "*buttonLine.marginHeight: 0",
    "*buttonLine.spacing: 0",
    "*buttonLine.entryBorder: 0",
    "*buttonLine.entryAlignment: ALIGNMENT_CENTER",
    "*buttonLine.resizeWidth: True",
    "*buttonLine.resizeHeight: True",
    "*locationLine,shadowThickness: 1",
    "*locationLine.?.shadowThickness: 1",
    "*locationLine.?.highlightThickness: 0",
    "*locationLine.?.highlightOnEnter: False",
    "*locationLine.switchButton.marginWidth: 0",
    "*locationLine.marginWidth: 0",
    "*locationLine.marginHeight: 0",
    "*locationLine.spacing: 0",
    "*locationLine.entryBorder: 0",
    "*locationLine.entryAlignment: ALIGNMENT_CENTER",
    "*locationLine.resizeWidth: True",
    "*locationLine.resizeHeight: True",
    "*locationLine.marginWidth: 0",
    "*locationLine.marginHeight: 2",
    "*locationLine.XmTextField.topOffset: 3",
    "*locationLine.XmTextField.bottomOffset: 3",
    "*locationLine.XmTextField.marginWidth: 2",
    "*locationLine.XmTextField.marginHeight: 2",
    "*locationLine.XmTextField.rightOffset: 4",
    "*locationLine.XmTextField*background: white",
    "*offLine.?.shadowThickness: 1",
    "*offLine.?.highlightThickness: 0",
    "*offLine.?.highlightOnEnter: False",
    "*offLine.?.marginLeft: 3",
    "*offLine.?.marginRight: 1",
    "*offLine.?.marginHeight: 0",
    "*offLine.spacing: 0",
    "*nodeText.highlightThickness: 0",
    "*nodeText.shadowThickness: 1",
    "*nodeText.borderWidth: 0",
    "*aboutDialog*shadowThickness: 1",
    "*aboutDialog.XmSeparatorGadget.shadowThickness: 2",
    "*aboutDialog.dialogTitle: About Minfo: ",
    "*helpDialog*shadowThickness: 1",
    "*helpDialog.XmSeparatorGadget.shadowThickness: 2",
    "*helpDialog.dialogTitle: Help for Minfo: ",
    "*searchDialog.XmPushButton.shadowThickness: 1",
    "*searchDialog.XmPushButtonGadget.shadowThickness: 1",
    "*searchDialog.XmSeparatorGadget.shadowThickness: 2",
    "*searchDialog.dialogTitle: Search:",
    "*searchDialog.findButton.labelString: Find",
    "*searchDialog.clearButton.labelString: Clear",
    "*searchDialog.closeButton.labelString: Close",
    "*searchDialog.*.findLabel.labelString: Find: ",
    "*searchDialog.*.caseToggle.labelString: Case Sensitive",
    "*searchDialog.*.findToggle.labelString: Find Backwards",
    "*menuDialog.dialogTitle: Select From Menu",
    "*menuDialog.*.shadowThickness: 1",
    "*menuDialog.XmSeparatorGadget.shadowThickness: 2",
    "*xrefDialog.dialogTitle: Select a Refference",
    "*xrefDialog.*.shadowThickness: 1",
    "*xrefDialog.XmSeparatorGadget.shadowThickness: 2",
    "*fileDialog.dialogTitle: Select a DIR Node File",
    "*fileDialog.*.shadowThickness: 1",
    "*fileDialog.XmSeparatorGadget.shadowThickness: 2",
    "*dirFailedDialog.dialogTitle: Error!",
    "*dirFailedDialog.*.shadowThickness: 1",
    "*dirFailedDialog.XmSeparatorGadget.shadowThickness: 2",
    "*searchFailedDialog.dialogTitle: Error in Search!",
    "*searchFailedDialog.*.shadowThickness: 1",
    "*searchFailedDialog.XmSeparatorGadget.shadowThickness: 2",
    "*highlightBackgroundPixel: lemon chiffon",
    "*highlightForegroundPixel: navy",
    "*visitedForegroundPixel: maroon",
    "*popupMenu.*.shadowThickness: 1",
    NULL
};

static XtResource new_resources[] =
{
    {"dirPath", "DirPath",
     XmRString, sizeof(String),
     XtOffsetOf(app_data_t, path), XmRString,
     (XtPointer) "/usr/info/:/usr/local/info/:/usr/tex/info/"},
    {"dirFile", "DirFile",
     XmRString, sizeof(String),
     XtOffsetOf(app_data_t, file), XmRString,
     (XtPointer) "dir"},
    {"topNode", "TopNode",
     XmRString, sizeof(String),
     XtOffsetOf(app_data_t, node), XmRString,
     (XtPointer) "Top"},
    {"highlightForegroundPixel", "HighlightForegroundPixel",
     XmRPixel, sizeof(Pixel),
     XtOffsetOf(app_data_t, hfgpixel), XmRString,
     (XtPointer) "white"},
    {"highlightBackgroundPixel", "HighlightBackgroundPixel",
     XmRPixel, sizeof(Pixel),
     XtOffsetOf(app_data_t, hbgpixel), XmRString,
     (XtPointer) "black"},
    {"visitedForegroundPixel", "VisitedForegroundPixel",
     XmRPixel, sizeof(Pixel),
     XtOffsetOf(app_data_t, vfgpixel), XmRString,
     (XtPointer) "white"},
    {"head1Font", "Head1Font", XtRFontStruct,
     sizeof(XFontStruct *), XtOffsetOf(app_data_t, h1_font), XtRString,
     (XtPointer) "fixed"
    },
    {"head2Font", "Head2Font", XtRFontStruct,
     sizeof(XFontStruct *), XtOffsetOf(app_data_t, h2_font), XtRString,
     (XtPointer) "fixed"
    },
    {"head3Font", "Head3Font", XtRFontStruct,
     sizeof(XFontStruct *), XtOffsetOf(app_data_t, h3_font), XtRString,
     (XtPointer) "fixed"
    },
    {"head4Font", "Head4Font", XtRFontStruct,
     sizeof(XFontStruct *), XtOffsetOf(app_data_t, h4_font), XtRString,
     (XtPointer) "fixed"
    },
    {NULL, NULL},
};

static XrmOptionDescRec options[] =
{
    {"-file", ".dirFile", XrmoptionSepArg, NULL},
    {"-path", ".dirPath", XrmoptionSepArg, NULL},
};

/*
 * Public widgets we reffer to in callbacks. They should be removed
 * after adding the multi-instace facilities.
 */
Widget top_level;
static Widget tip_shell;
static Widget tools_on;
static Widget tools_off;
static Widget tools_on_tip;
static Widget tools_off_tip;
static Widget location_on;
static Widget location_off;
static Widget location_on_tip;
static Widget location_off_tip;
static Widget separator;
static Widget popup_menu;

static void home_pushed(Widget w,
			XtPointer client_data, XtPointer call_data)
{
    struct page *page = lookup_node_in_file("Top", "dir");
    if (page)
	display_page(page);
    else
	show_error_dialog("Info directory file not found!");
}

static void next_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    struct page *page;

    page = NULL;

    if (current_page)
	page = lookup_node_in_file(current_page->Next, current_page->file);
    if (page)
	display_page(page);
    else
	show_error_dialog("Next node not found!");
}

static void prev_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    struct page *page;

    page = NULL;

    if (current_page)
	page = lookup_node_in_file(current_page->Prev, current_page->file);
    if (page)
	display_page(page);
    else
	show_error_dialog("Preview node not found!");
}

static void top_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    struct page *page;

    page = NULL;

    if (current_page)
	page = lookup_node_in_file(current_page->Up, current_page->file);
    if (page)
	display_page(page);
    else
	show_error_dialog("Up node not found!");
}

static void back_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    struct page_stack *tmp;
    struct page_stack *fwrd;

    if (!page_stack)
	return;

    if (!page_stack->prev)
	XtSetSensitive(back_button, False);
    if (current_page) {
	fwrd = (struct page_stack *) malloc(sizeof(struct page_stack));
	fwrd->prev = fwrd_stack;
	fwrd->page = current_page;
	fwrd_stack = fwrd;
    }
    current_page = NULL;
    display_page(page_stack->page);
    tmp = page_stack;
    page_stack = page_stack->prev;
    free(tmp);
    XtSetSensitive(fwrd_button, True);
}

static void fwrd_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    struct page_stack *tmp;

    if (!fwrd_stack)
	return;
    if (!fwrd_stack->prev)
	XtSetSensitive(fwrd_button, False);
    current_page = NULL;
    display_page(fwrd_stack->page);
    tmp = fwrd_stack;
    fwrd_stack = fwrd_stack->prev;
    free(tmp);
}

/*
 * Return the page pointer to indicate sccess and NULL otherwise.
 */
static struct page *jump_to_xref(struct menu *xref)
{
    char *node;
    char *file;
    struct page *page;

    page = NULL;
    node = file = NULL;

    if (*xref->node == '(') {	/* File indirection */
	file = strdup(xref->node + 1);

	if ((node = strchr(file, ')'))) {
	    *(node++) = '\0';
	}
	if (!node || *node == '\0')
	    node = "Top";
    } else {
	node = xref->node;
	file = strdup(current_page->file);
    }

    page = lookup_node_in_file(node, file);
    free(file);

    if (page)
	xref->visited = True;
    return page;
}

/*
 * Try to jump to the xref, with error reporting.
 * 
 * return 1 on error and 0 otherwise.
 */
static int change_to_xref(struct menu *xref, char *errmsg)
{
    struct page *page;

    if (!xref)
	return 1;

    if ((page = jump_to_xref(xref)))
	display_page(page);
    else {
	show_error_dialog(errmsg);
	return 1;
    }

    return 0;
}

static void load_menu_by_name(Widget dialog, XtPointer client_data, XtPointer call_data)
{
    char *menu_item = NULL;
    XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;

    if (cbs) {
	struct menu *xref;

	XmStringGetLtoR(cbs->value, XmFONTLIST_DEFAULT_TAG, &menu_item);
	xref = current_page->menu;
	while (xref && strcmp(xref->name, menu_item))
	    xref = xref->next;
	XtFree(menu_item);
	if (change_to_xref(xref, "Menu item not found!"))
	    XtPopdown(XtParent(dialog));
    }
}

static void load_menu_by_node(Widget dialog, XtPointer client_data, XtPointer call_data)
{
    char *menu_node = NULL;
    XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;

    if (cbs) {
	struct menu *xref;

	XmStringGetLtoR(cbs->value, XmFONTLIST_DEFAULT_TAG, &menu_node);
	xref = current_page->menu;
	while (xref && strcmp(xref->node, menu_node))
	    xref = xref->next;
	XtFree(menu_node);
	if (change_to_xref(xref, "Menu node not found!"))
	    XtPopdown(XtParent(dialog));
    }
}

static void go_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
    int item_no = (int) client_data;

    switch (item_no) {
    case 0:
	next_pushed(widget, client_data, call_data);
	break;
    case 1:
	prev_pushed(widget, client_data, call_data);
	break;
    case 2:
	top_pushed(widget, client_data, call_data);
	break;
    case 5:
	home_pushed(widget, client_data, call_data);
	break;
    }
}

static void go_menu_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
    int item_no = (int) client_data;

    if (item_no == 0 || item_no == 1) {		/* Go to menu item/node */
	Widget dialog;
	int n;
	int i;
	XmString *str;
	XmString t;

	if (current_page) {
	    struct menu *menu = current_page->menu;
	    if (menu) {
		n = 0;
		while (menu) {
		    menu = menu->next;
		    n++;
		}

		str = (XmString *) XtMalloc(n * sizeof(XmString));
		if (item_no == 0)
		    t = XmStringCreateLocalized("Please select an menu item:");
		else
		    t = XmStringCreateLocalized("Please select an menu node:");
		menu = current_page->menu;
		for (i = 0; i < n; i++) {
		    if (item_no == 0)
			str[i] = XmStringCreateLocalized(menu->name);
		    else
			str[i] = XmStringCreateLocalized(menu->node);
		    menu = menu->next;
		}

		dialog = (Widget) XmCreateSelectionDialog(top_level,
						  "menuDialog", NULL, 0);
		XtUnmanageChild((void *) XmSelectionBoxGetChild(dialog,
						  XmDIALOG_HELP_BUTTON));
		XtUnmanageChild((void *) XmSelectionBoxGetChild(dialog,
						 XmDIALOG_APPLY_BUTTON));
		XtVaSetValues(dialog,
			      XmNlistLabelString, t,
			      XmNlistItems, str,
			      XmNlistItemCount, n,
			      XmNmustMatch, True,
			      NULL);
		if (item_no == 0)
		    XtAddCallback(dialog, XmNokCallback,
				  load_menu_by_name, NULL);
		else
		    XtAddCallback(dialog, XmNokCallback,
				  load_menu_by_node, NULL);

		XtAddCallback(dialog, XmNcancelCallback,
			      (void *) XtDestroyWidget, NULL);
		XmStringFree(t);
		while (--i >= 0)
		    XmStringFree(str[i]);	/* free elements of array */
		XtFree((void *) str);	/* now free array pointer */

		XtManageChild(dialog);
		XtPopup(XtParent(dialog), XtGrabNone);
	    }
	} else
	    show_error_dialog("There isn't any node currently loaded!");
    }
}


static void load_xref_by_name(Widget dialog,
			      XtPointer client_data, XtPointer call_data)
{
    char *xref_item = NULL;
    XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;

    if (cbs) {
	struct menu *xref;

	XmStringGetLtoR(cbs->value, XmFONTLIST_DEFAULT_TAG, &xref_item);
	xref = current_page->note;
	while (xref && strcmp(xref->name, xref_item))
	    xref = xref->next;
	XtFree(xref_item);
	if (change_to_xref(xref, "Cross refference not found!"))
	    XtPopdown(XtParent(dialog));
    }
}


static void load_xref_by_node(Widget dialog, XtPointer client_data, XtPointer call_data)
{
    char *xref_node = NULL;
    XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;

    if (cbs) {
	struct menu *xref;

	XmStringGetLtoR(cbs->value, XmFONTLIST_DEFAULT_TAG, &xref_node);
	xref = current_page->note;
	while (xref && strcmp(xref->node, xref_node))
	    xref = xref->next;
	XtFree(xref_node);
	if (change_to_xref(xref, "Cress refference node not found!"))
	    XtPopdown(XtParent(dialog));
    }
}

static void go_xref_cb(Widget widget,
		       XtPointer client_data, XtPointer call_data)
{
    int item_no = (int) client_data;

    if (item_no == 0 || item_no == 1) {		/* Go to menu item/node */
	Widget dialog;
	int n;
	int i;
	XmString *str;
	XmString t;

	if (current_page) {
	    struct menu *xref = current_page->note;

	    if (xref) {
		n = 0;
		while (xref) {
		    if (!xref->hide)
			++n;
		    xref = xref->next;

		}

		str = (XmString *) XtMalloc(n * sizeof(XmString));
		if (item_no == 0)
		    t = XmStringCreateLocalized("Please select a refference:");
		else
		    t = XmStringCreateLocalized(
				       "Please select a reffered node:");
		xref = current_page->note;
		i = 0;
		while (xref) {
		    if (!xref->hide) {
			if (item_no == 0)
			    str[i] = XmStringCreateLocalized(xref->name);
			else
			    str[i] = XmStringCreateLocalized(xref->node);
			++i;
		    }
		    xref = xref->next;
		}

		dialog = (Widget) XmCreateSelectionDialog(top_level,
						  "xrefDialog", NULL, 0);
		XtUnmanageChild((void *) XmSelectionBoxGetChild(dialog,
						  XmDIALOG_HELP_BUTTON));
		XtUnmanageChild((void *) XmSelectionBoxGetChild(dialog,
						 XmDIALOG_APPLY_BUTTON));
		XtVaSetValues(dialog,
			      XmNlistLabelString, t,
			      XmNlistItems, str,
			      XmNlistItemCount, n,
			      XmNmustMatch, True,
			      NULL);
		if (item_no == 0)
		    XtAddCallback(dialog, XmNokCallback,
				  load_xref_by_name, NULL);
		else
		    XtAddCallback(dialog, XmNokCallback,
				  load_xref_by_node, NULL);

		XtAddCallback(dialog, XmNcancelCallback,
			      (void *) XtDestroyWidget, NULL);
		XmStringFree(t);
		while (--i >= 0)
		    XmStringFree(str[i]);	/* free elements of array */
		XtFree((void *) str);	/* now free array pointer */

		XtManageChild(dialog);
		XtPopup(XtParent(dialog), XtGrabNone);
	    }
	} else
	    show_error_dialog("There isn't any node currently loaded!");
    }
}

#define MSG \
"Use the File menu entry to open different info directories\n\
Use the Go menu entry to navigate through the info files.\n\
Click on menu entries and notes to get through them.\n\
The two leftmost buttons are used to handle the history list\n\
of allready traversed Nodes. Hey if You know how to click on\n\
Buttons, You should know too how to handle this program.\n\n\
There are two command line options:\n\n\
    -file FILENAME  will loade file as Top node file\n\
    -path PATH      will override the default search path for files"

#define ABOUT \
"This program was kindly written by:\n\n"\
"© Copyright 1996,1997 Marcin Dalecki\n\n"\
"in the hope that it may be usefull for anybody who\n"\
"feels that the emacs-info interface is an anachronism\n"\
"at the end of the 90's!\n\n"\
"You can reach me currently at the following email\n"\
"address:\n\n"\
"<dalecki@Math.Uni-Goettingen.de>\n\n"\
"The program version is 1.7.18."


/* 
 * The help button in the help menu from the menubar was selected.
 * Display help information defined above for how to use the program.
 * This is done by creating a Motif information dialog box.  Again,
 * make the dialog static so we can reuse it.
 */
static void help_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
    Widget dialog;
    int item_no = (int) client_data;
    Arg args[3];
    int i;
    XmString msg;
    Display *dsp;
    Window win;
    static Pixmap image;
    static int once = 1;

    dsp = XtDisplay(top_level);
    win = XRootWindowOfScreen(XtScreen(top_level));
    if (once) {
	XpmCreatePixmapFromData(dsp, win, dalecki_xpm, &image, NULL, NULL);
	once = 0;
    }
    if (item_no != 0)
	msg = XmStringCreateLtoR(MSG, XmFONTLIST_DEFAULT_TAG);
    else
	msg = XmStringCreateLtoR(ABOUT, XmFONTLIST_DEFAULT_TAG);

    i = 0;
    XtSetArg(args[i], XmNmessageString, msg);
    i++;
    if (item_no == 0) {
	XtSetArg(args[i], XmNmessageAlignment, XmALIGNMENT_CENTER);
	i++;
	XtSetArg(args[i], XmNsymbolPixmap, image);
	i++;
    }
    if (item_no == 0)
	dialog = (Widget) XmCreateInformationDialog(top_level,
						 "aboutDialog", args, i);
    else
	dialog = (Widget) XmCreateInformationDialog(top_level,
						  "helpDialog", args, i);
    XtUnmanageChild((void *) XmMessageBoxGetChild(dialog,
						  XmDIALOG_HELP_BUTTON));
    XtUnmanageChild((void *) XmMessageBoxGetChild(dialog,
						XmDIALOG_CANCEL_BUTTON));
    XtManageChild(dialog);
    XtPopup(XtParent(dialog), XtGrabNone);
}

#undef MSG
#undef ABOUT


static void got_pointer(Widget w, XtPointer client_data,
			XEvent * ev, Boolean * cont)
{
    Position x, y;
    XmTextPosition posl;
    struct menu *xref;

    if (!current_page)		/* nowhere to search in */
	return;

    x = ev->xbutton.x;
    y = ev->xbutton.y;

    posl = XmTextXYToPos(w, x, y);

    if ((xref = check_in_xref(current_page->menu, posl, posl)))
	change_to_xref(xref, "Menu item not found!");
    else if ((xref = check_in_xref(current_page->note, posl, posl)))
	change_to_xref(xref, "Note not found!");
}

static void event_callback_dispatch(Widget w, XtPointer client_data,
				    XEvent * ev, Boolean * cont)
{
    if (ev->type == ButtonPress && ((XButtonPressedEvent *) ev)->button == 1)
	got_pointer(w, client_data, ev, cont);
}

static void turn_off_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Boolean first_time = True;

    XtUnmanageChild(tools_on);
    XtManageChild(tools_off);
    if (first_time) {
	TipAddWidget(tip_shell, tools_off_tip, "Navigation Toolbar", 0, 0);
	first_time = False;
    }
    XtVaSetValues(location_on, XmNtopWidget, tools_off, NULL);
    XtVaSetValues(location_off, XmNtopWidget, tools_off, NULL);
    if (XtIsManaged(location_off)) {
	XtVaSetValues(location_off,
		      XmNtopAttachment, XmATTACH_FORM,
		      XmNleftAttachment, XmATTACH_WIDGET,
		      XmNleftWidget, tools_off,
		      NULL);
    }
}

static void turn_on_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    XtUnmanageChild(tools_off);
    XtManageChild(tools_on);
    XtVaSetValues(location_on, XmNtopWidget, tools_on, NULL);
    XtVaSetValues(location_off, XmNtopWidget, tools_on, NULL);
    if (XtIsManaged(location_off)) {
	XtVaSetValues(location_off,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopWidget, tools_on,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      NULL);
    }
}

static void location_off_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Boolean first_time = True;

    XtUnmanageChild(location_on);
    XtManageChild(location_off);
    if (first_time) {
	TipAddWidget(tip_shell, location_off_tip, "Location Toolbar", 0, 0);
	first_time = False;
    }
    XtVaSetValues(separator, XmNtopWidget, location_off, NULL);
    if (XtIsManaged(tools_on)) {
	XtVaSetValues(location_off,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopWidget, tools_on,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      NULL);
    } else {
	XtVaSetValues(location_off,
		      XmNtopAttachment, XmATTACH_FORM,
		      XmNleftAttachment, XmATTACH_WIDGET,
		      XmNleftWidget, tools_off,
		      NULL);
    }
}

static void location_on_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    XtUnmanageChild(location_off);
    XtManageChild(location_on);
    XtVaSetValues(separator, XmNtopWidget, location_on, NULL);
}


void popup_cb(Widget w, XtPointer client_data,
	      XButtonPressedEvent * event,
	      Boolean * continue_to_dispatch_return)
{
    Widget popup = (Widget) client_data;

    if (event->button == 3) {
	XmMenuPosition(popup, event);
	XtManageChild(popup);
    }
}

static void popup_menu_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int num = (int) client_data;

    switch (num) {
    case 0:			/*      Next     */
	next_pushed(w, client_data, call_data);
	break;
    case 1:			/*      Prev    */
	prev_pushed(w, client_data, call_data);
	break;
    case 2:			/*      Top   */
	top_pushed(w, client_data, call_data);
	break;
    }
}


/*************************************************************************
 *  Main function starts here
 *************************************************************************/

int main(int argc, char **argv)
{
    XtAppContext app;
    Cardinal i;
    Arg al[20];
    Widget widget;
    Widget main_window;
    Widget menu_bar;
    Widget form;
    Widget scroll_area;
    Widget menu_pullright;
    XmString str[32];
    XmString help;
    struct page *first_page;
    XtSetLanguageProc(NULL, NULL, NULL);
    top_level = XtAppInitialize(&app, "MInfo",
				options, XtNumber(options),
				&argc, argv, fallback_resources,
				NULL, 0);

    XtGetApplicationResources(top_level, (XtPointer) & app_data, new_resources,
			      XtNumber(new_resources), NULL, 0);

    main_window = XtVaCreateWidget("mainWindow",
				   xmMainWindowWidgetClass, top_level,
				   NULL);

    tip_shell = XtVaCreatePopupShell("tipShell", tipWidgetClass, main_window,
				     NULL);
    setup_icon(top_level, icon_xpm);

    i = create_XmString_list(str, "File\nGo\nBookmarks");
    help = XmStringCreateLocalized("Help");
    menu_bar = XmVaCreateSimpleMenuBar(main_window, "menuBar",
				       XmVaCASCADEBUTTON, str[0], NULL,
				       XmVaCASCADEBUTTON, str[1], NULL,
				       XmVaCASCADEBUTTON, str[2], NULL,
				       XmVaCASCADEBUTTON, help, NULL,
				       NULL);
    free_XmString_list(str, i);

    /* Tell the menubar which button is the help menu  */
    if ((widget = XtNameToWidget(menu_bar, "button_3")))
	XtVaSetValues(menu_bar, XmNmenuHelpWidget, widget, NULL);

    /* First menu is the File menu -- callback is file_cb()  */
    i = create_XmString_list(str, "Open File...\nAlt+O\nExit\nAlt+Q");
    XmVaCreateSimplePulldownMenu(menu_bar, "fileMenu", 0, file_cb,
		  XmVaPUSHBUTTON, str[0], 'O', "Alt ~Ctrl<Key>o", str[1],
				 XmVaSEPARATOR,
		  XmVaPUSHBUTTON, str[2], 'x', "Alt ~Ctrl<Key>q", str[3],
				 NULL);
    free_XmString_list(str, i);

    /* Second menu is the Go menu -- callback is go_cb() */
    i = create_XmString_list(str,
			     "Next\n"
			     "Alt+Right\n"
			     "Preview\n"
			     "Alt+Left\n"
			     "Top\n"
			     "Alt+Up\n"
			     "Menu by...\n"
			     "Cross Refference by...\n"
			     "Info Directory Node");
    widget = XmVaCreateSimplePulldownMenu(menu_bar, "goMenu", 1, go_cb,
	  XmVaPUSHBUTTON, str[0], NULL, "Alt ~Ctrl<Key>osfRight", str[1],
	   XmVaPUSHBUTTON, str[2], NULL, "Alt ~Ctrl<Key>osfLeft", str[3],
	     XmVaPUSHBUTTON, str[4], NULL, "Alt ~Ctrl<Key>osfUp", str[5],
					  XmVaSEPARATOR,
					  XmVaCASCADEBUTTON, str[6], 'M',
					  XmVaCASCADEBUTTON, str[7], 'R',
					  XmVaSEPARATOR,
				XmVaPUSHBUTTON, str[8], NULL, NULL, NULL,
					  NULL);
    free_XmString_list(str, i);

    next_menu_button = XtNameToWidget(widget, "button_0");
    prev_menu_button = XtNameToWidget(widget, "button_1");
    top_menu_button = XtNameToWidget(widget, "button_2");
    menu_menu_pullright = XtNameToWidget(widget, "button_3");
    xref_menu_pullright = XtNameToWidget(widget, "button_4");


    i = create_XmString_list(str, "Item\nNode");
    menu_pullright = XmVaCreateSimplePulldownMenu(widget,
		      "pullRight", 4 /* menu item offset */ , go_menu_cb,
				 XmVaPUSHBUTTON, str[0], 'I', NULL, NULL,
				 XmVaPUSHBUTTON, str[1], 'N', NULL, NULL,
						  NULL);

    menu_item_button = XtNameToWidget(menu_pullright, "button_0");
    menu_node_button = XtNameToWidget(menu_pullright, "button_1");

    menu_pullright = XmVaCreateSimplePulldownMenu(widget,
		      "pullRight", 5 /* menu item offset */ , go_xref_cb,
				 XmVaPUSHBUTTON, str[0], 'I', NULL, NULL,
				 XmVaPUSHBUTTON, str[1], 'N', NULL, NULL,
						  NULL);
    xref_item_button = XtNameToWidget(menu_pullright, "button_0");
    xref_node_button = XtNameToWidget(menu_pullright, "button_1");

    free_XmString_list(str, i);
    XtManageChild(widget);

    XtSetSensitive(next_menu_button, False);
    XtSetSensitive(prev_menu_button, False);
    XtSetSensitive(top_menu_button, False);
    XtSetSensitive(menu_menu_pullright, False);
    XtSetSensitive(menu_item_button, False);
    XtSetSensitive(menu_node_button, False);
    XtSetSensitive(xref_menu_pullright, False);
    XtSetSensitive(xref_item_button, False);
    XtSetSensitive(xref_node_button, False);

    /* Third menu is the bookmark menu -- callback is bookmark_cb() */
    i = create_XmString_list(str, "Add Bookmark\n"
			     "Alt+A\n"
			     "Delete Bookmark\n"
			     "Alt+D\n"
			     "Bookmarks:");
    bookmark_menu = XmVaCreateSimplePulldownMenu(menu_bar,
						 "fileMenu", 2,
						 book_cb,
		  XmVaPUSHBUTTON, str[0], 'A', "Alt ~Ctrl<Key>x", str[1],
		  XmVaPUSHBUTTON, str[2], 'D', "Alt ~Ctrl<Key>x", str[3],
						 XmVaSEPARATOR,
						 XmVaTITLE, str[4],
						 NULL);
    free_XmString_list(str, i);

    widget = XtNameToWidget(bookmark_menu, "button_0");
    XtVaSetValues(widget, XmNuserData, (void *) 0, 0);
    widget = XtNameToWidget(bookmark_menu, "button_1");
    XtVaSetValues(widget, bookmark_menu, (void *) 0, 0);
    widget = XtNameToWidget(bookmark_menu, "separator_0");
    XtVaSetValues(widget, XmNuserData, (void *) 0, 0);
    widget = XtNameToWidget(bookmark_menu, "label_0");
    XtVaSetValues(widget, XmNuserData, (void *) 0, 0);
    /* Fourth menu is the help menu -- callback is help_cb() */
    str[0] = XmStringCreateLocalized("About Minfo");
    XmVaCreateSimplePulldownMenu(menu_bar, "help_menu", 3, help_cb,
				 XmVaPUSHBUTTON, str[0], 'A', NULL, NULL,
				 XmVaSEPARATOR,
				 XmVaPUSHBUTTON, help, 'H', NULL, NULL,
				 NULL);
    XmStringFree(str[0]);
    XmStringFree(help);		/* we're done with it; now we can free it */

    XtManageChild(menu_bar);

    form = XtVaCreateWidget("toolsForm", xmFormWidgetClass, main_window,
			    XmNorientation, XmVERTICAL,
			    NULL);

    tools_on = XtVaCreateWidget("buttonLine", xmRowColumnWidgetClass, form,
				XmNorientation, XmHORIZONTAL,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				NULL);
    tools_off = XtVaCreateWidget("offLine", xmFormWidgetClass, form, NULL);

    XtSetArg(al[0], XmNtraversalOn, False);	/* shared !! */
    tools_on_tip = XmCreatePixmapPushButton(tools_on, "switchButton",
					    vertical_xpm,
					    vertical_xpm,
					    NULL,
					    al, 1);
    XtAddCallback(tools_on_tip, XmNactivateCallback, turn_off_cb, NULL);
    XtManageChild(tools_on_tip);
    tools_off_tip = XmCreatePixmapPushButton(tools_off, "switchButton",
					     horizontal_xpm,
					     horizontal_xpm,
					     NULL,
					     al, 1);
    XtAddCallback(tools_off_tip, XmNactivateCallback, turn_on_cb, NULL);
    XtManageChild(tools_off_tip);

    /* Move backward Button */
    back_button = XmCreatePixmapPushButton(tools_on, "backButton",
					   back_arm_xpm,
					   back_arm_xpm,
					   back_dis_xpm,
					   al, 1);
    XtSetSensitive(back_button, False);
    XtAddCallback(back_button, XmNactivateCallback, back_pushed, NULL);
    XtManageChild(back_button);

    /* Move forward Button */
    fwrd_button = XmCreatePixmapPushButton(tools_on, "forwardButton",
					   fwrd_arm_xpm,
					   fwrd_arm_xpm,
					   fwrd_dis_xpm,
					   al, 1);
    XtSetSensitive(fwrd_button, False);
    XtAddCallback(fwrd_button, XmNactivateCallback, fwrd_pushed, NULL);
    XtManageChild(fwrd_button);

    /* Preview Node Button */
    prev_button = XmCreatePixmapPushButton(tools_on, "prevButton",
					   prev_arm_xpm,
					   prev_arm_xpm,
					   prev_dis_xpm,
					   al, 1);
    XtSetSensitive(prev_button, False);
    XtAddCallback(prev_button, XmNactivateCallback, prev_pushed, NULL);
    XtManageChild(prev_button);

    /* Top Node Button  */
    top_button = XmCreatePixmapPushButton(tools_on, "topButton",
					  top_arm_xpm,
					  top_arm_xpm,
					  top_dis_xpm,
					  al, 1);
    XtSetSensitive(top_button, False);
    XtAddCallback(top_button, XmNactivateCallback, top_pushed, NULL);
    XtManageChild(top_button);

    /* Next Node Button */
    next_button = XmCreatePixmapPushButton(tools_on, "nextButton",
					   next_arm_xpm,
					   next_arm_xpm,
					   next_dis_xpm,
					   al, 1);
    XtSetSensitive(next_button, False);
    XtAddCallback(next_button, XmNactivateCallback, next_pushed, NULL);
    XtManageChild(next_button);

    /* Home Button */
    home_button = XmCreatePixmapPushButton(tools_on, "homeButton",
					   home_xpm,
					   home_xpm,
					   NULL,
					   al, 1);
    XtAddCallback(home_button, XmNactivateCallback, home_pushed, NULL);
    XtManageChild(home_button);

    /* Search Button */
    search_button = XmCreatePixmapPushButton(tools_on, "searchButton",
					     search_xpm,
					     search_xpm,
					     NULL,
					     al, 1);
    XtAddCallback(search_button, XmNactivateCallback, search_pushed, NULL);
    XtManageChild(search_button);

    XtManageChild(tools_on);
    location_on = XtVaCreateWidget("locationLine", xmFormWidgetClass, form,
				   XmNtraversalOn, False,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, tools_on,
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNfractionBase, 1,
				   NULL);

    location_off = XtVaCreateWidget("offLine", xmFormWidgetClass, form,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget, tools_on,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    NULL);

    XtSetArg(al[0], XmNtraversalOn, False);	/* shared !! */
    location_off_tip = XmCreatePixmapPushButton(location_off, "switchButton",
						horizontal_xpm,
						horizontal_xpm,
						NULL,
						al, 1);
    XtAddCallback(location_off_tip, XmNactivateCallback, location_on_cb, NULL);
    XtManageChild(location_off_tip);
    location_on_tip = XmCreatePixmapPushButton(location_on, "switchButton",
					       vertical_xpm,
					       vertical_xpm,
					       NULL,
					       al, 1);
    XtAddCallback(location_on_tip, XmNactivateCallback, location_off_cb, NULL);
    XtManageChild(location_on_tip);
    widget = XtVaCreateManagedWidget("Location: ", xmLabelWidgetClass, location_on,
				     XmNtraversalOn, False,
				     XmNtopAttachment, XmATTACH_POSITION,
				     XmNtopPosition, 0,
				     XmNtopOffset, 0,
				  XmNbottomAttachment, XmATTACH_POSITION,
				     XmNbottomPosition, 1,
				     XmNleftAttachment, XmATTACH_WIDGET,
				     XmNleftWidget, location_on_tip,
				     NULL);

    location_text = XtVaCreateManagedWidget("locationText",
				     xmTextFieldWidgetClass, location_on,
					    XmNeditable, False,
					    XmNtraversalOn, False,
					    XmNresizeWidth, True,
					 XmNcursorPositionVisible, False,
				     XmNtopAttachment, XmATTACH_POSITION,
					    XmNtopPosition, 0,
				      XmNleftAttachment, XmATTACH_WIDGET,
					    XmNleftWidget, widget,
				       XmNrightAttachment, XmATTACH_FORM,
				      XmNbottomAttachment, XmATTACH_FORM,
					    XmNbottomPosition, 1,
					    NULL);
    XmTextFieldSetString(location_text, "No Info File Loaded");

    XtManageChild(location_on);

    separator = XtVaCreateManagedWidget("separator",
					xmSeparatorGadgetClass, form,
				    XmNseparatorType, XmSHADOW_ETCHED_IN,
					XmNtopAttachment, XmATTACH_WIDGET,
					XmNtopWidget, location_on,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL);


    /*
     * Create the text area for displaying node contents.
     */
    scroll_area =
	XtVaCreateManagedWidget("scrollArea",
				xmScrolledWindowWidgetClass, form,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, separator,
				XmNtopOffset, 2,
				XmNleftAttachment, XmATTACH_FORM,
				XmNleftOffset, 4,
				XmNrightAttachment, XmATTACH_FORM,
				XmNrightOffset, 4,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNbottomOffset, 4,
				XmNspacing, 2,
				NULL);

    node_text = XtVaCreateWidget("nodeText",
				 xmTextWidgetClass, scroll_area,
				 XmNrows, 40,
				 XmNcolumns, 80,
				 XmNeditable, False,
				 XmNeditMode, XmMULTI_LINE_EDIT,
				 XmNscrollHorizontal, False,
				 XmNwordWrap, True,
				 XmNcursorPositionVisible, False,
				 XmNautoShowCursorPosition, True,
				 NULL);

    XmAddTabGroup(scroll_area);
    XtOverrideTranslations(node_text,
			   XtParseTranslationTable(text_translations));
    XtManageChild(node_text);

    XtVaSetValues(node_text, XmNshadowThickness, 0, NULL);

    /*
     * Setup a different drawing routine, we will be using instead of the
     * default. This is the hack, which makes it possible to implement
     * syntax highlighting in a reasonable manner, without using whole
     * special widgets.
     */
    ((XmTextWidget) node_text)->text.output->Draw = do_draw;

    XtManageChild(main_window);
    XtManageChild(form);

    /*
     * Make the collors of the scroll bar look less ugly!
     * Sample them from an push-button.
     */
    XtVaGetValues(scroll_area, XmNverticalScrollBar, &widget, NULL);
    if (widget) {
	XtVaGetValues(home_button, XmNbackground, &background_pixel,
		      XmNtopShadowColor, &top_pixel,
		      XmNbottomShadowColor, &bottom_pixel,
		      XmNarmColor, &trough_pixel,
		      NULL);
	XtVaSetValues(widget,
		      XmNbackground, background_pixel,
		      XmNtopShadowColor, top_pixel,
		      XmNbottomShadowColor, bottom_pixel,
		      XmNtroughColor, trough_pixel,
		      NULL);
    }
    /*
     * Finally realize the widget and startup...
     */

    XtRealizeWidget(top_level);
    i = create_XmString_list(str, "Next\nPreview\nTop");
    popup_menu =
	XmVaCreateSimplePopupMenu(node_text,
				  "popupMenu", popup_menu_cb,
				XmVaPUSHBUTTON, str[0], NULL, NULL, NULL,
				XmVaPUSHBUTTON, str[1], NULL, NULL, NULL,
				XmVaPUSHBUTTON, str[2], NULL, NULL, NULL,
				  XmNpopupEnabled, True,
				  NULL);
    free_XmString_list(str, i);
    popup_next = XtNameToWidget(popup_menu, "button_0");
    popup_prev = XtNameToWidget(popup_menu, "button_1");
    popup_top = XtNameToWidget(popup_menu, "button_2");
    XtSetSensitive(popup_next, False);
    XtSetSensitive(popup_prev, False);
    XtSetSensitive(popup_top, False);

    XtAddEventHandler(node_text, ButtonPressMask, False,
		      (XtPointer) popup_cb, popup_menu);
    XtAddEventHandler(node_text, ButtonPressMask,
		      False, event_callback_dispatch, NULL);

    /* 
     * Communicate the final file path to whoever needs it in file_ops
     * globally! Make shure it was really allocated.
     * And make sure, that we are able to open files from the
     * command line.
     */
    file_path = (char *) malloc(strlen(app_data.path) + 16);
    strcpy(file_path, ":");
    strcat(file_path, app_data.path);
    page_stack = NULL;
    fwrd_stack = NULL;

    first_page = lookup_node_in_file("Top", app_data.file);
    if (first_page)
	display_page(first_page);

    TipAddWidget(tip_shell, back_button, "Backward", 0, 0);
    TipAddWidget(tip_shell, fwrd_button, "Forward", 0, 0);
    TipAddWidget(tip_shell, prev_button, "Preview node", 0, 0);
    TipAddWidget(tip_shell, top_button, "Top node", 0, 0);
    TipAddWidget(tip_shell, next_button, "Next node", 0, 0);
    TipAddWidget(tip_shell, home_button, "Top directory node", 0, 0);
    TipAddWidget(tip_shell, search_button, "Search in current node", 0, 0);

    TipAddWidget(tip_shell, tools_on_tip, "Navigation Toolbar", 0, 0);
    TipAddWidget(tip_shell, location_on_tip, "Location Toolbar", 0, 0);

    /* Spin until we get sick */
    TipAppMainLoop(app);

    return 0;			/* hope not */
}
