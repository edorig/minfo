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
#include <stdio.h>
#include <string.h>

#include "parse.h"

void normalize_white_space(char **text)
{
    char *n;
    unsigned int i;

    while (**text && isspace(**text))
	++ * text;

    while ((n = strstr(*text, "  "))) {
	for (i = 0; i < strlen(n); i++)
	    n[i] = n[i + 1];
    }
}

/*
 * This routine parses the information in an indirection file
 */
struct top *parse_top(char *text)
{
    char *buf;
    char *buf_dup;
    char *buf_tail;
    char *token;

    struct top *top;
    struct indirect *indirect = NULL;
    struct node *node = NULL;

    if (!text)
	return NULL;

    text = strstr(text, "\x1f\n");

    if (!text)
	return NULL;		/* no heading in file */

    text += 2;

    if (text != strstr(text, "Indirect:\n"))
	return NULL;
    else
	text += strlen("Indirect:\n");

    /* We are now at the top of the indirection table.
     */

    top = NULL;
    buf_dup = buf = strdup(text);
    buf_tail = buf + strlen(buf);

    while (buf < buf_tail) {
	struct indirect *curr;
	char *dp;
	int length;

	if ((dp = strchr(buf, '\n')))
	    *dp = '\0';

	if (!strcmp(buf, "\x1f"))
	    break;

	length = strlen(buf);

	dp = strchr(buf, ':');

	if (dp)
	    *dp = '\0';

	curr = (struct indirect *) malloc(sizeof(struct indirect));

	curr->next = NULL;
	curr->file = strdup(buf);
	curr->tag = atoi(dp + 1);
	if (!top) {
	    top = (struct top *) malloc(sizeof(struct top));
	    top->node = NULL;
	    top->indirect = curr;
	} else
	    indirect->next = curr;
	indirect = curr;

	buf += length + 1;
    }
    free(buf_dup);

    /*
     * Now parse the tag table (if any);
     */
    if (top && (text = strstr(text, "\x1f\nTag Table:\n(Indirect)\n"))) {
	text += strlen("\x1f\nTag Table:\n(Indirect)\n");
	buf_dup = buf = strdup(text);
	buf_tail = buf + strlen(buf);

	while (buf < buf_tail) {
	    struct node *curr;
	    char *dp;
	    int length;

	    if ((dp = strchr(buf, '\n')))
		*dp = '\0';


	    if (!strcmp(buf, "\x1f"))
		break;

	    length = strlen(buf);

	    dp = strchr(buf, '\x7f');

	    if (dp)
		*dp = '\0';

	    if ((token = strstr(buf, "Node: ")))
		token += strlen("Node: ");
	    /* if the string comparison fails, token=NULL no node exists */ 	    
	    /*	EO: If debugging is needed:     if (token==NULL) fprintf(stderr,"NULL pointer with: %s\n",buf); */ 
            
	   	    
	    if (token!=NULL) {
	      curr = (struct node *) malloc(sizeof(struct node));
	    curr->next = NULL;
	    curr->node = strdup(token);

	    curr->tag = atoi(dp + 1);
	    curr->file = NULL;
	    };
	    
	    /* lookup filename; 
	     * we are assuming here that the indirection tabe is sorted in
	     * lower to upper order.
	     */
	    if (top->indirect) {
		struct indirect *last;

		indirect = top->indirect;
		last = indirect;
		while (indirect && (curr->tag >= indirect->tag)) {
		    last = indirect;
		    indirect = indirect->next;
		}
		if (last)
		    curr->file = last->file;
	    }
	    if (!top->node)
		top->node = curr;
	    else
		node->next = curr;
	    node = curr;

	    buf += length + 1;
	}
	free(buf_dup);
    }
    return top;
}

/*
 * This routine parses the menuentries in a page
 */
static struct menu *parse_menu(char *text)
{
    char *buf;
    char *buf_head;
    char *buf_tail;
    struct menu *first;
    struct menu *last;
    int length;
    int start = 0;
    int stop = 0;
    int ustart = 0;
    int ustop = 0;

    if (!text)
	return NULL;

    last = first = NULL;

    /*
     * Split up into lines...
     */
    buf_head = strdup(text);
    buf_tail = buf_head + strlen(buf_head);

