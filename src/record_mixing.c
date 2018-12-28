#include "global.h"
#include "alloc.h"
#include "random.h"
#include "constants/items.h"
#include "text.h"
#include "item.h"
#include "task.h"
#include "constants/species.h"
#include "save.h"
#include "load_save.h"
#include "pokemon.h"
#include "cable_club.h"
#include "link.h"
#include "link_rfu.h"
#include "tv.h"
#include "battle_tower.h"
#include "window.h"
#include "mystery_event_script.h"
#include "secret_base.h"
#include "mauville_old_man.h"
#include "sound.h"
#include "constants/songs.h"
#include "menu.h"
#include "overworld.h"
#include "field_screen_effect.h"
#include "fldeff_misc.h"
#include "script.h"
#include "event_data.h"
#include "lilycove_lady.h"
#include "strings.h"
#include "string_util.h"
#include "record_mixing.h"
#include "new_game.h"
#include "daycare.h"
#include "international_string_util.h"
#include "constants/battle_frontier.h"

extern void ReceiveSecretBasesData(struct SecretBaseRecord *, size_t, u8);
extern void ReceiveEasyChatPairsData(struct EasyChatPair *, size_t, u8);

// Static type declarations

struct RecordMixingHallRecords
{
    struct RankingHall1P hallRecords1P[HALL_FACILITIES_COUNT][2][6];
    struct RankingHall2P hallRecords2P[2][6];
};

struct PlayerRecordsRS
{
    struct SecretBaseRecord secretBases[SECRET_BASES_COUNT];
    TVShow tvShows[TV_SHOWS_COUNT];
    PokeNews pokeNews[POKE_NEWS_COUNT];
    OldMan oldMan;
    struct EasyChatPair easyChatPairs[5];
    struct RecordMixingDayCareMail dayCareMail;
    struct RSBattleTowerRecord battleTowerRecord;
    u16 giftItem;
    u16 filler11C8[0x32];
};

struct PlayerRecordsEmerald
{
    /* 0x0000 */ struct SecretBaseRecord secretBases[SECRET_BASES_COUNT];
    /* 0x0c80 */ TVShow tvShows[TV_SHOWS_COUNT];
    /* 0x1004 */ PokeNews pokeNews[POKE_NEWS_COUNT];
    /* 0x1044 */ OldMan oldMan;
    /* 0x1084 */ struct EasyChatPair easyChatPairs[5];
    /* 0x10ac */ struct RecordMixingDayCareMail dayCareMail;
    /* 0x1124 */ struct EmeraldBattleTowerRecord battleTowerRecord;
    /* 0x1210 */ u16 giftItem;
    /* 0x1214 */ LilycoveLady lilycoveLady;
    /* 0x1254 */ struct Apprentice apprentice[2];
    /* 0x12dc */ struct PlayerHallRecords hallRecords;
    /* 0x1434 */ u8 field_1434[0x10];
}; // 0x1444

union PlayerRecords
{
    struct PlayerRecordsRS ruby;
    struct PlayerRecordsEmerald emerald;
};

// Static RAM declarations

static IWRAM_DATA bool8 gUnknown_03001130;
static IWRAM_DATA struct SecretBaseRecord *sSecretBasesSave;
static IWRAM_DATA TVShow *sTvShowsSave;
static IWRAM_DATA PokeNews *sPokeNewsSave;
static IWRAM_DATA OldMan *sOldManSave;
static IWRAM_DATA struct EasyChatPair *sEasyChatPairsSave;
static IWRAM_DATA struct RecordMixingDayCareMail *gUnknown_03001148;
static IWRAM_DATA void *sBattleTowerSave;
static IWRAM_DATA LilycoveLady *sLilycoveLadySave;
static IWRAM_DATA void *sApprenticesSave;
static IWRAM_DATA void *sBattleTowerSave_Duplicate;
static IWRAM_DATA u32 sRecordStructSize;
static IWRAM_DATA u8 gUnknown_03001160;
static IWRAM_DATA u32 filler_03001164;
static IWRAM_DATA struct PlayerHallRecords *gUnknown_03001168[3];

static EWRAM_DATA struct RecordMixingDayCareMail gUnknown_02039F9C = {0};
static EWRAM_DATA union PlayerRecords *sReceivedRecords = NULL;
static EWRAM_DATA union PlayerRecords *sSentRecord = NULL;

// Static ROM declarations

static void Task_RecordMixing_Main(u8 taskId);
static void sub_80E7324(u8 taskId);
static void Task_SendPacket(u8 taskId);
static void Task_CopyReceiveBuffer(u8 taskId);
static void Task_SendPacket_SwitchToReceive(u8 taskId);
static void *LoadPtrFromTaskData(const u16 *asShort);
static void StorePtrInTaskData(void *records, u16 *a1);
static u8 GetMultiplayerId_(void);
static void *GetPlayerRecvBuffer(u8);
static void ReceiveOldManData(OldMan *, size_t, u8);
static void ReceiveBattleTowerData(void *battleTowerRecord, size_t, u8);
static void ReceiveLilycoveLadyData(LilycoveLady *, size_t, u8);
static void sub_80E7B2C(const u8 *);
static void ReceiveDaycareMailData(struct RecordMixingDayCareMail *, size_t, u8, TVShow *);
static void ReceiveGiftItem(u16 *item, u8 which);
static void sub_80E7FF8(u8 taskId);
static void sub_80E8110(struct Apprentice *arg0, struct Apprentice *arg1);
static void ReceiveApprenticeData(struct Apprentice *arg0, size_t arg1, u32 arg2);
static void ReceiveRankingHallRecords(struct PlayerHallRecords *hallRecords, size_t arg1, u32 arg2);
static void sub_80E89F8(struct RecordMixingDayCareMail *dst);
static void SanitizeDayCareMailForRuby(struct RecordMixingDayCareMail *src);
static void SanitizeEmeraldBattleTowerRecord(struct EmeraldBattleTowerRecord *arg0);
static void SanitizeRubyBattleTowerRecord(struct RSBattleTowerRecord *src);

// .rodata

static const u8 gUnknown_0858CF8C[] = {1, 0};

