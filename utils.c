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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <Xm/MessageB.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/TextP.h>

#include "fops.h"
#include "display.h"
#include "utils.h"

/*
 * Create an array STR of simple Motif strings. The DATA is an '\n' separated
 * list of strings. There will hardly be ever the need for strings longer then
 * 512 characters. Returning the number of allocated string.
 */

int create_XmString_list(XmString * str, char *const data)
{
    char *buf;
    char *chunk;
    int i = 0;
    int end;

    chunk = buf = strdup(data);

    do {
	char *chunk_end = strchr(chunk, '\n');
	if (!chunk_end)
	    end = 1;
	else {
	    *chunk_end = '\0';
	    end = 0;
	}
	str[i] = XmStringCreateLocalized(chunk);
	++i;
	chunk = strchr(chunk, '\0') + 1;
    } while (!end);

    free(buf);

    return i;
}

/*
 * Free N Motif strings in LIST.
 */
void free_XmString_list(XmString * list, int n)
{
    int i;
    for (i = 0; i < n; ++i)
	XmStringFree(list[i]);
}

void show_error_dialog(char *message)
{
    Widget dialog;
    Arg args[5];

    XmString msg;

    msg = XmStringCreateSimple(message);
    XtSetArg(args[0], XmNmessageString, msg);
    XtSetArg(args[1], XmNautoUnmanage, True);
    XtSetArg(args[2], XmNnoResize, True);
    XtSetArg(args[3], XmNresizePolicy, XmRESIZE_NONE);
    dialog = (Widget) XmCreateErrorDialog(top_level,
					  "dirFailedDialog", args, 4);


    manage_centered(dialog);

    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
    XmStringFree(msg);

    XtPopup(XtParent(dialog), XtGrabNone);
}


static Widget search_dialog = NULL;
static Widget search_field = NULL;

static struct {
    int forward;
    int ignorecase;
} search_data;

static void case_toggled(Widget widget,
			 XtPointer client_data, XtPointer call_data)
{
    XmToggleButtonCallbackStruct *state =
    (XmToggleButtonCallbackStruct *) call_data;

    if (state->set)
	search_data.ignorecase = False;
    else
	search_data.ignorecase = True;
}

static void direction_toggled(Widget widget,
			      XtPointer client_data, XtPointer call_data)
{
    XmToggleButtonCallbackStruct *state =
    (XmToggleButtonCallbackStruct *) call_data;

    if (state->set)
	search_data.forward = False;
    else
	search_data.forward = True;
}

static void find_callback(Widget w,
			  XtPointer client_data,
			  XtPointer call_data)
{
    String needle;
    XmTextPosition found;
    XmTextPosition current;
    XmTextDirection direction;

    needle = XmTextFieldGetString((Widget) client_data);
    if (!needle || !strcmp(needle, "")) {
	show_error_dialog("Nothing to search for.");
	XtFree(needle);
	return;
    }
    if (!node_text) {
	show_error_dialog("Nothing to search in.");
	XtFree(needle);
	return;
    }
    if (search_data.forward)
	direction = XmTEXT_FORWARD;
    else
	direction = XmTEXT_BACKWARD;

    current = XmTextGetInsertionPosition(node_text);

    if (!XmTextFindString(node_text, current, needle, direction, &found)) {
	Widget dialog;
	Arg args[5];
	int i;

	XmString msg;
	msg = XmStringCreateSimple("Search string not found!");
	i = 0;
	XtSetArg(args[i], XmNmessageString, msg);
	i++;

	dialog = (Widget) XmCreateInformationDialog(top_level,
					  "searchFailedDialog", args, i);
	XtUnmanageChild((void *) XmMessageBoxGetChild(dialog,
						  XmDIALOG_HELP_BUTTON));
	XtUnmanageChild((void *) XmMessageBoxGetChild(dialog,
						XmDIALOG_CANCEL_BUTTON));
	XmStringFree(msg);
	XtManageChild(dialog);
	XtPopup(XtParent(dialog), XtGrabNone);
	XtFree(needle);
	return;
    } else {
	XmTextSetInsertionPosition(node_text, found);
	/* And now highlight the found string.
	 * thereafter try to set the insertion position beyoind it!
	 */
	XmTextSetSelection(node_text, found, found + strlen(needle),
			   CurrentTime);
	XmTextSetInsertionPosition(node_text, found + strlen(needle));

    }
    XtFree(needle);
}

static void clear_callback(Widget w,
			   XtPointer client_data,
			   XtPointer call_data)
{
    XmTextFieldSetString((Widget) client_data, "");
    XmTextSetInsertionPosition(node_text, 0);
}

static void close_callback(Widget w,
			   XtPointer client_data,
			   XtPointer call_data)
{
    XtUnmanageChild((Widget) client_data);
    XmTextSetInsertionPosition(node_text, 0);
    XmTextClearSelection(node_text, CurrentTime);
}