    for (buf = buf_head; buf < buf_tail; buf += length + 1) {
	struct menu *menu;
	char *name;
	char *node;
	char *end;
	char *dot;
	char *tail;
	char *token;

	tail = strchr(buf, '\n');
	if (tail)
	    *tail = '\0';

	length = strlen(buf);

	if (strlen(buf) <= 3 || strncmp(buf, "* ", 2))
	    continue;
	token = buf + strlen("* ");
	start = buf - buf_head;
	ustart = start + strlen("* ");
	name = node = NULL;
	/*
	 * First try if it is a double node menu entry.
	 * Thereafter try if it is a menu to a node different then
	 * it's name.
	 */
	if ((end = strstr(token, "::"))) {
	    *end = '\0';
	    name = strdup(token);
	    node = strdup(token);
	    stop = end - buf_head + 2;
	    ustop = end - buf_head;
	} else {
	    if (!(end = strstr(token, ": ")) && !(end = strstr(token, ":\t")))
		continue;
	    *end = '\0';
	    ustop = end - buf_head;

	    end += strlen(": ");

	    while (strchr(" \t", *end))
		++end;

	    if (!strcmp(token, "menu") || !strcmp(token, "Menu"))
		continue;
	    if (*end == '(') {
		char *tmp;

		if (!(tmp = strchr(end, ')')))
		    continue;
		if (!(tmp = strchr(tmp, '.')))
		    continue;
		dot = tmp;

	    } else if (!(dot = strchr(end, '.')))
		continue;
	    else
		ustop = dot - buf_head;
	    *dot = '\0';
	    stop = dot - buf_head + 1;
	    name = strdup(token);
	    node = strdup(end);
	}

	/*
	 * Gotcha!
	 */
	menu = (struct menu *) malloc(sizeof(struct menu));
	menu->next = NULL;
	menu->name = name;
	menu->hide = 0;
	menu->start = start;
	menu->stop = stop;
	menu->ustart = ustart;
	menu->ustop = ustop;
	menu->visited = False;
	while (node && isspace(*node))
	    node++;
	menu->node = node;
	if (!first)
	    last = first = menu;
	else if (last)
	    last->next = menu;
	last = menu;
    }

    free(buf_head);
    return first;
}


/*
 * This routine parses the notes in a page
 */
static struct menu *parse_notes(char *text)
{
    char *buf;
    char *buf_head;
    char *buf_tail;
    struct menu *first;
    struct menu *last;
    int simple;

    int start = 0;
    int stop = 0;
    int ustart = 0;
    int ustop = 0;

    if (!text)
	return NULL;

    last = first = NULL;

    buf_head = buf = strdup(text);
    buf_tail = buf_head + strlen(buf_head);

    /*
     * First replace all newlines with spaces.
     */
    buf = buf_head;
    while ((buf = strchr(buf, '\n')))
	*(buf++) = ' ';

    /*
     * Search for notes.
     */
    buf = buf_head;
    for (;;) {
	struct menu *xref;
	char *n;
	char *N;
	char *next;
	char *name;
	char *node;

	n = strstr(buf, "*Note ");
	N = strstr(buf, "*note ");

	if (!n && !N)
	    break;

	if (!n)
	    next = N;
	else if (!N)
	    next = n;
	else if (n < N)
	    next = n;
	else
	    next = N;

	start = next - buf_head;	/* remember start of node text */

	buf = next + strlen("*Note ");

	ustart = buf - buf_head;

	if (!(next = strchr(buf, ':')))
	    break;

	ustop = next - buf_head;
	if ((next[1] != ':') && (next[1] != ' '))
	    continue;

	if (next[1] == ':')
	    simple = 1;
	else
	    simple = 0;
	*next = '\0';

	normalize_white_space(&buf);
	name = strdup(buf);
	stop = buf - buf_head;
	if (simple) {
	    node = strdup(name);
	    buf = next + 1;
	    stop = buf - buf_head + 1;
	} else {
	    buf = next + 2;
	    if (buf >= buf_tail) {
		free(name);
		break;
	    }
	    next = strpbrk(buf, ",.");
	    if (!next) {
		free(name);
		continue;
	    }
	    if (next == strstr(next, ".info")) {
		next += strlen(".info");
		next = strpbrk(next, ",.");
		if (!next) {
		    free(name);
		    continue;
		}
	    } else if (next == strstr(next, "info")) {
		next += strlen("info");
		next = strpbrk(next, ",.");
		if (!next) {
		    free(name);
		    continue;
		}
	    }
	    *next = '\0';
	    normalize_white_space(&buf);
	    node = strdup(buf);
	    buf = next + 1;
	    stop = buf - buf_head;
	}

	/*
	 * Gotcha! we have found a cross reference.
	 */

	xref = first;
	while (xref) {
	    if (!strcmp(xref->name, name) && !strcmp(xref->node, node))
		break;

	    xref = xref->next;
	}
	if (!xref) {
	    xref = (struct menu *) malloc(sizeof(struct menu));
	    xref->hide = 0;
	} else {
	    xref = (struct menu *) malloc(sizeof(struct menu));
	    xref->hide = 1;
	}
	xref->next = NULL;
	xref->name = name;
	xref->node = node;
	xref->start = start;
	xref->stop = stop;
	xref->ustart = ustart;
	xref->ustop = ustop;
	xref->visited = False;
	if (!first)
	    last = first = xref;
	else if (last)
	    last->next = xref;
	last = xref;
    };