static const u8 gUnknown_0858CF8E[][3] =
{
    {1, 2, 0},
    {2, 0, 1},
};

static const u8 gUnknown_0858CF94[][4] =
{
    {1, 0, 3, 2},
    {3, 0, 1, 2},
    {2, 0, 3, 1},
    {1, 3, 0, 2},
    {2, 3, 0, 1},
    {3, 2, 0, 1},
    {1, 2, 3, 0},
    {2, 3, 1, 0},
    {3, 2, 1, 0},
};

static const u8 gUnknown_0858CFB8[3][2] =
{
    {0, 1},
    {1, 2},
    {2, 0},
};

static const u8 gUnknown_0858CFBE[3][4] =
{
    {0, 1, 2, 3},
    {0, 2, 1, 3},
    {0, 3, 2, 1},
};

// .text

#define BUFFER_CHUNK_SIZE 200

void sub_80E6BE8(void)
{
    sub_80B37D4(Task_RecordMixing_Main);
}

// these variables were const in R/S, but had to become changeable because of saveblocks changing RAM position
static void SetSrcLookupPointers(void)
{
    sSecretBasesSave = gSaveBlock1Ptr->secretBases;
    sTvShowsSave = gSaveBlock1Ptr->tvShows;
    sPokeNewsSave = gSaveBlock1Ptr->pokeNews;
    sOldManSave = &gSaveBlock1Ptr->oldMan;
    sEasyChatPairsSave = gSaveBlock1Ptr->easyChatPairs;
    gUnknown_03001148 = &gUnknown_02039F9C;
    sBattleTowerSave = &gSaveBlock2Ptr->frontier.towerPlayer;
    sLilycoveLadySave = &gSaveBlock1Ptr->lilycoveLady;
    sApprenticesSave = gSaveBlock2Ptr->apprentices;
    sBattleTowerSave_Duplicate = &gSaveBlock2Ptr->frontier.towerPlayer;
}

static void PrepareUnknownExchangePacket(struct PlayerRecordsRS *dest)
{
    memcpy(dest->secretBases, sSecretBasesSave, sizeof(dest->secretBases));
    memcpy(dest->tvShows, sTvShowsSave, sizeof(dest->tvShows));
    sub_80F14F8(dest->tvShows);
    memcpy(dest->pokeNews, sPokeNewsSave, sizeof(dest->pokeNews));
    memcpy(&dest->oldMan, sOldManSave, sizeof(dest->oldMan));
    memcpy(dest->easyChatPairs, sEasyChatPairsSave, sizeof(dest->easyChatPairs));
    sub_80E89F8(&dest->dayCareMail);
    EmeraldBattleTowerRecordToRuby(sBattleTowerSave, &dest->battleTowerRecord);

    if (GetMultiplayerId() == 0)
        dest->giftItem = GetRecordMixingGift();
}

static void PrepareExchangePacketForRubySapphire(struct PlayerRecordsRS *dest)
{
    memcpy(dest->secretBases, sSecretBasesSave, sizeof(dest->secretBases));
    sub_80EB18C(dest->secretBases);
    memcpy(dest->tvShows, sTvShowsSave, sizeof(dest->tvShows));
    sub_80F1208(dest->tvShows);
    memcpy(dest->pokeNews, sPokeNewsSave, sizeof(dest->pokeNews));
    memcpy(&dest->oldMan, sOldManSave, sizeof(dest->oldMan));
    sub_8120B70(&dest->oldMan);
    memcpy(dest->easyChatPairs, sEasyChatPairsSave, sizeof(dest->easyChatPairs));
    sub_80E89F8(&dest->dayCareMail);
    SanitizeDayCareMailForRuby(&dest->dayCareMail);
    EmeraldBattleTowerRecordToRuby(sBattleTowerSave, &dest->battleTowerRecord);
    SanitizeRubyBattleTowerRecord(&dest->battleTowerRecord);

    if (GetMultiplayerId() == 0)
        dest->giftItem = GetRecordMixingGift();
}

static void PrepareExchangePacket(void)
{
    sub_80E9914();
    sub_80F0BB8();
    SetSrcLookupPointers();

    if (Link_AnyPartnersPlayingRubyOrSapphire())
    {
        if (sub_800A03C() == 0)
            PrepareUnknownExchangePacket(&sSentRecord->ruby);
        else
            PrepareExchangePacketForRubySapphire(&sSentRecord->ruby);
    }
    else
    {
        memcpy(sSentRecord->emerald.secretBases, sSecretBasesSave, sizeof(sSentRecord->emerald.secretBases));
        memcpy(sSentRecord->emerald.tvShows, sTvShowsSave, sizeof(sSentRecord->emerald.tvShows));
        memcpy(sSentRecord->emerald.pokeNews, sPokeNewsSave, sizeof(sSentRecord->emerald.pokeNews));
        memcpy(&sSentRecord->emerald.oldMan, sOldManSave, sizeof(sSentRecord->emerald.oldMan));
        memcpy(&sSentRecord->emerald.lilycoveLady, sLilycoveLadySave, sizeof(sSentRecord->emerald.lilycoveLady));
        memcpy(sSentRecord->emerald.easyChatPairs, sEasyChatPairsSave, sizeof(sSentRecord->emerald.easyChatPairs));
        sub_80E89F8(&sSentRecord->emerald.dayCareMail);
        memcpy(&sSentRecord->emerald.battleTowerRecord, sBattleTowerSave, sizeof(sSentRecord->emerald.battleTowerRecord));
        SanitizeEmeraldBattleTowerRecord(&sSentRecord->emerald.battleTowerRecord);

        if (GetMultiplayerId() == 0)
            sSentRecord->emerald.giftItem = GetRecordMixingGift();

        sub_80E8110(sSentRecord->emerald.apprentice, sApprenticesSave);
        GetPlayerHallRecords(&sSentRecord->emerald.hallRecords);
    }
}

