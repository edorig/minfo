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

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <Xm/FileSB.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/TextP.h>

#include "fops.h"
#include "display.h"
#include "utils.h"

char *lookup_file_in_path(char *name, char *path)
{
    char *token;
    char *end;
    char *buf_head;
    struct stat statb;
    FILE *fp;

    buf_head = strdup(path);

    /* 
     * Go throught the path string, which is separated by double colons.
     */
    token = buf_head;
    while (token) {
	char *buf = malloc(strlen(token) + strlen(name) + 6);
	char *auf = malloc(strlen(token) + strlen(name) + 6);
	int len;

	if ((end = strchr(token, ':')))
	    *(end++) = '\0';

	strcpy(buf, token);
	len = strlen(buf);
	if (len > 0) {
	    if (buf[strlen(buf) - 1] != '/')
		strcat(buf, "/");
	}
	strcat(buf, name);
	strcpy(auf, buf);
	strcat(auf, ".gz");

	if (access(buf, R_OK) == -1 && access(auf, R_OK) == -1)
	    strcat(buf, ".info");

	free(auf);

	if (stat(buf, &statb) == -1 || (statb.st_mode & S_IFMT) != S_IFREG ||
	    !(fp = fopen(buf, "r"))) {
	    FILE *input;
	    /*
	     * Try it with a compressed file.
	     */
	    char command[2048];	/* Sometimes I'm just to lazy too ... */
	    char *readbuf;
	    int size;
	    int readed;
	    int tmp;

	    strcpy(command, "zcat ");
	    strcat(command, buf);
	    strcat(command, " 2> /dev/null");

	    if ((input = popen(command, "r"))) {
		readed = 0;
		size = 0;
		readbuf = (char *) malloc(4096 * sizeof(char) + 2);
		while ((tmp = fread(readbuf + size, sizeof(char), 4096, input))) {
		    readed += tmp;
		    size += 4096;
		    readbuf = (char *) realloc(readbuf, 4096 + size + 2);
		}
		if (readed) {
		    char *text;

		    readbuf[readed] = '\0';
		    text = strdup(readbuf);
		    free(readbuf);
		    free(buf_head);

		    return text;
		}
		free(readbuf);
	    }
	    free(buf);
	} else {
	    char *text = NULL;

	    /* 
	     * Put the contents of the file in the Text widget by
	     * allocating enough space for the entire file, reading the
	     * file into the allocated space, and using
	     * XmTextFieldSetString() to show the file.
	     */
	    if (!(text = (char *) malloc((unsigned) (statb.st_size + 1)))) {
		fprintf(stderr, "Can't alloc enough space for %s\n", buf);
		free(buf);
		fclose(fp);
	    } else {
		if (!fread(text, sizeof(char), statb.st_size + 1, fp)) {
		    fprintf(stderr, "Warning: may not have read entire file!\n");
		}
		text[statb.st_size] = '\0';	/* be sure to NULL-terminate */

		free(buf);
		free(buf_head);
		return text;
	    }
	}
	token = end;
    }

    free(buf_head);
    return NULL;
}


char *file_path;

struct file_cache {
    struct file_cache *next;
    char *file;
    struct top *top;		/* not zero if it is an indirection table */
    struct page *page;		/* of course every file has an contents */
} *file_cache = NULL;

static struct page *lookup_node_in_file_rr(int rr, char *node, char *file)
{
    struct file_cache *cache;
    struct page *page;
    char *tmp;
    char *name_dup;

    /*
     * Bump out if we are ging to call ourselfs in infinite recursion!
     * This helps with broken info files, containing not present refferences.
     */

    if (rr <= 0)
	return NULL;

    /* 
     * Handle the case of directory nodes.
     */

    if (!strcmp(node, "(dir)") || !strcmp(node, "(DIR)")) {
	node = "Top";
	file = "dir";
    }
    /* 
     * First look if the file is allready cached.
     */

