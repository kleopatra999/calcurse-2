/*
 * Calcurse - text-based organizer
 *
 * Copyright (c) 2004-2013 calcurse Development Team <misc@calcurse.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the
 *        following disclaimer in the documentation and/or other
 *        materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Send your feedback or comments to : misc@calcurse.org
 * Calcurse home page : http://calcurse.org
 *
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "calcurse.h"

struct attribute {
	int color[7];
	int nocolor[7];
};

static struct attribute attr;

/*
 * Define window attributes (for both color and non-color terminals):
 * ATTR_HIGHEST are for window titles
 * ATTR_HIGH are for month and days names
 * ATTR_MIDDLE are for the selected day inside calendar panel
 * ATTR_LOW are for days inside calendar panel which contains an event
 * ATTR_LOWEST are for current day inside calendar panel
 */
void custom_init_attr(void)
{
	attr.color[ATTR_HIGHEST] = COLOR_PAIR(COLR_CUSTOM);
	attr.color[ATTR_HIGH] = COLOR_PAIR(COLR_HIGH);
	attr.color[ATTR_MIDDLE] = COLOR_PAIR(COLR_RED);
	attr.color[ATTR_LOW] = COLOR_PAIR(COLR_CYAN);
	attr.color[ATTR_LOWEST] = COLOR_PAIR(COLR_YELLOW);
	attr.color[ATTR_TRUE] = COLOR_PAIR(COLR_GREEN);
	attr.color[ATTR_FALSE] = COLOR_PAIR(COLR_RED);

	attr.nocolor[ATTR_HIGHEST] = A_BOLD;
	attr.nocolor[ATTR_HIGH] = A_REVERSE;
	attr.nocolor[ATTR_MIDDLE] = A_REVERSE;
	attr.nocolor[ATTR_LOW] = A_UNDERLINE;
	attr.nocolor[ATTR_LOWEST] = A_BOLD;
	attr.nocolor[ATTR_TRUE] = A_BOLD;
	attr.nocolor[ATTR_FALSE] = A_DIM;
}

/* Apply window attribute */
void custom_apply_attr(WINDOW * win, int attr_num)
{
	if (colorize)
		wattron(win, attr.color[attr_num]);
	else
		wattron(win, attr.nocolor[attr_num]);
}

/* Remove window attribute */
void custom_remove_attr(WINDOW * win, int attr_num)
{
	if (colorize)
		wattroff(win, attr.color[attr_num]);
	else
		wattroff(win, attr.nocolor[attr_num]);
}

/* Draws the configuration bar */
void custom_config_bar(void)
{
	const int SMLSPC = 2;
	const int SPC = 15;

	custom_apply_attr(win[STA].p, ATTR_HIGHEST);
	mvwaddstr(win[STA].p, 0, 2, "Q");
	mvwaddstr(win[STA].p, 1, 2, "G");
	mvwaddstr(win[STA].p, 0, 2 + SPC, "L");
	mvwaddstr(win[STA].p, 1, 2 + SPC, "S");
	mvwaddstr(win[STA].p, 0, 2 + 2 * SPC, "C");
	mvwaddstr(win[STA].p, 1, 2 + 2 * SPC, "N");
	mvwaddstr(win[STA].p, 0, 2 + 3 * SPC, "K");
	custom_remove_attr(win[STA].p, ATTR_HIGHEST);

	mvwaddstr(win[STA].p, 0, 2 + SMLSPC, _("Exit"));
	mvwaddstr(win[STA].p, 1, 2 + SMLSPC, _("General"));
	mvwaddstr(win[STA].p, 0, 2 + SPC + SMLSPC, _("Layout"));
	mvwaddstr(win[STA].p, 1, 2 + SPC + SMLSPC, _("Sidebar"));
	mvwaddstr(win[STA].p, 0, 2 + 2 * SPC + SMLSPC, _("Color"));
	mvwaddstr(win[STA].p, 1, 2 + 2 * SPC + SMLSPC, _("Notify"));
	mvwaddstr(win[STA].p, 0, 2 + 3 * SPC + SMLSPC, _("Keys"));

	wnoutrefresh(win[STA].p);
	wmove(win[STA].p, 0, 0);
	wins_doupdate();
}

static void layout_selection_bar(void)
{
	struct binding quit = { _("Exit"), KEY_GENERIC_QUIT };
	struct binding select = { _("Select"), KEY_GENERIC_SELECT };
	struct binding up = { _("Up"), KEY_MOVE_UP };
	struct binding down = { _("Down"), KEY_MOVE_DOWN };
	struct binding left = { _("Left"), KEY_MOVE_LEFT };
	struct binding right = { _("Right"), KEY_MOVE_RIGHT };
	struct binding help = { _("Help"), KEY_GENERIC_HELP };

	struct binding *bindings[] = {
		&quit, &select, &up, &down, &left, &right, &help
	};
	int bindings_size = ARRAY_SIZE(bindings);

	keys_display_bindings_bar(win[STA].p, bindings, bindings_size, 0,
				  bindings_size, NULL);
}

#define NBLAYOUTS     8
#define LAYOUTSPERCOL 2