static void ReceiveExchangePacket(u32 which)
{
    if (Link_AnyPartnersPlayingRubyOrSapphire())
    {
        // Ruby/Sapphire
        sub_80E7B2C((void *)sReceivedRecords->ruby.tvShows);
        ReceiveSecretBasesData(sReceivedRecords->ruby.secretBases, sizeof(struct PlayerRecordsRS), which);
        ReceiveDaycareMailData(&sReceivedRecords->ruby.dayCareMail, sizeof(struct PlayerRecordsRS), which, sReceivedRecords->ruby.tvShows);
        ReceiveBattleTowerData(&sReceivedRecords->ruby.battleTowerRecord, sizeof(struct PlayerRecordsRS), which);
        ReceiveTvShowsData(sReceivedRecords->ruby.tvShows, sizeof(struct PlayerRecordsRS), which);
        ReceivePokeNewsData(sReceivedRecords->ruby.pokeNews, sizeof(struct PlayerRecordsRS), which);
        ReceiveOldManData(&sReceivedRecords->ruby.oldMan, sizeof(struct PlayerRecordsRS), which);
        ReceiveEasyChatPairsData(sReceivedRecords->ruby.easyChatPairs, sizeof(struct PlayerRecordsRS), which);
        ReceiveGiftItem(&sReceivedRecords->ruby.giftItem, which);
    }
    else
    {
        // Emerald
        sub_80E7B2C((void *)sReceivedRecords->emerald.tvShows);
        ReceiveSecretBasesData(sReceivedRecords->emerald.secretBases, sizeof(struct PlayerRecordsEmerald), which);
        ReceiveTvShowsData(sReceivedRecords->emerald.tvShows, sizeof(struct PlayerRecordsEmerald), which);
        ReceivePokeNewsData(sReceivedRecords->emerald.pokeNews, sizeof(struct PlayerRecordsEmerald), which);
        ReceiveOldManData(&sReceivedRecords->emerald.oldMan, sizeof(struct PlayerRecordsEmerald), which);
        ReceiveEasyChatPairsData(sReceivedRecords->emerald.easyChatPairs, sizeof(struct PlayerRecordsEmerald), which);
        ReceiveDaycareMailData(&sReceivedRecords->emerald.dayCareMail, sizeof(struct PlayerRecordsEmerald), which, sReceivedRecords->emerald.tvShows);
        ReceiveBattleTowerData(&sReceivedRecords->emerald.battleTowerRecord, sizeof(struct PlayerRecordsEmerald), which);
        ReceiveGiftItem(&sReceivedRecords->emerald.giftItem, which);
        ReceiveLilycoveLadyData(&sReceivedRecords->emerald.lilycoveLady, sizeof(struct PlayerRecordsEmerald), which);
        ReceiveApprenticeData(sReceivedRecords->emerald.apprentice, sizeof(struct PlayerRecordsEmerald), (u8) which);
        ReceiveRankingHallRecords(&sReceivedRecords->emerald.hallRecords, sizeof(struct PlayerRecordsEmerald), (u8) which);
    }
}

static void PrintTextOnRecordMixing(const u8 *src)
{
    NewMenuHelpers_DrawDialogueFrame(0, 0);
    AddTextPrinterParameterized(0, 1, src, 0, 1, 0, NULL);
    CopyWindowToVram(0, 3);
}

#define tCounter data[0]

static void Task_RecordMixing_SoundEffect(u8 taskId)
{
    if (++gTasks[taskId].tCounter == 50)
    {
        PlaySE(SE_W213);
        gTasks[taskId].tCounter = 0;
    }
}

#undef tCounter

#define tState        data[0]
#define tSndEffTaskId data[15]

static void Task_RecordMixing_Main(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case 0: // init
        sSentRecord = malloc(sizeof(union PlayerRecords));
        sReceivedRecords = malloc(sizeof(union PlayerRecords) * 4);
        sub_8009628(gSpecialVar_0x8005);
        VarSet(VAR_TEMP_0, 1);
        gUnknown_03001130 = FALSE;
        PrepareExchangePacket();
        CreateRecordMixingSprite();
        tState = 1;
        data[10] = CreateTask(sub_80E7324, 80);
        tSndEffTaskId = CreateTask(Task_RecordMixing_SoundEffect, 81);
        break;
    case 1: // wait for sub_80E7324
        if (!gTasks[data[10]].isActive)
        {
            tState = 2;
            FlagSet(FLAG_SYS_MIX_RECORD);
            DestroyRecordMixingSprite();
            DestroyTask(tSndEffTaskId);
        }
        break;
    case 2:
        data[10] = CreateTask(sub_80E7FF8, 10);
        tState = 3;
        PlaySE(SE_W226);
        break;
    case 3: // wait for sub_80E7FF8
        if (!gTasks[data[10]].isActive)
        {
            tState = 4;
            if (gWirelessCommType == 0)
                data[10] = sub_80B3050();

            PrintTextOnRecordMixing(gText_RecordMixingComplete);
            data[8] = 0;
        }
        break;
    case 4: // wait 60 frames
        if (++data[8] > 60)
            tState = 5;
        break;
    case 5:
        if (!gTasks[data[10]].isActive)
        {
            free(sReceivedRecords);
            free(sSentRecord);
            sub_808729C();
            if (gWirelessCommType != 0)
            {
                CreateTask(sub_80AF2B4, 10);
            }
            sub_8197434(0, 1);
            DestroyTask(taskId);
            EnableBothScriptContexts();
        }
        break;
    }
}

#undef tState
#undef tSndEffTaskId