    cache = file_cache;
    while (cache && strcmp(cache->file, file))
	cache = cache->next;

    /* If not allready found there, try to look it up and then add it to
     * the cache, return NULL otherwise.
     */
    if (!cache) {
	char *text;

	text = lookup_file_in_path(file, file_path);
	if (!text)
	    return NULL;

	cache = (struct file_cache *) malloc(sizeof(struct file_cache));
	cache->top = parse_top(text);
	cache->file = strdup(file);
	if (!cache->top) {
	    struct page *page;
	    cache->page = parse_file(file, text);
	    page = cache->page;
	} else
	    cache->page = NULL;

	/* Add it to the cache.
	 */
	cache->next = file_cache;
	file_cache = cache;
    }
    /*
     * Try it in the current file from the cache.
     */
    page = cache->page;

    while (page && page->Node && strcmp(page->Node, node)) {
	page = page->next;
    }

    if (page && page->Node)
	return page;

    /* 
     * There is an associated indirection table try it there.
     */
    if (cache->top) {
	struct node *temp;

	for (temp = cache->top->node; temp; temp = temp->next) {
	    if (!strcmp(temp->node, node))
		return lookup_node_in_file_rr(rr - 1, node, temp->file);
	}
    }
    /* 
     * Last resort, try to find the indirection table.
     */

    name_dup = strdup(cache->file);
    if ((tmp = strstr(name_dup, ".info-"))) {
	tmp[5] = '\0';
	page = lookup_node_in_file_rr(rr - 1, node, name_dup);
	free(name_dup);
	return page;
    } else if ((tmp = strstr(name_dup, "info-"))) {
	tmp[4] = '\0';
	page = lookup_node_in_file_rr(rr - 1, node, name_dup);
	free(name_dup);
	return page;
    } else {
	name_dup = (char *) realloc(name_dup, strlen(name_dup) + 6);
	strcat(name_dup, ".info");
	page = lookup_node_in_file_rr(rr - 1, node, name_dup);
	free(name_dup);
	return page;
    }
    free(name_dup);

    return NULL;
}

struct page *lookup_node_in_file(char *node, char *file)
{
    return lookup_node_in_file_rr(100, node, file);
}

static void load_file(Widget dialog, XtPointer client_data, XtPointer call_data)
{
    char *file = NULL;
    char *file_dup;
    struct page *page;
    char *current_path;
    char *used_path;
    char *saved_path;
    char *trunc;

    XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;

    if (!cbs)
	return;

    if (!XmStringGetLtoR(cbs->value, XmFONTLIST_DEFAULT_TAG, &file))
	return;			/* internal error */
    XtPopdown(XtParent(dialog));

    /*
     * Preprocess the filename.
     * 1. Remove the compression endings .gz .Z .z
     * 2. Remove the leading path and save it
     */

    file_dup = current_path = file;
    trunc = strrchr(file, '.');
    if (trunc && (!strcmp(trunc, ".gz") || !strcmp(trunc, ".Z")
		  || !strcmp(trunc, ".z"))) {
	/* Remove compression marks they are only confusing.
	 * We are handling them "automagically" anyway.
	 */
	*trunc = '\0';
    }
    saved_path = file_path;
    if ((trunc = strrchr(file, '/'))) {
	file = trunc + 1;
	*trunc = '\0';
    } else {
	current_path = "";	/* No no ".", syscalls are allways relative to the
				   current path */
    }

    /*
     * This ensures, that the path to the file with it's full path name
     * will be our first candidate in lookups...
     */

    used_path = (char *) malloc(strlen(current_path) + 1
				+ strlen(file_path) + 1 + 2);

    strcpy(used_path, current_path);
    strcat(used_path, "/:");
    strcat(used_path, file_path);

    /* 
     * Communicate the new extended file search path.
     */
    file_path = used_path;
    page = lookup_node_in_file("Top", file);
    if (page) {
	display_page(page);
	/*
	 * Add the path to the file search path! (Actually it's allready done).
	 */
	file_path = used_path;
	free(saved_path);
    } else {
	free(used_path);
	file_path = saved_path;

	/* Inform the user about what happaned!
	 */

	show_error_dialog("File inaccessible, or it doesn't "
			  "contain any Top info node!");
    }
    XtFree(file_dup);
}