/* Used to display available layouts in layout configuration menu. */
static void display_layout_config(struct window *lwin, int mark,
				  int cursor)
{
#define CURSOR			(32 | A_REVERSE)
#define MARK			88
#define LAYOUTH                  5
#define LAYOUTW                  9
	const char *box = "[ ]";
	const int BOXSIZ = strlen(box);
	const int NBCOLS = NBLAYOUTS / LAYOUTSPERCOL;
	const int COLSIZ = LAYOUTW + BOXSIZ + 1;
	const int XSPC = (lwin->w - NBCOLS * COLSIZ) / (NBCOLS + 1);
	const int XOFST = (lwin->w - NBCOLS * (XSPC + COLSIZ)) / 2;
	const int YSPC =
	    (lwin->h - 8 - LAYOUTSPERCOL * LAYOUTH) / (LAYOUTSPERCOL + 1);
	const int YOFST = (lwin->h - LAYOUTSPERCOL * (YSPC + LAYOUTH)) / 2;
	enum { YPOS, XPOS, NBPOS };
	int pos[NBLAYOUTS][NBPOS];
	const char *layouts[LAYOUTH][NBLAYOUTS] = {
		{"+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+"},
		{"|   | c |", "|   | t |", "| c |   |", "| t |   |", "|   | c |", "|   | a |", "| c |   |", "| a |   |"},
		{"| a +---+", "| a +---+", "+---+ a |", "|---+ a |", "| t +---+", "| t +---+", "+---+ t |", "+---+ t |"},
		{"|   | t |", "|   | c |", "| t |   |", "| c |   |", "|   | a |", "|   | c |", "| a |   |", "| c |   |"},
		{"+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+", "+---+---+"}
	};
	int i;

	for (i = 0; i < NBLAYOUTS; i++) {
		pos[i][YPOS] =
		    YOFST + (i % LAYOUTSPERCOL) * (YSPC + LAYOUTH);
		pos[i][XPOS] =
		    XOFST + (i / LAYOUTSPERCOL) * (XSPC + COLSIZ);
	}

	for (i = 0; i < NBLAYOUTS; i++) {
		int j;

		mvwaddstr(lwin->p, pos[i][YPOS] + 2, pos[i][XPOS], box);
		if (i == mark)
			custom_apply_attr(lwin->p, ATTR_HIGHEST);
		for (j = 0; j < LAYOUTH; j++) {
			mvwaddstr(lwin->p, pos[i][YPOS] + j,
				  pos[i][XPOS] + BOXSIZ + 1,
				  layouts[j][i]);
		}
		if (i == mark)
			custom_remove_attr(lwin->p, ATTR_HIGHEST);
	}
	mvwaddch(lwin->p, pos[mark][YPOS] + 2, pos[mark][XPOS] + 1, MARK);
	mvwaddch(lwin->p, pos[cursor][YPOS] + 2, pos[cursor][XPOS] + 1,
		 CURSOR);

	layout_selection_bar();
	wnoutrefresh(win[STA].p);
	wnoutrefresh(lwin->p);
	wins_doupdate();
	if (notify_bar())
		notify_update_bar();
}

/* Choose the layout */
void custom_layout_config(void)
{
	struct scrollwin hwin;
	struct window conf_win;
	int ch, mark, cursor, need_reset;
	const char *label = _("layout configuration");
	const char *help_text =
	    _("With this configuration menu, one can choose where panels will be\n"
	     "displayed inside calcurse screen. \n"
	     "It is possible to choose between eight different configurations.\n"
	     "\nIn the configuration representations, letters correspond to:\n\n"
	     "       'c' -> calendar panel\n\n"
	     "       'a' -> appointment panel\n\n"
	     "       't' -> todo panel\n\n");

	conf_win.p = NULL;
	custom_confwin_init(&conf_win, label);
	cursor = mark = wins_layout() - 1;
	display_layout_config(&conf_win, mark, cursor);
	clear();

	while ((ch =
		keys_getch(win[KEY].p, NULL, NULL)) != KEY_GENERIC_QUIT) {
		need_reset = 0;
		switch (ch) {
		case KEY_GENERIC_HELP:
			help_wins_init(&hwin, 0, 0,
				       (notify_bar())? row - 3 : row - 2,
				       col);
			mvwprintw(hwin.pad.p, 1, 0, help_text,
				  SBARMINWIDTH);
			hwin.total_lines = 7;
			wins_scrollwin_display(&hwin);
			wgetch(hwin.win.p);
			wins_scrollwin_delete(&hwin);
			need_reset = 1;
			break;
		case KEY_GENERIC_SELECT:
			mark = cursor;
			break;
		case KEY_MOVE_DOWN:
			if (cursor % LAYOUTSPERCOL < LAYOUTSPERCOL - 1)
				cursor++;
			break;
		case KEY_MOVE_UP:
			if (cursor % LAYOUTSPERCOL > 0)
				cursor--;
			break;
		case KEY_MOVE_LEFT:
			if (cursor >= LAYOUTSPERCOL)
				cursor -= LAYOUTSPERCOL;
			break;
		case KEY_MOVE_RIGHT:
			if (cursor < NBLAYOUTS - LAYOUTSPERCOL)
				cursor += LAYOUTSPERCOL;
			break;
		case KEY_GENERIC_CANCEL:
			need_reset = 1;
			break;
		}

		if (resize) {
			resize = 0;
			endwin();
			wins_refresh();
			curs_set(0);
			need_reset = 1;
		}

		if (need_reset)
			custom_confwin_init(&conf_win, label);

		display_layout_config(&conf_win, mark, cursor);
	}
	wins_set_layout(mark + 1);
	delwin(conf_win.p);
}