static void sub_80E7324(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->data[0])
    {
    case 0:
        PrintTextOnRecordMixing(gText_MixingRecords);
        task->data[8] = 0x708;
        task->data[0] = 400;
        ClearLinkCallback_2();
        break;
    case 100: // wait 20 frames
        if (++task->data[12] > 20)
        {
            task->data[12] = 0;
            task->data[0] = 101;
        }
        break;
    case 101:
        {
            u8 players = GetLinkPlayerCount_2();
            if (IsLinkMaster() == TRUE)
            {
                if (players == sub_800AA48())
                {
                    PlaySE(SE_PIN);
                    task->data[0] = 201;
                    task->data[12] = 0;
                }
            }
            else
            {
                PlaySE(SE_BOO);
                task->data[0] = 301;
            }
        }
        break;
    case 201:
        if (sub_800AA48() == GetLinkPlayerCount_2() && ++task->data[12] > (GetLinkPlayerCount_2() * 30))
        {
            sub_800A620();
            task->data[0] = 1;
        }
        break;
    case 301:
        if (sub_800AA48() == GetLinkPlayerCount_2())
            task->data[0] = 1;
        break;
    case 400: // wait 20 frames
        if (++task->data[12] > 20)
        {
            task->data[0] = 1;
            task->data[12] = 0;
        }
        break;
    case 1: // wait for handshake
        if (gReceivedRemoteLinkPlayers != 0)
        {
            ConvertIntToDecimalStringN(gStringVar1, GetMultiplayerId_(), STR_CONV_MODE_LEADING_ZEROS, 2);
            task->data[0] = 5;
        }
        break;
    case 2:
        {
            u8 subTaskId;

            task->data[6] = GetLinkPlayerCount_2();
            task->data[0] = 0;
            task->data[5] = GetMultiplayerId_();
            task->func = Task_SendPacket;
            if (Link_AnyPartnersPlayingRubyOrSapphire())
            {
                StorePtrInTaskData(sSentRecord, (u16 *)&task->data[2]);
                subTaskId = CreateTask(Task_CopyReceiveBuffer, 80);
                task->data[10] = subTaskId;
                gTasks[subTaskId].data[0] = taskId;
                StorePtrInTaskData(sReceivedRecords, (u16 *)&gTasks[subTaskId].data[5]);
                sRecordStructSize = sizeof(struct PlayerRecordsRS);
            }
            else
            {
                StorePtrInTaskData(sSentRecord, (u16 *)&task->data[2]);
                subTaskId = CreateTask(Task_CopyReceiveBuffer, 80);
                task->data[10] = subTaskId;
                gTasks[subTaskId].data[0] = taskId;
                StorePtrInTaskData(sReceivedRecords, (u16 *)&gTasks[subTaskId].data[5]);
                sRecordStructSize = sizeof(struct PlayerRecordsEmerald);
            }
        }
        break;
    case 5: // wait 60 frames
        if (++task->data[10] > 60)
        {
            task->data[10] = 0;
            task->data[0] = 2;
        }
        break;
    }
}

static void Task_SendPacket(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    // does this send the data 24 times?

    switch (task->data[0])
    {
    case 0: // Copy record data to send buffer
        {
            void *recordData = LoadPtrFromTaskData(&task->data[2]) + task->data[4] * BUFFER_CHUNK_SIZE;

            memcpy(gBlockSendBuffer, recordData, BUFFER_CHUNK_SIZE);
            task->data[0]++;
        }
        break;
    case 1:
        if (GetMultiplayerId() == 0)
            sub_800A4D8(1);
        task->data[0]++;
        break;
    case 2:
        break;
    case 3:
        task->data[4]++;
        if (task->data[4] == sRecordStructSize / 200 + 1)
            task->data[0]++;
        else
            task->data[0] = 0;
        break;
    case 4:
        if (!gTasks[task->data[10]].isActive)
            task->func = Task_SendPacket_SwitchToReceive;
        break;
    }
}

static void Task_CopyReceiveBuffer(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    u8 status = GetBlockReceivedStatus();
    u8 handledPlayers = 0;

    if (status == sub_800A9D8())
    {
        u8 i;

        for (i = 0; i < GetLinkPlayerCount(); i++)
        {
            void *dest;
            void *src;

            if ((status >> i) & 1)
            {
                dest = LoadPtrFromTaskData((u16 *)&task->data[5]) + task->data[i + 1] * BUFFER_CHUNK_SIZE + sRecordStructSize * i;
                src = GetPlayerRecvBuffer(i);
                if ((task->data[i + 1] + 1) * BUFFER_CHUNK_SIZE > sRecordStructSize)
                    memcpy(dest, src, sRecordStructSize - task->data[i + 1] * BUFFER_CHUNK_SIZE);
                else
                    memcpy(dest, src, BUFFER_CHUNK_SIZE);
                ResetBlockReceivedFlag(i);
                task->data[i + 1]++;
                if (task->data[i + 1] == sRecordStructSize / BUFFER_CHUNK_SIZE + 1)
                    handledPlayers++;
            }
        }
        gTasks[task->data[0]].data[0]++;
    }

    if (handledPlayers == GetLinkPlayerCount())
        DestroyTask(taskId);
}

static void sub_80E776C(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    if (!gTasks[task->data[10]].isActive)
        DestroyTask(taskId);
}

static void Task_ReceivePacket(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    task->func = sub_80E776C;
    if (gUnknown_03001130 == TRUE)
        ReceiveExchangePacket(task->data[5]);
}

static void Task_SendPacket_SwitchToReceive(u8 taskId)
{
    gTasks[taskId].func = Task_ReceivePacket;
    gUnknown_03001130 = TRUE;
}

static void *LoadPtrFromTaskData(const u16 *asShort)
{
    return (void *)(asShort[0] | (asShort[1] << 16));
}

static void StorePtrInTaskData(void *records, u16 *asShort)
{
    asShort[0] = (u32)records;
    asShort[1] = ((u32)records >> 16);
}

static u8 GetMultiplayerId_(void)
{
    return GetMultiplayerId();
}

static void *GetPlayerRecvBuffer(u8 id)
{
    return gBlockRecvBuffer[id];
}

static void ShufflePlayerIndices(u32 *data)
{
    u32 i;
    u32 linkTrainerId;
    u32 players = GetLinkPlayerCount();

    switch (players)
    {
    case 2:
        for (i = 0; i < 2; i++)
            data[i] = gUnknown_0858CF8C[i];
        break;
    case 3:
        linkTrainerId = GetLinkPlayerTrainerId(0) % 2;
        for (i = 0; i < 3; i++)
            data[i] = gUnknown_0858CF8E[linkTrainerId][i];
        break;
    case 4:
        linkTrainerId = GetLinkPlayerTrainerId(0) % 9;
        for (i = 0; i < 4; i++)
            data[i] = gUnknown_0858CF94[linkTrainerId][i];
        break;
    }
}