/* Any item the user selects from the File menu calls this function.
 * It will either be "Open" (item_no == 0) or "Quit" (item_no == 1).
 */
void file_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
    static Widget dialog;	/* make it static for reuse */
    int item_no = (int) client_data;

    if (item_no == 1)		/* the "quit" item */
	exit(0);

    /* "Open" was selected.  Create a Motif FileSelectionDialog w/callback */
    if (!dialog) {
	dialog = (Widget) XmCreateFileSelectionDialog(top_level,
						  "fileDialog", NULL, 0);
	XtAddCallback(dialog, XmNokCallback, load_file, NULL);
	XtAddCallback(dialog, XmNcancelCallback,
		      (void *) XtUnmanageChild, NULL);

	XtUnmanageChild((void *) XmFileSelectionBoxGetChild(dialog,
						  XmDIALOG_HELP_BUTTON));

    }
#if 0
    XtVaSetValues(dialog, XmNfileFilterStyle, XmFILTER_HIDDEN_FILES,
		  XmNwidth, 500,
		  NULL);
#endif
    XtManageChild(dialog);
    XtPopup(XtParent(dialog), XtGrabNone);
}

/* Any item the user selects from the File menu calls this function.
 * It will either be "Add Bookmark" (item_no == 0) or 
 * "Delete Bookmark" (item_no == 1).
 */

struct page *bookmarks = NULL;

static void bookmark_button_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    struct page *page = (struct page *) client_data;
    struct page *user_data;
    char *params[1];

    /* get the index of the shell command and verify that it's in range */
    XtVaGetValues(w, XmNuserData, &user_data, 0);

    /* consistency checks */
    if (!user_data || !page)
	return;
    if (user_data != page)
	return;

    display_page(page);


}

void book_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
    WidgetList items;
    int nitems;
    /* EO: Needed on Intel 64 bit: with a 32 bit int, the program will freeze 
       because perm will always be zero. On Intel 32 bit, this is not necessary. */   
    long int perm;
    int i;
    int item_no = (int) client_data;


    if (item_no == 0) {
	Widget button;
	XmString label;
	char str[1024];

	if (!current_page) {
	    show_error_dialog("Nothing to be marked!\n");

	    return;
	}
	strcpy(str, "(");
	strcat(str, current_page->File);
	strcat(str, ") ");
	strcat(str, current_page->Node);
	label = XmStringCreateSimple(str);
	button = XtVaCreateManagedWidget("bookmarkButton",
				  xmPushButtonWidgetClass, bookmark_menu,
				      XmNuserData, (void *) current_page,
					 XmNlabelString, label,
					 NULL);
	XtAddCallback(button, XmNactivateCallback,
		      bookmark_button_cb, (void *) current_page);
	return;
    }
    /* the delete item */
    if (item_no == 1) {

	/*
	 * Fetch the list of current bookmark items.
	 */
	XtVaGetValues(bookmark_menu,
		      XmNchildren, &items,
		      XmNnumChildren, &nitems, NULL);


	/* Destroy the oldest item in the list */
	for (i = 0; i < nitems; ++i) {
	  /* EO: What follows is a bit ugly, the pointer on the bookmarked page gets is cast into an integer, but trying to declare void *perm and using perm as a pointer here is causing a SIGSEGV */   
	    XtVaGetValues(items[i], XmNuserData, &perm, 0);
	    if (perm) {
		/* unamanging before destroying stops parent 
		   from displaying */
		XtUnmanageChild(items[i]);
		XtDestroyWidget(items[i]);
		break;
	    }
	}
    }
}