#undef NBLAYOUTS
#undef LAYOUTSPERCOL

/* Sidebar configuration screen. */
void custom_sidebar_config(void)
{
	struct scrollwin hwin;
	struct binding quit = { _("Exit"), KEY_GENERIC_QUIT };
	struct binding inc = { _("Width +"), KEY_MOVE_UP };
	struct binding dec = { _("Width -"), KEY_MOVE_DOWN };
	struct binding help = { _("Help"), KEY_GENERIC_HELP };
	struct binding *bindings[] = {
		&inc, &dec, &help, &quit
	};
	const char *help_text =
	    _("This configuration screen is used to change the width of the side bar.\n"
	     "The side bar is the part of the screen which contains two panels:\n"
	     "the calendar and, depending on the chosen layout, either the todo list\n"
	     "or the appointment list.\n\n"
	     "The side bar width can be up to 50%% of the total screen width, but\n"
	     "can't be smaller than %d characters wide.\n\n");
	int ch, bindings_size;

	bindings_size = ARRAY_SIZE(bindings);

	keys_display_bindings_bar(win[STA].p, bindings, bindings_size, 0,
				  bindings_size, NULL);
	wins_doupdate();

	while ((ch =
		keys_getch(win[KEY].p, NULL, NULL)) != KEY_GENERIC_QUIT) {
		switch (ch) {
		case KEY_MOVE_UP:
			wins_sbar_winc();
			break;
		case KEY_MOVE_DOWN:
			wins_sbar_wdec();
			break;
		case KEY_GENERIC_HELP:
			help_wins_init(&hwin, 0, 0,
				       (notify_bar())? row - 3 : row - 2,
				       col);
			mvwaddstr(hwin.pad.p, 1, 0, help_text);
			hwin.total_lines = 6;
			wins_scrollwin_display(&hwin);
			wgetch(hwin.win.p);
			wins_scrollwin_delete(&hwin);
			break;
		case KEY_RESIZE:
			break;
		default:
			continue;
		}

		if (resize) {
			resize = 0;
			wins_reset();
		} else {
			wins_reinit_panels();
			wins_update_border(FLAG_ALL);
			wins_update_panels(FLAG_ALL);
			keys_display_bindings_bar(win[STA].p, bindings,
						  bindings_size, 0,
						  bindings_size, NULL);
			wins_doupdate();
		}
	}
}

static void set_confwin_attr(struct window *cwin)
{
	cwin->h = (notify_bar())? row - 3 : row - 2;
	cwin->w = col;
	cwin->x = cwin->y = 0;
}

/*
 * Create a configuration window and initialize status and notification bar
 * (useful in case of window resize).
 */
void custom_confwin_init(struct window *confwin, const char *label)
{
	if (confwin->p) {
		erase_window_part(confwin->p, confwin->x, confwin->y,
				  confwin->x + confwin->w,
				  confwin->y + confwin->h);
		delwin(confwin->p);
	}

	wins_get_config();
	set_confwin_attr(confwin);
	confwin->p = newwin(confwin->h, col, 0, 0);
	box(confwin->p, 0, 0);
	wins_show(confwin->p, label);
	delwin(win[STA].p);
	win[STA].p =
	    newwin(win[STA].h, win[STA].w, win[STA].y, win[STA].x);
	keypad(win[STA].p, TRUE);
	if (notify_bar()) {
		notify_reinit_bar();
		notify_update_bar();
	}
}

static void color_selection_bar(void)
{
	struct binding quit = { _("Exit"), KEY_GENERIC_QUIT };
	struct binding select = { _("Select"), KEY_GENERIC_SELECT };
	struct binding nocolor = { _("No color"), KEY_GENERIC_CANCEL };
	struct binding up = { _("Up"), KEY_MOVE_UP };
	struct binding down = { _("Down"), KEY_MOVE_DOWN };
	struct binding left = { _("Left"), KEY_MOVE_LEFT };
	struct binding right = { _("Right"), KEY_MOVE_RIGHT };

	struct binding *bindings[] = {
		&quit, &nocolor, &up, &down, &left, &right, &select
	};
	int bindings_size = ARRAY_SIZE(bindings);

	keys_display_bindings_bar(win[STA].p, bindings, bindings_size, 0,
				  bindings_size, NULL);
}

/*
 * Used to display available colors in color configuration menu.
 * This is useful for window resizing.
 */