static void ReceiveOldManData(OldMan *oldMan, size_t recordSize, u8 which)
{
    u8 version;
    u16 language;
    OldMan *dest;
    u32 mixIndices[4];

    ShufflePlayerIndices(mixIndices);
    dest = (void *)oldMan + recordSize * mixIndices[which];
    version = gLinkPlayers[mixIndices[which]].version;
    language = gLinkPlayers[mixIndices[which]].language;

    if (Link_AnyPartnersPlayingRubyOrSapphire())
        SanitizeReceivedRubyOldMan(dest, version, language);
    else
        SanitizeReceivedEmeraldOldMan(dest, version, language);

    memcpy(sOldManSave, (void *)oldMan + recordSize * mixIndices[which], sizeof(OldMan));
    ResetMauvilleOldManFlag();
}

static void ReceiveBattleTowerData(void *battleTowerRecord, size_t recordSize, u8 which)
{
    struct EmeraldBattleTowerRecord *dest;
    struct BattleTowerPokemon *btPokemon;
    u32 mixIndices[4];
    s32 i;

    ShufflePlayerIndices(mixIndices);
    if (Link_AnyPartnersPlayingRubyOrSapphire())
    {
        if (RubyBattleTowerRecordToEmerald((void *)battleTowerRecord + recordSize * mixIndices[which], (void *)battleTowerRecord + recordSize * which) == TRUE)
        {
            dest = (void *)battleTowerRecord + recordSize * which;
            dest->language = gLinkPlayers[mixIndices[which]].language;
            CalcEmeraldBattleTowerChecksum(dest);
        }
    }
    else
    {
        memcpy((void *)battleTowerRecord + recordSize * which, (void *)battleTowerRecord + recordSize * mixIndices[which], sizeof(struct EmeraldBattleTowerRecord));
        dest = (void *)battleTowerRecord + recordSize * which;
        for (i = 0; i < 4; i ++)
        {
            btPokemon = &dest->party[i];
            if (btPokemon->species != SPECIES_NONE && IsStringJapanese(btPokemon->nickname))
                ConvertInternationalString(btPokemon->nickname, LANGUAGE_JAPANESE);
        }
        CalcEmeraldBattleTowerChecksum(dest);
    }
    PutNewBattleTowerRecord((void *)battleTowerRecord + recordSize * which);
}

static void ReceiveLilycoveLadyData(LilycoveLady *lilycoveLady, size_t recordSize, u8 which)
{
    LilycoveLady *dest;
    u32 mixIndices[4];

    ShufflePlayerIndices(mixIndices);
    memcpy((void *)lilycoveLady + recordSize * which, sLilycoveLadySave, sizeof(LilycoveLady));

    if (GetLilycoveLadyId() == 0)
    {
        dest = malloc(sizeof(LilycoveLady));
        if (dest == NULL)
            return;

        memcpy(dest, sLilycoveLadySave, sizeof(LilycoveLady));
    }
    else
    {
        dest = NULL;
    }

    memcpy(sLilycoveLadySave, (void *)lilycoveLady + recordSize * mixIndices[which], sizeof(LilycoveLady));
    sub_818DA78();
    if (dest != NULL)
    {
        sub_818E570(dest);
        free(dest);
    }
}

static u8 sub_80E7A9C(struct DayCareMail *rmMail)
{
    return rmMail->message.itemId;
}

static void sub_80E7AA4(struct RecordMixingDayCareMail *src, size_t recordSize, u8 (*idxs)[2], u8 which0, u8 which1)
{
    struct DayCareMail buffer;
    struct RecordMixingDayCareMail *mail1;
    struct RecordMixingDayCareMail *mail2;

    mail1 = (void *)src + recordSize * idxs[which0][0];
    memcpy(&buffer, &mail1->mail[idxs[which0][1]], sizeof(struct DayCareMail));
    mail2 = (void *)src + recordSize * idxs[which1][0];
    memcpy(&mail1->mail[idxs[which0][1]], &mail2->mail[idxs[which1][1]], sizeof(struct DayCareMail));
    memcpy(&mail2->mail[idxs[which1][1]], &buffer, sizeof(struct DayCareMail));
}

static void sub_80E7B2C(const u8 *src)
{
    u8 sum;
    s32 i;

    sum = 0;
    for (i = 0; i < 256; i ++)
        sum += src[i];

    gUnknown_03001160 = sum;
}

static u8 sub_80E7B54(void)
{
    return gUnknown_03001160;
}

