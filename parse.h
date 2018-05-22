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

#ifndef PARSE_H
#define PARSE_H

#include <Xm/Text.h>

struct indirect {		/* table of indirection files */
    struct indirect *next;
    char *file;
    int tag;
};

struct node {			/* node in indirection table */
    struct node *next;
    char *node;
    int tag;
    char *file;
};

struct top {			/* top indirect file */
    struct indirect *indirect;
    struct node *node;
};

struct menu {
    struct menu *next;
    char *name;
    char *node;
    int hide;			/* don't show duplicate xref's or menu entries in menus */

    int start;			/* offset of it's start in page text */
    int stop;			/* offset of first char past it's end in page text */
    int ustart;			/* where to do underlining */
    int ustop;			/* where to stop underlining */
    Boolean visited;		/* had this link allready been visited */
};


/*
 * Structure describing an chunk of text, which will recive special 
 * treatment during redrawing of it
 */

enum chunk_kind {
    NO_CHUNK,
    HEAD1,
    HEAD2,
    HEAD3,
    HEAD4,
    ULINE1,
    ULINE2,
    ULINE3,
    ULINE4,
    FOOTNOTE,
    MENUHEAD,
    LISTITEM,
    SUBITEM
};

struct chunk {
    struct chunk *next;
    enum chunk_kind kind;
    int start;
    int stop;
};

struct page {
    struct page *next;		/* next in page cache */
    char *file;

    char *File;
    char *Node;
    char *Next;
    char *Prev;
    char *Up;

    char *text;			/* The actual contents of this page */

    struct menu *menu;		/* list of menu items in this page */
    struct menu *note;		/* list of xrefs in this page */

    XmTextPosition saved_pos;	/* preserved text position if any */
    int saved_sbar;		/* preserved scrollbar positino if any */

    struct chunk *chunk;	/* displaying information */
};

extern struct top *parse_top(char *text);
struct page *parse_file(char *file, char *text);

extern void normalize_white_space(char **text);

extern void debug_print_top(struct top *top);
extern void debug_print_page(struct page *page);

#endif				/* PARSE_H */