static void
display_color_config(struct window *cwin, int *mark_fore, int *mark_back,
		     int cursor, int theme_changed)
{
#define	SIZE 			(2 * (NBUSERCOLORS + 1))
#define DEFAULTCOLOR		255
#define DEFAULTCOLOR_EXT	-1
#define CURSOR			(32 | A_REVERSE)
#define MARK			88

	const char *fore_txt = _("Foreground");
	const char *back_txt = _("Background");
	const char *default_txt = _("(terminal's default)");
	const char *bar = "          ";
	const char *box = "[ ]";
	const unsigned Y = 3;
	const unsigned XOFST = 5;
	const unsigned YSPC = (cwin->h - 8) / (NBUSERCOLORS + 1);
	const unsigned BARSIZ = strlen(bar);
	const unsigned BOXSIZ = strlen(box);
	const unsigned XSPC = (cwin->w - 2 * BARSIZ - 2 * BOXSIZ - 6) / 3;
	const unsigned XFORE = XSPC;
	const unsigned XBACK = 2 * XSPC + BOXSIZ + XOFST + BARSIZ;
	enum { YPOS, XPOS, NBPOS };
	unsigned i;
	int pos[SIZE][NBPOS];
	short colr_fore, colr_back;
	int colr[SIZE] = {
		COLR_RED, COLR_GREEN, COLR_YELLOW, COLR_BLUE,
		COLR_MAGENTA, COLR_CYAN, COLR_DEFAULT,
		COLR_RED, COLR_GREEN, COLR_YELLOW, COLR_BLUE,
		COLR_MAGENTA, COLR_CYAN, COLR_DEFAULT
	};

	for (i = 0; i < NBUSERCOLORS + 1; i++) {
		pos[i][YPOS] = Y + YSPC * (i + 1);
		pos[NBUSERCOLORS + i + 1][YPOS] = Y + YSPC * (i + 1);
		pos[i][XPOS] = XFORE;
		pos[NBUSERCOLORS + i + 1][XPOS] = XBACK;
	}

	if (colorize) {
		if (theme_changed) {
			pair_content(colr[*mark_fore], &colr_fore, 0L);
			if (colr_fore == 255)
				colr_fore = -1;
			pair_content(colr[*mark_back], &colr_back, 0L);
			if (colr_back == 255)
				colr_back = -1;
			init_pair(COLR_CUSTOM, colr_fore, colr_back);
		} else {
			/* Retrieve the actual color theme. */
			pair_content(COLR_CUSTOM, &colr_fore, &colr_back);

			if ((colr_fore == DEFAULTCOLOR)
			    || (colr_fore == DEFAULTCOLOR_EXT)) {
				*mark_fore = NBUSERCOLORS;
			} else {
				for (i = 0; i < NBUSERCOLORS + 1; i++)
					if (colr_fore == colr[i])
						*mark_fore = i;
			}

			if ((colr_back == DEFAULTCOLOR)
			    || (colr_back == DEFAULTCOLOR_EXT)) {
				*mark_back = SIZE - 1;
			} else {
				for (i = 0; i < NBUSERCOLORS + 1; i++)
					if (colr_back ==
					    colr[NBUSERCOLORS + 1 + i])
						*mark_back =
						    NBUSERCOLORS + 1 + i;
			}
		}
	}

	/* color boxes */
	for (i = 0; i < SIZE - 1; i++) {
		mvwaddstr(cwin->p, pos[i][YPOS], pos[i][XPOS], box);
		wattron(cwin->p, COLOR_PAIR(colr[i]) | A_REVERSE);
		mvwaddstr(cwin->p, pos[i][YPOS], pos[i][XPOS] + XOFST,
			  bar);
		wattroff(cwin->p, COLOR_PAIR(colr[i]) | A_REVERSE);
	}

	/* Terminal's default color */
	i = SIZE - 1;
	mvwaddstr(cwin->p, pos[i][YPOS], pos[i][XPOS], box);
	wattron(cwin->p, COLOR_PAIR(colr[i]));
	mvwaddstr(cwin->p, pos[i][YPOS], pos[i][XPOS] + XOFST, bar);
	wattroff(cwin->p, COLOR_PAIR(colr[i]));
	mvwaddstr(cwin->p, pos[NBUSERCOLORS][YPOS] + 1,
		  pos[NBUSERCOLORS][XPOS] + XOFST, default_txt);
	mvwaddstr(cwin->p, pos[SIZE - 1][YPOS] + 1,
		  pos[SIZE - 1][XPOS] + XOFST, default_txt);

	custom_apply_attr(cwin->p, ATTR_HIGHEST);
	mvwaddstr(cwin->p, Y, XFORE + XOFST, fore_txt);
	mvwaddstr(cwin->p, Y, XBACK + XOFST, back_txt);
	custom_remove_attr(cwin->p, ATTR_HIGHEST);

	if (colorize) {
		mvwaddch(cwin->p, pos[*mark_fore][YPOS],
			 pos[*mark_fore][XPOS] + 1, MARK);
		mvwaddch(cwin->p, pos[*mark_back][YPOS],
			 pos[*mark_back][XPOS] + 1, MARK);
	}

	mvwaddch(cwin->p, pos[cursor][YPOS], pos[cursor][XPOS] + 1,
		 CURSOR);
	color_selection_bar();
	wnoutrefresh(win[STA].p);
	wnoutrefresh(cwin->p);
	wins_doupdate();
	if (notify_bar())
		notify_update_bar();
}