static void ReceiveDaycareMailData(struct RecordMixingDayCareMail *src, size_t recordSize, u8 which, TVShow *shows)
{
    u16 i, j;
    u8 linkPlayerCount;
    u8 tableId;
    struct RecordMixingDayCareMail *_src;
    u8 which0, which1;
    void *ptr;
    u8 sp04[4];
    u8 sp08[4];
    struct RecordMixingDayCareMail *sp0c[4];
    u8 sp1c[4][2];
    u8 sp24[4][2];
    u8 sp34;
    u16 oldSeed;
    bool32 anyRS;

    oldSeed = Random2();
    SeedRng2(gLinkPlayers[0].trainerId);
    linkPlayerCount = GetLinkPlayerCount();
    for (i = 0; i < 4; i++)
    {
        sp04[i] = 0xFF;
        sp08[i] = 0;
        sp1c[i][0] = 0;
        sp1c[i][1] = 0;
    }

    anyRS = Link_AnyPartnersPlayingRubyOrSapphire();
    for (i = 0; i < GetLinkPlayerCount(); i++)
    {
        u32 language, version;

        _src = (void *)src + i * recordSize;
        language = gLinkPlayers[i].language;
        version = gLinkPlayers[i].version & 0xFF;
        for (j = 0; j < _src->numDaycareMons; j ++)
        {
            u16 otNameLanguage, nicknameLanguage;
            struct DayCareMail *recordMixingMail = &_src->mail[j];

            if (!recordMixingMail->message.itemId)
                continue;

            if (anyRS)
            {
                if (StringLength(recordMixingMail->OT_name) <= 5)
                {
                    otNameLanguage = LANGUAGE_JAPANESE;
                }
                else
                {
                    StripExtCtrlCodes(recordMixingMail->OT_name);
                    otNameLanguage = language;
                }

                if (recordMixingMail->monName[0] == EXT_CTRL_CODE_BEGIN && recordMixingMail->monName[1] == EXT_CTRL_CODE_JPN)
                {
                    StripExtCtrlCodes(recordMixingMail->monName);
                    nicknameLanguage = LANGUAGE_JAPANESE;
                }
                else
                {
                    nicknameLanguage = language;
                }

                if (version == VERSION_RUBY || version == VERSION_SAPPHIRE)
                {
                    recordMixingMail->gameLanguage = otNameLanguage;
                    recordMixingMail->monLanguage = nicknameLanguage;
                }
            }
            else if (language == LANGUAGE_JAPANESE)
            {
                if (IsStringJapanese(recordMixingMail->OT_name))
                    recordMixingMail->gameLanguage = LANGUAGE_JAPANESE;
                else
                    recordMixingMail->gameLanguage = GAME_LANGUAGE;

                if (IsStringJapanese(recordMixingMail->monName))
                    recordMixingMail->monLanguage = LANGUAGE_JAPANESE;
                else
                    recordMixingMail->monLanguage = GAME_LANGUAGE;
            }
        }
    }

    sp34 = 0;
    for (i = 0; i < linkPlayerCount; i++)
    {
        _src = (void *)src + i * recordSize;
        if (_src->numDaycareMons == 0)
            continue;

        for (j = 0; j < _src->numDaycareMons; j ++)
        {
            if (!_src->holdsItem[j])
                sp1c[i][j] = 1;
        }
    }

    j = 0;
    for (i = 0; i < linkPlayerCount; i++)
    {
        _src = (void *)src + i * recordSize;
        if (sp1c[i][0] == TRUE || sp1c[i][1] == TRUE)
            sp34++;

        if (sp1c[i][0] == TRUE && sp1c[i][1] == FALSE)
        {
            sp24[j][0] = i;
            sp24[j][1] = 0;
            j++;
        }
        else if (sp1c[i][0] == FALSE && sp1c[i][1] == TRUE)
        {
            sp24[j][0] = i;
            sp24[j][1] = 1;
            j++;
        }
        else if (sp1c[i][0] == TRUE && sp1c[i][1] == TRUE)
        {
            u32 var1, var2;

            sp24[j][0] = i;
            var1 = sub_80E7A9C(&_src->mail[0]);
            var2 = sub_80E7A9C(&_src->mail[1]);
            if (!var1 && var2)
            {
                register u8 one asm("r0") = 1; // boo, a fakematch
                sp24[j][1] = one;
            }
            else if ((var1 && var2) || (!var1 && !var2))
            {
                 sp24[j][1] = Random2() % 2;
            }
            else if (var1 && !var2)
            {
                sp24[j][1] = 0;
            }
            j++;
        }
    }

    for (i = 0; i < 4; i++)
    {
        _src = &src[which * recordSize];
        sp0c[i] = _src;
    }

    tableId = sub_80E7B54() % 3;
    switch (sp34)
    {
    case 2:
        sub_80E7AA4(src, recordSize, sp24, 0, 1);
        break;
    case 3:
        which0 = gUnknown_0858CFB8[tableId][0];
        which1 = gUnknown_0858CFB8[tableId][1];
        sub_80E7AA4(src, recordSize, sp24, which0, which1);
        break;
    case 4:
        ptr = sp24;
        which0 = gUnknown_0858CFBE[tableId][0];
        which1 = gUnknown_0858CFBE[tableId][1];
        sub_80E7AA4(src, recordSize, ptr, which0, which1);
        which0 = gUnknown_0858CFBE[tableId][2];
        which1 = gUnknown_0858CFBE[tableId][3];
        sub_80E7AA4(src, recordSize, ptr, which0, which1);
        break;
    }

    _src = (void *)src + which * recordSize;
    memcpy(&gSaveBlock1Ptr->daycare.mons[0].mail, &_src->mail[0], sizeof(struct DayCareMail));
    memcpy(&gSaveBlock1Ptr->daycare.mons[1].mail, &_src->mail[1], sizeof(struct DayCareMail));
    SeedRng(oldSeed);
}

static void ReceiveGiftItem(u16 *item, u8 which)
{
    if (which != 0 && *item != ITEM_NONE && GetPocketByItemId(*item) == POCKET_KEY_ITEMS)
    {
        if (!CheckBagHasItem(*item, 1) && !CheckPCHasItem(*item, 1) && AddBagItem(*item, 1))
        {
            VarSet(VAR_TEMP_1, *item);
            StringCopy(gStringVar1, gLinkPlayers[0].name);
            if (*item == ITEM_EON_TICKET)
                FlagSet(FLAG_SYS_HAS_EON_TICKET);
        }
        else
        {
            VarSet(VAR_TEMP_1, ITEM_NONE);
        }
    }
}

static void sub_80E7FF8(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->data[0])
    {
    case 0:
        task->data[0]++;
        break;
    case 1:
        if (Link_AnyPartnersPlayingRubyOrSapphire())
            task->data[0]++;
        else
            task->data[0] = 6;
        break;
    case 2:
        sub_8076D5C();
        sub_8153430();
        task->data[0] ++;
        break;
    case 3:
        if (sub_8153474())
        {
            sav2_gender2_inplace_and_xFE();
            task->data[0] = 4;
            task->data[1] = 0;
        }
        break;
    case 4:
        if (++task->data[1] > 10)
        {
            sub_800AC34();
            task->data[0] ++;
        }
        break;
    case 5:
        if (gReceivedRemoteLinkPlayers == 0)
            DestroyTask(taskId);
        break;
    case 6:
        if (!sub_801048C(0))
        {
            CreateTask(sub_8153688, 5);
            task->data[0] ++;
        }
        break;
    case 7:
        if (!FuncIsActiveTask(sub_8153688))
        {
            if (gWirelessCommType)
            {
                sub_801048C(1);
                task->data[0] = 8;
            }
            else
            {
                task->data[0] = 4;
            }
        }
        break;
    case 8:
        sub_800ADF8();
        task->data[0] ++;
        break;
    case 9:
        if (sub_800A520())
            DestroyTask(taskId);
        break;
    }
}

// New Emerald functions

