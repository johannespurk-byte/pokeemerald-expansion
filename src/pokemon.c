#include "global.h"
#include "malloc.h"
#include "apprentice.h"
#include "battle.h"
#include "battle_ai_util.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "battle_setup.h"
#include "battle_tower.h"
#include "battle_z_move.h"
#include "swsh_summary_screen.h"
#include "caps.h"
#include "data.h"
#include "daycare.h"
#include "dexnav.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "evolution_scene.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "field_weather.h"
#include "fishing.h"
#include "follower_npc.h"
#include "frontier_util.h"
#include "graphics.h"
#include "item.h"
#include "link.h"
#include "m4a.h"
#include "main.h"
#include "move_relearner.h"
#include "naming_screen.h"
#include "overworld.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokeblock.h"
#include "pokemon.h"
#include "pokemon_animation.h"
#include "pokemon_icon.h"
#include "pokemon_summary_screen.h"
#include "pokemon_storage_system.h"
#include "pokerus.h"
#include "random.h"
#include "recorded_battle.h"
#include "regions.h"
#include "rtc.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "test_runner.h"
#include "text.h"
#include "trainer.h"
#include "trainer_hill.h"
#include "util.h"
#include "constants/abilities.h"
#include "constants/battle_frontier.h"
#include "constants/battle_move_effects.h"
#include "constants/battle_partner.h"
#include "constants/battle_script_commands.h"
#include "constants/battle_string_ids.h"
#include "constants/cries.h"
#include "constants/event_objects.h"
#include "constants/form_change_types.h"
#include "constants/item_effects.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/regions.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/union_room.h"
#include "constants/weather.h"
#include "level_scaling.h"

extern u16 gSpecialVar_ItemId;

#define FRIENDSHIP_EVO_THRESHOLD ((P_FRIENDSHIP_EVO_THRESHOLD >= GEN_8) ? 160 : 220)

struct SpeciesItem
{
    enum Species species;
    enum Item item;
};

static u16 CalculateBoxMonChecksum(struct BoxPokemon *boxMon);
static u16 CalculateBoxMonChecksumDecrypt(struct BoxPokemon *boxMon);
static u16 CalculateBoxMonChecksumReencrypt(struct BoxPokemon *boxMon);
static union PokemonSubstruct *GetSubstruct(struct BoxPokemon *boxMon, u32 personality, enum SubstructType substructType);
static void EncryptBoxMon(struct BoxPokemon *boxMon);
static void DecryptBoxMon(struct BoxPokemon *boxMon);
static void Task_PlayMapChosenOrBattleBGM(u8 taskId);
void TrySpecialOverworldEvo();

EWRAM_DATA static u8 sLearningMoveTableID = 0;
EWRAM_DATA u8 gPartiesCount[MAX_BATTLE_TRAINERS] = {0};
EWRAM_DATA struct Pokemon gParties[MAX_BATTLE_TRAINERS][PARTY_SIZE] = {0};
EWRAM_DATA struct SpriteTemplate gMultiuseSpriteTemplate = {0};
EWRAM_DATA static struct MonSpritesGfxManager *sMonSpritesGfxManagers[MON_SPR_GFX_MANAGERS_COUNT] = {NULL};
EWRAM_DATA u8 gTriedEvolving = 0;
EWRAM_DATA u16 gFollowerSteps = 0;

struct Pokemon (*const gPlayerPartyPtr)[6] = &gParties[B_TRAINER_PLAYER];
u8 (*const gPlayerPartyCountPtr) = &gPartiesCount[B_TRAINER_PLAYER];
struct Pokemon (*const gEnemyPartyPtr)[6] = &gParties[B_TRAINER_OPPONENT_A];
u8 (*const gEnemyPartyCountPtr) = &gPartiesCount[B_TRAINER_OPPONENT_A];

#include "data/abilities.h"

// Used in an unreferenced function in RS.
// Unreferenced here and in FRLG.
struct CombinedMove
{
    u16 move1;
    u16 move2;
    u16 newMove;
};

static const struct CombinedMove sCombinedMoves[2] =
{
    {MOVE_EMBER, MOVE_GUST, MOVE_HEAT_WAVE},
    {0xFFFF, 0xFFFF, 0xFFFF}
};

// NOTE: The order of the elements in the array below is irrelevant.
// To reorder the pokedex, see the values in include/constants/pokedex.h.

#define KANTO_TO_NATIONAL(name)     [KANTO_DEX_##name - 1] = NATIONAL_DEX_##name,
#define HOENN_TO_NATIONAL(name)     [HOENN_DEX_##name - 1] = NATIONAL_DEX_##name,

static const enum NationalDexOrder sKantoToNationalOrder[KANTO_DEX_COUNT] =
{
    FOREACH_SPECIES_IN_KANTO_DEX_ORDER(KANTO_TO_NATIONAL)
};


// Assigns all Hoenn Dex Indexes to a National Dex Index
static const enum NationalDexOrder sHoennToNationalOrder[HOENN_DEX_COUNT - 1] =
{
    FOREACH_SPECIES_IN_HOENN_DEX_ORDER(HOENN_TO_NATIONAL)
};

// In Battle Palace, moves are chosen based on the Pokémon's nature rather than by the player
// Moves are grouped into "Attack", "Defense", or "Support" (see PALACE_MOVE_GROUP_*)
// Each nature has a certain percent chance of selecting a move from a particular group
// and a separate percent chance for each group when at or below 50% HP
// The table below doesn't list percentages for Support because you can subtract the other two
// Support percentages are listed in comments off to the side instead
#define PALACE_STYLE(atk, def, atkLow, defLow) {atk, atk + def, atkLow, atkLow + defLow}