/* Color theme configuration. */
void custom_color_config(void)
{
	struct window conf_win;
	int ch, cursor, need_reset, theme_changed;
	int mark_fore, mark_back;
	const char *label = _("color theme");

	conf_win.p = 0;
	custom_confwin_init(&conf_win, label);
	mark_fore = NBUSERCOLORS;
	mark_back = SIZE - 1;
	cursor = 0;
	theme_changed = 0;
	display_color_config(&conf_win, &mark_fore, &mark_back, cursor,
			     theme_changed);
	clear();

	while ((ch =
		keys_getch(win[KEY].p, NULL, NULL)) != KEY_GENERIC_QUIT) {
		need_reset = 0;
		theme_changed = 0;

		switch (ch) {
		case KEY_GENERIC_SELECT:
			colorize = 1;
			need_reset = 1;
			theme_changed = 1;
			if (cursor > NBUSERCOLORS)
				mark_back = cursor;
			else
				mark_fore = cursor;
			break;

		case KEY_MOVE_DOWN:
			if (cursor < SIZE - 1)
				++cursor;
			break;

		case KEY_MOVE_UP:
			if (cursor > 0)
				--cursor;
			break;

		case KEY_MOVE_LEFT:
			if (cursor > NBUSERCOLORS)
				cursor -= (NBUSERCOLORS + 1);
			break;

		case KEY_MOVE_RIGHT:
			if (cursor <= NBUSERCOLORS)
				cursor += (NBUSERCOLORS + 1);
			break;

		case KEY_GENERIC_CANCEL:
			colorize = 0;
			need_reset = 1;
			break;
		}

		if (resize) {
			resize = 0;
			endwin();
			wins_refresh();
			curs_set(0);
			need_reset = 1;
		}

		if (need_reset)
			custom_confwin_init(&conf_win, label);

		display_color_config(&conf_win, &mark_fore, &mark_back,
				     cursor, theme_changed);
	}
	delwin(conf_win.p);
}

/* Prints the general options. */
static int print_general_options(WINDOW * win)
{
	enum {
		AUTO_SAVE,
		AUTO_GC,
		PERIODIC_SAVE,
		CONFIRM_QUIT,
		CONFIRM_DELETE,
		SYSTEM_DIAGS,
		PROGRESS_BAR,
		FIRST_DAY_OF_WEEK,
		OUTPUT_DATE_FMT,
		INPUT_DATE_FMT,
		NB_OPTIONS
	};
	const int XPOS = 1;
	const int YOFF = 3;
	int y;
	char *opt[NB_OPTIONS] = {
		"general.autosave = ",
		"general.autogc = ",
		"general.periodicsave = ",
		"general.confirmquit = ",
		"general.confirmdelete = ",
		"general.systemdialogs = ",
		"general.progressbar = ",
		"general.firstdayofweek = ",
		"format.outputdate = ",
		"format.inputdate = "
	};

	y = 0;
	mvwprintw(win, y, XPOS, "[1] %s      ", opt[AUTO_SAVE]);
	print_bool_option_incolor(win, conf.auto_save, y,
				  XPOS + 4 + strlen(opt[AUTO_SAVE]));
	mvwaddstr(win, y + 1, XPOS,
		  _("(if set to YES, automatic save is done when quitting)"));
	y += YOFF;
	mvwprintw(win, y, XPOS, "[2] %s      ", opt[AUTO_GC]);
	print_bool_option_incolor(win, conf.auto_gc, y,
				  XPOS + 4 + strlen(opt[AUTO_GC]));
	mvwaddstr(win, y + 1, XPOS,
		  _("(run the garbage collector when quitting)"));
	y += YOFF;
	mvwprintw(win, y, XPOS, "[3] %s      ", opt[PERIODIC_SAVE]);
	custom_apply_attr(win, ATTR_HIGHEST);
	mvwprintw(win, y, XPOS + 4 + strlen(opt[PERIODIC_SAVE]), "%d",
		  conf.periodic_save);
	custom_remove_attr(win, ATTR_HIGHEST);
	mvwaddstr(win, y + 1, XPOS,
		  _("(if not null, automatically save data every 'periodic_save' "
		   "minutes)"));
	y += YOFF;
	mvwprintw(win, y, XPOS, "[4] %s      ", opt[CONFIRM_QUIT]);
	print_bool_option_incolor(win, conf.confirm_quit, y,
				  XPOS + 4 + strlen(opt[CONFIRM_QUIT]));
	mvwaddstr(win, y + 1, XPOS,
		  _("(if set to YES, confirmation is required before quitting)"));
	y += YOFF;
	mvwprintw(win, y, XPOS, "[5] %s      ", opt[CONFIRM_DELETE]);
	print_bool_option_incolor(win, conf.confirm_delete, y,
				  XPOS + 4 + strlen(opt[CONFIRM_DELETE]));
	mvwaddstr(win, y + 1, XPOS,
		  _("(if set to YES, confirmation is required "
		    "before deleting an event)"));
	y += YOFF;
	mvwprintw(win, y, XPOS, "[6] %s      ", opt[SYSTEM_DIAGS]);
	print_bool_option_incolor(win, conf.system_dialogs, y,
				  XPOS + 4 + strlen(opt[SYSTEM_DIAGS]));
	mvwaddstr(win, y + 1, XPOS,
		  _("(if set to YES, messages about loaded "
		    "and saved data will be displayed)"));
	y += YOFF;
	mvwprintw(win, y, XPOS, "[7] %s      ", opt[PROGRESS_BAR]);
	print_bool_option_incolor(win, conf.progress_bar, y,
				  XPOS + 4 + strlen(opt[PROGRESS_BAR]));
	mvwaddstr(win, y + 1, XPOS,
		  _("(if set to YES, progress bar will be displayed "
		    "when saving data)"));
	y += YOFF;
	mvwprintw(win, y, XPOS, "[8] %s      ", opt[FIRST_DAY_OF_WEEK]);
	custom_apply_attr(win, ATTR_HIGHEST);
	mvwaddstr(win, y, XPOS + 4 + strlen(opt[FIRST_DAY_OF_WEEK]),
		  ui_calendar_week_begins_on_monday()? _("Monday") :
		  _("Sunday"));
	custom_remove_attr(win, ATTR_HIGHEST);
	mvwaddstr(win, y + 1, XPOS,
		  _("(specifies the first day of week in the calendar view)"));
	y += YOFF;
	mvwprintw(win, y, XPOS, "[9] %s      ", opt[OUTPUT_DATE_FMT]);
	custom_apply_attr(win, ATTR_HIGHEST);
	mvwaddstr(win, y, XPOS + 4 + strlen(opt[OUTPUT_DATE_FMT]),
		  conf.output_datefmt);
	custom_remove_attr(win, ATTR_HIGHEST);
	mvwaddstr(win, y + 1, XPOS,
		  _("(Format of the date to be displayed in non-interactive mode)"));
	y += YOFF;
	mvwprintw(win, y, XPOS, "[0] %s      ", opt[INPUT_DATE_FMT]);
	custom_apply_attr(win, ATTR_HIGHEST);
	mvwprintw(win, y, XPOS + 4 + strlen(opt[INPUT_DATE_FMT]), "%d",
		  conf.input_datefmt);
	custom_remove_attr(win, ATTR_HIGHEST);
	mvwaddstr(win, y + 1, XPOS,
		  _("(Format to be used when entering a date: "));
	mvwprintw(win, y + 2, XPOS, " (1) %s, (2) %s, (3) %s, (4) %s)",
		  datefmt_str[0], datefmt_str[1], datefmt_str[2],
		  datefmt_str[3]);

	return y + YOFF;
}