static void sub_80E8110(struct Apprentice *dst, struct Apprentice *src)
{
    s32 i, id;
    s32 var_2C, var_28, var_24, r8;

    dst[0].playerName[0] = EOS;
    dst[1].playerName[0] = EOS;

    dst[0] = src[0];

    var_28 = 0;
    var_24 = 0;
    var_2C = 0;
    r8 = 0;
    for (i = 0; i < 2; i++)
    {
        id = ((i + gSaveBlock2Ptr->playerApprentice.field_B2_1) % 3) + 1;
        if (src[id].playerName[0] != EOS)
        {
            if (GetTrainerId(src[id].playerId) != GetTrainerId(gSaveBlock2Ptr->playerTrainerId))
            {
                r8++;
                var_2C = id;
            }
            if (GetTrainerId(src[id].playerId) == GetTrainerId(gSaveBlock2Ptr->playerTrainerId))
            {
                var_24++;
                var_28 = id;
            }
        }
    }

    if (r8 == 0 && var_24 != 0)
    {
        r8 = var_24;
        var_2C = var_28;
    }

    switch (r8)
    {
    case 1:
        dst[1] = src[var_2C];
        break;
    case 2:
        if (Random2() > 0x3333)
        {
            dst[1] = src[gSaveBlock2Ptr->playerApprentice.field_B2_1 + 1];
        }
        else
        {
            dst[1] = src[((gSaveBlock2Ptr->playerApprentice.field_B2_1 + 1) % 3 + 1)];
        }
        break;
    }
}

void GetPlayerHallRecords(struct PlayerHallRecords *dst)
{
    s32 i, j;

    for (i = 0; i < HALL_FACILITIES_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            CopyTrainerId(dst->onePlayer[i][j].id, gSaveBlock2Ptr->playerTrainerId);
            dst->onePlayer[i][j].language = GAME_LANGUAGE;
            StringCopy(dst->onePlayer[i][j].name, gSaveBlock2Ptr->playerName);
        }
    }

    for (j = 0; j < 2; j++)
    {
        dst->twoPlayers[j].language = GAME_LANGUAGE;
        CopyTrainerId(dst->twoPlayers[j].id1, gSaveBlock2Ptr->playerTrainerId);
        CopyTrainerId(dst->twoPlayers[j].id2, gSaveBlock2Ptr->frontier.field_EF1[j]);
        StringCopy(dst->twoPlayers[j].name1, gSaveBlock2Ptr->playerName);
        StringCopy(dst->twoPlayers[j].name2, gSaveBlock2Ptr->frontier.field_EE1[j]);
    }

    for (i = 0; i < 2; i++)
    {
        dst->onePlayer[0][i].winStreak = gSaveBlock2Ptr->frontier.towerRecordWinStreaks[FRONTIER_MODE_SINGLES][i];
        dst->onePlayer[1][i].winStreak = gSaveBlock2Ptr->frontier.towerRecordWinStreaks[FRONTIER_MODE_DOUBLES][i];
        dst->onePlayer[2][i].winStreak = gSaveBlock2Ptr->frontier.towerRecordWinStreaks[FRONTIER_MODE_MULTIS][i];
        dst->onePlayer[3][i].winStreak = gSaveBlock2Ptr->frontier.domeRecordWinStreaks[FRONTIER_MODE_SINGLES][i];
        dst->onePlayer[4][i].winStreak = gSaveBlock2Ptr->frontier.palaceRecordWinStreaks[FRONTIER_MODE_SINGLES][i];
        dst->onePlayer[5][i].winStreak = gSaveBlock2Ptr->frontier.arenaRecordStreaks[i];
        dst->onePlayer[6][i].winStreak = gSaveBlock2Ptr->frontier.factoryRecordWinStreaks[FRONTIER_MODE_SINGLES][i];
        dst->onePlayer[7][i].winStreak = gSaveBlock2Ptr->frontier.pikeRecordStreaks[i];
        dst->onePlayer[8][i].winStreak = gSaveBlock2Ptr->frontier.pyramidRecordStreaks[i];

        dst->twoPlayers[i].winStreak = gSaveBlock2Ptr->frontier.towerRecordWinStreaks[FRONTIER_MODE_LINK_MULTIS][i];
    }
}

