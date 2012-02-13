#define WIN32_LEAN_AND_MEAN
#define NO_SDK

#include "GMLuaModule.h"
#include "LibraryHandle.h"
#include "FonixTalk.h"

GMOD_MODULE(Startup, Cleanup);

static CLibraryHandle g_hFonix;
static FonixHandle_t g_hLangHandle = {0};
static const char *g_pchVoices[] =
{
	"[:np]", // Paul 
	"[:nb]", // Betty
	"[:nh]", // Harry
	"[:nf]", // Frank 
	"[:nd]", // Dennis
	"[:nk]", // Kit
	"[:nu]", // Ursula
	"[:nr]", // Rita
	"[:nw]", // Wendy
};

static int FonixTalk_SetRate(lua_State *L)
{
	Lua()->CheckType(1, GLua::TYPE_NUMBER);

	int nRate = Lua()->GetInteger(1);
	if( nRate < 50 || nRate > 600 )
		nRate = 180;

	int nResult = g_hFonix.StdCall<int>("TextToSpeechSetRate", g_hLangHandle.Index[0], nRate);

	Lua()->PushLong(nResult);
	return 1;
}

static int FonixTalk_Reset(lua_State *L)
{
	// 0018F804   004058EB  RETURN to speak.004058EB from speak._TextToSpeechReset@8
	int nResult = g_hFonix.StdCall<int>("TextToSpeechReset", g_hLangHandle.Index[0], 0);

	Lua()->PushLong(nResult);
	return 1;
}

static int FonixTalk_SetVoice(lua_State *L)
{
	Lua()->CheckType(1, GLua::TYPE_NUMBER);

	int nVoice = Lua()->GetInteger(1);
	if( nVoice < 0 || nVoice > _countof(g_pchVoices) )
		nVoice = 0;

	const char *pchVoice = g_pchVoices[nVoice];
	int nResult = g_hFonix.StdCall<int>("TextToSpeechSpeakA", g_hLangHandle.Index[0], pchVoice, 1);

	Lua()->PushLong(nResult);
	return 1;
}

static int FonixTalk_Speak(lua_State *L)
{
	Lua()->CheckType(1, GLua::TYPE_STRING);

	const char *pchSpeak = Lua()->GetString(1);

	int nResult = g_hFonix.StdCall<int>("TextToSpeechSpeakA", g_hLangHandle.Index[0], pchSpeak, 1);

	Lua()->PushLong(nResult);
	return 1;
}

static int FonixTalk_SetPitch(lua_State *L)
{
	Lua()->CheckType(1, GLua::TYPE_NUMBER);
	
	int nResult = 0;
	int nPitch = Lua()->GetInteger(1);

	if( nPitch < 50 || nPitch > 350 )
		nPitch = 100;

	FonixParams_t* pParams = NULL;
	unsigned short* dwDummy2 = 0;
	unsigned short* dwDummy3 = 0;
	unsigned short* dwDummy4 = 0;

	nResult = g_hFonix.StdCall<int>("TextToSpeechGetSpeakerParams", g_hLangHandle.Index[0], 0, &pParams, &dwDummy2, &dwDummy3, &dwDummy4);
	if( nResult != 0 || pParams == NULL )
		return 0;

	pParams->Pitch = (unsigned short)nPitch;
	pParams->Dummy1 = 50;

	nResult = g_hFonix.StdCall<int>("TextToSpeechSetSpeakerParams", g_hLangHandle.Index[0], pParams);
	if( nResult != 0 )
		return 0;

	Lua()->PushLong(1);
	return 1;
}

int __cdecl FonixCallback(int a1, WPARAM wParam)
{
	int nResult = 0;
	return nResult;
}

int Startup(lua_State* L)
{
	if( !g_hFonix.Load("FonixTalk.dll") ) 
		return 1;

	int nResult = 0;

	unsigned int nLang = g_hFonix.StdCall<int>("TextToSpeechStartLangA", "US");

	nResult = g_hFonix.StdCall<int>("TextToSpeechSelectLang", 0, nLang);
	if( nResult != 1 ) 
		return 1;

	nResult = g_hFonix.StdCall<int>("TextToSpeechStartupEx", &g_hLangHandle, 0xFFFFFFFF, 2, &FonixCallback, nLang);
	if( nResult != 0 )
		return 1;

	Lua()->NewGlobalTable("FonixTalk");
	ILuaObject *pFonixTalk = Lua()->GetGlobal("FonixTalk");
	{
		pFonixTalk->SetMember("Speak", FonixTalk_Speak);
		pFonixTalk->SetMember("SetPitch", FonixTalk_SetPitch);
		pFonixTalk->SetMember("SetVoice", FonixTalk_SetVoice);
		pFonixTalk->SetMember("SetRate", FonixTalk_SetRate);
		pFonixTalk->SetMember("Reset", FonixTalk_Reset);
	}
	pFonixTalk->UnReference();

	Lua()->SetGlobal("FONIXTALK_VOICE_PAUL",	(float)0);
	Lua()->SetGlobal("FONIXTALK_VOICE_BETTY",	(float)1);
	Lua()->SetGlobal("FONIXTALK_VOICE_HARRY",	(float)2);
	Lua()->SetGlobal("FONIXTALK_VOICE_FRANK",	(float)3);
	Lua()->SetGlobal("FONIXTALK_VOICE_DENNIS",	(float)4);
	Lua()->SetGlobal("FONIXTALK_VOICE_KIT",		(float)5);
	Lua()->SetGlobal("FONIXTALK_VOICE_URSULA",	(float)6);
	Lua()->SetGlobal("FONIXTALK_VOICE_RITA",	(float)7);
	Lua()->SetGlobal("FONIXTALK_VOICE_WENDY",	(float)8);

	return 0;
}

int Cleanup(lua_State* L)
{
	g_hFonix.Free();
	return 0;
}