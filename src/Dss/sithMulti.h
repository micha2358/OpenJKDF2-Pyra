#ifndef _SITHMULTI_H
#define _SITHMULTI_H

#include "types.h"
#include "globals.h"

#include "stdPlatform.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define sithMulti_Startup_ADDR (0x004C9AE0)
#define sithMulti_Shutdown_ADDR (0x004C9CB0)
#define sithMulti_ServerLeft_ADDR (0x004C9D00)
#define sithMulti_CreatePlayer_ADDR (0x004C9FC0)
#define sithMulti_InitTick_ADDR (0x004CA140)
#define sithMulti_LobbyMessage_ADDR (0x004CA1B0)
#define sithMulti_map_init_related_ADDR (0x004CA310)
#define sithMulti_sub_4CA3B0_ADDR (0x004CA3B0)
#define sithMulti_sub_4CA410_ADDR (0x004CA410)
#define sithMulti_sub_4CA470_ADDR (0x004CA470)
#define sithMulti_sendmsgidk4_ADDR (0x004CA610)
#define sithMulti_SendWelcome_ADDR (0x004CA710)
#define sithMulti_ProcessJoinLeave_ADDR (0x004CA780)
#define sithMulti_ProcessJoin_unused_ADDR (0x004CA910)
#define sithMulti_SendLeaveJoin_ADDR (0x004CA9C0)
#define sithMulti_ProcessLeaveJoin_ADDR (0x004CAAF0)
#define sithMulti_SendJoinRequest_ADDR (0x004CADB0)
#define sithMulti_ProcessJoinRequest_ADDR (0x004CAE50)
#define sithMulti_Send36_ADDR (0x004CB200)
#define sithMulti_SendChat_ADDR (0x004CB250)
#define sithMulti_ProcessChat_ADDR (0x004CB2E0)
#define sithMulti_SendPing_ADDR (0x004CB390)
#define sithMulti_ProcessPing_ADDR (0x004CB3E0)
#define sithMulti_ProcessPingResponse_ADDR (0x004CB410)
#define sithMulti_SendQuit_ADDR (0x004CB4A0)
#define sithMulti_ProcessQuit_ADDR (0x004CB4F0)
#define sithMulti_HandleTimeLimit_ADDR (0x004CB690)
#define sithMulti_SyncScores_ADDR (0x004CBC00)
#define sithMulti_IterPlayersnothingidk_ADDR (0x004CBC10)
#define sithMulti_SetHandleridk_ADDR (0x004CBC40)
#define sithMulti_HandleDeath_ADDR (0x004CBC50)
#define sithMulti_ProcessScore_ADDR (0x004CBDE0)
#define sithMulti_EndLevel_ADDR (0x004CBF90)
#define sithMulti_GetSpawnIdx_ADDR (0x004CBFC0)
#define sithMulti_FreeThing_ADDR (0x004CC110)

#define NETMSG_START intptr_t craftingPacket = (intptr_t)&sithComm_netMsgTmp.pktData[0];
#define NETMSG_START_2 intptr_t craftingPacket = (intptr_t)&stdComm_cogMsgTmp.pktData[0];

/*
 * Pyra/ARMv7: DSS/network/savegame packets are byte-packed and are not
 * guaranteed to have natural alignment for 16/32-bit values. Dereferencing
 * cast pointers here is undefined behaviour and can raise SIGBUS on ARM.
 * memcpy preserves the exact packet layout and lets the compiler generate
 * alignment-safe loads/stores.
 */
#define FIX_ALIGN(t) (1)

#define NETMSG_PUSHU8(x) { uint8_t _v = (uint8_t)(x); memcpy((void*)craftingPacket, &_v, sizeof(_v)); craftingPacket += sizeof(_v); };
#define NETMSG_PUSHU16(x) { uint16_t _v = (uint16_t)(x); memcpy((void*)craftingPacket, &_v, sizeof(_v)); craftingPacket += sizeof(_v); };
#define NETMSG_PUSHS16(x) { int16_t _v = (int16_t)(x); memcpy((void*)craftingPacket, &_v, sizeof(_v)); craftingPacket += sizeof(_v); };
#define NETMSG_PUSHU32(x) { uint32_t _v = (uint32_t)(x); memcpy((void*)craftingPacket, &_v, sizeof(_v)); craftingPacket += sizeof(_v); };
#define NETMSG_PUSHS32(x) { int32_t _v = (int32_t)(x); memcpy((void*)craftingPacket, &_v, sizeof(_v)); craftingPacket += sizeof(_v); };
#define NETMSG_PUSHF32(x) { flex32_t _v = (flex32_t)(x); memcpy((void*)craftingPacket, &_v, sizeof(_v)); craftingPacket += sizeof(_v); };
#define NETMSG_PUSHVEC2(x) { rdVector2 _v = (x); memcpy((void*)craftingPacket, &_v, sizeof(_v)); craftingPacket += sizeof(_v); };
#define NETMSG_PUSHVEC3(x) { rdVector3 _v = (x); memcpy((void*)craftingPacket, &_v, sizeof(_v)); craftingPacket += sizeof(_v); };
#define NETMSG_PUSHVEC3I(x) { rdVector3i _v = (x); memcpy((void*)craftingPacket, &_v, sizeof(_v)); craftingPacket += sizeof(_v); };
#define NETMSG_PUSHMAT34(x) { rdMatrix34 _v = (x); memcpy((void*)craftingPacket, &_v, sizeof(_v)); craftingPacket += sizeof(_v); };
#define NETMSG_PUSHSTR(x,l) { _strncpy((char*)craftingPacket, (x), (l)-1); ((char*)craftingPacket)[(l)-1] = 0; craftingPacket += (l); };
#define NETMSG_PUSHWSTR(x,l) { _wcsncpy((wchar_t*)craftingPacket, (x), (l)-1); ((wchar_t*)craftingPacket)[(l)-1] = 0; craftingPacket += (l*sizeof(wchar_t)); };


