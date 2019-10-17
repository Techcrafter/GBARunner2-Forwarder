/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "nds_loader_arm9.h"
#include "inifile.h"
#include "perGameSettings.h"
#include "fileCopy.h"
#include "flashcard.h"

const char* settingsinipath = "sd:/_nds/TWiLightMenu/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

std::string ndsPath;
std::string romfolder;
std::string filename;

static std::string romPath;

/*
 * Remove trailing slashes from a pathname, if present.
 * @param path Pathname to modify.
 */
void RemoveTrailingSlashes(std::string& path)
{
	while (!path.empty() && path[path.size()-1] == '/') {
		path.resize(path.size()-1);
	}
}

static int launchType = 1;	// 0 = Slot-1, 1 = SD/Flash card, 2 = SD/Flash card (Direct boot)
static bool useBootstrap = true;
static bool bootstrapFile = false;
static bool homebrewBootstrap = true;

TWL_CODE void LoadSettings(void) {
	// GUI
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", 0);
	homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", 0);
	// nds-bootstrap
	CIniFile bootstrapini( bootstrapinipath );
	ndsPath = bootstrapini.GetString( "NDS-BOOTSTRAP", "NDS_PATH", "");
}

using namespace std;

void stop (void) {
	while (1) {
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

TWL_CODE int lastRunROM() {
	LoadSettings();
	std::string bootstrapPath = (bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");

	std::vector<char*> argarray;
	argarray.push_back(strdup(bootstrapPath.c_str()));
	argarray.at(0) = (char*)bootstrapPath.c_str();

			CIniFile bootstrapini("sd:/_nds/nds-bootstrap.ini");
			bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", "sd:/_nds/GBARunner2_arm7dldi_dsi.nds");
			bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "");
			bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", "");
			bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", bstrap_language);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", 1);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
			bootstrapini.SaveIniFile( "sd:/_nds/nds-bootstrap.ini");
			int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], false, true, false, true, true);
			iprintf ("Start failed. Error %i\n", err);
	return -1;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	extern char *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();

	if (!fatInitDefault()) {
		consoleDemoInit();
		printf("fatInitDefault failed!");
		stop();
	}

	fifoWaitValue32(FIFO_USER_06);

	int err = lastRunROM();
	consoleDemoInit();
	iprintf ("Start failed. Error %i", err);
	stop();

	return 0;
}
