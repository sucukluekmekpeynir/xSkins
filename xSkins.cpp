#include <stdio.h>
#include "xLiteMem.h"
#include "xSkins.h"

HANDLE hProcess = NULL;

DWORD dwClientState = 0;
DWORD dwLocalPlayer = 0;
DWORD dwEntityList = 0;
DWORD m_hViewModel = 0;
DWORD m_iViewModelIndex = 0;
DWORD m_flFallbackWear = 0;
DWORD m_nFallbackPaintKit = 0;
DWORD m_iItemIDHigh = 0;
DWORD m_iEntityQuality = 0;
DWORD m_iItemDefinitionIndex = 0;
DWORD m_hActiveWeapon = 0;
DWORD m_hMyWeapons = 0;
DWORD m_nModelIndex = 0;

UINT GetModelIndexByName(const char* modelName)
{
	DWORD cstate = (DWORD)ReadMem(hProcess, dwClientState, NULL, sizeof(DWORD));

	// CClientState + 0x529C -> INetworkStringTable* m_pModelPrecacheTable
	DWORD nst = (DWORD)ReadMem(hProcess, cstate + 0x529C, NULL, sizeof(DWORD));

	// INetworkStringTable + 0x40 -> INetworkStringDict* m_pItems
	DWORD nsd = (DWORD)ReadMem(hProcess, nst + 0x40, NULL, sizeof(DWORD));

	// INetworkStringDict + 0xC -> void* m_pItems
	DWORD nsdi = (DWORD)ReadMem(hProcess, nsd + 0xC, NULL, sizeof(DWORD));

	for (UINT i = 0; i < 1024; i++)
	{
		DWORD nsdi_i = (DWORD)ReadMem(hProcess, nsdi + i * 0x34 + 0xC, NULL, sizeof(DWORD));
		char str[128] = { 0 };
		if (ReadMem(hProcess, nsdi_i, str, sizeof(str)))
		{
			if (_stricmp(str, modelName) == 0)
			{
				return i;
			}
		}
	}

	return 0;
}
UINT GetModelIndex(const short itemIndex)
{
	UINT ret = 0;
	switch (itemIndex)
	{
	case WEAPON_KNIFE:
		ret = GetModelIndexByName("models/weapons/v_knife_default_ct.mdl");
		break;
	case WEAPON_KNIFE_T:
		ret = GetModelIndexByName("models/weapons/v_knife_default_t.mdl");
		break;
	case WEAPON_KNIFE_BAYONET:
		ret = GetModelIndexByName("models/weapons/v_knife_bayonet.mdl");
		break;
	case WEAPON_KNIFE_FLIP:
		ret = GetModelIndexByName("models/weapons/v_knife_flip.mdl");
		break;
	case WEAPON_KNIFE_GUT:
		ret = GetModelIndexByName("models/weapons/v_knife_gut.mdl");
		break;
	case WEAPON_KNIFE_KARAMBIT:
		ret = GetModelIndexByName("models/weapons/v_knife_karam.mdl");
		break;
	case WEAPON_KNIFE_M9_BAYONET:
		ret = GetModelIndexByName("models/weapons/v_knife_m9_bay.mdl");
		break;
	case WEAPON_KNIFE_TACTICAL:
		ret = GetModelIndexByName("models/weapons/v_knife_tactical.mdl");
		break;
	case WEAPON_KNIFE_FALCHION:
		ret = GetModelIndexByName("models/weapons/v_knife_falchion_advanced.mdl");
		break;
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		ret = GetModelIndexByName("models/weapons/v_knife_survival_bowie.mdl");
		break;
	case WEAPON_KNIFE_BUTTERFLY:
		ret = GetModelIndexByName("models/weapons/v_knife_butterfly.mdl");
		break;
	case WEAPON_KNIFE_PUSH:
		ret = GetModelIndexByName("models/weapons/v_knife_push.mdl");
		break;
	case WEAPON_KNIFE_URSUS:
		ret = GetModelIndexByName("models/weapons/v_knife_ursus.mdl");
		break;
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
		ret = GetModelIndexByName("models/weapons/v_knife_gypsy_jackknife.mdl");
		break;
	case WEAPON_KNIFE_STILETTO:
		ret = GetModelIndexByName("models/weapons/v_knife_stiletto.mdl");
		break;
	case WEAPON_KNIFE_WIDOWMAKER:
		ret = GetModelIndexByName("models/weapons/v_knife_widowmaker.mdl");
		break;
	default:
		break;
	}
	return ret;
}
UINT GetWeaponSkin(const short itemIndex)
{
	// set your desired weapon skin values here
	UINT paint = 0;
	switch (itemIndex)
	{
	case WEAPON_DEAGLE:
		paint = 711;
		break;
	case WEAPON_GLOCK:
		paint = 38;
		break;
	case WEAPON_AK47:
		paint = 180;
		break;
	case WEAPON_AWP:
		paint = 344;
		break;
	case WEAPON_M4A1:
		paint = 309;
		break;
	case WEAPON_SSG08:
		paint = 222;
		break;
	case WEAPON_M4A1_SILENCER:
		paint = 445;
		break;
	case WEAPON_USP_SILENCER:
		paint = 653;
		break;
	default:
		break;
	}
	return paint;
}