    free(buf_head);
    return first;
}

/*
 * Return a list of text areas, which will recive special treatment during
 * redisplaying.
 */

static struct chunk *parse_chunks(char *text)
{
    char *buf;
    char *buf_head;
    char *buf_tail;
    char *last;
    struct chunk *first = NULL;

    if (!text)
	return NULL;

    buf_head = buf = strdup(text);
    buf_tail = buf_head + strlen(buf_head);

    /*
     * First split the whole thing into a series of lines.
     */

    buf = buf_head;
    while ((buf = strchr(buf, '\n')))
	*(buf++) = '\0';

    /*
     * Heck for headers.
     */
    last = NULL;
    for (buf = buf_head; buf < buf_tail; buf += strlen(buf) + 1) {
	/*
	 * We start checking only if there is some previous line allready.
	 */

	if (last) {
	    int llen = strlen(last);
	    int blen = strlen(buf);

	    if (llen == blen && llen > 0) {
		int i;
		int check;

		check = 1;
		for (i = 0; i < blen; ++i)
		    if (buf[i] != '*') {
			check = 0;
			break;
		    }
		if (check) {
		    struct chunk *tmp;

		    tmp = (struct chunk *) malloc(sizeof(struct chunk));
		    tmp->kind = ULINE1;
		    tmp->start = buf - buf_head;
		    tmp->stop = buf + blen - buf_head;
		    tmp->next = first;
		    first = tmp;

		    tmp = (struct chunk *) malloc(sizeof(struct chunk));
		    tmp->kind = HEAD1;
		    tmp->start = last - buf_head;
		    tmp->stop = last + llen - buf_head;
		    tmp->next = first;
		    first = tmp;
		}
		check = 1;
		for (i = 0; i < blen; ++i)
		    if (buf[i] != '=') {
			check = 0;
			break;
		    }
		if (check) {
		    struct chunk *tmp;

		    tmp = (struct chunk *) malloc(sizeof(struct chunk));
		    tmp->kind = ULINE2;
		    tmp->start = buf - buf_head;
		    tmp->stop = buf + blen - buf_head;
		    tmp->next = first;
		    first = tmp;

		    tmp = (struct chunk *) malloc(sizeof(struct chunk));
		    tmp->kind = HEAD2;
		    tmp->start = last - buf_head;
		    tmp->stop = last + llen - buf_head;
		    tmp->next = first;
		    first = tmp;
		}
		check = 1;
		for (i = 0; i < blen; ++i)
		    if (buf[i] != '-') {
			check = 0;
			break;
		    }
		if (check) {
		    struct chunk *tmp;

		    tmp = (struct chunk *) malloc(sizeof(struct chunk));
		    tmp->kind = ULINE3;
		    tmp->start = buf - buf_head;
		    tmp->stop = buf + blen - buf_head;
		    tmp->next = first;
		    first = tmp;

		    tmp = (struct chunk *) malloc(sizeof(struct chunk));
		    tmp->kind = HEAD3;
		    tmp->start = last - buf_head;
		    tmp->stop = last + llen - buf_head;
		    tmp->next = first;
		    first = tmp;
		}
		check = 1;
		for (i = 0; i < blen; ++i)
		    if (buf[i] != '.') {
			check = 0;
			break;
		    }
		if (check) {
		    struct chunk *tmp;

		    tmp = (struct chunk *) malloc(sizeof(struct chunk));
		    tmp->kind = ULINE4;
		    tmp->start = buf - buf_head;
		    tmp->stop = buf + blen - buf_head;
		    tmp->next = first;
		    first = tmp;

		    tmp = (struct chunk *) malloc(sizeof(struct chunk));
		    tmp->kind = HEAD4;
		    tmp->start = last - buf_head;
		    tmp->stop = last + llen - buf_head;
		    tmp->next = first;
		    first = tmp;
		}
	    }
	}
	last = buf;
    }

