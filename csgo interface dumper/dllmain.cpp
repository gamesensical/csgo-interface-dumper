// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <fstream>
#include <string>
#include <tchar.h>
#include <psapi.h>

std::ofstream file;

void log_stuff(const char * msg, ...)
{
	char buf[1024];
	va_list va;
	va_start(va, msg);
	_vsnprintf_s(buf, 1024, msg, va);
	va_end(va);
	file << buf;
}

class interface_reg
{
public:
	BYTE			createfn[4];
	const char		*name;
	interface_reg	*next;
};

void dump_interfaces(const char* interfaces) {
	void* createinterface = GetProcAddress(GetModuleHandleA(interfaces), "CreateInterface");
	if (createinterface) {

		auto var01 = ((uintptr_t)createinterface + 0x8);
		if (!var01) return;

		auto var02 = *(unsigned short*)((uintptr_t)createinterface + 0x5);
		if (!var02) return;

		auto var03 = *(unsigned short*)((uintptr_t)createinterface + 0x7);
		if (!var03) return;

		auto var04 = (uintptr_t)(var01 + (var02 - var03));
		if (!var04) return;

		interface_reg* interface_registry = **(interface_reg***)(var04 + 0x6);
		if (!interface_registry) return;

		log_stuff("%s:\n", interfaces);
		for (interface_reg* pCur = interface_registry; pCur; pCur = pCur->next) {
			log_stuff("\t- %s\n", pCur->name);
		}
		log_stuff("\n");
	}
}

int get_and_dump_modules()
{
	HMODULE hMods[1024];
	HANDLE hProcess = GetCurrentProcess();
	DWORD cbNeeded;
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
			TCHAR szModName[MAX_PATH]; // whole path + dll
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
				TCHAR szBaseName[MAX_PATH]; // just the dll
				if (GetModuleBaseNameA(hProcess, hMods[i], szBaseName, sizeof(szBaseName) / sizeof(TCHAR))) {
					file.open("dumped_interfaces.txt", std::ios::app);
					dump_interfaces(szModName); // change this to szBaseName if you only want the dll name instead of the whole path
					file.close();
				}	
			}
		}
	}
	CloseHandle(hProcess);
	return 0;
}

static unsigned long __stdcall do_stuff(void *arg) {
	remove("dumped_interfaces.txt");
	get_and_dump_modules();
	return true;
}

static unsigned long __stdcall stop_doing_stuff(void *arg) {
	Sleep(500);
	Beep(500, 350);
	FreeLibraryAndExitThread((HMODULE)arg, 0);
}

int __stdcall DllMain(HMODULE self, unsigned long reason_for_call, void *reserved) {
	HANDLE cheat_thread, free_thread;
	if (reason_for_call == DLL_PROCESS_ATTACH) {
		cheat_thread = CreateThread(nullptr, 0, &do_stuff, nullptr, 0, nullptr);
		if (!cheat_thread) return 0;
		free_thread = CreateThread(nullptr, 0, &stop_doing_stuff, self, 0, nullptr);
		if (!free_thread) return 0;
		CloseHandle(cheat_thread);
		return true;
	}
	return false;
}