void custom_set_swsiz(struct scrollwin *sw)
{
	sw->win.x = 0;
	sw->win.y = 0;
	sw->win.h = (notify_bar())? row - 3 : row - 2;
	sw->win.w = col;

	sw->pad.x = 1;
	sw->pad.y = 3;
	sw->pad.h = BUFSIZ;
	sw->pad.w = col - 2 * sw->pad.x - 1;
}

/* General configuration. */
void custom_general_config(void)
{
	struct scrollwin cwin;
	const char *number_str =
	    _("Enter an option number to change its value");
	const char *keys =
	    _("(Press '^P' or '^N' to move up or down, 'Q' to quit)");
	const char *output_datefmt_str =
	    _("Enter the date format (see 'man 3 strftime' for possible formats) ");
	const char *input_datefmt_prefix = _("Enter the date format: ");
	const char *periodic_save_str =
	    _("Enter the delay, in minutes, between automatic saves (0 to disable) ");
	int ch;
	int val;
	char *buf;

	clear();
	custom_set_swsiz(&cwin);
	cwin.label = _("general options");
	wins_scrollwin_init(&cwin);
	wins_show(cwin.win.p, cwin.label);
	status_mesg(number_str, keys);
	cwin.total_lines = print_general_options(cwin.pad.p);
	wins_scrollwin_display(&cwin);

	buf = mem_malloc(BUFSIZ);
	while ((ch = wgetch(win[KEY].p)) != 'q') {
		buf[0] = '\0';

		switch (ch) {
		case CTRL('N'):
			wins_scrollwin_down(&cwin, 1);
			break;
		case CTRL('P'):
			wins_scrollwin_up(&cwin, 1);
			break;
		case '1':
			conf.auto_save = !conf.auto_save;
			break;
		case '2':
			conf.auto_gc = !conf.auto_gc;
			break;
		case '3':
			status_mesg(periodic_save_str, "");
			if (updatestring(win[STA].p, &buf, 0, 1) == 0) {
				val = atoi(buf);
				if (val >= 0)
					conf.periodic_save = val;
				if (conf.periodic_save > 0)
					io_start_psave_thread();
				else if (conf.periodic_save == 0)
					io_stop_psave_thread();
			}
			status_mesg(number_str, keys);
			break;
		case '4':
			conf.confirm_quit = !conf.confirm_quit;
			break;
		case '5':
			conf.confirm_delete = !conf.confirm_delete;
			break;
		case '6':
			conf.system_dialogs = !conf.system_dialogs;
			break;
		case '7':
			conf.progress_bar = !conf.progress_bar;
			break;
		case '8':
			ui_calendar_change_first_day_of_week();
			break;
		case '9':
			status_mesg(output_datefmt_str, "");
			strncpy(buf, conf.output_datefmt,
				strlen(conf.output_datefmt) + 1);
			if (updatestring(win[STA].p, &buf, 0, 1) == 0) {
				strncpy(conf.output_datefmt, buf,
					strlen(buf) + 1);
			}
			status_mesg(number_str, keys);
			break;
		case '0':
			val = status_ask_simplechoice(input_datefmt_prefix,
						      datefmt_str,
						      DATE_FORMATS);
			if (val != -1)
				conf.input_datefmt = val;
			break;
		}

		if (resize) {
			resize = 0;
			wins_reset();
			wins_scrollwin_delete(&cwin);
			custom_set_swsiz(&cwin);
			wins_scrollwin_init(&cwin);
			wins_show(cwin.win.p, cwin.label);
			cwin.first_visible_line = 0;
			delwin(win[STA].p);
			win[STA].p =
			    newwin(win[STA].h, win[STA].w, win[STA].y,
				   win[STA].x);
			keypad(win[STA].p, TRUE);
			if (notify_bar()) {
				notify_reinit_bar();
				notify_update_bar();
			}
		}

		status_mesg(number_str, keys);
		cwin.total_lines = print_general_options(cwin.pad.p);
		wins_scrollwin_display(&cwin);
	}
	mem_free(buf);
	wins_scrollwin_delete(&cwin);
}

