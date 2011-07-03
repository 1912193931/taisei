
#ifndef OPTMENU_H
#define OPTMENU_H

#include "menu.h"

void create_options_menu(MenuData *m);

void draw_options_menu(MenuData *m);
int options_menu_loop(MenuData *m);

typedef int (*BindingGetter)(void*);
typedef int (*BindingSetter)(void*, int);

typedef struct OptionBinding {
	char **values;
	int valcount;
	BindingGetter getter;
	BindingSetter setter;
	int selected;
	int configentry;
	int enabled;
	char *optname;
} OptionBinding;

#endif

