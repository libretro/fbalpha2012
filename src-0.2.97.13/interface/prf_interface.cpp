// Profiling support
#include "burner.h"

bool bProfileOkay = false;
unsigned int nProfileSelect = 0;

static InterfaceInfo ProfileInfo = { NULL, NULL, NULL };

#if defined (BUILD_WIN32)
	extern struct ProfileDo cntDoPerfCount;
#elif defined (BUILD_SDL)
	extern struct ProfileDo cntDoPerfCount;
#endif

static struct ProfileDo* pProfileDo[] =
{
#if defined (BUILD_WIN32)
	&cntDoPerfCount,
#elif defined (BUILD_SDL)
	&cntDoPerfCount,
#endif
};

#define PROFILE_LEN (sizeof(pProfileDo) / sizeof(pProfileDo[0]))

int ProfileExit()
{
	IntInfoFree(&ProfileInfo);

	if (!bProfileOkay || nProfileSelect >= PROFILE_LEN) {
		return 1;
	}
	bProfileOkay = false;

	return pProfileDo[nProfileSelect]->ProfileExit();
}

int ProfileInit()
{
	int nRet;

	if (nProfileSelect >= PROFILE_LEN) {
		return 1;
	}

	if ((nRet = pProfileDo[nProfileSelect]->ProfileInit()) == 0) {
		bProfileOkay = true;
	}

	return nRet;
}

int ProfileProfileStart(int nSubSystem)
{
	if (!bProfileOkay || nProfileSelect >= PROFILE_LEN) {
		return 1;
	}

	return pProfileDo[nProfileSelect]->ProfileStart(nSubSystem);
}

int ProfileProfileEnd(int nSubSystem)
{
	if (!bProfileOkay || nProfileSelect >= PROFILE_LEN) {
		return 1;
	}

	return pProfileDo[nProfileSelect]->ProfileEnd(nSubSystem);
}

double ProfileProfileReadLast(int nSubSystem)
{
	if (!bProfileOkay || nProfileSelect >= PROFILE_LEN) {
		return 0.0;
	}

	return pProfileDo[nProfileSelect]->ProfileReadLast(nSubSystem);
}

double ProfileProfileReadAverage(int nSubSystem)
{
	if (!bProfileOkay || nProfileSelect >= PROFILE_LEN) {
		return 0.0;
	}

	return pProfileDo[nProfileSelect]->ProfileReadAverage(nSubSystem);
}

InterfaceInfo* ProfileGetInfo()
{
	if (IntInfoInit(&ProfileInfo)) {
		IntInfoFree(&ProfileInfo);
		return NULL;
	}

	if (bProfileOkay) {

		ProfileInfo.pszModuleName = pProfileDo[nProfileSelect]->szModuleName;

	 	if (pProfileDo[nProfileSelect]->GetPluginSettings) {
			pProfileDo[nProfileSelect]->GetPluginSettings(&ProfileInfo);
		}
	} else {
		IntInfoAddStringInterface(&ProfileInfo, _T("Profiling module not initialised"));
	}

	return &ProfileInfo;
}
