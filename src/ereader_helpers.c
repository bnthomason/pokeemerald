#include "global.h"
#include "malloc.h"
#include "decompress.h"
#include "ereader_helpers.h"
#include "link.h"
#include "main.h"
#include "union_room.h"
#include "save.h"
#include "sprite.h"
#include "task.h"
#include "util.h"
#include "trainer_hill.h"
#include "constants/easy_chat.h"
#include "constants/trainers.h"
#include "constants/moves.h"
#include "constants/items.h"

struct Unknown030012C8
{
    bool8 isParent;
    u8 state;
    u8 isSio;
    u8 downloadSuccessful;
    u8 error;
    u32 *unk8;
    s32 unkC;
    s32 unk10;
    u32 unk14;
};

static void EReader_KeyRead(void);
static bool16 isParentLink(u8);
static void EnableSio(void);
static void EReader_StopTimer(void);
static void sub_81D3F1C(u32, u32*, u32*);
static void EReader_SetTimer(void);

static struct Unknown030012C8 gUnknown_030012C8;
static u16 gUnknown_030012E0;
static u16 gUnknown_030012E2;
static u16 gUnknown_030012E4;
static u16 gUnknown_030012E6;
static u32 gUnknown_030012E8;
static u16 gUnknown_030012EC;
static u16 gUnknown_030012EE;
static u16 gUnknown_030012F0;
static u16 gUnknown_030012F2;
static u16 gUnknown_030012F4;

