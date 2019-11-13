#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <switch.h>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iterator>
#include <dirent.h>
#include <cstdio>
#include <filesystem>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <vector>
#include <stdlib.h>
#include <stdlib.h>
#include "FileSystem.hpp"

using namespace std;

//traduction
bool isSpanish()
{
			setInitialize();
			u64 lcode = 0;
			s32 lang = 1;
			setGetSystemLanguage(&lcode);
			setMakeLanguage(lcode, &lang);
				switch(lang)
				{
					case 5:
					case 14:
					return true;
					   break;
					default:
					return false;
						break;
				}
			setsysExit();
		return false;
}

	bool fileExists(const char* path)
	{
		FILE* f = fopen(path, "rb");
		if (f)
		{
			fclose(f);
			return true;
		}
		return false;
	}


	bool led_on(void)
	{
				Result rc=0;
					size_t i;
					size_t total_entries;
					u64 UniquePadIds[2];
					HidsysNotificationLedPattern pattern;
					hidsysExit();
					rc = hidsysInitialize();
					if (R_FAILED(rc)) {
						printf("hidsysInitialize(): 0x%x\n", rc);
					}


					memset(&pattern, 0, sizeof(pattern));
					// Setup Breathing effect pattern data.
					pattern.baseMiniCycleDuration = 0x8;             // 100ms.
					pattern.totalMiniCycles = 0x2;                   // 3 mini cycles. Last one 12.5ms.
					pattern.totalFullCycles = 0x6;                   // Repeat Time.
					pattern.startIntensity = 0x2;                    // 13%.

					pattern.miniCycles[0].ledIntensity = 0xF;        // 100%.
					pattern.miniCycles[0].transitionSteps = 0xF;     // 15 steps. Transition time 1.5s.
					pattern.miniCycles[0].finalStepDuration = 0x0;   // Forced 12.5ms.
					pattern.miniCycles[1].ledIntensity = 0x2;        // 13%.
					pattern.miniCycles[1].transitionSteps = 0xF;     // 15 steps. Transition time 1.5s.
					pattern.miniCycles[1].finalStepDuration = 0x0;   // Forced 12.5ms.
					
					rc = hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, UniquePadIds, 2, &total_entries);

					if (R_SUCCEEDED(rc)) {
						for(i=0; i<total_entries; i++) { // System will skip sending the subcommand to controllers where this isn't available.
							rc = hidsysSetNotificationLedPattern(&pattern, UniquePadIds[i]);
						}
					}
				hidsysExit();
	return 0;
	}

bool install()
{
		//Initialize proc
		printf("\x1b[32;1m*\x1b[0m Initialize proc\n");
		consoleUpdate(NULL);
		fsInitialize();
		pmdmntInitialize();
		pmshellInitialize();
		
		mkdir("sdmc:/BCAT", 0777);
		pmshellTerminateProcessByTitleId(0x010000000000000C);
		pmshellTerminateProcessByTitleId(0x80000000000000A1);
		pmshellTerminateProcessByTitleId(0x80000000000000A2);

		//Locking 80000000000000A1
		printf("\x1b[32;1m*\x1b[0m Locking 80000000000000A1\n");
		consoleUpdate(NULL);
		FsFileSystem acc;
		fsMount_SystemSaveData(&acc, 0x80000000000000A1);
		fsdevMountDevice("save", acc);
		mkdir("sdmc:/BCAT/80000000000000A1", 0777);
		fs::copyDirToDir("save:/","sdmc:/BCAT/80000000000000A1/");
		fs::DeleteDir("save:/");
		mkdir("save:/current.msgpack", 0777);
		rename("save:/posted", "save:/p");
		fs::WriteFile("save:/posted", "LOCK");
		rename("save:/data", "save:/d");
		fs::WriteFile("save:/data", "LOCK");
		fsdevCommitDevice("save");
		fsdevUnmountDevice("save"); 
		fsFsClose(&acc);

		//Locking 80000000000000A2
		printf("\x1b[32;1m*\x1b[0m Locking 80000000000000A2\n");
		consoleUpdate(NULL);
		fsMount_SystemSaveData(&acc, 0x80000000000000A2);
		fsdevMountDevice("save", acc);
		mkdir("sdmc:/BCAT/80000000000000A2", 0777);
		fs::copyDirToDir("save:/","sdmc:/BCAT/80000000000000A2/");
		fs::DeleteDir("save:/");
		mkdir("save:/current.msgpack", 0777);
		rename("save:/posted", "save:/p");
		fs::WriteFile("save:/posted", "LOCK");
		rename("save:/data", "save:/d");
		fs::WriteFile("save:/data", "LOCK");
		fsdevCommitDevice("save");
		fsdevUnmountDevice("save"); 
		fsFsClose(&acc);

		//exit proc
		printf("\x1b[32;1m*\x1b[0m exit proc\n");
		consoleUpdate(NULL);
		pmdmntExit();
		pmshellExit();
		fsExit();
		socketExit();
		fsdevUnmountAll();
		led_on();	
		bpcInitialize();
		bpcRebootSystem();
		bpcExit();
return 0;
}