UINT LoadSkins(const char* file, char*** names, UINT** values)
{
	FILE* fp;
	UINT i = 0;

	// make sure this txt file is encoded as ANSI
	if (fopen_s(&fp, file, "r") == 0)
	{
		char line[64];
		while (fgets(line, sizeof(line), fp))
		{
			// remove trailing newline char
			size_t len = strlen(line) - 1;
			if (line[len] == '\n') {
				line[len] = '\0';
			}

			// use this for splitting lines
			char* pch = strstr(line, ": ");
			if (!pch) { continue; }

			*values = (UINT*)realloc(*values, (i + 1) * sizeof(UINT));
			(*values)[i] = atoi(line);

			*names = (char**)realloc(*names, (i + 1) * sizeof(char*));
			(*names)[i] = _strdup(pch + 2);

			i++;
		}

		if (fp) { fclose(fp); }
	}

	return i;
}
void SortSkins(UINT count, char*** names, UINT** values)
{
	UINT vtmp;
	char* ntmp;

	// bubble sort algorithm
	for (UINT i = 0; i < count; i++)
	{
		for (UINT j = 0; j < count; j++)
		{
			if (strcmp((*names)[i], (*names)[j]) < 0)
			{
				vtmp = (*values)[i];
				ntmp = (*names)[i];

				(*values)[i] = (*values)[j];
				(*names)[i] = (*names)[j];

				(*values)[j] = vtmp;
				(*names)[j] = ntmp;
			}
		}
	}
}

void PrintMenu(const char* title, char** name, UINT sz, UINT x)
{
	Sleep(sz < 20 ? 150 : 20);
	printf("%s %s %s %s\t\t\t\r", title, x > 0 ? "<" : "|", name[x], x < sz ? ">" : "|");
}
UINT ItemSelect(const char* title, char** name, UINT sz)
{
	UINT x = 0; // index of current item
	PrintMenu(title, name, sz, x);

	while (!GetAsyncKeyState(VK_RETURN))
	{
		if (GetAsyncKeyState(VK_RIGHT) && x < sz)
		{
			PrintMenu(title, name, sz, ++x);
		}
		else if (GetAsyncKeyState(VK_LEFT) && x > 0)
		{
			PrintMenu(title, name, sz, --x);
		}
	}

	Sleep(50);
	return x;
}