static void
print_key_incolor(WINDOW * win, const char *option, int pos_y, int pos_x)
{
	const int color = ATTR_HIGHEST;

	RETURN_IF(!option, _("Undefined option!"));
	custom_apply_attr(win, color);
	mvwprintw(win, pos_y, pos_x, "%s ", option);
	custom_remove_attr(win, color);
}

static int
print_keys_bindings(WINDOW * win, int selected_row, int selected_elm,
		    int yoff)
{
	const int XPOS = 1;
	const int EQUALPOS = 23;
	const int KEYPOS = 25;
	int noelm, action, y;

	noelm = y = 0;
	for (action = 0; action < NBKEYS; action++) {
		char actionstr[BUFSIZ];
		int nbkeys;

		nbkeys = keys_action_count_keys(action);
		snprintf(actionstr, BUFSIZ, "%s", keys_get_label(action));
		if (action == selected_row)
			custom_apply_attr(win, ATTR_HIGHEST);
		mvwprintw(win, y, XPOS, "%s ", actionstr);
		mvwaddstr(win, y, EQUALPOS, "=");
		if (nbkeys == 0)
			mvwaddstr(win, y, KEYPOS, _("undefined"));
		if (action == selected_row)
			custom_remove_attr(win, ATTR_HIGHEST);
		if (nbkeys > 0) {
			if (action == selected_row) {
				const char *key;
				int pos;

				pos = KEYPOS;
				while ((key =
					keys_action_nkey(action,
							 noelm)) != NULL) {
					if (noelm == selected_elm)
						print_key_incolor(win, key,
								  y, pos);
					else
						mvwprintw(win, y, pos,
							  "%s ", key);
					noelm++;
					pos += strlen(key) + 1;
				}
			} else {
				mvwaddstr(win, y, KEYPOS,
					  keys_action_allkeys(action));
			}
		}
		y += yoff;
	}

	return noelm;
}

static void custom_keys_config_bar(void)
{
	struct binding quit = { _("Exit"), KEY_GENERIC_QUIT };
	struct binding info = { _("Key info"), KEY_GENERIC_HELP };
	struct binding add = { _("Add key"), KEY_ADD_ITEM };
	struct binding del = { _("Del key"), KEY_DEL_ITEM };
	struct binding up = { _("Up"), KEY_MOVE_UP };
	struct binding down = { _("Down"), KEY_MOVE_DOWN };
	struct binding left = { _("Prev Key"), KEY_MOVE_LEFT };
	struct binding right = { _("Next Key"), KEY_MOVE_RIGHT };

	struct binding *bindings[] = {
		&quit, &info, &add, &del, &up, &down, &left, &right
	};
	int bindings_size = ARRAY_SIZE(bindings);

	keys_display_bindings_bar(win[STA].p, bindings, bindings_size, 0,
				  bindings_size, NULL);
}