void search_pushed(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget form;
    Widget label;
    Widget find;
    Widget clear;
    Widget close;
    Widget case_toggle;
    Widget direction_toggle;

    if (!search_dialog) {
	search_dialog = XmCreateTemplateDialog(top_level, "searchDialog",
					       NULL, 0);

	search_data.forward = True;
	search_data.ignorecase = True;

	XmTextSetInsertionPosition(node_text, 0);
	XtVaSetValues(search_dialog,
		      XmNautoUnmanage, False,
		      XmNresizePolicy, XmRESIZE_NONE,
		      NULL);
	find = XtCreateManagedWidget("findButton", xmPushButtonGadgetClass,
				     search_dialog, NULL, 0);


	clear = XtCreateManagedWidget("clearButton", xmPushButtonGadgetClass,
				      search_dialog, NULL, 0);
	close = XtCreateManagedWidget("closeButton", xmPushButtonGadgetClass,
				      search_dialog, NULL, 0);

	/* 
	 * Create a manager widget to handle the dialog layout.
	 */

	form = XtVaCreateManagedWidget("form",
				       xmFormWidgetClass, search_dialog,
				       XmNorientation, XmHORIZONTAL,
				       XmNnumColumns, 2,
				       XmNallowResize, False,
				       NULL);

	search_field = XtVaCreateManagedWidget("findText",
					    xmTextFieldWidgetClass, form,
					       XmNcolumns, 44,
					       XmNeditable, True,
				       XmNrightAttachment, XmATTACH_FORM,
					 XmNtopAttachment, XmATTACH_FORM,
					       NULL);

	XtVaSetValues(search_dialog, XmNinitialFocus, search_field, NULL);

	label = XtVaCreateManagedWidget("findLabel", xmLabelWidgetClass, form,
					XmNleftAttachment, XmATTACH_FORM,
			      XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
					XmNtopWidget, search_field,
			   XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
					XmNbottomWidget, search_field,
				     XmNrightAttachment, XmATTACH_WIDGET,
					XmNrightWidget, search_field,
					NULL);


	case_toggle = XtVaCreateManagedWidget("caseToggle",
					 xmToggleButtonWidgetClass, form,
					      XmNshowAsDefault, True,
			     XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
					      XmNleftWidget, search_field,
				       XmNtopAttachment, XmATTACH_WIDGET,
					      XmNtopWidget, search_field,
					      NULL);

	XtAddCallback(case_toggle, XmNvalueChangedCallback,
		      case_toggled, NULL);

	direction_toggle = XtVaCreateManagedWidget("findToggle",
					 xmToggleButtonWidgetClass, form,
						   XmNshowAsDefault, True,
				      XmNleftAttachment, XmATTACH_WIDGET,
					      XmNleftWidget, case_toggle,
						   XmNleftOffset, 32,
				       XmNtopAttachment, XmATTACH_WIDGET,
					      XmNtopWidget, search_field,
						   NULL);
	XtAddCallback(direction_toggle, XmNvalueChangedCallback,
		      direction_toggled, NULL);

	XtAddCallback(find, XmNactivateCallback, find_callback, search_field);
	XtAddCallback(clear, XmNactivateCallback, clear_callback, search_field);
	XtAddCallback(close, XmNactivateCallback, close_callback, search_dialog);
	XtVaSetValues(search_dialog, XmNdefaultButton, find, NULL);

    }
    manage_centered(search_dialog);

    XtPopup(XtParent(search_dialog), XtGrabNone);

    XmProcessTraversal(search_field, XmTRAVERSE_CURRENT);
}


/*
 * Manage the dialog centered on pointer.
 */
void manage_centered(Widget dialog_child)
{
    Widget shell = XtParent(dialog_child);
    Window root, child;
    unsigned int mask;
    unsigned int width, height, border_width, depth;
    int x, y, win_x, win_y, maxX, maxY;
    Boolean mappedWhenManaged;

    /* Temporarily set value of XmNmappedWhenManaged
       to stop the dialog from popping up right away */
    XtVaGetValues(shell, XmNmappedWhenManaged, &mappedWhenManaged, 0);
    XtVaSetValues(shell, XmNmappedWhenManaged, False, 0);

    XtManageChild(dialog_child);

    /* Get the pointer position (x, y) */
    XQueryPointer(XtDisplay(shell), XtWindow(shell), &root, &child,
		  &x, &y, &win_x, &win_y, &mask);

    /* Translate the pointer position (x, y) into a position for the new
       window that will place the pointer at its center */
    XGetGeometry(XtDisplay(shell), XtWindow(shell), &root, &win_x, &win_y,
		 &width, &height, &border_width, &depth);
    width += 2 * border_width;
    height += 2 * border_width;
    x -= width / 2;
    y -= height / 2;

    /* Ensure that the dialog remains on screen */
    maxX = XtScreen(shell)->width - width;
    maxY = XtScreen(shell)->height - height;
    if (x < 0)
	x = 0;
    if (x > maxX)
	x = maxX;
    if (y < 0)
	y = 0;
    if (y > maxY)
	y = maxY;

    /* Set desired window position in the DialogShell */
    XtVaSetValues(shell, XmNx, x, XmNy, y, NULL);

    /* Map the widget */
    XtMapWidget(shell);

    /* Restore the value of XmNmappedWhenManaged */
    XtVaSetValues(shell, XmNmappedWhenManaged, mappedWhenManaged, 0);
}