void xSkins(const short knifeIndex, const UINT knifeSkin)
{
	const int itemIDHigh = -1;
	const int entityQuality = 3;
	const float fallbackWear = 0.0001f;

	UINT modelIndex = 0;
	DWORD localPlayer = 0;

	while (!GetAsyncKeyState(VK_F6))
	{
		// model index is different for each server and map
		// below is a simple way to keep track of local base in order to reset model index
		// while also avoiding doing unnecessary extra reads because of the external RPM overhead
		DWORD tempPlayer = (DWORD)ReadMem(hProcess, dwLocalPlayer, NULL, sizeof(DWORD));
		if (!tempPlayer) // client not connected to any server (works most of the time)
		{
			modelIndex = 0;
			continue;
		}
		else if (tempPlayer != localPlayer) // local base changed (new server join/demo record)
		{
			localPlayer = tempPlayer;
			modelIndex = 0;
		}

		while (!modelIndex)
		{
			modelIndex = GetModelIndex(knifeIndex);
		}

		// loop through m_hMyWeapons slots (8 will be enough)
		for (UINT i = 0; i < 8; i++)
		{
			// get entity of weapon in current slot
			DWORD currentWeapon = (DWORD)ReadMem(hProcess, localPlayer + m_hMyWeapons + i * 0x4, NULL, sizeof(DWORD)) & 0xfff;
			currentWeapon = (DWORD)ReadMem(hProcess, dwEntityList + (currentWeapon - 1) * 0x10, NULL, sizeof(DWORD));
			if (!currentWeapon) { continue; }

			short weaponIndex = (short)ReadMem(hProcess, currentWeapon + m_iItemDefinitionIndex, NULL, sizeof(short));
			UINT weaponSkin = GetWeaponSkin(weaponIndex);

			// for knives, set item and model related properties
			if (weaponIndex == WEAPON_KNIFE || weaponIndex == WEAPON_KNIFE_T || weaponIndex == knifeIndex)
			{
				WriteMem(hProcess, currentWeapon + m_iItemDefinitionIndex, &knifeIndex, sizeof(short));
				WriteMem(hProcess, currentWeapon + m_nModelIndex, &modelIndex, sizeof(UINT));
				WriteMem(hProcess, currentWeapon + m_iViewModelIndex, &modelIndex, sizeof(UINT));
				WriteMem(hProcess, currentWeapon + m_iEntityQuality, &entityQuality, sizeof(int));
				weaponSkin = knifeSkin;
			}

			if (weaponSkin) // set skin properties
			{
				WriteMem(hProcess, currentWeapon + m_iItemIDHigh, &itemIDHigh, sizeof(int));
				WriteMem(hProcess, currentWeapon + m_nFallbackPaintKit, &weaponSkin, sizeof(UINT));
				WriteMem(hProcess, currentWeapon + m_flFallbackWear, &fallbackWear, sizeof(float));
			}
		}

		// get entity of weapon in our hands
		DWORD activeWeapon = (DWORD)ReadMem(hProcess, localPlayer + m_hActiveWeapon, NULL, sizeof(DWORD)) & 0xfff;
		activeWeapon = (DWORD)ReadMem(hProcess, dwEntityList + (activeWeapon - 1) * 0x10, NULL, sizeof(DWORD));
		if (!activeWeapon) { continue; }

		short weaponIndex = (short)ReadMem(hProcess, activeWeapon + m_iItemDefinitionIndex, NULL, sizeof(short));
		if (weaponIndex != knifeIndex) { continue; } // skip if current weapon is not already set to chosen knife

		// get viewmodel entity
		DWORD activeViewModel = (DWORD)ReadMem(hProcess, localPlayer + m_hViewModel, NULL, sizeof(DWORD)) & 0xfff;
		activeViewModel = (DWORD)ReadMem(hProcess, dwEntityList + (activeViewModel - 1) * 0x10, NULL, sizeof(DWORD));
		if (!activeViewModel) { continue; }

		WriteMem(hProcess, activeViewModel + m_nModelIndex, &modelIndex, sizeof(UINT));
	}
}

