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
FsFileSystem loc;

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
	if (R_FAILED(rc)) {printf("hidsysInitialize(): 0x%x\n", rc);}

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
	if (R_SUCCEEDED(rc)) 
	{for(i=0; i<total_entries; i++) { rc = hidsysSetNotificationLedPattern(&pattern, UniquePadIds[i]);}}
	
	hidsysExit();
	return 0;
}

void espera(u32 timeS)
{
	u32 cout = 0;
	while (appletMainLoop()){cout++; if(cout >= timeS*1000000) break;}//1000000
}

void Lock(u64 TID)
{
	fsInitialize();
	char id[160];
	printf("\x1b[32;1m*\x1b[0m Locking %016lX\n",TID);
	consoleUpdate(NULL);


	mkdir("sdmc:/BCAT", 0777);
	mkdir("sdmc:/BCAT/Antes", 0777);
	mkdir("sdmc:/BCAT/Despues", 0777);
	fsMount_SystemSaveData(&loc, TID);
	fsdevMountDevice("save", loc);
	
	sprintf(id,"sdmc:/BCAT/Antes/%016lX/",TID);
	mkdir(id, 0777);
	fs::copyDirToDir("save:/",id);

	fs::DeleteDir("save:/");
	mkdir("save:/current.msgpack", 0777);
	rename("save:/posted", "save:/current.msgpack/posted");
	rename("save:/data", "save:/current.msgpack/data");
	fs::WriteFile("save:/posted", "LOCK");
	fs::WriteFile("save:/data", "LOCK");
	
	sprintf(id,"sdmc:/BCAT/Despues/%016lX/",TID);
	mkdir(id, 0777);
	fs::copyDirToDir("save:/",id);
	
	fsdevCommitDevice("save");
	fsdevUnmountDevice("save"); 
	fsFsClose(&loc);

}

void install()
{
	
	printf("\x1b[32;1m*\x1b[0m Klling BCAT proc\n");
	consoleUpdate(NULL);
	Result rc;
	pmshellInitialize();
//	pmshellTerminateProcessByTitleId(0x010000000000000C);
	if (R_FAILED(rc = pmshellTerminateProcessByTitleId(0x010000000000000C)))
    printf("\x1b[31;1m*\x1b[0m Error Klling BCAT\n");
	else{
	Lock(0x80000000000000A1);
	Lock(0x80000000000000A2);
	printf("\x1b[32;1m*\x1b[0m W COMPLETE\n");
	consoleUpdate(NULL);
	led_on();
	espera(5);
	bpcInitialize();
	bpcRebootSystem();
	bpcExit();
	}

	consoleUpdate(NULL);
	//exit proc
	pmshellExit();
	fsExit();
	socketExit();
	fsdevUnmountAll();
	espera(5);
}

void uninstall()
{
		//Initialize proc
		printf("\x1b[32;1m*\x1b[0m Klling BCAT proc\n");
		consoleUpdate(NULL);
		Result rc;
		pmshellInitialize();
		if (R_FAILED(rc = pmshellTerminateProcessByTitleId(0x010000000000000C)))
		printf("\x1b[31;1m*\x1b[0m Error Klling BCAT\n");
			
		//Just in Case
		pmshellTerminateProcessByTitleId(0x010000000000000E);//friends
		pmshellTerminateProcessByTitleId(0x010000000000001E);//account
		pmshellTerminateProcessByTitleId(0x010000000000001F);//ns
		pmshellTerminateProcessByTitleId(0x0100000000000020);//nfc
		pmshellTerminateProcessByTitleId(0x0100000000000022);//capsrv
		pmshellTerminateProcessByTitleId(0x0100000000000024);//ssl
		pmshellTerminateProcessByTitleId(0x0100000000000025);//nim
		pmshellTerminateProcessByTitleId(0x010000000000002B);//erpt
		pmshellTerminateProcessByTitleId(0x010000000000002E);//pctl
		pmshellTerminateProcessByTitleId(0x010000000000002F);//npns
		pmshellTerminateProcessByTitleId(0x0100000000000030);//eupld
		pmshellTerminateProcessByTitleId(0x0100000000000033);//es
		pmshellTerminateProcessByTitleId(0x0100000000000036);//creport
		pmshellTerminateProcessByTitleId(0x010000000000003A);//migration
		pmshellTerminateProcessByTitleId(0x010000000000003E);//olsc
		pmshellTerminateProcessByTitleId(0x0100000000001000);//qlaunch - make freeze?
		pmshellTerminateProcessByTitleId(0x0100000000001009);//miiEdit


		printf("\x1b[32;1m*\x1b[0m Mount System\n");
		consoleUpdate(NULL);
		FsFileSystem mySystem;
		fsOpenBisFileSystem(&mySystem, FsBisStorageId_System, "");
		fsdevMountDevice("sytem33", mySystem);
		
		//uninstall
		printf("\x1b[32;1m*\x1b[0m Uninstalling Lock-Logs\n");
		consoleUpdate(NULL);
		remove("sytem33:/save/80000000000000A1");
		remove("sytem33:/save/80000000000000A2");
		
		//umount system
		printf("\x1b[32;1m*\x1b[0m umount system\n");
		consoleUpdate(NULL);
		fsdevCommitDevice("sytem33");
		fsdevUnmountDevice("sytem33");
		fsFsClose(&mySystem);
		
		printf("\x1b[32;1m*\x1b[0m W COMPLETE\n");
		consoleUpdate(NULL);
		espera(5);
	bpcInitialize();
	bpcRebootSystem();
	bpcExit();
}

