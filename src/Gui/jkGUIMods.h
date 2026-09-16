#ifndef _JKGUI_MODS_H
#define _JKGUI_MODS_H

#include "types.h"

#include <locale.h>

#include "SDL2_helper.h"

#include "external/fcaseopen/fcaseopen.h"

void jkGuiMods_Startup();
void jkGuiMods_Shutdown();

void jkGuiMods_Show();
void jkGuiMods_PopulateEntries(Darray *pListDisplayed, jkGuiElement *element);

#endif // _JKGUI_MODS_H