    /*
     * Checking for footnotes
     */
    for (buf = buf_head; buf < buf_tail; buf += strlen(buf) + 1)
	if (!strcmp(buf, "   ---------- Footnotes ----------")) {
	    struct chunk *tmp;

	    tmp = (struct chunk *) malloc(sizeof(struct chunk));
	    tmp->kind = FOOTNOTE;
	    tmp->start = buf - buf_head;
	    tmp->stop = buf + strlen(buf) - buf_head;
	    tmp->next = first;
	    first = tmp;
	}
    /*
     * Checking for Menues. 
     */
    for (buf = buf_head; buf < buf_tail; buf += strlen(buf) + 1)
	/* There may be as well an title for this menu there.
	 */
	if (!strncmp(buf, "* Menu:", 7)) {
	    struct chunk *tmp;

	    tmp = (struct chunk *) malloc(sizeof(struct chunk));
	    tmp->kind = MENUHEAD;
	    tmp->start = buf - buf_head;
	    tmp->stop = buf + strlen(buf) - buf_head;
	    tmp->next = first;
	    first = tmp;
	}
    /*
     * Check for list item dot's
     */
    for (buf = buf_head; buf < buf_tail; buf += strlen(buf) + 1) {
	int start;
	int stop;
	struct chunk *tmp;

	/* check for first empty line */
	if (strlen(buf))
	    continue;
	buf += 1;
	if (buf >= buf_tail)
	    break;
	/* check for next line containing the item dots */
	if (strncmp(buf, "   * ", 5))
	    continue;
	start = buf - buf_head;
	stop = buf + strlen(buf) - buf_head;	/* not that interresting */

	/*
	 * Check later if anything past this position until end
	 * or the next empty line is indented by 5.
	 */

	/*
	 * Gotcha, accept it.
	 */
	tmp = (struct chunk *) malloc(sizeof(struct chunk));
	tmp->kind = LISTITEM;
	tmp->start = start;
	tmp->stop = stop;
	tmp->next = first;
	first = tmp;
    }

    /*
     * Check for list sub item dash
     */
    for (buf = buf_head; buf < buf_tail; buf += strlen(buf) + 1) {
	int start;
	int stop;
	struct chunk *tmp;

	/* check for first empty line */
	if (strlen(buf))
	    continue;
	buf += 1;
	if (buf >= buf_tail)
	    break;
	/* check for next line containing the item dots */
	if (strncmp(buf, "   - ", 5))
	    continue;
	start = buf - buf_head;
	stop = buf + strlen(buf) - buf_head;	/* not that interresting */

	/*
	 * Check later if anything past this position until end
	 * or the next empty line is indented by 5.
	 */

	/*
	 * Gotcha, accept it.
	 */
	tmp = (struct chunk *) malloc(sizeof(struct chunk));
	tmp->kind = SUBITEM;
	tmp->start = start;
	tmp->stop = stop;
	tmp->next = first;
	first = tmp;
    }

    free(buf_head);

    return first;
}

/*
 * This routine returns a list of nodes contained in an actual data file.
 */
struct page *parse_file(char *file, char *text)
{
    struct page *first;
    struct page *last;
    char *buf;
    char *buf_head;
    char *buf_tail;

    if (!text)
	return NULL;

    if (!text)
	return NULL;		/* no heading in file */

    if (!(text = strchr(text, '\x1f')))
	return NULL;
    text += 1;

    last = first = NULL;