#define NETMSG_END(msgid) { size_t len = (intptr_t)craftingPacket - (intptr_t)&sithComm_netMsgTmp.pktData[0]; \
                            sithComm_netMsgTmp.netMsg.flag_maybe = 0; \
                            sithComm_netMsgTmp.netMsg.cogMsgId = msgid; \
                            sithComm_netMsgTmp.netMsg.msg_size = len; \
                          };
#define NETMSG_LEN(msgid) ((intptr_t)craftingPacket - (intptr_t)&sithComm_netMsgTmp.pktData[0])

#define NETMSG_END_2(msgid) { size_t len = (intptr_t)craftingPacket - (intptr_t)&stdComm_cogMsgTmp.pktData[0]; \
                            stdComm_cogMsgTmp.netMsg.flag_maybe = 0; \
                            stdComm_cogMsgTmp.netMsg.cogMsgId = msgid; \
                            stdComm_cogMsgTmp.netMsg.msg_size = len; \
                          };
#define NETMSG_LEN_2(msgid) ((intptr_t)craftingPacket - (intptr_t)&stdComm_cogMsgTmp.pktData[0])

#define NETMSG_IN_START(x) intptr_t _readingPacket = (intptr_t)&x->pktData[0]; uint8_t _readingOutU8; \
uint16_t _readingOutU16; int16_t _readingOutS16; uint32_t _readingOutU32; \
int32_t _readingOutS32; flex32_t _readingOutFloat; rdVector2 _readingOutV2; \
rdVector3 _readingOutV3; rdVector3i _readingOutV3i; rdMatrix34 _readingOutM34;

#define CHECK_ALIGN(t) (1)

#define NETMSG_POPU8() (memcpy(&_readingOutU8, (const void*)_readingPacket, sizeof(_readingOutU8)), _readingPacket += sizeof(_readingOutU8), _readingOutU8)
#define NETMSG_POPU16() (memcpy(&_readingOutU16, (const void*)_readingPacket, sizeof(_readingOutU16)), _readingPacket += sizeof(_readingOutU16), _readingOutU16)
#define NETMSG_POPS16() (memcpy(&_readingOutS16, (const void*)_readingPacket, sizeof(_readingOutS16)), _readingPacket += sizeof(_readingOutS16), _readingOutS16)
#define NETMSG_POPU32() (memcpy(&_readingOutU32, (const void*)_readingPacket, sizeof(_readingOutU32)), _readingPacket += sizeof(_readingOutU32), _readingOutU32)
#define NETMSG_POPS32() (memcpy(&_readingOutS32, (const void*)_readingPacket, sizeof(_readingOutS32)), _readingPacket += sizeof(_readingOutS32), _readingOutS32)
#define NETMSG_POPF32() (memcpy(&_readingOutFloat, (const void*)_readingPacket, sizeof(_readingOutFloat)), _readingPacket += sizeof(_readingOutFloat), _readingOutFloat)
#define NETMSG_POPVEC2() (memcpy(&_readingOutV2, (const void*)_readingPacket, sizeof(_readingOutV2)), _readingPacket += sizeof(_readingOutV2), _readingOutV2)
#define NETMSG_POPVEC3() (memcpy(&_readingOutV3, (const void*)_readingPacket, sizeof(_readingOutV3)), _readingPacket += sizeof(_readingOutV3), _readingOutV3)
#define NETMSG_POPVEC3I() (memcpy(&_readingOutV3i, (const void*)_readingPacket, sizeof(_readingOutV3i)), _readingPacket += sizeof(_readingOutV3i), _readingOutV3i)
#define NETMSG_POPMAT34() (memcpy(&_readingOutM34, (const void*)_readingPacket, sizeof(_readingOutM34)), _readingPacket += sizeof(_readingOutM34), _readingOutM34)
#define NETMSG_POPSTR(x,l) { _strncpy((x), (char*)_readingPacket, (l)-1); (x)[(l)-1] = 0; _readingPacket += (l); }
#define NETMSG_POPWSTR(x,l) { _wcsncpy((x), (wchar_t*)_readingPacket, (l)-1); (x)[(l)-1] = 0; _readingPacket += (l*sizeof(wchar_t)); }