static const struct TrainerHillTrainer sTrainerHillTrainerTemplates_JP[] = {
    [0] = {
        .name = __("マキエ$$$$$   "),
        .facilityClass = FACILITY_CLASS_HEX_MANIAC,
        .unused = 0x1,
        .speechBefore = { EC_WORD_PREPOSTEROUS, EC_WORD_CASE, EC_WORD_THERE, EC_WORD_TO_HER, EC_WORD_CHALLENGE, EC_WORD_JOKING },
        .speechWin = { EC_WORD_HERS, EC_WORD_TRUMP_CARD, EC_MOVE2(SECRET_POWER), EC_WORD_USING, EC_WORD_WON, EC_WORD_EXCL_EXCL },
        .speechLose = { EC_WORD_TO_HER, EC_WORD_WIN, EC_WORD_JOKING, EC_WORD_HIGHS, EC_WORD_SCARY, EC_WORD_ELLIPSIS_EXCL },
        .speechAfter = { EC_WORD_IGNORANT, EC_WORD_SO, EC_WORD_TODAY, EC_WORD_NIGHTTIME, EC_WORD_YOU_RE, EC_WORD_ELLIPSIS_ELLIPSIS_ELLIPSIS },
        .mons = {
            [0] = NULL_BATTLE_TOWER_POKEMON,
            [1] = NULL_BATTLE_TOWER_POKEMON,
            [2] = NULL_BATTLE_TOWER_POKEMON,
            [3] = {
                .species = SPECIES_SWALOT,
                .heldItem = ITEM_SHELL_BELL,
                .moves = { MOVE_SLUDGE_BOMB, MOVE_SHADOW_BALL, MOVE_PAIN_SPLIT, MOVE_YAWN },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 55,
                .attackEV = 255,
                .defenseEV = 100,
                .speedEV = 0,
                .spAttackEV = 0,
                .spDefenseEV = 100,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 1,
                .personality = 0x80,
                .nickname = __("マルノーム$$$$$$"),
                .friendship = 255
            },
            [4] = {
                .species = SPECIES_DUSTOX,
                .heldItem = ITEM_BRIGHT_POWDER,
                .moves = { MOVE_SILVER_WIND, MOVE_SLUDGE_BOMB, MOVE_SHADOW_BALL, MOVE_GIGA_DRAIN },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 0,
                .attackEV = 255,
                .defenseEV = 0,
                .speedEV = 0,
                .spAttackEV = 255,
                .spDefenseEV = 0,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 0,
                .personality = 0x6,
                .nickname = __("ドクケイル$$$$$$"),
                .friendship = 255
            },
            [5] = {
                .species = SPECIES_RELICANTH,
                .heldItem = ITEM_QUICK_CLAW,
                .moves = { MOVE_ANCIENT_POWER, MOVE_SURF, MOVE_EARTHQUAKE, MOVE_AMNESIA },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 100,
                .attackEV = 0,
                .defenseEV = 0,
                .speedEV = 0,
                .spAttackEV = 155,
                .spDefenseEV = 255,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 0,
                .personality = 0x2f,
                .nickname = __("ジーランス$$$$$$"),
                .friendship = 255
            },
        }
    },
    [1] = {
        .name = __("ハルヒト$$$$   "),
        .facilityClass = FACILITY_CLASS_CAMPER,
        .unused = 0x1,
        .speechBefore = { EC_MOVE2(BOUNCE), EC_WORD_AS_MUCH_AS, EC_EMPTY_WORD, EC_WORD_THEY_RE, EC_WORD_STRONG, EC_WORD_EXCL },
        .speechWin = { EC_MOVE(FLY), EC_WORD_AS_MUCH_AS, EC_EMPTY_WORD, EC_WORD_THEY_RE, EC_WORD_HAPPY, EC_WORD_EXCL },
        .speechLose = { EC_MOVE2(MINIMIZE), EC_WORD_AS_MUCH_AS, EC_EMPTY_WORD, EC_WORD_THEY_RE, EC_WORD_SAD, EC_WORD_EXCL },
        .speechAfter = { EC_MOVE(BITE), EC_WORD_AS_MUCH_AS, EC_EMPTY_WORD, EC_WORD_THEY_RE, EC_WORD_ANGRY, EC_WORD_EXCL },
        .mons = {
            [0] = NULL_BATTLE_TOWER_POKEMON,
            [1] = NULL_BATTLE_TOWER_POKEMON,
            [2] = NULL_BATTLE_TOWER_POKEMON,
            [3] = {
                .species = SPECIES_CACTURNE,
                .heldItem = ITEM_QUICK_CLAW,
                .moves = { MOVE_GIGA_DRAIN, MOVE_FAINT_ATTACK, MOVE_THUNDER_PUNCH, MOVE_GROWTH },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 55,
                .attackEV = 0,
                .defenseEV = 100,
                .speedEV = 0,
                .spAttackEV = 255,
                .spDefenseEV = 100,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 0,
                .personality = 0x8c,
                .nickname = __("ノクタス$$$$$$$"),
                .friendship = 255
            },
            [4] = {
                .species = SPECIES_SWELLOW,
                .heldItem = ITEM_BRIGHT_POWDER,
                .moves = { MOVE_FACADE, MOVE_AERIAL_ACE, MOVE_QUICK_ATTACK, MOVE_DOUBLE_TEAM },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 255,
                .attackEV = 255,
                .defenseEV = 0,
                .speedEV = 0,
                .spAttackEV = 0,
                .spDefenseEV = 0,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 0,
                .personality = 0x80,
                .nickname = __("オオスバメ$$$$$$"),
                .friendship = 255
            },
            [5] = {
                .species = SPECIES_WHISCASH,
                .heldItem = ITEM_CHESTO_BERRY,
                .moves = { MOVE_SURF, MOVE_EARTHQUAKE, MOVE_AMNESIA, MOVE_REST },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 0,
                .attackEV = 255,
                .defenseEV = 0,
                .speedEV = 0,
                .spAttackEV = 255,
                .spDefenseEV = 0,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 0,
                .personality = 0x0,
                .nickname = __("ナマズン$$$$$$$"),
                .friendship = 255
            },
        }
    },
    [2] = {
        .name = __("メイコ$$$$$   "),
        .facilityClass = FACILITY_CLASS_SCHOOL_KID_F,
        .unused = 0x1,
        .speechBefore = { EC_WORD_SHINE, EC_WORD_POKEMON, EC_WORD_RELEASE, EC_WORD_WAS, EC_MOVE2(FRUSTRATION), EC_WORD_WITHOUT },
        .speechWin = { EC_WORD_SHINE, EC_WORD_POKEMON, EC_WORD_TO_HER, EC_MOVE2(PRESENT), EC_WORD_KNOWS, EC_WORD_WITHOUT },
        .speechLose = { EC_WORD_THAT, EC_WORD_ABOVE, EC_WORD_LOST, EC_WORD_STORES, EC_WORD_JOKING, EC_WORD_ELLIPSIS_ELLIPSIS_ELLIPSIS },
        .speechAfter = { EC_WORD_ENTERTAINING, EC_WORD_NONE, EC_WORD_HEY_QUES, EC_WORD_ALMOST, EC_WORD_EXCL, EC_EMPTY_WORD },
        .mons = {
            [0] = NULL_BATTLE_TOWER_POKEMON,
            [1] = NULL_BATTLE_TOWER_POKEMON,
            [2] = NULL_BATTLE_TOWER_POKEMON,
            [3] = {
                .species = SPECIES_DELCATTY,
                .heldItem = ITEM_LUM_BERRY,
                .moves = { MOVE_SING, MOVE_BODY_SLAM, MOVE_SHADOW_BALL, MOVE_IRON_TAIL },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 0,
                .attackEV = 255,
                .defenseEV = 0,
                .speedEV = 255,
                .spAttackEV = 0,
                .spDefenseEV = 0,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 0,
                .personality = 0x3,
                .nickname = __("エネコロロ$$$$$$"),
                .friendship = 255
            },
            [4] = {
                .species = SPECIES_ROSELIA,
                .heldItem = ITEM_LEFTOVERS,
                .moves = { MOVE_GIGA_DRAIN, MOVE_GRASS_WHISTLE, MOVE_TOXIC, MOVE_LEECH_SEED },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 255,
                .attackEV = 0,
                .defenseEV = 0,
                .speedEV = 0,
                .spAttackEV = 255,
                .spDefenseEV = 0,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 1,
                .personality = 0x6,
                .nickname = __("ロゼリア$$$$$$$"),
                .friendship = 255
            },
            [5] = {
                .species = SPECIES_BEAUTIFLY,
                .heldItem = ITEM_BRIGHT_POWDER,
                .moves = { MOVE_SILVER_WIND, MOVE_AERIAL_ACE, MOVE_ATTRACT, MOVE_PSYCHIC },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 100,
                .attackEV = 200,
                .defenseEV = 0,
                .speedEV = 0,
                .spAttackEV = 200,
                .spDefenseEV = 0,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 0,
                .personality = 0x6,
                .nickname = __("アゲハント$$$$$$"),
                .friendship = 255
            },
        }
    },
    [3] = {
        .name = __("ピエール$$$$   "),
        .facilityClass = FACILITY_CLASS_GENTLEMAN,
        .unused = 0x1,
        .speechBefore = { EC_WORD_SHE_WAS, EC_WORD_NO_1, EC_WORD_STRONG, EC_WORD_UNCLE, EC_WORD_THERE, EC_WORD_EXCL },
        .speechWin = { EC_WORD_HAHAHA, EC_WORD_TEACHER, EC_WORD_BECOMES, EC_WORD_GIVE, EC_WORD_IS_IT_QUES, EC_EMPTY_WORD },
        .speechLose = { EC_WORD_OUTSIDE, EC_WORD_UNCLE, EC_WORD_SURPRISE, EC_WORD_THESE, EC_WORD_HEY_QUES, EC_WORD_ELLIPSIS_EXCL },
        .speechAfter = { EC_WORD_HE_S, EC_WORD_NO_1, EC_WORD_STRONG, EC_WORD_CHILDREN, EC_WORD_CAN_T, EC_WORD_EXCL_EXCL },
        .mons = {
            [0] = NULL_BATTLE_TOWER_POKEMON,
            [1] = NULL_BATTLE_TOWER_POKEMON,
            [2] = NULL_BATTLE_TOWER_POKEMON,
            [3] = {
                .species = SPECIES_MAWILE,
                .heldItem = ITEM_BRIGHT_POWDER,
                .moves = { MOVE_CRUNCH, MOVE_FLAMETHROWER, MOVE_THUNDER_PUNCH, MOVE_COMET_PUNCH },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 0,
                .attackEV = 0,
                .defenseEV = 100,
                .speedEV = 0,
                .spAttackEV = 255,
                .spDefenseEV = 155,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 1,
                .personality = 0x0,
                .nickname = __("クチート$$$$$$$"),
                .friendship = 255
            },
            [4] = {
                .species = SPECIES_SHARPEDO,
                .heldItem = ITEM_SCOPE_LENS,
                .moves = { MOVE_SURF, MOVE_CRUNCH, MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 255,
                .attackEV = 0,
                .defenseEV = 0,
                .speedEV = 0,
                .spAttackEV = 255,
                .spDefenseEV = 0,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 0,
                .personality = 0x96,
                .nickname = __("サメハダー$$$$$$"),
                .friendship = 255
            },
            [5] = {
                .species = SPECIES_BANETTE,
                .heldItem = ITEM_LUM_BERRY,
                .moves = { MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_THUNDERBOLT, MOVE_WILL_O_WISP },
                .level = 0,
                .ppBonuses = 0x0,
                .hpEV = 255,
                .attackEV = 0,
                .defenseEV = 0,
                .speedEV = 0,
                .spAttackEV = 255,
                .spDefenseEV = 0,
                .otId = 0x10000000,
                .hpIV = 5,
                .attackIV = 5,
                .defenseIV = 5,
                .speedIV = 5,
                .spAttackIV = 5,
                .spDefenseIV = 5,
                .abilityNum = 0,
                .personality = 0x96,
                .nickname = __("ジュペッタ$$$$$$"),
                .friendship = 255
            },
        }
    },
};

static u8 GetNewTrainerHillTrainerID(void)
{
    return (gSaveBlock1Ptr->trainerHill.unused + 1) % 256;
}

static bool32 Struct_EReaderTrainerHillTrainer_ValidateChecksum(const struct EReaderTrainerHillTrainer *arg0)
{
    u32 checksum = CalcByteArraySum((u8 *)arg0, sizeof(struct EReaderTrainerHillTrainer) - sizeof(arg0->checksum));
    if (checksum != arg0->checksum)
        return FALSE;

    return TRUE;
}

bool8 EReader_IsReceivedDataValid(struct EReaderTrainerHillSet *buffer)
{
    u32 i;
    u32 checksum;
    u32 var0 = buffer->count;
    if (var0 == 0 || var0 > 8)
        return FALSE;

    for (i = 0; i < var0; i++)
    {
        if (!Struct_EReaderTrainerHillTrainer_ValidateChecksum(&buffer->unk_8[i]))
            return FALSE;
    }

    checksum = CalcByteArraySum((u8 *)buffer->unk_8, var0 * sizeof(struct EReaderTrainerHillTrainer));
    if (checksum != buffer->checksum)
        return FALSE;

    return TRUE;
}

static bool32 TrainerHill_VerifyChecksum(struct EReaderTrainerHillSet *buffer)
{
    u32 checksum;
    u32 var0 = buffer->count;
    if (var0 == 0 || var0 > 8)
        return FALSE;

    checksum = CalcByteArraySum((u8 *)buffer->unk_8, sizeof(struct EReaderTrainerHillSet) - offsetof(struct EReaderTrainerHillSet, unk_8));
    if (checksum != buffer->checksum)
        return FALSE;

    return TRUE;
}

static bool32 TryWriteTrainerHill_r(const struct EReaderTrainerHillSet *ttdata, struct TrHillTag *buffer2)
{
    int i;

    AGB_ASSERT_EX(ttdata->dummy == 0, "cereader_tool.c", 450);
    AGB_ASSERT_EX(ttdata->id == 0, "cereader_tool.c", 452);

    memset(buffer2, 0, 0x1000);
    buffer2->numTrainers = ttdata->count;
    buffer2->unused1 = GetNewTrainerHillTrainerID();
    buffer2->numFloors = (ttdata->count + 1) / 2;

    for (i = 0; i < ttdata->count; i++)
    {
        if (!(i & 1))
        {
            buffer2->floors[i / 2].trainerNum1 = ttdata->unk_8[i].unk0;
            buffer2->floors[i / 2].display = ttdata->unk_8[i].unk14C;
            buffer2->floors[i / 2].trainers[0] = ttdata->unk_8[i].unk4;
        }
        else
        {
            buffer2->floors[i / 2].trainerNum2 = ttdata->unk_8[i].unk0;
            buffer2->floors[i / 2].trainers[1] = ttdata->unk_8[i].unk4;
        }
    }

    if (i & 1)
    {
        buffer2->floors[i / 2].trainers[1] = sTrainerHillTrainerTemplates_JP[i / 2];
    }

    buffer2->checksum = CalcByteArraySum((u8 *)buffer2->floors, 4 * sizeof(struct TrHillFloor));
    if (TryWriteSpecialSaveSection(SECTOR_ID_TRAINER_HILL, (u8 *)buffer2) != SAVE_STATUS_OK)
        return FALSE;

    return TRUE;
}

bool32 TryWriteTrainerHill(struct EReaderTrainerHillSet *arg0)
{
    void *var0 = AllocZeroed(0x1000);
    bool32 result = TryWriteTrainerHill_r(arg0, var0);
    Free(var0);
    return result;
}

static bool32 TryReadTrainerHill_r(struct EReaderTrainerHillSet *dst, u8 *buffer)
{
    if (TryReadSpecialSaveSection(SECTOR_ID_TRAINER_HILL, buffer) != SAVE_STATUS_OK)
        return FALSE;

    memcpy(dst, buffer, sizeof(struct EReaderTrainerHillSet));
    if (!TrainerHill_VerifyChecksum(dst))
        return FALSE;

    return TRUE;
}

static bool32 TryReadTrainerHill(struct EReaderTrainerHillSet *arg0)
{
    u8 *var0 = AllocZeroed(0x1000);
    bool32 result = TryReadTrainerHill_r(arg0, var0);
    Free(var0);
    return result;
}

bool32 ReadTrainerHillAndValidate(void)
{
    struct EReaderTrainerHillSet *var0 = AllocZeroed(0x1000);
    bool32 result = TryReadTrainerHill(var0);
    Free(var0);
    return result;
}

int Unused_EReader_Send(u32 arg0, u32 *arg1)
{
    int result;

    EReaderHelper_SaveRegsState();
    while (1)
    {
        EReader_KeyRead();
        if (gUnknown_030012E2 & B_BUTTON)
            gShouldAdvanceLinkState = 2;

        gUnknown_030012E4 = EReaderHandleTransfer(1, arg0, arg1, NULL);

        if ((gUnknown_030012E4 & 0x3) == 0 && (gUnknown_030012E4 & 0x10))
        {
            result = 0;
            break;
        }

        if (gUnknown_030012E4 & 0x8)
        {
            result = 1;
            break;
        }

        if (gUnknown_030012E4 & 0x4)
        {
            result = 2;
            break;
        }

        gShouldAdvanceLinkState = 0;
        VBlankIntrWait();
    }

    CpuFill32(0, &gUnknown_030012C8, sizeof(struct Unknown030012C8));
    EReaderHelper_RestoreRegsState();
    return result;
}

int Unused_EReader_Recv(u32 *arg0)
{
    int result;

    EReaderHelper_SaveRegsState();
    while (1)
    {
        EReader_KeyRead();
        if (gUnknown_030012E2 & B_BUTTON)
            gShouldAdvanceLinkState = 2;

        gUnknown_030012E4 = EReaderHandleTransfer(0, 0, NULL, arg0);
        if ((gUnknown_030012E4 & 0x13) == 0x10)
        {
            result = 0;
            break;
        }

        if (gUnknown_030012E4 & 0x8)
        {
            result = 1;
            break;
        }

        if (gUnknown_030012E4 & 0x4)
        {
            result = 2;
            break;
        }

        gShouldAdvanceLinkState = 0;
        VBlankIntrWait();
    }

    CpuFill32(0, &gUnknown_030012C8, sizeof(struct Unknown030012C8));
    EReaderHelper_RestoreRegsState();
    return result;
}

static void EReader_Stop(void)
{
    REG_IME = 0;
    REG_IE &= ~(INTR_FLAG_TIMER3 | INTR_FLAG_SERIAL);
    REG_IME = 1;
    REG_SIOCNT = 0;
    REG_TM3CNT_H = 0;
    REG_IF = INTR_FLAG_TIMER3 | INTR_FLAG_SERIAL;
}

static void sub_81D3CBC(void)
{
    REG_IME = 0;
    REG_IE &= ~(INTR_FLAG_TIMER3 | INTR_FLAG_SERIAL);
    REG_IME = 1;
    REG_RCNT = 0;
    REG_SIOCNT = SIO_MULTI_MODE;
    REG_SIOCNT |= SIO_INTR_ENABLE | SIO_115200_BPS;
    REG_IME = 0;
    REG_IE |= INTR_FLAG_SERIAL;
    REG_IME = 1;

    if (!gUnknown_030012C8.state)
        CpuFill32(0, &gUnknown_030012C8, sizeof(struct Unknown030012C8));
}

static void sub_81D3D34(void)
{
    REG_RCNT = 0;
    REG_SIOCNT = SIO_32BIT_MODE | SIO_INTR_ENABLE;
    REG_SIOCNT |= SIO_MULTI_SD;
    gShouldAdvanceLinkState = 0;
    gUnknown_030012E6 = 0;
    gUnknown_030012E8 = 0;
}

u16 EReaderHandleTransfer(u8 arg0, u32 arg1, u32 *arg2, u32 *arg3)
{
    switch (gUnknown_030012C8.state)
    {
    case 0:
        sub_81D3CBC();
        gUnknown_030012C8.isSio = 1;
        gUnknown_030012C8.state = 1;
        break;
    case 1:
        if (isParentLink(arg0))
            EnableSio();

        if (gShouldAdvanceLinkState == 2)
        {
            gUnknown_030012C8.error = 2;
            gUnknown_030012C8.state = 6;
        }
        break;
    case 2:
        sub_81D3D34();
        sub_81D3F1C(arg1, arg2, arg3);
        gUnknown_030012C8.state = 3;
        // fall through
    case 3:
        if (gShouldAdvanceLinkState == 2)
        {
            gUnknown_030012C8.error = 2;
            gUnknown_030012C8.state = 6;
        }
        else
        {
            gUnknown_030012E6++;
            gUnknown_030012E8++;
            if (!gUnknown_030012C8.isParent && gUnknown_030012E8 > 60)
            {
                gUnknown_030012C8.error = 1;
                gUnknown_030012C8.state = 6;
            }

            if (gUnknown_030012C8.isSio != 2)
            {
                if (gUnknown_030012C8.isParent && gUnknown_030012E6 > 2)
                {
                    EnableSio();
                    gUnknown_030012C8.isSio = 2;
                }
                else
                {
                    EnableSio();
                    gUnknown_030012C8.isSio = 2;
                }
            }
        }
        break;
    case 4:
        sub_81D3CBC();
        gUnknown_030012C8.state = 5;
        break;
    case 5:
        if (gUnknown_030012C8.isParent == TRUE && gUnknown_030012E6 > 2)
            EnableSio();

        if (++gUnknown_030012E6 > 60)
        {
            gUnknown_030012C8.error = 1;
            gUnknown_030012C8.state = 6;
        }
        break;
    case 6:
        if (gUnknown_030012C8.isSio)
        {
            EReader_Stop();
            gUnknown_030012C8.isSio = 0;
        }
        break;
    }

    return gUnknown_030012C8.isSio | (gUnknown_030012C8.error << 2) | (gUnknown_030012C8.downloadSuccessful << 4);
}

static bool16 isParentLink(u8 arg0)
{
    u16 terminal = (*(vu32 *)REG_ADDR_SIOCNT) & (SIO_MULTI_SI | SIO_MULTI_SD);
    if (terminal == SIO_MULTI_SD && arg0)
    {
        gUnknown_030012C8.isParent = TRUE;
        return TRUE;
    }
    else
    {
        gUnknown_030012C8.isParent = FALSE;
        return FALSE;
    }
}

static void sub_81D3F1C(u32 arg0, u32 *arg1, u32 *arg2)
{
    if (gUnknown_030012C8.isParent)
    {
        REG_SIOCNT |= SIO_38400_BPS;
        gUnknown_030012C8.unk8 = arg1;
        REG_SIODATA32 = arg0;
        gUnknown_030012C8.unk10 = arg0 / 4 + 1;
        EReader_SetTimer();
    }
    else
    {
        REG_SIOCNT = REG_SIOCNT;
        gUnknown_030012C8.unk8 = arg2;
    }
}

static void EReader_SetTimer(void)
{
    REG_TM3CNT_L = 0xFDA7;
    REG_TM3CNT_H = TIMER_INTR_ENABLE;
    REG_IME = 0;
    REG_IE |= INTR_FLAG_TIMER3;
    REG_IME = 1;
}

void sub_81D3F9C(void)
{
    EReader_StopTimer();
    EnableSio();
}

void sub_81D3FAC(void)
{
    u16 i, playerCount, k;
    u32 value;
    u16 var0;
    u16 recvBuffer[4];

    switch (gUnknown_030012C8.state)
    {
    case 1:
        REG_SIOMLT_SEND = 0xCCD0; // Handshake id
        *(u64 *)recvBuffer = REG_SIOMLT_RECV;
        for (i = 0, playerCount = 0, k = 0; i < 4; i++)
        {
            if (recvBuffer[i] == 0xCCD0)
                playerCount++;
            else if (recvBuffer[i] != 0xFFFF)
                k++;
        }

        if (playerCount == 2 && k == 0)
            gUnknown_030012C8.state = 2;
        break;
    case 3:
        value = REG_SIODATA32;
        if (!gUnknown_030012C8.unkC && !gUnknown_030012C8.isParent)
            gUnknown_030012C8.unk10 = value / 4 + 1;

        if (gUnknown_030012C8.isParent == TRUE)
        {
            if (gUnknown_030012C8.unkC < gUnknown_030012C8.unk10)
            {
                REG_SIODATA32 = gUnknown_030012C8.unk8[gUnknown_030012C8.unkC];
                gUnknown_030012C8.unk14 += gUnknown_030012C8.unk8[gUnknown_030012C8.unkC];
            }
            else
            {
                REG_SIODATA32 = gUnknown_030012C8.unk14;
            }
        }
        else
        {
            if (gUnknown_030012C8.unkC > 0 && gUnknown_030012C8.unkC < gUnknown_030012C8.unk10 + 1)
            {
                gUnknown_030012C8.unk8[gUnknown_030012C8.unkC - 1] = value;
                gUnknown_030012C8.unk14 += value;
            }
            else if (gUnknown_030012C8.unkC)
            {
                if (gUnknown_030012C8.unk14 == value)
                    gUnknown_030012C8.downloadSuccessful = 1;
                else
                    gUnknown_030012C8.downloadSuccessful = 2;
            }

            gUnknown_030012E8 = 0;
        }

        if (++gUnknown_030012C8.unkC < gUnknown_030012C8.unk10 + 2)
        {
            if (gUnknown_030012C8.isParent)
                REG_TM3CNT_H |= TIMER_ENABLE;
            else
                EnableSio();
        }
        else
        {
            gUnknown_030012C8.state = 4;
            gUnknown_030012E6 = 0;
        }
        break;
    case 5:
        if (!gUnknown_030012C8.isParent)
            REG_SIOMLT_SEND = gUnknown_030012C8.downloadSuccessful;

        *(u64 *)recvBuffer = REG_SIOMLT_RECV;
        var0 = recvBuffer[1] - 1;
        if (var0 < 2)
        {
            if (gUnknown_030012C8.isParent == TRUE)
                gUnknown_030012C8.downloadSuccessful = recvBuffer[1];

            gUnknown_030012C8.state = 6;
        }
        break;
    }
}

static void EnableSio(void)
{
    REG_SIOCNT |= SIO_ENABLE;
}

static void EReader_StopTimer(void)
{
    REG_TM3CNT_H &= ~TIMER_ENABLE;
    REG_TM3CNT_L = 0xFDA7;
}

static void EReader_KeyRead(void)
{
    u16 keysMask = REG_KEYINPUT ^ KEYS_MASK;
    gUnknown_030012E2 = keysMask & ~gUnknown_030012E0;
    gUnknown_030012E0 = keysMask;
}

void EReaderHelper_SaveRegsState(void)
{
    gUnknown_030012EC = REG_IME;
    gUnknown_030012EE = REG_IE;
    gUnknown_030012F0 = REG_TM3CNT_H;
    gUnknown_030012F2 = REG_SIOCNT;
    gUnknown_030012F4 = REG_RCNT;
}

void EReaderHelper_RestoreRegsState(void)
{
    REG_IME = gUnknown_030012EC;
    REG_IE = gUnknown_030012EE;
    REG_TM3CNT_H = gUnknown_030012F0;
    REG_SIOCNT = gUnknown_030012F2;
    REG_RCNT = gUnknown_030012F4;
}

void sub_81D4238(void)
{
    CpuFill32(0, &gUnknown_030012C8, sizeof(struct Unknown030012C8));
}