void custom_keys_config(void)
{
	struct scrollwin kwin;
	int selrow, selelm, firstrow, lastrow, nbrowelm, nbdisplayed;
	int keyval, used, not_recognized;
	const char *keystr;
	WINDOW *grabwin;
	const int LINESPERKEY = 2;
	const int LABELLINES = 3;

	clear();
	custom_set_swsiz(&kwin);
	nbdisplayed = (kwin.win.h - LABELLINES) / LINESPERKEY;
	kwin.label = _("keys configuration");
	wins_scrollwin_init(&kwin);
	wins_show(kwin.win.p, kwin.label);
	custom_keys_config_bar();
	selrow = selelm = 0;
	nbrowelm =
	    print_keys_bindings(kwin.pad.p, selrow, selelm, LINESPERKEY);
	kwin.total_lines = NBKEYS * LINESPERKEY;
	wins_scrollwin_display(&kwin);
	firstrow = 0;
	lastrow = firstrow + nbdisplayed - 1;
	for (;;) {
		int ch;

		ch = keys_getch(win[KEY].p, NULL, NULL);
		switch (ch) {
		case KEY_MOVE_UP:
			if (selrow > 0) {
				selrow--;
				selelm = 0;
				if (selrow == firstrow) {
					firstrow--;
					lastrow--;
					wins_scrollwin_up(&kwin,
							  LINESPERKEY);
				}
			}
			break;
		case KEY_MOVE_DOWN:
			if (selrow < NBKEYS - 1) {
				selrow++;
				selelm = 0;
				if (selrow == lastrow) {
					firstrow++;
					lastrow++;
					wins_scrollwin_down(&kwin,
							    LINESPERKEY);
				}
			}
			break;
		case KEY_MOVE_LEFT:
			if (selelm > 0)
				selelm--;
			break;
		case KEY_MOVE_RIGHT:
			if (selelm < nbrowelm - 1)
				selelm++;
			break;
		case KEY_GENERIC_HELP:
			keys_popup_info(selrow);
			break;
		case KEY_ADD_ITEM:
#define WINROW 10
#define WINCOL 50
			do {
				used = 0;
				grabwin =
				    popup(WINROW, WINCOL,
					  (row - WINROW) / 2,
					  (col - WINCOL) / 2,
					  _("Press the key you want to assign to:"),
					  keys_get_label(selrow), 0);
				keyval = wgetch(grabwin);

				/* First check if this key would be recognized by calcurse. */
				if (keys_str2int(keys_int2str(keyval)) ==
				    -1) {
					not_recognized = 1;
					WARN_MSG(_("This key is not yet recognized by calcurse, "
						  "please choose another one."));
					werase(kwin.pad.p);
					nbrowelm =
					    print_keys_bindings(kwin.pad.p,
								selrow,
								selelm,
								LINESPERKEY);
					wins_scrollwin_display(&kwin);
					continue;
				} else {
					not_recognized = 0;
				}

				/* Is the binding used by this action already? If so, just end the reassignment */
				if (selrow == keys_get_action(keyval)) {
					delwin(grabwin);
					break;
				}

				used = keys_assign_binding(keyval, selrow);
				if (used) {
					enum key action;

					action = keys_get_action(keyval);
					WARN_MSG(_("This key is already in use for %s, "
						  "please choose another one."),
						 keys_get_label(action));
					werase(kwin.pad.p);
					nbrowelm =
					    print_keys_bindings(kwin.pad.p,
								selrow,
								selelm,
								LINESPERKEY);
					wins_scrollwin_display(&kwin);
				}
				delwin(grabwin);
			}
			while (used || not_recognized);
			nbrowelm++;
			if (selelm < nbrowelm - 1)
				selelm++;
#undef WINROW
#undef WINCOL
			break;
		case KEY_DEL_ITEM:
			keystr = keys_action_nkey(selrow, selelm);
			keyval = keys_str2int(keystr);
			keys_remove_binding(keyval, selrow);
			nbrowelm--;
			if (selelm > 0 && selelm <= nbrowelm)
				selelm--;
			break;
		case KEY_GENERIC_QUIT:
			if (keys_check_missing_bindings() != 0) {
				WARN_MSG(_("Some actions do not have any associated "
					  "key bindings!"));
			}
			wins_scrollwin_delete(&kwin);
			return;
		}
		custom_keys_config_bar();
		werase(kwin.pad.p);
		nbrowelm =
		    print_keys_bindings(kwin.pad.p, selrow, selelm,
					LINESPERKEY);
		wins_scrollwin_display(&kwin);
	}
}

void custom_config_main(void)
{
	const char *no_color_support =
	    _("Sorry, colors are not supported by your terminal\n"
	      "(Press [ENTER] to continue)");
	int ch;
	int old_layout;

	custom_config_bar();
	while ((ch = wgetch(win[KEY].p)) != 'q') {
		switch (ch) {
		case 'C':
		case 'c':
			if (has_colors()) {
				custom_color_config();
			} else {
				colorize = 0;
				wins_erase_status_bar();
				mvwaddstr(win[STA].p, 0, 0,
					  no_color_support);
				wgetch(win[KEY].p);
			}
			break;
		case 'L':
		case 'l':
			old_layout = wins_layout();
			custom_layout_config();
			if (wins_layout() != old_layout)
				wins_reset();
			break;
		case 'G':
		case 'g':
			custom_general_config();
			break;
		case 'N':
		case 'n':
			notify_config_bar();
			break;
		case 'K':
		case 'k':
			custom_keys_config();
			break;
		case 's':
		case 'S':
			custom_sidebar_config();
			break;
		default:
			continue;
		}
		wins_update(FLAG_ALL);
		wins_erase_status_bar();
		custom_config_bar();
	}
}