const struct NatureInfo gNaturesInfo[NUM_NATURES] =
{
    [NATURE_HARDY] =
    {
        .name = COMPOUND_STRING("Hardy"),
        .statUp = STAT_ATK,
        .statDown = STAT_ATK,
        .backAnim = 0,
        .pokeBlockAnim = {ANIM_HARDY, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(61, 7, 61, 7), //32% support >= 50% HP, 32% support < 50% HP
        .battlePalaceFlavorText = B_MSG_EAGER_FOR_MORE,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_LONELY] =
    {
        .name = COMPOUND_STRING("Lonely"),
        .statUp = STAT_ATK,
        .statDown = STAT_DEF,
        .backAnim = 2,
        .pokeBlockAnim = {ANIM_LONELY, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlSupportHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(20, 25, 84, 8), //55%,  8%
        .battlePalaceFlavorText = B_MSG_GLINT_IN_EYE,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_BRAVE] =
    {
        .name = COMPOUND_STRING("Brave"),
        .statUp = STAT_ATK,
        .statDown = STAT_SPEED,
        .backAnim = 0,
        .pokeBlockAnim = {ANIM_BRAVE, AFFINE_TURN_UP},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighDefenseLow,
        .battlePalacePercents = PALACE_STYLE(70, 15, 32, 60), //15%, 8%
        .battlePalaceFlavorText = B_MSG_GETTING_IN_POS,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_ADAMANT] =
    {
        .name = COMPOUND_STRING("Adamant"),
        .statUp = STAT_ATK,
        .statDown = STAT_SPATK,
        .backAnim = 0,
        .pokeBlockAnim = {ANIM_ADAMANT, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(38, 31, 70, 15), //31%, 15%
        .battlePalaceFlavorText = B_MSG_GLINT_IN_EYE,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_NAUGHTY] =
    {
        .name = COMPOUND_STRING("Naughty"),
        .statUp = STAT_ATK,
        .statDown = STAT_SPDEF,
        .backAnim = 0,
        .pokeBlockAnim = {ANIM_NAUGHTY, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlDefenseHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(20, 70, 70, 22), //10%, 8%
        .battlePalaceFlavorText = B_MSG_GLINT_IN_EYE,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_BOLD] =
    {
        .name = COMPOUND_STRING("Bold"),
        .statUp = STAT_DEF,
        .statDown = STAT_ATK,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_BOLD, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlSupportHighDefenseLow,
        .battlePalacePercents = PALACE_STYLE(30, 20, 32, 58), //50%, 10%
        .battlePalaceFlavorText = B_MSG_GETTING_IN_POS,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_DOCILE] =
    {
        .name = COMPOUND_STRING("Docile"),
        .statUp = STAT_DEF,
        .statDown = STAT_DEF,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_DOCILE, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(56, 22, 56, 22), //22%, 22%
        .battlePalaceFlavorText = B_MSG_EAGER_FOR_MORE,
        .battlePalaceSmokescreen = PALACE_TARGET_RANDOM,
    },
    [NATURE_RELAXED] =
    {
        .name = COMPOUND_STRING("Relaxed"),
        .statUp = STAT_DEF,
        .statDown = STAT_SPEED,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_RELAXED, AFFINE_TURN_UP_AND_DOWN},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlSupportHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(25, 15, 75, 15), //60%, 10%
        .battlePalaceFlavorText = B_MSG_GLINT_IN_EYE,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_IMPISH] =
    {
        .name = COMPOUND_STRING("Impish"),
        .statUp = STAT_DEF,
        .statDown = STAT_SPATK,
        .backAnim = 0,
        .pokeBlockAnim = {ANIM_IMPISH, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighDefenseLow,
        .battlePalacePercents = PALACE_STYLE(69, 6, 28, 55), //25%, 17%
        .battlePalaceFlavorText = B_MSG_GETTING_IN_POS,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_LAX] =
    {
        .name = COMPOUND_STRING("Lax"),
        .statUp = STAT_DEF,
        .statDown = STAT_SPDEF,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_LAX, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlSupportHighSupportLow,
        .battlePalacePercents = PALACE_STYLE(35, 10, 29, 6), //55%, 65%
        .battlePalaceFlavorText = B_MSG_GROWL_DEEPLY,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_TIMID] =
    {
        .name = COMPOUND_STRING("Timid"),
        .statUp = STAT_SPEED,
        .statDown = STAT_ATK,
        .backAnim = 2,
        .pokeBlockAnim = {ANIM_TIMID, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighSupportLow,
        .battlePalacePercents = PALACE_STYLE(62, 10, 30, 20), //28%, 50%
        .battlePalaceFlavorText = B_MSG_GROWL_DEEPLY,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_HASTY] =
    {
        .name = COMPOUND_STRING("Hasty"),
        .statUp = STAT_SPEED,
        .statDown = STAT_DEF,
        .backAnim = 0,
        .pokeBlockAnim = {ANIM_HASTY, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(58, 37, 88, 6), //5%, 6%
        .battlePalaceFlavorText = B_MSG_GLINT_IN_EYE,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_SERIOUS] =
    {
        .name = COMPOUND_STRING("Serious"),
        .statUp = STAT_SPEED,
        .statDown = STAT_SPEED,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_SERIOUS, AFFINE_TURN_DOWN},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlSupportHighSupportLow,
        .battlePalacePercents = PALACE_STYLE(34, 11, 29, 11), //55%, 60%
        .battlePalaceFlavorText = B_MSG_EAGER_FOR_MORE,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_JOLLY] =
    {
        .name = COMPOUND_STRING("Jolly"),
        .statUp = STAT_SPEED,
        .statDown = STAT_SPATK,
        .backAnim = 0,
        .pokeBlockAnim = {ANIM_JOLLY, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlSupportHighDefenseLow,
        .battlePalacePercents = PALACE_STYLE(35, 5, 35, 60), //60%, 5%
        .battlePalaceFlavorText = B_MSG_GETTING_IN_POS,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_NAIVE] =
    {
        .name = COMPOUND_STRING("Naive"),
        .statUp = STAT_SPEED,
        .statDown = STAT_SPDEF,
        .backAnim = 0,
        .pokeBlockAnim = {ANIM_NAIVE, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(56, 22, 56, 22), //22%, 22%
        .battlePalaceFlavorText = B_MSG_EAGER_FOR_MORE,
        .battlePalaceSmokescreen = PALACE_TARGET_RANDOM,
    },
    [NATURE_MODEST] =
    {
        .name = COMPOUND_STRING("Modest"),
        .statUp = STAT_SPATK,
        .statDown = STAT_ATK,
        .backAnim = 2,
        .pokeBlockAnim = {ANIM_MODEST, AFFINE_TURN_DOWN_SLOW},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlDefenseHighDefenseLow,
        .battlePalacePercents = PALACE_STYLE(35, 45, 34, 60), //20%, 6%
        .battlePalaceFlavorText = B_MSG_GETTING_IN_POS,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_MILD] =
    {
        .name = COMPOUND_STRING("Mild"),
        .statUp = STAT_SPATK,
        .statDown = STAT_DEF,
        .backAnim = 2,
        .pokeBlockAnim = {ANIM_MILD, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlDefenseHighSupportLow,
        .battlePalacePercents = PALACE_STYLE(44, 50, 34, 6), //6%, 60%
        .battlePalaceFlavorText = B_MSG_GROWL_DEEPLY,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_QUIET] =
    {
        .name = COMPOUND_STRING("Quiet"),
        .statUp = STAT_SPATK,
        .statDown = STAT_SPEED,
        .backAnim = 2,
        .pokeBlockAnim = {ANIM_QUIET, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(56, 22, 56, 22), //22%, 22%
        .battlePalaceFlavorText = B_MSG_EAGER_FOR_MORE,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_BASHFUL] =
    {
        .name = COMPOUND_STRING("Bashful"),
        .statUp = STAT_SPATK,
        .statDown = STAT_SPATK,
        .backAnim = 2,
        .pokeBlockAnim = {ANIM_BASHFUL, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlDefenseHighDefenseLow,
        .battlePalacePercents = PALACE_STYLE(30, 58, 30, 58), //12%, 12%
        .battlePalaceFlavorText = B_MSG_EAGER_FOR_MORE,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_RASH] =
    {
        .name = COMPOUND_STRING("Rash"),
        .statUp = STAT_SPATK,
        .statDown = STAT_SPDEF,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_RASH, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlSupportHighSupportLow,
        .battlePalacePercents = PALACE_STYLE(30, 13, 27, 6), //57%, 67%
        .battlePalaceFlavorText = B_MSG_GROWL_DEEPLY,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_CALM] =
    {
        .name = COMPOUND_STRING("Calm"),
        .statUp = STAT_SPDEF,
        .statDown = STAT_ATK,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_CALM, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlDefenseHighDefenseLow,
        .battlePalacePercents = PALACE_STYLE(40, 50, 25, 62), //10%, 13%
        .battlePalaceFlavorText = B_MSG_GETTING_IN_POS,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_GENTLE] =
    {
        .name = COMPOUND_STRING("Gentle"),
        .statUp = STAT_SPDEF,
        .statDown = STAT_DEF,
        .backAnim = 2,
        .pokeBlockAnim = {ANIM_GENTLE, AFFINE_TURN_DOWN_SLIGHT},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlDefenseHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(18, 70, 90, 5), //12%, 5%
        .battlePalaceFlavorText = B_MSG_GLINT_IN_EYE,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
    [NATURE_SASSY] =
    {
        .name = COMPOUND_STRING("Sassy"),
        .statUp = STAT_SPDEF,
        .statDown = STAT_SPEED,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_SASSY, AFFINE_TURN_UP_HIGH},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighSupportLow,
        .battlePalacePercents = PALACE_STYLE(88, 6, 22, 20), //6%, 58%
        .battlePalaceFlavorText = B_MSG_GROWL_DEEPLY,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_CAREFUL] =
    {
        .name = COMPOUND_STRING("Careful"),
        .statUp = STAT_SPDEF,
        .statDown = STAT_SPATK,
        .backAnim = 2,
        .pokeBlockAnim = {ANIM_CAREFUL, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlDefenseHighSupportLow,
        .battlePalacePercents = PALACE_STYLE(42, 50, 42, 5), //8%, 53%
        .battlePalaceFlavorText = B_MSG_GROWL_DEEPLY,
        .battlePalaceSmokescreen = PALACE_TARGET_WEAKER,
    },
    [NATURE_QUIRKY] =
    {
        .name = COMPOUND_STRING("Quirky"),
        .statUp = STAT_SPDEF,
        .statDown = STAT_SPDEF,
        .backAnim = 1,
        .pokeBlockAnim = {ANIM_QUIRKY, AFFINE_NONE},
        .natureGirlMessage = BattleFrontier_Lounge5_Text_NatureGirlAttackHighAttackLow,
        .battlePalacePercents = PALACE_STYLE(56, 22, 56, 22), //22%, 22%
        .battlePalaceFlavorText = B_MSG_EAGER_FOR_MORE,
        .battlePalaceSmokescreen = PALACE_TARGET_STRONGER,
    },
};

#include "data/graphics/pokemon.h"

#include "data/pokemon/trainer_class_lookups.h"
#include "data/pokemon/experience_tables.h"

#if P_LVL_UP_LEARNSETS >= GEN_9
#include "data/pokemon/level_up_learnsets/gen_9.h" // Scarlet/Violet
#elif P_LVL_UP_LEARNSETS >= GEN_8
#include "data/pokemon/level_up_learnsets/gen_8.h" // Sword/Shield
#elif P_LVL_UP_LEARNSETS >= GEN_7
#include "data/pokemon/level_up_learnsets/gen_7.h" // Ultra Sun/Ultra Moon
#elif P_LVL_UP_LEARNSETS >= GEN_6
#include "data/pokemon/level_up_learnsets/gen_6.h" // Omega Ruby/Alpha Sapphire
#elif P_LVL_UP_LEARNSETS >= GEN_5
#include "data/pokemon/level_up_learnsets/gen_5.h" // Black 2/White 2
#elif P_LVL_UP_LEARNSETS >= GEN_4
#include "data/pokemon/level_up_learnsets/gen_4.h" // HeartGold/SoulSilver
#elif P_LVL_UP_LEARNSETS >= GEN_3
#include "data/pokemon/level_up_learnsets/gen_3.h" // Ruby/Sapphire/Emerald
#elif P_LVL_UP_LEARNSETS >= GEN_2
#include "data/pokemon/level_up_learnsets/gen_2.h" // Crystal
#elif P_LVL_UP_LEARNSETS >= GEN_1
#include "data/pokemon/level_up_learnsets/gen_1.h" // Yellow
#endif

#include "data/pokemon/teachable_learnsets.h"
#include "data/pokemon/egg_moves.h"
#include "data/pokemon/form_species_tables.h"
#include "data/pokemon/form_change_tables.h"
#include "data/pokemon/form_change_table_pointers.h"
#include "data/pokemon/wild_encounter_ow_behavior.h"
#include "data/object_events/object_event_pic_tables_followers.h"

#include "data/pokemon/species_info.h"

#define PP_UP_SHIFTS(val)           val,        (val) << 2,        (val) << 4,        (val) << 6
#define PP_UP_SHIFTS_INV(val) (u8)~(val), (u8)~((val) << 2), (u8)~((val) << 4), (u8)~((val) << 6)

// PP Up bonuses are stored for a Pokémon as a single byte.
// There are 2 bits (a value 0-3) for each move slot that
// represent how many PP Ups have been applied.
// The following arrays take a move slot id and return:
// gPPUpGetMask - A mask to get the number of PP Ups applied to that move slot
// gPPUpClearMask - A mask to clear the number of PP Ups applied to that move slot
// gPPUpAddValues - A value to add to the PP Bonuses byte to apply 1 PP Up to that move slot
const u8 gPPUpGetMask[MAX_MON_MOVES]   = {PP_UP_SHIFTS(3)};
const u8 gPPUpClearMask[MAX_MON_MOVES] = {PP_UP_SHIFTS_INV(3)};
const u8 gPPUpAddValues[MAX_MON_MOVES] = {PP_UP_SHIFTS(1)};

const u8 gStatStageRatios[MAX_STAT_STAGE + 1][2] =
{
    {10, 40}, // -6, MIN_STAT_STAGE
    {10, 35}, // -5
    {10, 30}, // -4
    {10, 25}, // -3
    {10, 20}, // -2
    {10, 15}, // -1
    {10, 10}, //  0, DEFAULT_STAT_STAGE
    {15, 10}, // +1
    {20, 10}, // +2
    {25, 10}, // +3
    {30, 10}, // +4
    {35, 10}, // +5
    {40, 10}, // +6, MAX_STAT_STAGE
};

// The classes used by other players in the Union Room.
// These should correspond with the overworld graphics in sUnionRoomObjGfxIds
const u16 gUnionRoomFacilityClasses[NUM_UNION_ROOM_CLASSES * GENDER_COUNT] =
{
    // Male classes
    FACILITY_CLASS_COOLTRAINER_M,
    FACILITY_CLASS_BLACK_BELT,
    FACILITY_CLASS_CAMPER,
    FACILITY_CLASS_YOUNGSTER,
    FACILITY_CLASS_PSYCHIC_M,
    FACILITY_CLASS_BUG_CATCHER,
    FACILITY_CLASS_PKMN_BREEDER_M,
    FACILITY_CLASS_GUITARIST,
    // Female classes
    FACILITY_CLASS_COOLTRAINER_F,
    FACILITY_CLASS_HEX_MANIAC,
    FACILITY_CLASS_PICNICKER,
    FACILITY_CLASS_LASS,
    FACILITY_CLASS_PSYCHIC_F,
    FACILITY_CLASS_BATTLE_GIRL,
    FACILITY_CLASS_PKMN_BREEDER_F,
    FACILITY_CLASS_BEAUTY
};

const struct SpriteTemplate gBattlerSpriteTemplates[MAX_BATTLERS_COUNT] =
{
    [B_POSITION_PLAYER_LEFT] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gBattlerPicTable_PlayerLeft,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [B_POSITION_OPPONENT_LEFT] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpriteOpponentSide,
        .anims = NULL,
        .images = gBattlerPicTable_OpponentLeft,
        .affineAnims = gAffineAnims_BattleSpriteOpponentSide,
        .callback = SpriteCB_WildMon,
    },
    [B_POSITION_PLAYER_RIGHT] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpritePlayerSide,
        .anims = NULL,
        .images = gBattlerPicTable_PlayerRight,
        .affineAnims = gAffineAnims_BattleSpritePlayerSide,
        .callback = SpriteCB_BattleSpriteStartSlideLeft,
    },
    [B_POSITION_OPPONENT_RIGHT] = {
        .tileTag = TAG_NONE,
        .paletteTag = 0,
        .oam = &gOamData_BattleSpriteOpponentSide,
        .anims = NULL,
        .images = gBattlerPicTable_OpponentRight,
        .affineAnims = gAffineAnims_BattleSpriteOpponentSide,
        .callback = SpriteCB_WildMon
    },
};

static const struct SpriteTemplate sTrainerBackSpriteTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = 0,
    .oam = &gOamData_BattleSpritePlayerSide,
    .anims = NULL,
    .affineAnims = gAffineAnims_BattleSpritePlayerSide,
    .callback = SpriteCB_BattleSpriteStartSlideLeft,
};

#define NUM_SECRET_BASE_CLASSES 5
static const u8 sSecretBaseFacilityClasses[GENDER_COUNT][NUM_SECRET_BASE_CLASSES] =
{
    [MALE] = {
        FACILITY_CLASS_YOUNGSTER,
        FACILITY_CLASS_BUG_CATCHER,
        FACILITY_CLASS_RICH_BOY,
        FACILITY_CLASS_CAMPER,
        FACILITY_CLASS_COOLTRAINER_M
    },
    [FEMALE] = {
        FACILITY_CLASS_LASS,
        FACILITY_CLASS_SCHOOL_KID_F,
        FACILITY_CLASS_LADY,
        FACILITY_CLASS_PICNICKER,
        FACILITY_CLASS_COOLTRAINER_F
    }
};

static const u8 sGetMonDataEVConstants[] =
{
    MON_DATA_HP_EV,
    MON_DATA_ATK_EV,
    MON_DATA_DEF_EV,
    MON_DATA_SPEED_EV,
    MON_DATA_SPDEF_EV,
    MON_DATA_SPATK_EV
};

// For stat-raising items
static const enum Stat sStatsToRaise[] =
{
    STAT_ATK, STAT_ATK, STAT_DEF, STAT_SPEED, STAT_SPATK, STAT_SPDEF, STAT_ACC
};

// 3 modifiers each for how much to change friendship for different ranges
// 0-99, 100-199, 200+
static const s8 sFriendshipEventModifiers[][3] =
{
    [FRIENDSHIP_EVENT_GROW_LEVEL]      = { 5,  3,  2},
    [FRIENDSHIP_EVENT_VITAMIN]         = { 5,  3,  2},
    [FRIENDSHIP_EVENT_BATTLE_ITEM]     = { 1,  1,  0},
    [FRIENDSHIP_EVENT_LEAGUE_BATTLE]   = { 3,  2,  1},
    [FRIENDSHIP_EVENT_LEARN_TMHM]      = { 1,  1,  0},
    [FRIENDSHIP_EVENT_WALKING]         = { 1,  1,  1},
    [FRIENDSHIP_EVENT_FAINT_SMALL]     = {-1, -1, -1},
    [FRIENDSHIP_EVENT_FAINT_FIELD_PSN] = {-5, -5, -10},
    [FRIENDSHIP_EVENT_FAINT_LARGE]     = {-5, -5, -10},
    [FRIENDSHIP_EVENT_MASSAGE]         = { 3,  3,  3 },
};

static const struct SpeciesItem sAlteringCaveWildMonHeldItems[] =
{
    {SPECIES_NONE,      ITEM_NONE},
    {SPECIES_MAREEP,    ITEM_GANLON_BERRY},
    {SPECIES_PINECO,    ITEM_APICOT_BERRY},
    {SPECIES_HOUNDOUR,  ITEM_BIG_MUSHROOM},
    {SPECIES_TEDDIURSA, ITEM_PETAYA_BERRY},
    {SPECIES_AIPOM,     ITEM_BERRY_JUICE},
    {SPECIES_SHUCKLE,   ITEM_BERRY_JUICE},
    {SPECIES_STANTLER,  ITEM_PETAYA_BERRY},
    {SPECIES_SMEARGLE,  ITEM_SALAC_BERRY},
};

static const struct OamData sOamData_64x64 =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct SpriteTemplate sSpriteTemplate_64x64 =
{
    .tileTag = TAG_NONE,
    .paletteTag = TAG_NONE,
    .oam = &sOamData_64x64,
};

// NOTE: Reordering this array will break compatibility with existing
// saves.
static const u32 sCompressedStatuses[] =
{
    STATUS1_NONE,
    STATUS1_SLEEP_TURN(1),
    STATUS1_SLEEP_TURN(2),
    STATUS1_SLEEP_TURN(3),
    STATUS1_SLEEP_TURN(4),
    STATUS1_SLEEP_TURN(5),
    STATUS1_POISON,
    STATUS1_BURN,
    STATUS1_FREEZE,
    STATUS1_PARALYSIS,
    STATUS1_TOXIC_POISON,
    STATUS1_FROSTBITE,
};

// Attempt to detect situations where the BoxPokemon struct is unable to
// contain all the values.
// TODO: Is it possible to compute:
// - The maximum experience.
// - The maximum PP.
// - The maximum HP.
// - The maximum form countdown.

// The following definition of sBoxPokemonConstantsFit and STATIC_ASSERTS
// will prevent developers from compiling the game if the value of the
// constant on the left does not fit within the number of bits defined
// in BoxPokemon or its substructs (currently located in include/pokemon.h).

// To successfully compile, developers will need to do one of the following:
// 1) Decrease the size of the constant.
// 2) Increase the number of bits both on the struct AND in the corresponding assert. This will likely break user's saves unless there is free space after the member that is being adjsted.
// 3) Repurpose unused IDs.

// EXAMPLES
// If a developer has added enough new items so that ITEMS_COUNT now equals 1200, they could...
// 1) remove new items until ITEMS_COUNT is 1023, the max value that will fit in 10 bits.
// 2) change heldItem:10 to heldItem:11 AND change the below assert for ITEMS_COUNT to check for (1 << 11).
// 3) repurpose IDs from other items that aren't being used, like ITEM_GOLD_TEETH or ITEM_SS_TICKET until ITEMS_COUNT equals 1023, the max value that will fit in 10 bits.

UNUSED static const struct BoxPokemon sBoxPokemonConstantsFit =
{
    .language = NUM_LANGUAGES - 1,
    .hiddenNatureModifier = NUM_NATURES - 1,
    .compressedStatus = ARRAY_COUNT(sCompressedStatuses) - 1,
    .secure.substructs[0].type0 = {
        .species = NUM_SPECIES - 1,
        .teraType = NUMBER_OF_MON_TYPES - 1,
        .heldItem = ITEMS_COUNT - 1,
        .pokeball = POKEBALL_COUNT - 1,
    },
    .secure.substructs[1].type1 = {
        .move1 = MOVES_COUNT_ALL - 1,
        .move2 = MOVES_COUNT_ALL - 1,
        .move3 = MOVES_COUNT_ALL - 1,
        .move4 = MOVES_COUNT_ALL - 1,
    },
    .secure.substructs[2].type2 = {
        .hpEV = MAX_PER_STAT_EVS,
        .attackEV = MAX_PER_STAT_EVS,
        .defenseEV = MAX_PER_STAT_EVS,
        .speedEV = MAX_PER_STAT_EVS,
        .spAttackEV = MAX_PER_STAT_EVS,
        .spDefenseEV = MAX_PER_STAT_EVS,
    },
    .secure.substructs[3].type3 = {
        .metLocation = min(MAPSEC_COUNT, min(METLOC_SPECIAL_EGG, min(METLOC_IN_GAME_TRADE, METLOC_FATEFUL_ENCOUNTER))),
        .metLevel = MAX_LEVEL,
        .metGame = NUM_VERSIONS, // NOTE: NUM_VERSIONS is inclusive!
        .dynamaxLevel = MAX_DYNAMAX_LEVEL,
        .otGender = GENDER_COUNT - 1,
        .hpIV = MAX_PER_STAT_IVS,
        .attackIV = MAX_PER_STAT_IVS,
        .defenseIV = MAX_PER_STAT_IVS,
        .speedIV = MAX_PER_STAT_IVS,
        .spAttackIV = MAX_PER_STAT_IVS,
        .spDefenseIV = MAX_PER_STAT_IVS,
        .abilityNum = NUM_ABILITY_SLOTS - 1,
    },
};

STATIC_ASSERT(MAX_LEVEL <= 100, PokemonSubstruct0_experience_PotentiallyTooSmall); // Maximum of ~2 million exp.

static u32 CompressStatus(u32 status)
{
    s32 i;
    for (i = 0; i < ARRAY_COUNT(sCompressedStatuses); i++)
    {
        if (sCompressedStatuses[i] == status)
            return i;
    }
    return 0; // STATUS1_NONE
}

static u32 UncompressStatus(u32 compressedStatus)
{
    if (compressedStatus < ARRAY_COUNT(sCompressedStatuses))
        return sCompressedStatuses[compressedStatus];
    else
        return STATUS1_NONE;
}

void ZeroBoxMonData(struct BoxPokemon *boxMon)
{
    u8 *raw = (u8 *)boxMon;
    u32 i;
    for (i = 0; i < sizeof(struct BoxPokemon); i++)
        raw[i] = 0;
}

void ZeroMonData(struct Pokemon *mon)
{
    u32 arg;
    ZeroBoxMonData(&mon->box);
    arg = 0;
    SetMonData(mon, MON_DATA_STATUS, &arg);
    SetMonData(mon, MON_DATA_LEVEL, &arg);
    SetMonData(mon, MON_DATA_HP, &arg);
    SetMonData(mon, MON_DATA_MAX_HP, &arg);
    SetMonData(mon, MON_DATA_ATK, &arg);
    SetMonData(mon, MON_DATA_DEF, &arg);
    SetMonData(mon, MON_DATA_SPEED, &arg);
    SetMonData(mon, MON_DATA_SPATK, &arg);
    SetMonData(mon, MON_DATA_SPDEF, &arg);
    arg = MAIL_NONE;
    SetMonData(mon, MON_DATA_MAIL, &arg);
}

void ZeroPartyMons(struct Pokemon *party)
{
    for (s32 i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&party[i]);
}

void ZeroPlayerPartyMons(void)
{
    for (s32 i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gParties[B_TRAINER_PLAYER][i]);
    gPartiesCount[B_TRAINER_PLAYER] = 0;
}

void ZeroEnemyPartyMons(void)
{
    for (s32 i = 0; i < PARTY_SIZE; i++)
    {
        ZeroMonData(&gParties[B_TRAINER_OPPONENT_A][i]);
        ZeroMonData(&gParties[B_TRAINER_OPPONENT_B][i]);
    }
    gPartiesCount[B_TRAINER_OPPONENT_A] = 0;
    gPartiesCount[B_TRAINER_OPPONENT_B] = 0;
}

void CreateRandomMon(struct Pokemon *mon, enum Species species, u8 level)
{
    CreateRandomMonWithIVs(mon, species, level, USE_RANDOM_IVS);
}

void CreateRandomMonWithIVs(struct Pokemon *mon, enum Species species, u8 level, u8 fixedIv)
{
    CreateMonWithIVs(mon, species, level, Random32(), OTID_STRUCT_PLAYER_ID, fixedIv);
    GiveMonInitialMoveset(mon);
}

void CreateMon(struct Pokemon *mon, enum Species species, u8 level, u32 personality, struct OriginalTrainerId trainerId)
{
    u32 mail;
    ZeroMonData(mon);
    CreateBoxMon(&mon->box, species, level, personality, trainerId);
    SetMonData(mon, MON_DATA_LEVEL, &level);
    mail = MAIL_NONE;
    SetMonData(mon, MON_DATA_MAIL, &mail);
}

void CreateMonWithIVs(struct Pokemon *mon, enum Species species, u8 level, u32 personality, struct OriginalTrainerId trainerId, u8 fixedIV)
{
    CreateMon(mon, species, level, personality, trainerId);
    SetBoxMonIVs(&mon->box, fixedIV);
    CalculateMonStats(mon);
}

bool32 ComputePlayerShinyOdds(u32 personality, u32 value)
{
    if (P_FLAG_FORCE_NO_SHINY != 0 && FlagGet(P_FLAG_FORCE_NO_SHINY))
        return FALSE;

    if (P_FLAG_FORCE_SHINY != 0 && FlagGet(P_FLAG_FORCE_SHINY))
        return TRUE;

    if (P_ONLY_OBTAINABLE_SHINIES && (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE || (FlagGet(WE_FLAG_NO_CATCHING))))
        return FALSE;

    if (P_NO_SHINIES_WITHOUT_POKEBALLS && !HasAtLeastOnePokeBall())
        return FALSE;

    u32 totalRerolls = 0;

    if (CheckBagHasItem(ITEM_SHINY_CHARM, 1))
        totalRerolls += I_SHINY_CHARM_ADDITIONAL_ROLLS;

    if (LURE_STEP_COUNT != 0)
        totalRerolls += 1;

    totalRerolls += CalculateChainFishingShinyRolls();

    if (gDexNavSpecies)
        totalRerolls += CalculateDexNavShinyRolls();

    while (GET_SHINY_VALUE(value, personality) >= SHINY_ODDS && totalRerolls > 0)
    {
        personality = Random32();
        totalRerolls--;
    }

    return GET_SHINY_VALUE(value, personality) < SHINY_ODDS;
}

void SetBoxMonIVs(struct BoxPokemon *mon, u8 fixedIV)
{
    u32 i, value;

    if (fixedIV < USE_RANDOM_IVS)
    {
        for (i = 0; i < NUM_STATS; i++)
            SetBoxMonData(mon, MON_DATA_HP_IV + i, &fixedIV);
        return;
    }

    u32 iv;
    u32 ivRandom = Random32();
    enum Species species = GetBoxMonData(mon, MON_DATA_SPECIES);
    value = (u16)ivRandom;

    iv = value & MAX_IV_MASK;
    SetBoxMonData(mon, MON_DATA_HP_IV, &iv);
    iv = (value & (MAX_IV_MASK << 5)) >> 5;
    SetBoxMonData(mon, MON_DATA_ATK_IV, &iv);
    iv = (value & (MAX_IV_MASK << 10)) >> 10;
    SetBoxMonData(mon, MON_DATA_DEF_IV, &iv);

    value = (u16)(ivRandom >> 16);

    iv = value & MAX_IV_MASK;
    SetBoxMonData(mon, MON_DATA_SPEED_IV, &iv);
    iv = (value & (MAX_IV_MASK << 5)) >> 5;
    SetBoxMonData(mon, MON_DATA_SPATK_IV, &iv);
    iv = (value & (MAX_IV_MASK << 10)) >> 10;
    SetBoxMonData(mon, MON_DATA_SPDEF_IV, &iv);

    SetBoxMonPerfectIVs(mon, gSpeciesInfo[species].perfectIVCount);
}

void SetBoxMonPerfectIVs(struct BoxPokemon *mon, u32 numPerfect)
{
    if (!numPerfect)
        return;

    u32 i, iv = MAX_PER_STAT_IVS;
    if (numPerfect >= NUM_STATS)
    {
        for (i = 0; i < NUM_STATS; i++)
            SetBoxMonData(mon, MON_DATA_HP_IV + i, &iv);
        return;
    }

    enum Stat availableIVs[NUM_STATS];
    enum Stat selectedIvs[NUM_STATS];
    // Initialize a list of IV indices.
    for (i = 0; i < NUM_STATS; i++)
        availableIVs[i] = i;

    // Select the IVs that will be perfected.
    for (i = 0; i < numPerfect; i++)
    {
        u8 index = Random() % (NUM_STATS - i);
        selectedIvs[i] = availableIVs[index];
        RemoveIVIndexFromList(availableIVs, index);
        SetBoxMonData(mon, MON_DATA_HP_IV + selectedIvs[i], &iv);
    }
}

void CreateBoxMon(struct BoxPokemon *boxMon, enum Species species, u8 level, u32 personality, struct OriginalTrainerId trainerId)
{
    u8 speciesName[POKEMON_NAME_LENGTH + 1];
    u32 value;
    u16 checksum;
    bool32 isShiny;

    ZeroBoxMonData(boxMon);
    // Determine original trainer ID
    if (trainerId.method == OT_ID_RANDOM_NO_SHINY)
    {
        value = Random32();
        isShiny = FALSE;
    }
    else if (trainerId.method == OT_ID_PRESET)
    {
        value = trainerId.value;
        isShiny = GET_SHINY_VALUE(value, personality) < SHINY_ODDS;
    }
    else // Player is the OT
    {
        value = READ_OTID_FROM_SAVE;
        isShiny = ComputePlayerShinyOdds(personality, value);
    }

    SetBoxMonData(boxMon, MON_DATA_PERSONALITY, &personality);
    SetBoxMonData(boxMon, MON_DATA_OT_ID, &value);

    checksum = CalculateBoxMonChecksum(boxMon);
    SetBoxMonData(boxMon, MON_DATA_CHECKSUM, &checksum);
    EncryptBoxMon(boxMon);
    SetBoxMonData(boxMon, MON_DATA_IS_SHINY, &isShiny);
    StringCopy(speciesName, GetSpeciesName(species));
    SetBoxMonData(boxMon, MON_DATA_NICKNAME, speciesName);
    SetBoxMonData(boxMon, MON_DATA_LANGUAGE, &gGameLanguage);
    SetBoxMonData(boxMon, MON_DATA_OT_NAME, gSaveBlock2Ptr->playerName);
    SetBoxMonData(boxMon, MON_DATA_SPECIES, &species);
    SetBoxMonData(boxMon, MON_DATA_EXP, &gExperienceTables[gSpeciesInfo[species].growthRate][level]);
    SetBoxMonData(boxMon, MON_DATA_FRIENDSHIP, &gSpeciesInfo[species].friendship);
    value = GetCurrentRegionMapSectionId();
    SetBoxMonData(boxMon, MON_DATA_MET_LOCATION, &value);
    SetBoxMonData(boxMon, MON_DATA_MET_LEVEL, &level);
    SetBoxMonData(boxMon, MON_DATA_MET_GAME, &gGameVersion);
    value = BALL_POKE;
    SetBoxMonData(boxMon, MON_DATA_POKEBALL, &value);
    SetBoxMonData(boxMon, MON_DATA_OT_GENDER, &gSaveBlock2Ptr->playerGender);

    value = boxMon->personality & 0x1;
    u32 teraType = value == 0 ? GetSpeciesType(species, 0) : GetSpeciesType(species, 1);
    SetBoxMonData(boxMon, MON_DATA_TERA_TYPE, &teraType);
    //using gen 3-4 ability formula, it was changed in later gens
    if (GetSpeciesAbility(species, 1))
        SetBoxMonData(boxMon, MON_DATA_ABILITY_NUM, &value);
}

static bool32 IsValidGender(u32 gender)
{
    switch (gender)
    {
    case MON_MALE:
    case MON_FEMALE:
    case MON_GENDERLESS:
    case MON_GENDER_RANDOM:
        return TRUE;
    default:
        return FALSE;
    }
}

static void CleanIncompatibleGenderSpecies(enum Species species, u8 *gender)
{
    switch (gSpeciesInfo[species].genderRatio)
    {
    case MON_MALE:
    case MON_FEMALE:
    case MON_GENDERLESS:
        *gender = MON_GENDER_RANDOM;
        return;
    }
    if (*gender == MON_GENDERLESS)
        *gender = MON_GENDER_RANDOM;
}

u32 GetMonPersonality(enum Species species, u8 gender, u8 nature, u8 unownLetter)
{
    u32 personality, actualLetter;

    assertf(IsValidGender(gender), "invalid gender: %d", gender)
    {
        gender = MON_GENDER_RANDOM;
    }

    assertf(nature <= NATURE_RANDOM, "invalid nature: %d", nature)
    {
        nature = NATURE_RANDOM;
    }

    assertf(unownLetter <= NUM_UNOWN_FORMS, "invalid letter: %d", unownLetter)
    {
        unownLetter = RANDOM_UNOWN_LETTER;
    }

    CleanIncompatibleGenderSpecies(species, &gender);
    do
    {
        personality = Random32();
        actualLetter = GET_UNOWN_LETTER(personality);
    }
    while ((nature != GetNatureFromPersonality(personality) && nature != NATURE_RANDOM)
            || (gender != MON_GENDER_RANDOM && gender != GetGenderFromSpeciesAndPersonality(species, personality))
            || ((actualLetter != unownLetter - 1) && unownLetter > 0));
    return personality;
}

// This is only used to create Wally's Ralts.
void CreateMaleMon(struct Pokemon *mon, enum Species species, u8 level)
{
    u32 personality = GetMonPersonality(species, MON_MALE, NATURE_RANDOM, RANDOM_UNOWN_LETTER);
    CreateMonWithIVs(mon, species, level, personality, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    GiveMonInitialMoveset(mon);
}

void CreateMonWithIVsPersonality(struct Pokemon *mon, enum Species species, u8 level, u32 ivs, u32 personality)
{
    CreateMon(mon, species, level, personality, OTID_STRUCT_PLAYER_ID);
    SetMonData(mon, MON_DATA_IVS, &ivs);
    CalculateMonStats(mon);
    GiveMonInitialMoveset(mon);
}

void CreateBattleTowerMon(struct Pokemon *mon, struct BattleTowerPokemon *src)
{
    s32 i;
    u8 nickname[max(32, POKEMON_NAME_BUFFER_SIZE)];
    enum Language language;
    u8 value;

    CreateMon(mon, src->species, src->level, src->personality, OTID_STRUCT_PRESET(src->otId));

    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, src->moves[i], i);

    SetMonData(mon, MON_DATA_PP_BONUSES, &src->ppBonuses);
    SetMonData(mon, MON_DATA_HELD_ITEM, &src->heldItem);
    SetMonData(mon, MON_DATA_FRIENDSHIP, &src->friendship);

    StringCopy(nickname, src->nickname);

    if (nickname[0] == EXT_CTRL_CODE_BEGIN && nickname[1] == EXT_CTRL_CODE_JPN)
    {
        language = LANGUAGE_JAPANESE;
        StripExtCtrlCodes(nickname);
    }
    else
    {
        language = GAME_LANGUAGE;
    }

    SetMonData(mon, MON_DATA_LANGUAGE, &language);
    SetMonData(mon, MON_DATA_NICKNAME, nickname);
    SetMonData(mon, MON_DATA_HP_EV, &src->hpEV);
    SetMonData(mon, MON_DATA_ATK_EV, &src->attackEV);
    SetMonData(mon, MON_DATA_DEF_EV, &src->defenseEV);
    SetMonData(mon, MON_DATA_SPEED_EV, &src->speedEV);
    SetMonData(mon, MON_DATA_SPATK_EV, &src->spAttackEV);
    SetMonData(mon, MON_DATA_SPDEF_EV, &src->spDefenseEV);
    value = src->abilityNum;
    SetMonData(mon, MON_DATA_ABILITY_NUM, &value);
    value = src->hpIV;
    SetMonData(mon, MON_DATA_HP_IV, &value);
    value = src->attackIV;
    SetMonData(mon, MON_DATA_ATK_IV, &value);
    value = src->defenseIV;
    SetMonData(mon, MON_DATA_DEF_IV, &value);
    value = src->speedIV;
    SetMonData(mon, MON_DATA_SPEED_IV, &value);
    value = src->spAttackIV;
    SetMonData(mon, MON_DATA_SPATK_IV, &value);
    value = src->spDefenseIV;
    SetMonData(mon, MON_DATA_SPDEF_IV, &value);
    MonRestorePP(mon);
    CalculateMonStats(mon);
}

void CreateBattleTowerMon_HandleLevel(struct Pokemon *mon, struct BattleTowerPokemon *src, bool8 lvl50)
{
    s32 i;
    u8 nickname[max(32, POKEMON_NAME_BUFFER_SIZE)];
    u8 level;
    enum Language language;
    u8 value;

    if (gSaveBlock2Ptr->frontier.lvlMode != FRONTIER_LVL_50)
        level = GetFrontierEnemyMonLevel(gSaveBlock2Ptr->frontier.lvlMode);
    else if (lvl50)
        level = FRONTIER_MAX_LEVEL_50;
    else
        level = src->level;

    CreateMon(mon, src->species, level, src->personality, OTID_STRUCT_PRESET(src->otId));

    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, src->moves[i], i);

    SetMonData(mon, MON_DATA_PP_BONUSES, &src->ppBonuses);
    SetMonData(mon, MON_DATA_HELD_ITEM, &src->heldItem);
    SetMonData(mon, MON_DATA_FRIENDSHIP, &src->friendship);

    StringCopy(nickname, src->nickname);

    if (nickname[0] == EXT_CTRL_CODE_BEGIN && nickname[1] == EXT_CTRL_CODE_JPN)
    {
        language = LANGUAGE_JAPANESE;
        StripExtCtrlCodes(nickname);
    }
    else
    {
        language = GAME_LANGUAGE;
    }

    SetMonData(mon, MON_DATA_LANGUAGE, &language);
    SetMonData(mon, MON_DATA_NICKNAME, nickname);
    SetMonData(mon, MON_DATA_HP_EV, &src->hpEV);
    SetMonData(mon, MON_DATA_ATK_EV, &src->attackEV);
    SetMonData(mon, MON_DATA_DEF_EV, &src->defenseEV);
    SetMonData(mon, MON_DATA_SPEED_EV, &src->speedEV);
    SetMonData(mon, MON_DATA_SPATK_EV, &src->spAttackEV);
    SetMonData(mon, MON_DATA_SPDEF_EV, &src->spDefenseEV);
    value = src->abilityNum;
    SetMonData(mon, MON_DATA_ABILITY_NUM, &value);
    value = src->hpIV;
    SetMonData(mon, MON_DATA_HP_IV, &value);
    value = src->attackIV;
    SetMonData(mon, MON_DATA_ATK_IV, &value);
    value = src->defenseIV;
    SetMonData(mon, MON_DATA_DEF_IV, &value);
    value = src->speedIV;
    SetMonData(mon, MON_DATA_SPEED_IV, &value);
    value = src->spAttackIV;
    SetMonData(mon, MON_DATA_SPATK_IV, &value);
    value = src->spDefenseIV;
    SetMonData(mon, MON_DATA_SPDEF_IV, &value);
    MonRestorePP(mon);
    CalculateMonStats(mon);
}

void CreateApprenticeMon(struct Pokemon *mon, const struct Apprentice *src, u8 monId)
{
    s32 i;
    u16 evAmount;
    u8 language;
    u32 otId = gApprentices[src->id].otId;
    u32 personality = ((gApprentices[src->id].otId >> 8) | ((gApprentices[src->id].otId & 0xFF) << 8))
                    + src->party[monId].species + src->number;

    CreateMonWithIVs(mon,
              src->party[monId].species,
              GetFrontierEnemyMonLevel(src->lvlMode - 1),
              personality,
              OTID_STRUCT_PRESET(otId),
              MAX_PER_STAT_IVS);
    SetMonData(mon, MON_DATA_HELD_ITEM, &src->party[monId].item);
    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, src->party[monId].moves[i], i);

    evAmount = MAX_TOTAL_EVS / NUM_STATS;
    for (i = 0; i < NUM_STATS; i++)
        SetMonData(mon, MON_DATA_HP_EV + i, &evAmount);

    language = src->language;
    SetMonData(mon, MON_DATA_LANGUAGE, &language);
    SetMonData(mon, MON_DATA_OT_NAME, GetApprenticeNameInLanguage(src->id, language));
    CalculateMonStats(mon);
}

void ConvertPokemonToBattleTowerPokemon(struct Pokemon *mon, struct BattleTowerPokemon *dest)
{
    s32 i;
    u16 heldItem;

    dest->species = GetMonData(mon, MON_DATA_SPECIES);
    heldItem = GetMonData(mon, MON_DATA_HELD_ITEM);

    if (heldItem == ITEM_ENIGMA_BERRY_E_READER)
        heldItem = ITEM_NONE;

    dest->heldItem = heldItem;

    for (i = 0; i < MAX_MON_MOVES; i++)
        dest->moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);

    dest->level = GetMonData(mon, MON_DATA_LEVEL);
    dest->ppBonuses = GetMonData(mon, MON_DATA_PP_BONUSES);
    dest->otId = GetMonData(mon, MON_DATA_OT_ID);
    dest->hpEV = GetMonData(mon, MON_DATA_HP_EV);
    dest->attackEV = GetMonData(mon, MON_DATA_ATK_EV);
    dest->defenseEV = GetMonData(mon, MON_DATA_DEF_EV);
    dest->speedEV = GetMonData(mon, MON_DATA_SPEED_EV);
    dest->spAttackEV = GetMonData(mon, MON_DATA_SPATK_EV);
    dest->spDefenseEV = GetMonData(mon, MON_DATA_SPDEF_EV);
    dest->friendship = GetMonData(mon, MON_DATA_FRIENDSHIP);
    dest->hpIV = GetMonData(mon, MON_DATA_HP_IV);
    dest->attackIV = GetMonData(mon, MON_DATA_ATK_IV);
    dest->defenseIV = GetMonData(mon, MON_DATA_DEF_IV);
    dest->speedIV  = GetMonData(mon, MON_DATA_SPEED_IV);
    dest->spAttackIV  = GetMonData(mon, MON_DATA_SPATK_IV);
    dest->spDefenseIV  = GetMonData(mon, MON_DATA_SPDEF_IV);
    dest->abilityNum = GetMonData(mon, MON_DATA_ABILITY_NUM);
    dest->personality = GetMonData(mon, MON_DATA_PERSONALITY);
    dest->status = GetMonData(mon, MON_DATA_STATUS);
    dest->level = GetMonData(mon, MON_DATA_LEVEL);
    dest->hp = GetMonData(mon, MON_DATA_HP);
    dest->maxHP = GetMonData(mon, MON_DATA_MAX_HP);
    dest->attack = GetMonData(mon, MON_DATA_ATK);
    dest->defense = GetMonData(mon, MON_DATA_DEF);
    dest->speed = GetMonData(mon, MON_DATA_SPEED);
    dest->spAttack = GetMonData(mon, MON_DATA_SPATK);
    dest->spDefense = GetMonData(mon, MON_DATA_SPDEF);
    dest->abilityNum = GetMonData(mon, MON_DATA_ABILITY_NUM);
    dest->otId = GetMonData(mon, MON_DATA_OT_ID);
    dest->types[0] = GetSpeciesType(dest->species, 0);
    dest->types[1] = GetSpeciesType(dest->species, 1);
    dest->types[2] = TYPE_MYSTERY;
    dest->isShiny = IsMonShiny(src);
    dest->affectionHearts = GetMonAffectionHearts(src);
    dest->ability = GetAbilityBySpecies(dest->species, dest->abilityNum);
    GetMonData(src, MON_DATA_NICKNAME, nickname);
    StringCopy_Nickname(dest->nickname, nickname);
    GetMonData(src, MON_DATA_OT_NAME, dest->otName);

    for (i = 0; i < NUM_BATTLE_STATS; i++)
        dst->statStages[i] = DEFAULT_STAT_STAGE;

    memset(&dst->volatiles, 0, sizeof(struct Volatiles));
}

void CopyPartyMonToBattleData(enum BattlerId battler, u32 partyIndex)
{
    struct Pokemon *party = GetBattlerParty(battler);
    PokemonToBattleMon(&party[partyIndex], &gBattleMons[battler]);
    gBattleStruct->battlerState[battler].hpOnSwitchout = gBattleMons[battler].hp;
    UpdateSentPokesToOpponentValue(battler);
    ClearTemporarySpeciesSpriteData(battler, FALSE, FALSE);
}

bool8 ExecuteTableBasedItemEffect(struct Pokemon *mon, enum Item item, u8 partyIndex, u8 moveIndex)
{
    return PokemonUseItemEffects(mon, item, partyIndex, moveIndex, FALSE);
}

#define UPDATE_FRIENDSHIP_FROM_ITEM()                                                                   \
{                                                                                                       \
    if ((!retVal || friendshipOnly) && !ShouldSkipFriendshipChange() && friendshipChange == 0)      \
    {                                                                                                   \
        friendshipChange = itemEffect[itemEffectParam];                                                 \
        friendship = GetMonData(mon, MON_DATA_FRIENDSHIP);                                              \
        friendship += CalculateFriendshipBonuses(mon,friendshipChange,holdEffect);                      \
        if (friendship < 0)                                                                             \
            friendship = 0;                                                                             \
        if (friendship > MAX_FRIENDSHIP)                                                                \
            friendship = MAX_FRIENDSHIP;                                                                \
        SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);                                              \
        retVal = FALSE;                                                                                 \
    }                                                                                                   \
}

// EXP candies store an index for this table in their holdEffectParam.
const u32 sExpCandyExperienceTable[] = {
    [EXP_100 - 1] = 100,
    [EXP_800 - 1] = 800,
    [EXP_3000 - 1] = 3000,
    [EXP_10000 - 1] = 10000,
    [EXP_30000 - 1] = 30000,
};

// Returns TRUE if the item has no effect on the Pokémon, FALSE otherwise
bool8 PokemonUseItemEffects(struct Pokemon *mon, enum Item item, u8 partyIndex, u8 moveIndex, bool8 usedByAI)
{
    u32 dataUnsigned;
    s32 dataSigned, evCap;
    s32 friendship;
    s32 i;
    bool8 retVal = TRUE;
    const u8 *itemEffect;
    u8 itemEffectParam = ITEM_EFFECT_ARG_START;
    u32 temp1, temp2;
    s8 friendshipChange = 0;
    enum HoldEffect holdEffect;
    enum BattlerId battler = MAX_BATTLERS_COUNT;
    bool32 friendshipOnly = FALSE;
    enum Item heldItem;
    u8 effectFlags;
    s16 evChange;
    u16 evCount;
    u8 levelBefore;
    bool8 didLevelUp = FALSE;
    bool8 isLevelUpItem;

    // Determine the EV cap to use
    u32 maxAllowedEVs = !B_EV_ITEMS_CAP ? MAX_TOTAL_EVS : GetCurrentEVCap();

    // Get item hold effect
    heldItem = GetMonData(mon, MON_DATA_HELD_ITEM);
    if (heldItem == ITEM_ENIGMA_BERRY_E_READER)
    {
        #if FREE_ENIGMA_BERRY == FALSE
            holdEffect = gSaveBlock1Ptr->enigmaBerry.holdEffect;
        #else
            holdEffect = 0;
        #endif //FREE_ENIGMA_BERRY
    }
    else
        holdEffect = GetItemHoldEffect(heldItem);

    // Skip using the item if it won't do anything
    if (GetItemEffect(item) == NULL && item != ITEM_ENIGMA_BERRY_E_READER)
        return TRUE;

    // Get item effect
    itemEffect = GetItemEffect(item);
    isLevelUpItem = (itemEffect[3] & ITEM3_LEVEL_UP) != 0;
    levelBefore = GetMonData(mon, MON_DATA_LEVEL, NULL);

    // Do item effect
    for (i = 0; i < ITEM_EFFECT_ARG_START; i++)
    {
        switch (i)
        {

        // Handle ITEM0 effects (infatuation, Dire Hit, X Attack). ITEM0_SACRED_ASH is handled in party_menu.c
        // Now handled in item battle scripts.
        case 0:
            break;

        // Handle ITEM1 effects (in-battle stat boosting effects)
        // Now handled in item battle scripts.
        case 1:
            break;
        // Formerly used by the item effects of the X Sp. Atk and the X Accuracy
        case 2:
            break;

        // Handle ITEM3 effects (Guard Spec, Rare Candy, cure status)
        case 3:
            // Rare Candy / EXP Candy
            if ((itemEffect[i] & ITEM3_LEVEL_UP)
             && GetMonData(mon, MON_DATA_LEVEL) != MAX_LEVEL)
            {
                u8 param = GetItemHoldEffectParam(item);
                dataUnsigned = 0;

                if (param == 0) // Rare Candy
                {
                    dataUnsigned = gExperienceTables[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES, NULL)].growthRate][GetMonData(mon, MON_DATA_LEVEL, NULL) + 1];
                }
                else if (param - 1 < ARRAY_COUNT(sExpCandyExperienceTable)) // EXP Candies
                {
                    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
                    dataUnsigned = sExpCandyExperienceTable[param - 1] + GetMonData(mon, MON_DATA_EXP);

                    if (B_RARE_CANDY_CAP && B_EXP_CAP_TYPE == EXP_CAP_HARD)
                    {
                        u32 currentLevelCap = GetCurrentLevelCap();
                        if (dataUnsigned > gExperienceTables[gSpeciesInfo[species].growthRate][currentLevelCap])
                            dataUnsigned = gExperienceTables[gSpeciesInfo[species].growthRate][currentLevelCap];
                    }
                    else if (dataUnsigned > gExperienceTables[gSpeciesInfo[species].growthRate][MAX_LEVEL])
                    {
                        dataUnsigned = gExperienceTables[gSpeciesInfo[species].growthRate][MAX_LEVEL];
                    }
                }

                if (dataUnsigned != 0) // Failsafe
                {
                    SetMonData(mon, MON_DATA_EXP, &dataUnsigned);
                    CalculateMonStats(mon);
                    if (GetMonData(mon, MON_DATA_LEVEL, NULL) > levelBefore)
                        didLevelUp = TRUE;
                    retVal = FALSE;
                }
            }

            // Cure status
            if ((itemEffect[i] & ITEM3_SLEEP) && HealStatusConditions(mon, STATUS1_SLEEP, battler) == 0)
                retVal = FALSE;
            if ((itemEffect[i] & ITEM3_POISON) && HealStatusConditions(mon, STATUS1_PSN_ANY | STATUS1_TOXIC_COUNTER, battler) == 0)
                retVal = FALSE;
            if ((itemEffect[i] & ITEM3_BURN) && HealStatusConditions(mon, STATUS1_BURN, battler) == 0)
                retVal = FALSE;
            if ((itemEffect[i] & ITEM3_FREEZE) && HealStatusConditions(mon, STATUS1_ICY_ANY, battler) == 0)
                retVal = FALSE;
            if ((itemEffect[i] & ITEM3_PARALYSIS) && HealStatusConditions(mon, STATUS1_PARALYSIS, battler) == 0)
                retVal = FALSE;
            if ((itemEffect[i] & ITEM3_FROSTBITE) && HealStatusConditions(mon, STATUS1_FROSTBITE, battler) == 0)
                retVal = FALSE;
            break;

        // Handle ITEM4 effects (Change HP/Atk EVs, HP heal, PP heal, PP up, Revive, and evolution stones)
        case 4:
            effectFlags = itemEffect[i];

            // PP Up
            if (effectFlags & ITEM4_PP_UP)
            {
                u32 ppBonuses = GetMonData(mon, MON_DATA_PP_BONUSES);
                effectFlags &= ~ITEM4_PP_UP;
                dataUnsigned = (ppBonuses & gPPUpGetMask[moveIndex]) >> (moveIndex * 2);
                temp1 = CalculatePPWithBonus(GetMonData(mon, MON_DATA_MOVE1 + moveIndex), ppBonuses, moveIndex);
                if (dataUnsigned <= 2 && temp1 > 4)
                {
                    dataUnsigned = ppBonuses + gPPUpAddValues[moveIndex];
                    SetMonData(mon, MON_DATA_PP_BONUSES, &dataUnsigned);

                    dataUnsigned = CalculatePPWithBonus(GetMonData(mon, MON_DATA_MOVE1 + moveIndex), dataUnsigned, moveIndex) - temp1;
                    dataUnsigned = GetMonData(mon, MON_DATA_PP1 + moveIndex) + dataUnsigned;
                    SetMonData(mon, MON_DATA_PP1 + moveIndex, &dataUnsigned);
                    retVal = FALSE;
                }
            }
            temp1 = 0;

            // Loop through and try each of the remaining ITEM4 effects
            while (effectFlags != 0)
            {
                if (effectFlags & 1)
                {
                    switch (temp1)
                    {
                    case 0: // ITEM4_EV_HP
                    case 1: // ITEM4_EV_ATK
                        evCount = GetMonEVCount(mon);
                        temp2 = itemEffect[itemEffectParam];
                        dataSigned = GetMonData(mon, sGetMonDataEVConstants[temp1]);
                        s8 evDelta = (s8)itemEffect[itemEffectParam];
                        s16 evChange = evDelta;

                        if (evChange > 0) // Increasing EV (HP or Atk)
                        {
                            // Check if the total EV limit is reached
                            if (evCount >= maxAllowedEVs)
                                return TRUE;

                            // Ensure the increase does not exceed the max EV per stat (252)
                            evCap = (itemEffect[10] & ITEM10_IS_VITAMIN) ? EV_ITEM_RAISE_LIMIT : MAX_PER_STAT_EVS;

                            // Check if the per-stat limit is reached
                            if (dataSigned >= evCap)
                                return TRUE;  // Prevents item use if the per-stat cap is already reached

                            if (dataSigned + evChange > evCap)
                                temp2 = evCap - dataSigned;
                            else
                                temp2 = evChange;

                            // Ensure the total EVs do not exceed the maximum allowed (510)
                            if (evCount + temp2 > maxAllowedEVs)
                                temp2 = maxAllowedEVs - evCount;

                            // Prevent item use if no EVs can be increased
                            if (temp2 == 0)
                                return TRUE;

                            // Apply the EV increase
                            dataSigned += temp2;
                        }
                        else if (evChange < 0) // Decreasing EV (HP or Atk)
                        {
                            if (dataSigned == 0)
                            {
                                // No EVs to lose, but make sure friendship updates anyway
                                friendshipOnly = TRUE;
                                itemEffectParam++;
                                break;
                            }
                            dataSigned += evChange;
                            if (I_BERRY_EV_JUMP == GEN_4 && dataSigned > 100)
                                dataSigned = 100;
                            if (dataSigned < 0)
                                dataSigned = 0;
                        }
                        else // Reset EV (HP or Atk)
                        {
                            if (dataSigned == 0)
                                break;

                            dataSigned = 0;
                        }

                        // Update EVs and stats
                        SetMonData(mon, sGetMonDataEVConstants[temp1], &dataSigned);
                        CalculateMonStats(mon);
                        itemEffectParam++;
                        retVal = FALSE;
                        break;

                    case 2: // ITEM4_HEAL_HP
                    {
                        u32 currentHP = GetMonData(mon, MON_DATA_HP);
                        u32 maxHP = GetMonData(mon, MON_DATA_MAX_HP);
                        if (isLevelUpItem && !didLevelUp && (effectFlags & (ITEM4_REVIVE >> 2)))
                        {
                            itemEffectParam++;
                            break;
                        }
                        // Check use validity.
                        if ((effectFlags & (ITEM4_REVIVE >> 2) && currentHP != 0)
                              || (!(effectFlags & (ITEM4_REVIVE >> 2)) && currentHP == 0))
                        {
                            itemEffectParam++;
                            break;
                        }

                        // Get amount of HP to restore
                        dataUnsigned = itemEffect[itemEffectParam++];
                        switch (dataUnsigned)
                        {
                        case ITEM6_HEAL_HP_FULL:
                            dataUnsigned = maxHP - currentHP;
                            break;
                        case ITEM6_HEAL_HP_HALF:
                            dataUnsigned = maxHP / 2;
                            if (dataUnsigned == 0)
                                dataUnsigned = 1;
                            break;
                        case ITEM6_HEAL_HP_LVL_UP:
                            dataUnsigned = gBattleScripting.levelUpHP;
                            break;
                        case ITEM6_HEAL_HP_QUARTER:
                            dataUnsigned = maxHP / 4;
                            if (dataUnsigned == 0)
                                dataUnsigned = 1;
                            break;
                        }

                        // Only restore HP if not at max health
                        if (maxHP != currentHP)
                        {
                            // Restore HP
                            dataUnsigned = currentHP + dataUnsigned;
                            if (dataUnsigned > maxHP)
                                dataUnsigned = maxHP;
                            SetMonData(mon, MON_DATA_HP, &dataUnsigned);
                            retVal = FALSE;
                        }
                        effectFlags &= ~(ITEM4_REVIVE >> 2);
                        break;
                    }
                    case 3: // ITEM4_HEAL_PP
                        if (!(effectFlags & (ITEM4_HEAL_PP_ONE >> 3)))
                        {
                            // Heal PP for all moves
                            for (temp2 = 0; (signed)(temp2) < (signed)(MAX_MON_MOVES); temp2++)
                            {
                                enum Move move;
                                u32 ppBonus;
                                dataUnsigned = GetMonData(mon, MON_DATA_PP1 + temp2);
                                move = GetMonData(mon, MON_DATA_MOVE1 + temp2);
                                ppBonus = CalculatePPWithBonus(move, GetMonData(mon, MON_DATA_PP_BONUSES), temp2);
                                if (dataUnsigned != ppBonus)
                                {
                                    dataUnsigned += itemEffect[itemEffectParam];
                                    if (dataUnsigned > ppBonus)
                                        dataUnsigned = ppBonus;
                                    SetMonData(mon, MON_DATA_PP1 + temp2, &dataUnsigned);
                                    retVal = FALSE;
                                }
                            }
                            itemEffectParam++;
                        }
                        else
                        {
                            // Heal PP for one move
                            enum Move move;
                            dataUnsigned = GetMonData(mon, MON_DATA_PP1 + moveIndex);
                            move = GetMonData(mon, MON_DATA_MOVE1 + moveIndex);
                            u32 ppBonus = CalculatePPWithBonus(move, GetMonData(mon, MON_DATA_PP_BONUSES), moveIndex);
                            if (dataUnsigned != ppBonus)
                            {
                                dataUnsigned += itemEffect[itemEffectParam++];
                                if (dataUnsigned > ppBonus)
                                    dataUnsigned = ppBonus;
                                SetMonData(mon, MON_DATA_PP1 + moveIndex, &dataUnsigned);
                                retVal = FALSE;
                            }
                        }
                        break;

                    // cases 4-6 are ITEM4_HEAL_PP_ONE, ITEM4_PP_UP, and ITEM4_REVIVE, which
                    // are already handled above by other cases or before the loop

                    case 7: // ITEM4_EVO_STONE
                        {
                            bool32 canStopEvo = TRUE;
                            enum Species targetSpecies = GetEvolutionTargetSpecies(mon, EVO_MODE_ITEM_USE, item, NULL, &canStopEvo, CHECK_EVO);

                            if (targetSpecies != SPECIES_NONE)
                            {
                                GetEvolutionTargetSpecies(mon, EVO_MODE_ITEM_USE, item, NULL, &canStopEvo, DO_EVO);
                                BeginEvolutionScene(mon, targetSpecies, canStopEvo, i);
                                return FALSE;
                            }
                        }
                        break;
                    }
                }
                temp1++;
                effectFlags >>= 1;
                if (i == effectByte)
                    effectBit >>= 1;
            }
            break;

        // Handle ITEM5 effects (Change Def/SpDef/SpAtk/Speed EVs, PP Max, and friendship changes)
        case 5:
            effectFlags = itemEffect[i];
            temp1 = 0;

            // Loop through and try each of the ITEM5 effects
            while (effectFlags != 0)
            {
                if (effectFlags & 1)
                {
                    switch (temp1)
                    {
                    case 0: // ITEM5_EV_DEF
                    case 1: // ITEM5_EV_SPEED
                    case 2: // ITEM5_EV_SPDEF
                    case 3: // ITEM5_EV_SPATK
                        evCount = GetMonEVCount(mon);
                        temp2 = itemEffect[itemEffectParam];
                        dataSigned = GetMonData(mon, sGetMonDataEVConstants[temp1 + 2]);
                        s8 evDelta = (s8)itemEffect[itemEffectParam];
                        evChange = evDelta;
                        if (evChange > 0) // Increasing EV
                        {
                            // Check if the total EV limit is reached
                            if (evCount >= maxAllowedEVs)
                                return TRUE;

                            // Ensure the increase does not exceed the max EV per stat (252)
                            evCap = (itemEffect[10] & ITEM10_IS_VITAMIN) ? EV_ITEM_RAISE_LIMIT : MAX_PER_STAT_EVS;

                            // Check if the per-stat limit is reached
                            if (dataSigned >= evCap)
                                return TRUE;  // Prevents item use if the per-stat cap is already reached

                            if (dataSigned + evChange > evCap)
                                temp2 = evCap - dataSigned;
                            else
                                temp2 = evChange;

                            // Ensure the total EVs do not exceed the maximum allowed (510)
                            if (evCount + temp2 > maxAllowedEVs)
                                temp2 = maxAllowedEVs - evCount;

                            // Prevent item use if no EVs can be increased
                            if (temp2 == 0)
                                return TRUE;

                            // Apply the EV increase
                            dataSigned += temp2;
                        }
                        else if (evChange < 0) // Decreasing EV
                        {
                            if (dataSigned == 0)
                            {
                                // No EVs to lose, but make sure friendship updates anyway
                                friendshipOnly = TRUE;
                                itemEffectParam++;
                                break;
                            }
                            dataSigned += evChange;
                            if (I_BERRY_EV_JUMP == GEN_4 && dataSigned > 100)
                                dataSigned = 100;
                            if (dataSigned < 0)
                                dataSigned = 0;
                        }
                        else // Reset EV
                        {
                            if (dataSigned == 0)
                                break;

                            dataSigned = 0;
                        }

                        // Update EVs and stats
                        SetMonData(mon, sGetMonDataEVConstants[temp1 + 2], &dataSigned);
                        CalculateMonStats(mon);
                        retVal = FALSE;
                        itemEffectParam++;
                        break;

                    case 4: // ITEM5_PP_MAX
                    {
                        u32 ppBonuses = GetMonData(mon, MON_DATA_PP_BONUSES);
                        dataUnsigned = (ppBonuses & gPPUpGetMask[moveIndex]) >> (moveIndex * 2);
                        temp2 = CalculatePPWithBonus(GetMonData(mon, MON_DATA_MOVE1 + moveIndex), ppBonuses, moveIndex);

                        // Check if 3 PP Ups have been applied already, and that the move has a total PP of at least 5 (excludes Sketch)
                        if (dataUnsigned < 3 && temp2 >= 5)
                        {
                            dataUnsigned = ppBonuses + gPPUpAddValues[moveIndex];
                            SetMonData(mon, MON_DATA_PP_BONUSES, &dataUnsigned);

                            dataUnsigned = CalculatePPWithBonus(GetMonData(mon, MON_DATA_MOVE1 + moveIndex), dataUnsigned, moveIndex) - temp1;
                            dataUnsigned = GetMonData(mon, MON_DATA_PP1 + moveIndex) + dataUnsigned;
                            SetMonData(mon, MON_DATA_PP1 + moveIndex, &dataUnsigned);
                            retVal = FALSE;
                        }
                        break;
                    }
                    case 5: // ITEM5_FRIENDSHIP_LOW
                        // Changes to friendship are given differently depending on
                        // how much friendship the Pokémon already has.
                        // In general, Pokémon with lower friendship receive more,
                        // and Pokémon with higher friendship receive less.
                    if (GetMonData(mon, MON_DATA_FRIENDSHIP) < 100)
                        UPDATE_FRIENDSHIP_FROM_ITEM();
                        itemEffectParam++;
                        break;

                    case 6: // ITEM5_FRIENDSHIP_MID
                    if (GetMonData(mon, MON_DATA_FRIENDSHIP) >= 100 && GetMonData(mon, MON_DATA_FRIENDSHIP) < 200)
                        UPDATE_FRIENDSHIP_FROM_ITEM();
                        itemEffectParam++;
                        break;

                    case 7: // ITEM5_FRIENDSHIP_HIGH
                    if (GetMonData(mon, MON_DATA_FRIENDSHIP) >= 200)
                        UPDATE_FRIENDSHIP_FROM_ITEM();
                        itemEffectParam++;
                        break;
                    }
                }
                temp1++;
                effectFlags >>= 1;
                if (i == effectByte)
                    effectBit >>= 1;
            }
            break;
        }
    }
    return retVal;
}

bool8 HealStatusConditions(struct Pokemon *mon, u32 healMask, enum BattlerId battler)
{
    u32 status = GetMonData(mon, MON_DATA_STATUS, 0);

    PREPARE_MON_NICK_BUFFER(gBattleTextBuff1, battler, gBattlerPartyIndexes[battler]);

    if (status & healMask)
    {
        if (status & STATUS1_PARALYSIS)
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_CURED_PARALYSIS;
        else if (status & STATUS1_POISON || status & STATUS1_TOXIC_POISON)
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_CURED_POISON;
        else if (status & STATUS1_BURN)
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_CURED_BURN;
        else if (status & STATUS1_SLEEP)
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_CURED_SLEEP;
        else if (status & STATUS1_FREEZE)
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_CURED_FREEZE;
        else if (status & STATUS1_FROSTBITE)
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_CURED_FROSTBITE;
        status &= ~healMask;
        SetMonData(mon, MON_DATA_STATUS, &status);
        if (gMain.inBattle && battler != MAX_BATTLERS_COUNT)
        {
            gBattleMons[battler].status1 &= ~healMask;
            if ((healMask & STATUS1_SLEEP))
            {
                u32 battlerSide = GetBattlerSide(battler);
                struct Pokemon *party = GetBattlerParty(battler);

                for (u32 i = 0; i < PARTY_SIZE; i++)
                {
                    if (&party[i] == mon)
                    {
                        TryDeactivateSleepClause(battlerSide, i);
                        break;
                    }
                }
            }
        }
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

u8 GetItemEffectParamOffset(enum BattlerId battler, enum Item itemId, u8 effectByte, u8 effectBit)
{
    const u8 *temp;
    const u8 *itemEffect;
    u8 offset;
    int i;
    u8 j;
    u8 effectFlags;

    offset = ITEM_EFFECT_ARG_START;

    temp = GetItemEffect(itemId);

    if (temp != NULL && !temp && itemId != ITEM_ENIGMA_BERRY_E_READER)
        return 0;

    if (itemId == ITEM_ENIGMA_BERRY_E_READER)
    {
        temp = gEnigmaBerries[battler].itemEffect;
    }

    itemEffect = temp;

    for (i = 0; i < ITEM_EFFECT_ARG_START; i++)
    {
        switch (i)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            if (i == effectByte)
                return 0;
            break;
        case 4:
            effectFlags = itemEffect[4];
            if (effectFlags & ITEM4_PP_UP)
                effectFlags &= ~(ITEM4_PP_UP);
            j = 0;
            while (effectFlags)
            {
                if (effectFlags & 1)
                {
                    switch (j)
                    {
                    case 2: // ITEM4_HEAL_HP
                        if (effectFlags & (ITEM4_REVIVE >> 2))
                            effectFlags &= ~(ITEM4_REVIVE >> 2);
                        // fallthrough
                    case 0: // ITEM4_EV_HP
                        if (i == effectByte && (effectFlags & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 1: // ITEM4_EV_ATK
                        if (i == effectByte && (effectFlags & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 3: // ITEM4_HEAL_PP
                        if (i == effectByte && (effectFlags & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 7: // ITEM4_EVO_STONE
                        if (i == effectByte)
                            return 0;
                        break;
                    }
                }
                j++;
                effectFlags >>= 1;
                if (i == effectByte)
                    effectBit >>= 1;
            }
            break;
        case 5:
            effectFlags = itemEffect[5];
            j = 0;
            while (effectFlags)
            {
                if (effectFlags & 1)
                {
                    switch (j)
                    {
                    case 0: // ITEM5_EV_DEF
                    case 1: // ITEM5_EV_SPEED
                    case 2: // ITEM5_EV_SPDEF
                    case 3: // ITEM5_EV_SPATK
                    case 4: // ITEM5_PP_MAX
                    case 5: // ITEM5_FRIENDSHIP_LOW
                    case 6: // ITEM5_FRIENDSHIP_MID
                        if (i == effectByte && (effectFlags & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 7: // ITEM5_FRIENDSHIP_HIGH
                        if (i == effectByte)
                            return 0;
                        break;
                    }
                }
                j++;
                effectFlags >>= 1;
                if (i == effectByte)
                    effectBit >>= 1;
            }
            break;
        }
    }

    return offset;
}

static void BufferStatRoseMessage(enum Stat statIdx)
{
    gBattlerTarget = gBattlerInMenuId;
    StringCopy(gBattleTextBuff1, gStatNamesTable[sStatsToRaise[statIdx]]);
    if (B_X_ITEMS_BUFF >= GEN_7)
    {
        StringCopy(gBattleTextBuff2, gText_StatSharply);
        StringAppend(gBattleTextBuff2, gText_StatRose);
    }
    else
    {
        StringCopy(gBattleTextBuff2, gText_StatRose);
    }
    BattleStringExpandPlaceholdersToDisplayedString(gText_DefendersStatRose);
}

u8 *UseStatIncreaseItem(enum Item itemId)
{
    const u8 *itemEffect;

    if (itemId == ITEM_ENIGMA_BERRY_E_READER)
    {
        if (gMain.inBattle)
            itemEffect = gEnigmaBerries[gBattlerInMenuId].itemEffect;
        else
        #if FREE_ENIGMA_BERRY == FALSE
            itemEffect = gSaveBlock1Ptr->enigmaBerry.itemEffect;
        #else
            itemEffect = 0;
        #endif //FREE_ENIGMA_BERRY
    }
    else
    {
        itemEffect = GetItemEffect(itemId);
    }

    gPotentialItemEffectBattler = gBattlerInMenuId;

    if (itemEffect[0] & ITEM0_DIRE_HIT)
    {
        gBattlerAttacker = gBattlerInMenuId;
        BattleStringExpandPlaceholdersToDisplayedString(gText_PkmnGettingPumped);
    }

    switch (itemEffect[1])
    {
    case ITEM1_X_ATTACK:
        BufferStatRoseMessage(STAT_ATK);
        break;
    case ITEM1_X_DEFENSE:
        BufferStatRoseMessage(STAT_DEF);
        break;
    case ITEM1_X_SPEED:
        BufferStatRoseMessage(STAT_SPEED);
        break;
    case ITEM1_X_SPATK:
        BufferStatRoseMessage(STAT_SPATK);
        break;
    case ITEM1_X_SPDEF:
        BufferStatRoseMessage(STAT_SPDEF);
        break;
    case ITEM1_X_ACCURACY:
        BufferStatRoseMessage(STAT_ACC);
        break;
    }

    if (itemEffect[3] & ITEM3_GUARD_SPEC)
    {
        gBattlerAttacker = gBattlerInMenuId;
        BattleStringExpandPlaceholdersToDisplayedString(gText_PkmnShroudedInMist);
    }

    return gDisplayedStringBattle;
}

u8 GetNature(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_PERSONALITY, 0) % NUM_NATURES;
}

u8 GetNatureFromPersonality(u32 personality)
{
    return personality % NUM_NATURES;
}

enum Species GetGMaxTargetSpecies(enum Species species)
{
    const struct FormChange *formChanges = GetSpeciesFormChanges(species);
    u32 i;
    for (i = 0; formChanges != NULL && formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
    {
        if (formChanges[i].method == FORM_CHANGE_BATTLE_GIGANTAMAX)
            return formChanges[i].targetSpecies;
    }
    return species;
}

bool32 DoesMonMeetAdditionalConditions(struct Pokemon *mon, const struct EvolutionParam *params, struct Pokemon *tradePartner, u32 partyId, bool32 *canStopEvo, enum EvoState evoState)
{
    u32 i, j;
    enum Item heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, 0);
    u32 gender = GetMonGender(mon);
    u32 friendship = GetMonData(mon, MON_DATA_FRIENDSHIP, 0);
    u32 attack = GetMonData(mon, MON_DATA_ATK, 0);
    u32 defense = GetMonData(mon, MON_DATA_DEF, 0);
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY, 0);
    u16 upperPersonality = personality >> 16;
    u32 weather = GetCurrentWeather();
    u32 nature = GetNature(mon);
    bool32 removeHoldItem = FALSE;
    enum Item removeBagItem = ITEM_NONE;
    u32 removeBagItemCount = 0;
    u32 evolutionTracker = GetMonData(mon, MON_DATA_EVOLUTION_TRACKER, 0);
    enum Species partnerSpecies;
    enum Item partnerHeldItem;
    enum HoldEffect partnerHoldEffect;

    if (tradePartner != NULL)
    {
        partnerSpecies = GetMonData(tradePartner, MON_DATA_SPECIES, 0);
        partnerHeldItem = GetMonData(tradePartner, MON_DATA_HELD_ITEM, 0);

        if (partnerHeldItem == ITEM_ENIGMA_BERRY_E_READER)
        #if FREE_ENIGMA_BERRY == FALSE
            partnerHoldEffect = gSaveBlock1Ptr->enigmaBerry.holdEffect;
        #else
            partnerHoldEffect = 0;
        #endif //FREE_ENIGMA_BERRY
        else
            partnerHoldEffect = GetItemHoldEffect(partnerHeldItem);
    }
    else
    {
        partnerSpecies = SPECIES_NONE;
        partnerHeldItem = ITEM_NONE;
        partnerHoldEffect = HOLD_EFFECT_NONE;
    }

    // Check for additional conditions (only if the primary method passes). Skips if there's no additional conditions.
    for (i = 0; params != NULL && params[i].condition != CONDITIONS_END; i++)
    {
        enum EvolutionConditions condition = params[i].condition;
        bool32 currentCondition = FALSE;

        switch (condition)
        {
        // Gen 2
        case IF_GENDER:
            if (gender == params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_MIN_FRIENDSHIP:
            // Freundschafts‑Entwicklungen deaktiviert — diese Bedingung wird absichtlich ignoriert.
            currentCondition = FALSE;
            break;
        case IF_ATK_GT_DEF:
            if (attack > defense)
                currentCondition = TRUE;
            break;
        case IF_ATK_EQ_DEF:
            if (attack == defense)
                currentCondition = TRUE;
            break;
        case IF_ATK_LT_DEF:
            if (attack < defense)
                currentCondition = TRUE;
            break;
        case IF_TIME:
            if (GetTimeOfDay() == params[i].arg1)
                currentCondition = TRUE;

            break;
        case IF_NOT_TIME:
            if (GetTimeOfDay() != params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_HOLD_ITEM:
            if (heldItem == params[i].arg1)
            {
                currentCondition = TRUE;
                removeHoldItem = TRUE;
            }
            break;
        // Gen 3
        case IF_PID_UPPER_MODULO_10_GT:
            if ((upperPersonality % 10) > params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_PID_UPPER_MODULO_10_EQ:
            if ((upperPersonality % 10) == params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_PID_UPPER_MODULO_10_LT:
            if ((upperPersonality % 10) < params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_MIN_BEAUTY:
        {
            u32 beauty = GetMonData(mon, MON_DATA_BEAUTY, 0);
            if (beauty >= params[i].arg1)
                currentCondition = TRUE;
            break;
        }
        case IF_MIN_COOLNESS:
        {
            u32 coolness = GetMonData(mon, MON_DATA_COOL, 0);
            if (coolness >= params[i].arg1)
                currentCondition = TRUE;
            break;
        }
        case IF_MIN_SMARTNESS:
        // remember that even though it's called "Smart/Smartness" here,
        // from gen 6 and up it's known as "Clever/Cleverness."
        {
            u32 smartness = GetMonData(mon, MON_DATA_SMART, 0);
            if (smartness >= params[i].arg1)
                currentCondition = TRUE;
            break;
        }
        case IF_MIN_TOUGHNESS:
        {
            u32 toughness = GetMonData(mon, MON_DATA_TOUGH, 0);
            if (toughness >= params[i].arg1)
                currentCondition = TRUE;
            break;
        }
        case IF_MIN_CUTENESS:
        {
            u32 cuteness = GetMonData(mon, MON_DATA_CUTE, 0);
            if (cuteness >= params[i].arg1)
                currentCondition = TRUE;
            break;
        }
        // Gen 4
        case IF_SPECIES_IN_PARTY:
            for (j = 0; j < PARTY_SIZE; j++)
            {
                if (GetMonData(&gParties[B_TRAINER_PLAYER][j], MON_DATA_SPECIES) == params[i].arg1)
                {
                    currentCondition = TRUE;
                    break;
                }
            }
            break;
        case IF_IN_MAP:
            if (params[i].arg1 == ((gSaveBlock1Ptr->location.mapGroup) << 8 | gSaveBlock1Ptr->location.mapNum))
                currentCondition = TRUE;
            break;
        case IF_IN_MAPSEC:
            if (gMapHeader.regionMapSectionId == params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_KNOWS_MOVE:
            if (MonKnowsMove(mon, params[i].arg1))
                currentCondition = TRUE;
            break;
        // Gen 5
        case IF_TRADE_PARTNER_SPECIES:
            if (params[i].arg1 == partnerSpecies && partnerHoldEffect != HOLD_EFFECT_PREVENT_EVOLVE)
                currentCondition = TRUE;
            break;
        // Gen 6
        case IF_TYPE_IN_PARTY:
            for (j = 0; j < PARTY_SIZE; j++)
            {
                enum Species currSpecies = GetMonData(&gParties[B_TRAINER_PLAYER][j], MON_DATA_SPECIES);
                if (GetSpeciesType(currSpecies, 0) == params[i].arg1
                 || GetSpeciesType(currSpecies, 1) == params[i].arg1)
                {
                    currentCondition = TRUE;
                    break;
                }
            }
            break;
        case IF_WEATHER:
            if (params[i].arg1 == WEATHER_RAIN)
            {
                if (weather == WEATHER_RAIN || weather == WEATHER_RAIN_THUNDERSTORM || weather == WEATHER_DOWNPOUR)
                    currentCondition = TRUE;
            }
            else if (params[i].arg1 == WEATHER_FOG)
            {
                if (weather == WEATHER_FOG_DIAGONAL || weather == WEATHER_FOG_HORIZONTAL)
                    currentCondition = TRUE;
            }
            else if (weather == params[i].arg1)
            {
                currentCondition = TRUE;
            }
            break;
        case IF_KNOWS_MOVE_TYPE:
            for (j = 0; j < MAX_MON_MOVES; j++)
            {
                if (GetMoveType(GetMonData(mon, MON_DATA_MOVE1 + j)) == params[i].arg1)
                {
                    currentCondition = TRUE;
                    break;
                }
            }
            break;
        // Gen 8
        case IF_NATURE:
            if (nature == params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_AMPED_NATURE:
            switch (nature)
            {
            case NATURE_HARDY:
            case NATURE_BRAVE:
            case NATURE_ADAMANT:
            case NATURE_NAUGHTY:
            case NATURE_DOCILE:
            case NATURE_IMPISH:
            case NATURE_LAX:
            case NATURE_HASTY:
            case NATURE_JOLLY:
            case NATURE_NAIVE:
            case NATURE_RASH:
            case NATURE_SASSY:
            case NATURE_QUIRKY:
                currentCondition = TRUE;
                break;
            }
            break;
        case IF_LOW_KEY_NATURE:
            switch (nature)
            {
            case NATURE_LONELY:
            case NATURE_BOLD:
            case NATURE_RELAXED:
            case NATURE_TIMID:
            case NATURE_SERIOUS:
            case NATURE_MODEST:
            case NATURE_MILD:
            case NATURE_QUIET:
            case NATURE_BASHFUL:
            case NATURE_CALM:
            case NATURE_GENTLE:
            case NATURE_CAREFUL:
                currentCondition = TRUE;
                break;
            }
            break;
        case IF_RECOIL_DAMAGE_GE:
            if (evolutionTracker >= params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_CURRENT_DAMAGE_GE:
        {
            u32 currentHp = GetMonData(mon, MON_DATA_HP);
            if (currentHp != 0 && (GetMonData(mon, MON_DATA_MAX_HP) - currentHp >= params[i].arg1))
                currentCondition = TRUE;
            break;
        }
        case IF_CRITICAL_HITS_GE:
            if (partyId != PARTY_SIZE && gPartyCriticalHits[partyId] >= params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_USED_MOVE_X_TIMES:
            if (evolutionTracker >= params[i].arg2)
                currentCondition = TRUE;
            break;
        // Gen 9
        case IF_DEFEAT_X_WITH_ITEMS:
            if (evolutionTracker >= params[i].arg3)
                currentCondition = TRUE;
            break;
        case IF_PID_MODULO_100_GT:
            if ((personality % 100) > params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_PID_MODULO_100_EQ:
            if ((personality % 100) == params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_PID_MODULO_100_LT:
            if ((personality % 100) < params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_MIN_OVERWORLD_STEPS:
            if (mon == GetFirstLiveMon() && gFollowerSteps >= params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_BAG_ITEM_COUNT:
            if (CheckBagHasItem(params[i].arg1, params[i].arg2))
            {
                currentCondition = TRUE;
                removeBagItem = params[i].arg1;
                removeBagItemCount = params[i].arg2;
                if (canStopEvo != NULL)
                    *canStopEvo = FALSE;
            }
            break;
        case IF_REGION:
            if (GetCurrentRegion() == params[i].arg1)
                currentCondition = TRUE;
            break;
        case IF_NOT_REGION:
            if (GetCurrentRegion() != params[i].arg1)
                currentCondition = TRUE;
            break;
        case CONDITIONS_END:
            break;
        }

        // check if an evolution is about to happen and items should be removed
        if (evoState == DO_EVO)
        {
            if (removeHoldItem)
            {
                enum Item heldItem = ITEM_NONE;
                SetMonData(mon, MON_DATA_HELD_ITEM, &heldItem);
            }

            if (removeBagItem != ITEM_NONE)
                RemoveBagItem(removeBagItem, removeBagItemCount);
        }

        if (currentCondition == FALSE)
            return FALSE;
    }

    return TRUE;
}

enum Species GetEvolutionTargetSpecies(struct Pokemon *mon, enum EvolutionMode mode, u16 evolutionItem, struct Pokemon *tradePartner, bool32 *canStopEvo, enum EvoState evoState)
{
    int i, j, k;
    enum Species targetSpecies = SPECIES_NONE;
    enum Species species = GetMonData(mon, MON_DATA_SPECIES, 0);
    enum Item heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, 0);
    u32 level = GetMonData(mon, MON_DATA_LEVEL, 0);
    enum HoldEffect holdEffect;
    const struct Evolution *evolutions = GetSpeciesEvolutions(species);

    if (evolutions == NULL)
        return SPECIES_NONE;

    if (heldItem == ITEM_ENIGMA_BERRY_E_READER)
    #if FREE_ENIGMA_BERRY == FALSE
        holdEffect = gSaveBlock1Ptr->enigmaBerry.holdEffect;
    #else
        holdEffect = 0;
    #endif //FREE_ENIGMA_BERRY
    else
        holdEffect = GetItemHoldEffect(heldItem);

    // Prevent evolution with Everstone, unless we're just viewing the party menu with an evolution item
    if (holdEffect == HOLD_EFFECT_PREVENT_EVOLVE
        && mode != EVO_MODE_ITEM_CHECK
        && (P_KADABRA_EVERSTONE < GEN_4 || species != SPECIES_KADABRA))
        return SPECIES_NONE;

    switch (mode)
    {
    case EVO_MODE_NORMAL:
    case EVO_MODE_BATTLE_ONLY:
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            bool32 conditionsMet = FALSE;
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            // Check main primary evolution method
            switch (evolutions[i].method)
            {
            case EVO_LEVEL:
                if (evolutions[i].param <= level)
                    conditionsMet = TRUE;
                break;
            case EVO_LEVEL_BATTLE_ONLY:
                if (mode == EVO_MODE_BATTLE_ONLY && evolutions[i].param <= level)
                    conditionsMet = TRUE;
                break;
            default:
                break;
            }

            // Special: If the evolution has an IF_MIN_FRIENDSHIP additional condition,
            // allow it to trigger via a level gate (use override minLevel if available, otherwise default 15).
            if (!conditionsMet && evolutions[i].params != NULL)
            {
                for (j = 0; evolutions[i].params[j].condition != CONDITIONS_END; j++)
                {
                    if (evolutions[i].params[j].condition == IF_MIN_FRIENDSHIP)
                    {
                        const struct EvolutionOverride *override = GetEvolutionOverride(evolutions[i].targetSpecies);
                        u8 minLevel = override != NULL ? override->minimumLevel : 15;
                        if ((u8)level >= minLevel)
                        {
                            // Temporarily satisfy friendship check by setting friendship to required value,
                            // so DoesMonMeetAdditionalConditions continues to work for other params.
                            u8 reqFriend = (u8)evolutions[i].params[j].arg1;
                            u8 oldFriend = GetMonData(mon, MON_DATA_FRIENDSHIP, 0);
                            SetMonData(mon, MON_DATA_FRIENDSHIP, &reqFriend);
                            conditionsMet = DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, PARTY_SIZE, canStopEvo, CHECK_EVO);
                            // restore friendship
                            SetMonData(mon, MON_DATA_FRIENDSHIP, &oldFriend);

                            // If the temp-check passes, set conditionsMet true (we keep it).
                            // If not, conditionsMet remains FALSE.
                        }
                        break;
                    }
                }
            }

            if (conditionsMet && DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, PARTY_SIZE, canStopEvo, evoState))
            {
                // All checks passed, so stop checking the rest of the evolutions.
                // This is different from vanilla where the loop continues.
                // If you have overlapping evolutions, put the ones you want to happen first on top of the list.
                targetSpecies = evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    case EVO_MODE_TRADE:
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            bool32 conditionsMet = FALSE;
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_TRADE:
                // Skip trade evolutions that require holding specific item for plain trades.
                if (evolutions[i].params != NULL)
                {
                    bool32 requiresHoldItem = FALSE;
                    for (j = 0; evolutions[i].params[j].condition != CONDITIONS_END; j++)
                    {
                        if (evolutions[i].params[j].condition == IF_HOLD_ITEM)
                        {
                            requiresHoldItem = TRUE;
                            break;
                        }
                    }
                    if (requiresHoldItem)
                    {
                        break;
                    }
                }
                conditionsMet = TRUE;
                break;
            default:
                break;
            }

            if (conditionsMet)
            {
                // Handle potential IF_MIN_FRIENDSHIP in params same way as above (temp friendship set)
                bool32 paramsMet = FALSE;
                if (evolutions[i].params != NULL)
                {
                    bool32 handled = FALSE;
                    for (j = 0; evolutions[i].params[j].condition != CONDITIONS_END; j++)
                    {
                        if (evolutions[i].params[j].condition == IF_MIN_FRIENDSHIP)
                        {
                            handled = TRUE;
                            const struct EvolutionOverride *override = GetEvolutionOverride(evolutions[i].targetSpecies);
                            u8 minLevel = override != NULL ? override->minimumLevel : 15;
                            if ((u8)level >= minLevel)
                            {
                                u8 reqFriend = (u8)evolutions[i].params[j].arg1;
                                u8 oldFriend = GetMonData(mon, MON_DATA_FRIENDSHIP, 0);
                                SetMonData(mon, MON_DATA_FRIENDSHIP, &reqFriend);
                                paramsMet = DoesMonMeetAdditionalConditions(mon, evolutions[i].params, tradePartner, PARTY_SIZE, canStopEvo, CHECK_EVO);
                                SetMonData(mon, MON_DATA_FRIENDSHIP, &oldFriend);
                            }
                            else
                            {
                                paramsMet = FALSE;
                            }
                            break;
                        }
                    }
                    if (!handled)
                        paramsMet = DoesMonMeetAdditionalConditions(mon, evolutions[i].params, tradePartner, PARTY_SIZE, canStopEvo, evoState);
                }
                else
                {
                    paramsMet = DoesMonMeetAdditionalConditions(mon, evolutions[i].params, tradePartner, PARTY_SIZE, canStopEvo, evoState);
                }

                if (paramsMet)
                {
                    targetSpecies = evolutions[i].targetSpecies;
                    break;
                }
            }
        }
        break;
    case EVO_MODE_ITEM_USE:
    case EVO_MODE_ITEM_CHECK:
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            bool32 conditionsMet = FALSE;
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_ITEM:
                if (evolutions[i].param == evolutionItem)
                    conditionsMet = TRUE;
                break;
            case EVO_TRADE:
                if (evolutions[i].params != NULL)
                {
                    for (j = 0; evolutions[i].params[j].condition != CONDITIONS_END; j++)
                    {
                        if (evolutions[i].params[j].condition == IF_HOLD_ITEM && evolutions[i].params[j].arg1 == evolutionItem)
                        {
                            // Temporarily pretend the mon is holding the item to let DoesMonMeetAdditionalConditions validate other conditions.
                            enum Item oldHeld = GetMonData(mon, MON_DATA_HELD_ITEM);
                            SetMonData(mon, MON_DATA_HELD_ITEM, &evolutionItem);
                            if (DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, PARTY_SIZE, canStopEvo, CHECK_EVO))
                                conditionsMet = TRUE;
                            SetMonData(mon, MON_DATA_HELD_ITEM, &oldHeld);
                            break;
                        }
                    }
                }
                break;
            default:
                break;
            }

            if (conditionsMet)
            {
                // same IF_MIN_FRIENDSHIP handling as above
                bool32 paramsMet = FALSE;
                if (evolutions[i].params != NULL)
                {
                    bool32 handled = FALSE;
                    for (j = 0; evolutions[i].params[j].condition != CONDITIONS_END; j++)
                    {
                        if (evolutions[i].params[j].condition == IF_MIN_FRIENDSHIP)
                        {
                            handled = TRUE;
                            const struct EvolutionOverride *override = GetEvolutionOverride(evolutions[i].targetSpecies);
                            u8 minLevel = override != NULL ? override->minimumLevel : 15;
                            if ((u8)level >= minLevel)
                            {
                                u8 reqFriend = (u8)evolutions[i].params[j].arg1;
                                u8 oldFriend = GetMonData(mon, MON_DATA_FRIENDSHIP, 0);
                                SetMonData(mon, MON_DATA_FRIENDSHIP, &reqFriend);
                                paramsMet = DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, PARTY_SIZE, canStopEvo, CHECK_EVO);
                                SetMonData(mon, MON_DATA_FRIENDSHIP, &oldFriend);
                            }
                            else
                            {
                                paramsMet = FALSE;
                            }
                            break;
                        }
                    }
                    if (!handled)
                        paramsMet = DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, PARTY_SIZE, canStopEvo, evoState);
                }
                else
                {
                    paramsMet = DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, PARTY_SIZE, canStopEvo, evoState);
                }

                if (paramsMet)
                {
                    targetSpecies = evolutions[i].targetSpecies;
                    if (evolutions[i].method == EVO_ITEM && canStopEvo != NULL)
                        *canStopEvo = FALSE;
                    break;
                }
            }
        }
        break;
    case EVO_MODE_BATTLE_SPECIAL:
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            bool32 conditionsMet = FALSE;
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_BATTLE_END:
                conditionsMet = TRUE;
                break;
            default:
                break;
            }

            if (conditionsMet && DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, evolutionItem, canStopEvo, evoState))
            {
                targetSpecies = evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    case EVO_MODE_OVERWORLD_SPECIAL:
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            bool32 conditionsMet = FALSE;
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;

            switch (evolutions[i].method)
            {
            case EVO_SPIN:
                if (gSpecialVar_0x8000 == evolutions[i].param)
                    conditionsMet = TRUE;
                break;
            default:
                break;
            }

            if (conditionsMet && DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, PARTY_SIZE, canStopEvo, evoState))
            {
                targetSpecies = evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    case EVO_MODE_SCRIPT_TRIGGER:
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
                continue;
            if (evolutions[i].method != EVO_SCRIPT_TRIGGER)
                continue;
            if (evolutions[i].param != evolutionItem)
                continue;

            // Handle IF_MIN_FRIENDSHIP as level-gate as above
            bool32 paramsMet = FALSE;
            if (evolutions[i].params != NULL)
            {
                bool32 handled = FALSE;
                for (j = 0; evolutions[i].params[j].condition != CONDITIONS_END; j++)
                {
                    if (evolutions[i].params[j].condition == IF_MIN_FRIENDSHIP)
                    {
                        handled = TRUE;
                        const struct EvolutionOverride *override = GetEvolutionOverride(evolutions[i].targetSpecies);
                        u8 minLevel = override != NULL ? override->minimumLevel : 15;
                        if ((u8)level >= minLevel)
                        {
                            u8 reqFriend = (u8)evolutions[i].params[j].arg1;
                            u8 oldFriend = GetMonData(mon, MON_DATA_FRIENDSHIP, 0);
                            SetMonData(mon, MON_DATA_FRIENDSHIP, &reqFriend);
                            paramsMet = DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, PARTY_SIZE, canStopEvo, CHECK_EVO);
                            SetMonData(mon, MON_DATA_FRIENDSHIP, &oldFriend);
                        }
                        else
                        {
                            paramsMet = FALSE;
                        }
                        break;
                    }
                }
                if (!handled)
                    paramsMet = DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, PARTY_SIZE, canStopEvo, evoState);
            }
            else
            {
                paramsMet = DoesMonMeetAdditionalConditions(mon, evolutions[i].params, NULL, PARTY_SIZE, canStopEvo, evoState);
            }

            if (paramsMet)
            {
                targetSpecies = evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    }

    // Pikachu, Meowth, Eevee and Duraludon cannot evolve if they have the
    // Gigantamax Factor. We assume that is because their evolutions
    // do not have a Gigantamax Form.
    if (GetMonData(mon, MON_DATA_GIGANTAMAX_FACTOR)
     && GetGMaxTargetSpecies(species) != species
     && GetGMaxTargetSpecies(targetSpecies) == targetSpecies)
    {
        return SPECIES_NONE;
    }

    return targetSpecies;
}