int main(int argc, char **argv)
{
	//Keys
	u32 minus = 0;
	u32 more = 0;
	u32 LT = 0;
	u32 RT = 0;
	while (appletMainLoop())
	{

	    hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
		
		minus = 0;
		more = 0;
		LT = 0;
		RT = 0;
		if (kHeld & KEY_ZL)
			LT = 2;
		if (kHeld & KEY_ZR)
			RT = 2;
		if (kHeld & KEY_MINUS)
			minus = 2;
		if (kHeld & KEY_PLUS)
			more = 2;

		consoleInit(NULL);
			printf("\x1b[32;1m*\x1b[0m %s v%s Kronos2308, BCAT LOCKER\n",TITLE, VERSION);
				if(isSpanish())
				{
					printf("\n\x1b[30;1m SE BORRANRAN Y BLOKEARAN LOS LOGS\x1b[0m\n");
					printf("\n\x1b[30;1m SI NO SABES LO QUE HACES, PRESIONA B PARA ABORTAR \x1b[0m\n");
					printf("\n\n\x1b[30;1m-------- ---------------- --------\x1b[0m\n\n");
					printf("\x1b[30;1m PULSA \x1b[3%u;1m -\x1b[3%u;1m +\x1b[3%u;1m ZR\x1b[3%u;1m ZL \x1b[0m \x1b[30;1m PARA PROCEDER\n\n",minus,more,RT,LT);
				}else{
					printf("\n\x1b[30;1m THE LOGS WILL BE DELETED AND LOCKED  \x1b[0m\n");
					printf("\n\x1b[30;1m IF YOU DON'T KNOW WHAT YOU DO, PRESS B FOR ABORT \x1b[0m\n");
					printf("\n\n\x1b[30;1m-------- ---------------- --------\x1b[0m\n\n");
					printf("\x1b[30;1m PRESS \x1b[3%u;1m -\x1b[3%u;1m +\x1b[3%u;1m ZR\x1b[3%u;1m ZL \x1b[0m \x1b[30;1m TO PROCEED\n\n",minus,more,RT,LT);
				}
		consoleUpdate(NULL);
		
		
		if ((kDown & KEY_ZL || kDown & KEY_ZR || kDown & KEY_MINUS || kDown & KEY_PLUS) && (kHeld & KEY_ZL && kHeld & KEY_ZR && kHeld & KEY_MINUS && kHeld & KEY_PLUS))
		{
			install();
			break;
		}

		if (kDown & KEY_A || kDown & KEY_B || kDown & KEY_Y || kDown & KEY_X)
		{
			break;
		}
	}	

	//Cansel
	fsExit();
    socketExit();
    fsdevUnmountAll();
	pmshellExit();
	consoleExit(NULL);
	return 0;
}