int main(int argc, char **argv)
{
	while (appletMainLoop())
	{
	    hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

		consoleInit(NULL);
			printf("\x1b[32;1m*\x1b[0m %s v%s Kronos2308, BCAT LOCKER\n",TITLE, VERSION);
				if(isSpanish())
				{
					printf("\n\x1b[30;1m SE BORRARAN Y BLOKEARAN LOS LOGS\x1b[0m\n");
					printf("\n\x1b[30;1m ASI SE PODRA JUGAR ONLINE SIN SER BANEADO\x1b[0m\n");
					printf("\n\x1b[30;1m RECUERDA SOLO USAR JUEGOS ORIGINALES\x1b[0m\n");
					printf("\n\x1b[30;1m LUEGO DE INSTALAR QUITA LOS DNS E INCOGNITO\x1b[0m\n");
					printf("\n\x1b[30;1m No me hago responsable de:\n Baneos, Bricks o Aniquilasion Nuclear\x1b[0m\n");
					printf("\n\n\x1b[30;1m-------- ---------------- --------\x1b[0m\n\n");
					printf("\n\x1b[30;1m PULSA \x1b[32;1mA\x1b[30;1m PARA INTALAR\n");
					printf("\n\x1b[30;1m PULSA \x1b[32;1mB\x1b[30;1m PARA SALIR \x1b[0m\n\n");
					if(fs::DirExists("sdmc:/BCAT"))
					printf("\n\x1b[30;1m PULSA \x1b[32;1m- +\x1b[30;1m PARA DESINTALAR\n\n");
				}else{
					printf("\n\x1b[30;1m THE LOGS WILL BE DELETED AND LOCKED  \x1b[0m\n");
					printf("\n\n\x1b[30;1m SO YOU CAN PLAY ONLINE WITHOUT BEING BANED \x1b[0m\n\n");
					printf("\n\n\x1b[30;1m REMEMBER, ONLY USE ORIGINAL GAMES\x1b[0m\n\n");
					printf("\n\n\x1b[30;1m AFTER INSTALLING REMOVE THE DNS AND INCOGNITO\x1b[0m\n\n");
					printf("\n\n\x1b[30;1m I am not responsable for:\n Bans, Bricks or full Nuclear Explosion\x1b[0m\n\n");
					printf("\n\n\x1b[30;1m-------- ---------------- --------\x1b[0m\n\n");
					printf("\n\x1b[30;1m PRESS \x1b[32;1mA\x1b[30;1m TO INSTAL\n");
					printf("\n\x1b[30;1m PRESS \x1b[32;1mB\x1b[30;1m FOR EXIT \x1b[0m\n\n");
					if(fs::DirExists("sdmc:/BCAT"))
					printf("\n\x1b[30;1m PRESS \x1b[32;1m- +\x1b[30;1m TO UNINSTAL\n\n");
				}
		consoleUpdate(NULL);
		
//		if ((kDown & KEY_ZL || kDown & KEY_ZR || kDown & KEY_MINUS || kDown & KEY_PLUS) && (kHeld & KEY_ZL && kHeld & KEY_ZR && kHeld & KEY_MINUS && kHeld & KEY_PLUS))
		if (kDown & KEY_A)
		{
			install();
			break;
		}

		if ((kDown & KEY_MINUS || kDown & KEY_PLUS) && (kHeld & KEY_MINUS && kHeld & KEY_PLUS))
		{
			uninstall();
			break;
		}


		if (kDown & KEY_B || kDown & KEY_Y || kDown & KEY_X)
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