    buf_head = buf = strdup(text);
    buf_tail = buf + strlen(buf);
    while (buf < buf_tail) {
	struct page *page;
	char *head;
	char *tail;
	int length;

	tail = strchr(buf, '\x1f');
	if (tail)
	    *tail = '\0';

	length = strlen(buf);

	page = (struct page *) malloc(sizeof(struct page));
	page->next = NULL;
	page->file = strdup(file);
	page->File = NULL;
	page->Node = NULL;
	page->Next = NULL;
	page->Prev = NULL;
	page->Up = NULL;
	page->text = NULL;
	page->menu = NULL;
	page->note = NULL;
	page->saved_pos = 0;
	page->saved_sbar = 0;

	if (!first)
	    last = first = page;
	else if (last)
	    last->next = page;
	last = page;

	/* 
	 * Parse the head of the page
	 */

	head = strchr(buf + 1, '\n');
	if (head) {
	    char *end;
	    char *begin;
	    char *cont;

	    *head = '\0';
	    cont = head + 1;
	    if ((begin = strstr(buf, "\nFile: "))
		&& ((end = strchr(begin + strlen("\nFile: "), ','))
		|| (end = strpbrk(begin + strlen("\nFile: "), " \t")))) {
		*end = '\0';
		page->File = strdup(begin + strlen("\nFile: "));
		head = end + 1;
	    }
	    if ((begin = strstr(head, "Node: "))
		&& ((end = strchr(begin + strlen("Node: "), ','))
		  || (end = strpbrk(begin + strlen("Node: "), " \t")))) {
		*end = '\0';
		page->Node = strdup(begin + strlen("Node: "));
		head = end + 1;
	    }
	    if ((begin = strstr(head, "Next: "))
		&& ((end = strchr(begin + strlen("Next: "), ','))
		  || (end = strpbrk(begin + strlen("Next: "), " \t")))) {
		*end = '\0';
		page->Next = strdup(begin + strlen("Next: "));
		head = end + 1;
	    }
	    if ((begin = strstr(head, "Prev: "))
		&& ((end = strchr(begin + strlen("Prev: "), ','))
		  || (end = strpbrk(begin + strlen("Prev: "), " \t")))) {
		*end = '\0';
		page->Prev = strdup(begin + strlen("Prev: "));
		head = end + 1;
	    }
	    if ((begin = strstr(head, "Up: "))) {
		page->Up = strdup(begin + strlen("Up: "));
	    }
	    page->text = strdup(cont);
	} else
	    page->text = NULL;
#if 0
	printf("File: %s\nNode: %s\nPrev %s\nNext %s\nUp: %s\n\n",
	       page->File, page->Node, page->Prev, page->Next, page->Up);
#endif
	/*
	 * Parse the menu entries of the node.
	 */
	page->menu = parse_menu(page->text);
	page->note = parse_notes(page->text);
	page->chunk = parse_chunks(page->text);

	buf += length + 1;
    }
    free(buf_head);

    return first;
}

#if 0
/*
 * Routines usefull for debugging output. Enable as needed.
 */

void debug_print_top(struct top *top)
{
    struct indirect *indirect;
    struct node *node;

    if (!top)
	return;

    puts("\nIndirections:");
    for (indirect = top->indirect; indirect; indirect = indirect->next)
	printf("\tfile: %s, tag: %d\n", indirect->file, indirect->tag);
    puts("\nNode indirections:");
    for (node = top->node; node; node = node->next)
	printf("\tnode: %s, file: %s tag: %d\n",
	       node->node, node->file, node->tag);

}


void debug_print_page(struct page *page)
{
    if (!page)
	return;
    puts("BEGIN PAGE");
    printf("\tFile: [%s]\n", page->File);
    printf("\tNode: [%s]\n", page->Node);
    printf("\tNext: [%s]\n", page->Next);
    printf("\tPrev: [%s]\n", page->Prev);
    printf("\tUp: [%s]\n", page->Up);
    if (page->menu) {
	struct menu *menu;
	puts("\tBEGIN MENU");
	for (menu = page->menu; menu; menu = menu->next)
	    printf("\t\tName: [%s], Node: [%s]\n", menu->name, menu->node);
	puts("\tEND MENU");
    }
    puts(page->text);
    puts("END PAGE");
}
#endif