static bool32 sub_80E841C(struct Apprentice *arg0, struct Apprentice *arg1)
{
    s32 i;

    for (i = 0; i < 4; i++)
    {
        if (GetTrainerId(arg0->playerId) == GetTrainerId(arg1[i].playerId)
            && arg0->number == arg1[i].number)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void ReceiveApprenticeData(struct Apprentice *arg0, size_t arg1, u32 arg2)
{
    s32 i, r7, r8;
    struct Apprentice *structPtr;
    u32 mixIndices[4];
    u32 structId;

    ShufflePlayerIndices(mixIndices);
    structPtr = (void*)(arg0) + (arg1 * mixIndices[arg2]);
    r7 = 0;
    r8 = 0;
    for (i = 0; i < 2; i++)
    {
        if (structPtr[i].playerName[0] != EOS && !sub_80E841C(&structPtr[i], &gSaveBlock2Ptr->apprentices[0]))
        {
            r7++;
            r8 = i;
        }
    }

    switch (r7)
    {
    case 1:
        structId = gSaveBlock2Ptr->playerApprentice.field_B2_1 + 1;
        gSaveBlock2Ptr->apprentices[structId] = structPtr[r8];
        gSaveBlock2Ptr->playerApprentice.field_B2_1 = (gSaveBlock2Ptr->playerApprentice.field_B2_1 + 1) % 3;
        break;
    case 2:
        for (i = 0; i < 2; i++)
        {
            structId = ((i ^ 1) + gSaveBlock2Ptr->playerApprentice.field_B2_1) % 3 + 1;
            gSaveBlock2Ptr->apprentices[structId] = structPtr[i];
        }
        gSaveBlock2Ptr->playerApprentice.field_B2_1 = (gSaveBlock2Ptr->playerApprentice.field_B2_1 + 2) % 3;
        break;
    }
}

static void sub_80E8578(struct RecordMixingHallRecords *dst, void *hallRecords, size_t recordSize, u32 arg3, s32 linkPlayerCount)
{
    s32 i, j, k, l;
    s32 var_68;

    k = 0;
    i = 0;
    while (1)
    {
        if (i >= linkPlayerCount)
            break;
        if (i != arg3)
            gUnknown_03001168[k++] = hallRecords;

        if (k == 3)
            break;
        hallRecords += recordSize;
        i++;
    }

    for (i = 0; i < HALL_FACILITIES_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
        {
            for (k = 0; k < 3; k++)
                dst->hallRecords1P[i][j][k] = gSaveBlock2Ptr->hallRecords1P[i][j][k];

            for (k = 0; k < linkPlayerCount - 1; k++)
            {
                var_68 = 0;
                for (l = 0; l < 3; l++)
                {
                    if (GetTrainerId(dst->hallRecords1P[i][j][l].id) == GetTrainerId(gUnknown_03001168[k]->onePlayer[i][j].id))
                    {
                        var_68++;
                        if (dst->hallRecords1P[i][j][l].winStreak < gUnknown_03001168[k]->onePlayer[i][j].winStreak)
                            dst->hallRecords1P[i][j][l] = gUnknown_03001168[k]->onePlayer[i][j];
                    }
                }
                if (var_68 == 0)
                    dst->hallRecords1P[i][j][k + 3] = gUnknown_03001168[k]->onePlayer[i][j];
            }
        }
    }

    for (j = 0; j < 2; j++)
    {
        for (k = 0; k < 3; k++)
            dst->hallRecords2P[j][k] = gSaveBlock2Ptr->hallRecords2P[j][k];

        for (k = 0; k < linkPlayerCount - 1; k++)
        {
            var_68 = 0;
            for (l = 0; l < 3; l++)
            {
                if (GetTrainerId(dst->hallRecords2P[j][l].id1) == GetTrainerId(gUnknown_03001168[k]->twoPlayers[j].id1)
                    && GetTrainerId(dst->hallRecords2P[j][l].id2) == GetTrainerId(gUnknown_03001168[k]->twoPlayers[j].id2))
                {
                    var_68++;
                    if (dst->hallRecords2P[j][l].winStreak < gUnknown_03001168[k]->twoPlayers[j].winStreak)
                        dst->hallRecords2P[j][l] = gUnknown_03001168[k]->twoPlayers[j];
                }
            }
            if (var_68 == 0)
                dst->hallRecords2P[j][k + 3] = gUnknown_03001168[k]->twoPlayers[j];
        }
    }
}

static void sub_80E8880(struct RankingHall1P *arg0, struct RankingHall1P *arg1)
{
    s32 i, j;

    for (i = 0; i < 3; i++)
    {
        s32 highestWinStreak = 0;
        s32 highestId = -1;
        for (j = 0; j < 6; j++)
        {
            if (arg1[j].winStreak > highestWinStreak)
            {
                highestId = j;
                highestWinStreak = arg1[j].winStreak;
            }
        }

        if (highestId >= 0)
        {
            arg0[i] = arg1[highestId];
            arg1[highestId].winStreak = 0;
        }
    }
}

static void sub_80E88CC(struct RankingHall2P *arg0, struct RankingHall2P *arg1)
{
    s32 i, j;

    for (i = 0; i < 3; i++)
    {
        s32 highestWinStreak = 0;
        s32 highestId = -1;
        for (j = 0; j < 6; j++)
        {
            if (arg1[j].winStreak > highestWinStreak)
            {
                highestId = j;
                highestWinStreak = arg1[j].winStreak;
            }
        }

        if (highestId >= 0)
        {
            arg0[i] = arg1[highestId];
            arg1[highestId].winStreak = 0;
        }
    }
}

static void sub_80E8924(struct RecordMixingHallRecords *arg0)
{
    s32 i, j;

    for (i = 0; i < HALL_FACILITIES_COUNT; i++)
    {
        for (j = 0; j < 2; j++)
            sub_80E8880(gSaveBlock2Ptr->hallRecords1P[i][j], arg0->hallRecords1P[i][j]);
    }
    for (j = 0; j < 2; j++)
        sub_80E88CC(gSaveBlock2Ptr->hallRecords2P[j], arg0->hallRecords2P[j]);
}

static void ReceiveRankingHallRecords(struct PlayerHallRecords *hallRecords, size_t recordSize, u32 arg2)
{
    u8 linkPlayerCount = GetLinkPlayerCount();
    struct RecordMixingHallRecords *largeStructPtr = AllocZeroed(sizeof(struct RecordMixingHallRecords));

    sub_80E8578(largeStructPtr, hallRecords, recordSize, arg2, linkPlayerCount);
    sub_80E8924(largeStructPtr);

    Free(largeStructPtr);
}

static void sub_80E89F8(struct RecordMixingDayCareMail *dst)
{
    gUnknown_02039F9C.mail[0] = gSaveBlock1Ptr->daycare.mons[0].mail;
    gUnknown_02039F9C.mail[1] = gSaveBlock1Ptr->daycare.mons[1].mail;
    InitDaycareMailRecordMixing(&gSaveBlock1Ptr->daycare, &gUnknown_02039F9C);
    *dst = *gUnknown_03001148;
}

static void SanitizeDayCareMailForRuby(struct RecordMixingDayCareMail *src)
{
    s32 i;

    for (i = 0; i < src->numDaycareMons; i++)
    {
        struct DayCareMail *mail = &src->mail[i];
        if (mail->message.itemId != 0)
        {
            if (mail->gameLanguage != LANGUAGE_JAPANESE)
                PadNameString(mail->OT_name, EXT_CTRL_CODE_BEGIN);

            ConvertInternationalString(mail->monName, mail->monLanguage);
        }
    }
}

static void SanitizeRubyBattleTowerRecord(struct RSBattleTowerRecord *src)
{

}

static void SanitizeEmeraldBattleTowerRecord(struct EmeraldBattleTowerRecord *dst)
{
    s32 i;

    for (i = 0; i < 4; i++)
    {
        struct BattleTowerPokemon *towerMon = &dst->party[i];
        if (towerMon->species != 0)
            StripExtCtrlCodes(towerMon->nickname);
    }

    CalcEmeraldBattleTowerChecksum(dst);
}