#define NETMSG_IN_END {}

extern int jkGuiNetHost_bIsDedicated;

void sithMulti_SetHandleridk(sithMultiHandler_t a1);
void sithMulti_SendChat(const char *pStr, int arg0, int arg1);
void sithMulti_ProcessScore();
int sithMulti_ProcessChat(sithCogMsg *msg);

HRESULT sithMulti_CreatePlayer(const wchar_t *a1, const wchar_t *a2, const char *a3, const char *a4, int a5, int a6, int multiModeFlags, int rate, int a9);
int sithMulti_Startup();
void sithMulti_FreeThing(int a1);
void sithMulti_Shutdown();
int sithMulti_SendJoinRequest(int sendto_id);
MATH_FUNC int sithMulti_GetSpawnIdx(sithThing *pPlayerThing);
void sithMulti_SyncScores();
int sithMulti_map_init_related();
int sithMulti_ResetNetState();
void sithMulti_CleanupThings(sithWorld *pWorld);
void sithMulti_sendmsgidk4(int playerIdx);
void sithMulti_ProcessJoin_unused(int playerIdx);
void sithMulti_Send36(int param1, int param2, int sendtoId);
void sithMulti_HandleDeath(sithPlayerInfo *pPlayerInfo, sithThing *pKilledThing, sithThing *pKilledByThing);
void sithMulti_EndLevel(uint32_t a1, int a2);
void sithMulti_SendWelcome(int a1, int playerIdx, int sendtoId);
void sithMulti_SendQuit(int idx);
int sithMulti_LobbyMessage();
int sithMulti_ProcessJoinLeave(sithCogMsg *msg);
int sithMulti_ProcessPing(sithCogMsg *msg);
int sithMulti_ProcessPingResponse(sithCogMsg *msg);
int sithMulti_ProcessQuit(sithCogMsg *msg);
int sithMulti_ServerLeft(int32_t a, sithEventInfo* b);
void sithMulti_SendLeaveJoin(int sendtoId, int bSync);
int sithMulti_ProcessLeaveJoin(sithCogMsg *msg);
void sithMulti_sub_4CA470(int a1);
void sithMulti_InitTick(uint32_t tickrate);
int sithMulti_ProcessJoinRequest(sithCogMsg *msg);
void sithMulti_HandleTimeLimit(int deltaMs);
uint32_t sithMulti_IterPlayersnothingidk(int net_id);
int sithMulti_SendPing(int sendtoId);

//static void (*sithMulti_Startup)() = (void*)sithMulti_Startup_ADDR;
//static void (*sithMulti_FreeThing)(int a1) = (void*)sithMulti_FreeThing_ADDR;
//static int (*sithMulti_SendQuit)(int a1) = (void*)sithMulti_SendQuit_ADDR;
//static void (*sithMulti_SyncScores)(void) = (void*)sithMulti_SyncScores_ADDR;
//static void (*sithMulti_Shutdown)() = (void*)sithMulti_Shutdown_ADDR;
//static int (*sithMulti_LobbyMessage)() = (void*)sithMulti_LobbyMessage_ADDR;
//static void (*sithMulti_HandleTimeLimit)(int) = (void*)sithMulti_HandleTimeLimit_ADDR;
//static int (*sithMulti_SendWelcome)(int,int,int) = (void*)sithMulti_SendWelcome_ADDR;
//static void (*sithMulti_HandleDeath)(sithPlayerInfo *a1, sithThing *killed, sithThing *killed_by) = (void*)sithMulti_HandleDeath_ADDR;
//static int (*sithMulti_CreatePlayer)(wchar_t *a1, wchar_t *a2, char *a3, char *a4, int a5, int a6, int a7, int a8, int a9) = (void*)sithMulti_CreatePlayer_ADDR;
//static uint32_t (*sithMulti_InitTick)(uint32_t) = (void*)sithMulti_InitTick_ADDR;
//static int (*sithMulti_ServerLeft)() = (void*)sithMulti_ServerLeft_ADDR;
//static int (*sithMulti_SendJoinRequest)(int a1) = (void*)sithMulti_SendJoinRequest_ADDR;

#ifdef __cplusplus
}
#endif


#endif // _SITHMULTI_H
