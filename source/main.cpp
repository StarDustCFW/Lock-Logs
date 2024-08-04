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
#include "led.hpp"
using namespace std;
//traduction
bool isSpanish = false;
void set_LANG()
{
			setInitialize();
			u64 lcode = 0;
			SetLanguage lang;
			setGetSystemLanguage(&lcode);
			setMakeLanguage(lcode, &lang);
				switch(lang)
				{
					case 5:
					case 14:
					isSpanish =  true;
					   break;
					default:
					isSpanish =  false;
						break;
				}
			setsysExit();
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
	AccountUid uid;
	fsdevMountSystemSaveData("save", FsSaveDataSpaceId_System, TID,  uid);
	
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
}

void install()
{
	//close BCAT ,lock and reboot
	printf("\x1b[32;1m*\x1b[0m Klling BCAT proc\n");
	consoleUpdate(NULL);
	Result rc;
	pmshellInitialize();
	if (R_FAILED(rc = pmshellTerminateProgram(0x010000000000000C)))
    printf("\x1b[31;1m*\x1b[0m Error Klling BCAT\n");
	else{
	Lock(0x80000000000000A1);
	Lock(0x80000000000000A2);
	printf("\x1b[32;1m*\x1b[0m W COMPLETE\n");
	consoleUpdate(NULL);
	led_on(1);
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
		if (R_FAILED(rc = pmshellTerminateProgram(0x010000000000000C)))
		printf("\x1b[31;1m*\x1b[0m Error Klling BCAT\n");
			
		//Just in Case
		pmshellTerminateProgram(0x010000000000000E);//friends
		pmshellTerminateProgram(0x010000000000001E);//account
		pmshellTerminateProgram(0x010000000000001F);//ns
		pmshellTerminateProgram(0x0100000000000020);//nfc
		pmshellTerminateProgram(0x0100000000000022);//capsrv
		pmshellTerminateProgram(0x0100000000000024);//ssl
		pmshellTerminateProgram(0x0100000000000025);//nim
		pmshellTerminateProgram(0x010000000000002B);//erpt
		pmshellTerminateProgram(0x010000000000002E);//pctl
		pmshellTerminateProgram(0x010000000000002F);//npns
		pmshellTerminateProgram(0x0100000000000030);//eupld
		pmshellTerminateProgram(0x0100000000000033);//es
		pmshellTerminateProgram(0x0100000000000036);//creport
		pmshellTerminateProgram(0x010000000000003A);//migration
		pmshellTerminateProgram(0x010000000000003E);//olsc
		pmshellTerminateProgram(0x0100000000001000);//qlaunch - make freeze?
		pmshellTerminateProgram(0x0100000000001009);//miiEdit


		printf("\x1b[32;1m*\x1b[0m Mount System\n");
		consoleUpdate(NULL);
		FsFileSystem mySystem;
		fsOpenBisFileSystem(&mySystem, FsBisPartitionId_System, "");
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
set_LANG();
	PadState pad;
	padConfigureInput(8, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);


	while (appletMainLoop())
	{
		/*
	    hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
		*/
		padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        u64 kHeld = padGetButtons(&pad);

		consoleInit(NULL);
			printf("\x1b[32;1m*\x1b[0m %s v%s Kronos2308, BCAT LOCKER\n",TITLE, VERSION);
				if(isSpanish)
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
		
		if (kDown & HidNpadButton_A)
		{
			install();
			break;
		}

		if ((kDown & HidNpadButton_Minus || kDown & HidNpadButton_Plus) && (kHeld & HidNpadButton_Minus && kHeld & HidNpadButton_Plus))
		{
			uninstall();
			break;
		}


		if (kDown & HidNpadButton_B || kDown & HidNpadButton_Y || kDown & HidNpadButton_X)
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