int main()
{
	printf("[xSkins] External Knife & Skin Changer\n");

	char** skinNames = 0;
	UINT* skinIDs = 0;

	UINT count = LoadSkins("skins.txt", &skinNames, &skinIDs);
	if (!count || !skinNames || !skinIDs)
	{
		printf("[!] Error loading skins from file!\n");
		return 1;
	}

	printf("[+] Loaded %d skins from file\n", count);
	SortSkins(count, &skinNames, &skinIDs);

	UINT knifeID = ItemSelect("Select your knife model:", knifeNames, sizeof(knifeIDs) / sizeof(knifeIDs[0]) - 1);
	printf("\n");
	UINT skinID = ItemSelect("Select your knife skin:", skinNames, count - 1);
	printf("\n");

	printf("[+] Selected knife: %s | %s\n", knifeNames[knifeID], skinNames[skinID]);
	UINT chosenSkin = skinIDs[skinID];

	free(skinNames);
	free(skinIDs);

	DWORD dwProcessId = GetProcessIdByProcessName("csgo.exe");
	printf("[+] csgo.exe process id: %d\n", dwProcessId);

	DWORD dwClientBase = GetModuleBaseAddress(dwProcessId, "client_panorama.dll");
	printf("[+] client_panorama.dll base: 0x%x\n", dwClientBase);

	DWORD dwClientSize = GetModuleSize(dwProcessId, "client_panorama.dll");
	printf("[+] client_panorama.dll size: 0x%x\n", dwClientSize);

	DWORD dwEngineBase = GetModuleBaseAddress(dwProcessId, "engine.dll");
	printf("[+] engine.dll base: 0x%x\n", dwEngineBase);

	DWORD dwEngineSize = GetModuleSize(dwProcessId, "engine.dll");
	printf("[+] engine.dll size: 0x%x\n", dwEngineSize);

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		printf("[!] Error opening handle to csgo!\n");
		return 1;
	}

	PBYTE pbEngine = (PBYTE)malloc(dwEngineSize);
	if (ReadMem(hProcess, dwEngineBase, pbEngine, dwEngineSize))
	{
		BYTE bClientState[16] = "\xA1\xAA\xAA\xAA\xAA\x33\xD2\x6A\x00\x6A\x00\x33\xC9\x89\xB0";
		dwClientState = FindPattern(hProcess,
			dwEngineBase,
			dwEngineSize,
			pbEngine,
			bClientState,
			sizeof(bClientState) - 1,
			0xAA,
			0x1,
			0x0,
			TRUE,
			FALSE);
		printf("[+] dwClientState: 0x%x\n", dwClientState);
	}
	free(pbEngine);

	PBYTE pbClient = (PBYTE)malloc(dwClientSize);
	if (ReadMem(hProcess, dwClientBase, pbClient, dwClientSize))
	{
		BYTE bEntityList[17] = "\xBB\xAA\xAA\xAA\xAA\x83\xFF\x01\x0F\x8C\xAA\xAA\xAA\xAA\x3B\xF8";
		dwEntityList = FindPattern(hProcess,
			dwClientBase,
			dwClientSize,
			pbClient,
			bEntityList,
			sizeof(bEntityList) - 1,
			0xAA,
			0x1,
			0x0,
			TRUE,
			FALSE);
		printf("[+] dwEntityList: 0x%x\n", dwEntityList);

		BYTE bLocalPlayer[23] = "\x8D\x34\x85\xAA\xAA\xAA\xAA\x89\x15\xAA\xAA\xAA\xAA\x8B\x41\x08\x8B\x48\x04\x83\xF9\xFF";
		dwLocalPlayer = FindPattern(hProcess,
			dwClientBase,
			dwClientSize,
			pbClient,
			bLocalPlayer,
			sizeof(bLocalPlayer) - 1,
			0xAA,
			0x3,
			0x4,
			TRUE,
			FALSE);
		printf("[+] dwLocalPlayer: 0x%x\n", dwLocalPlayer);

		BYTE bGetAllClasses[16] = "\x44\x54\x5F\x54\x45\x57\x6F\x72\x6C\x64\x44\x65\x63\x61\x6C";
		DWORD dwGetAllClasses = FindPattern(hProcess,
			dwClientBase,
			dwClientSize,
			pbClient,
			bGetAllClasses,
			sizeof(bGetAllClasses) - 1,
			0xAA,
			0x0,
			0x0,
			FALSE,
			FALSE);

		BYTE bGetAllClasses2[4] = {
			(dwGetAllClasses >> 8 * 0) & 0xFF,
			(dwGetAllClasses >> 8 * 1) & 0xFF,
			(dwGetAllClasses >> 8 * 2) & 0xFF,
			(dwGetAllClasses >> 8 * 3) & 0xFF
		};

		// this dynamic pattern requires an unique generated wildcard
		UCHAR wc = GetWildcard(bGetAllClasses2, sizeof(bGetAllClasses2), 0x0);
		dwGetAllClasses = FindPattern(hProcess,
			dwClientBase,
			dwClientSize,
			pbClient,
			bGetAllClasses2,
			sizeof(bGetAllClasses2),
			wc,
			0x2B,
			0x0,
			TRUE,
			FALSE);
		printf("[+] dwGetAllClasses: 0x%x\n", dwGetAllClasses);

		m_hViewModel = FindNetvar(hProcess, dwGetAllClasses, "DT_BasePlayer", "m_hViewModel[0]");
		printf("[+] m_hViewModel: 0x%x\n", m_hViewModel);

		m_iViewModelIndex = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseCombatWeapon", "m_iViewModelIndex");
		printf("[+] m_iViewModelIndex: 0x%x\n", m_iViewModelIndex);

		m_flFallbackWear = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseAttributableItem", "m_flFallbackWear");
		printf("[+] m_flFallbackWear: 0x%x\n", m_flFallbackWear);

		m_nFallbackPaintKit = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseAttributableItem", "m_nFallbackPaintKit");
		printf("[+] m_nFallbackPaintKit: 0x%x\n", m_nFallbackPaintKit);

		m_iItemIDHigh = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseAttributableItem", "m_iItemIDHigh");
		printf("[+] m_iItemIDHigh: 0x%x\n", m_iItemIDHigh);

		m_iEntityQuality = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseAttributableItem", "m_iEntityQuality");
		printf("[+] m_iEntityQuality: 0x%x\n", m_iEntityQuality);

		m_iItemDefinitionIndex = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseAttributableItem", "m_iItemDefinitionIndex");
		printf("[+] m_iItemDefinitionIndex: 0x%x\n", m_iItemDefinitionIndex);

		m_hActiveWeapon = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseCombatCharacter", "m_hActiveWeapon");
		printf("[+] m_hActiveWeapon: 0x%x\n", m_hActiveWeapon);

		m_hMyWeapons = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseCombatCharacter", "m_hMyWeapons");
		printf("[+] m_hMyWeapons: 0x%x\n", m_hMyWeapons);

		m_nModelIndex = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseViewModel", "m_nModelIndex");
		printf("[+] m_nModelIndex: 0x%x\n", m_nModelIndex);
	}
	free(pbClient);

	if (dwClientState &&
		dwLocalPlayer &&
		dwEntityList &&
		m_hViewModel &&
		m_iViewModelIndex &&
		m_flFallbackWear &&
		m_nFallbackPaintKit &&
		m_iItemIDHigh &&
		m_iEntityQuality &&
		m_iItemDefinitionIndex &&
		m_hActiveWeapon &&
		m_hMyWeapons &&
		m_nModelIndex)
	{
		// start main thread
		xSkins(knifeIDs[knifeID], chosenSkin);
	}

	CloseHandle(hProcess);
	return 0;
}