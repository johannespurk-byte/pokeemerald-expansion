#include "global.h"
#include "malloc.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_gfx_sfx_util.h"
#include "battle_interface.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "battle_pyramid_bag.h"
#include "bg.h"
#include "contest.h"
#include "data.h"
#include "decompress.h"
#include "easy_chat.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "evolution_scene.h"
#include "field_control_avatar.h"
#include "field_effect.h"
#include "field_move.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_specials.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "fldeff.h"
#include "fldeff_misc.h"
#include "follower_npc.h"
#include "frontier_util.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item.h"
#include "item_menu.h"
#include "item_use.h"
#include "caps.h"
#include "link.h"
#include "link_rfu.h"
#include "mail.h"
#include "main.h"
#include "menu.h"
#include "menu_helpers.h"
#include "menu_specialized.h"
#include "metatile_behavior.h"
#include "move_relearner.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "player_pc.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_jump.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "pokerus.h"
#include "region_map.h"
#include "reshow_battle_screen.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "start_menu.h"
#include "stat_editor.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trade.h"
#include "union_room.h"
#include "window.h"
#include "constants/battle.h"
#include "constants/battle_frontier.h"
#include "constants/field_effects.h"
#include "constants/field_move.h"
#include "constants/form_change_types.h"
#include "constants/item_effects.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "stat_editor.h"

#if !SWSH_PARTY_MENU

enum {
    MENU_SUMMARY,
    MENU_STAT_EDITOR,
    MENU_SWITCH,
    MENU_FOLLOWER,
    MENU_FOLLOWER_SET,
    MENU_FOLLOWER_RETURN,
    MENU_FOLLOWER_UNSET,
    MENU_CANCEL1,
    MENU_ITEM,
    MENU_GIVE,
    MENU_TAKE_ITEM,
    MENU_MOVE_ITEM,
    MENU_MAIL,
    MENU_TAKE_MAIL,
    MENU_READ,
    MENU_CANCEL2,
    MENU_SHIFT,
    MENU_SEND_OUT,
    MENU_ENTER,
    MENU_NO_ENTRY,
    MENU_STORE,
    MENU_REGISTER,
    MENU_TRADE1,
    MENU_TRADE2,
    MENU_TOSS,
    MENU_CATALOG_BULB,
    MENU_CATALOG_OVEN,
    MENU_CATALOG_WASHING,
    MENU_CATALOG_FRIDGE,
    MENU_CATALOG_FAN,
    MENU_CATALOG_MOWER,
    MENU_CHANGE_FORM,
    MENU_CHANGE_ABILITY,
    MENU_EVOLVE,        // <--- Neu hier eingefügt
    MENU_FIELD_MOVES
};
// IDs for the action lists that appear when a party mon is selected
enum {
    ACTIONS_NONE,
    ACTIONS_SWITCH,
    ACTIONS_SHIFT,
    ACTIONS_SEND_OUT,
    ACTIONS_ENTER,
    ACTIONS_NO_ENTRY,
    ACTIONS_STORE,
    ACTIONS_SUMMARY_ONLY,
    ACTIONS_ITEM,
    ACTIONS_MAIL,
    ACTIONS_REGISTER,
    ACTIONS_TRADE,
    ACTIONS_SPIN_TRADE,
    ACTIONS_TAKEITEM_TOSS,
    ACTIONS_ROTOM_CATALOG,
    ACTIONS_ZYGARDE_CUBE,
    ACTIONS_FOLLOWER_SET,
    ACTIONS_FOLLOWER_SET_RETURN,
    ACTIONS_FOLLOWER_UNSET,
    ACTIONS_FOLLOWER_UNSET_RETURN,
};

enum {
    PARTY_BOX_LEFT_COLUMN,
    PARTY_BOX_RIGHT_COLUMN,
};

enum {
    TAG_POKEBALL = 1200,
    TAG_POKEBALL_SMALL,
    TAG_STATUS_ICONS,
};

#define TAG_HELD_ITEM 55120

#define PARTY_PAL_SELECTED     (1 << 0)
#define PARTY_PAL_FAINTED      (1 << 1)
#define PARTY_PAL_TO_SWITCH    (1 << 2)
#define PARTY_PAL_MULTI_ALT    (1 << 3)
#define PARTY_PAL_SWITCHING    (1 << 4)
#define PARTY_PAL_TO_SOFTBOIL  (1 << 5)
#define PARTY_PAL_NO_MON       (1 << 6)
#define PARTY_PAL_UNUSED       (1 << 7)

#define MENU_DIR_DOWN     1
#define MENU_DIR_UP      -1
#define MENU_DIR_RIGHT    2
#define MENU_DIR_LEFT    -2

enum {
    // Window ids 0-5 are implicitly assigned to each party Pokémon in InitPartyMenuBoxes
    WIN_MSG = PARTY_SIZE,
};

struct PartyMenuBoxInfoRects
{
    void (*blitFunc)(u8, u8, u8, u8, u8, bool8);
    u8 dimensions[24];
    u8 descTextLeft;
    u8 descTextTop;
    u8 descTextWidth;
    u8 descTextHeight;
};

struct PartyMenuInternal
{
    TaskFunc task;
    MainCallback exitCallback;
    u32 chooseHalf:1;
    u32 lastSelectedSlot:3;  // Used to return to same slot when going left/right bewtween columns
    u32 spriteIdConfirmPokeball:7;
    u32 spriteIdCancelPokeball:7;
    u32 messageId:14;
    u8 windowId[3];
    u8 actions[9];
    u8 numActions;
    // In vanilla Emerald, only the first 0xB0 hwords (0x160 bytes) are actually used.
    // However, a full 0x100 hwords (0x200 bytes) are allocated.
    // It is likely that the 0x160 value used below is a constant defined by
    // bin2c, the utility used to encode the compressed palette data.
    u16 palBuffer[BG_PLTT_SIZE / sizeof(u16)];
    s16 data[16];
};

struct PartyMenuBox
{
    const struct PartyMenuBoxInfoRects *infoRects;
    const u8 *spriteCoords;
    u8 windowId;
    u8 monSpriteId;
    u8 itemSpriteId;
    u8 pokeballSpriteId;
    u8 statusSpriteId;
};

// EWRAM vars
static EWRAM_DATA struct PartyMenuInternal *sPartyMenuInternal = NULL;
EWRAM_DATA struct PartyMenu gPartyMenu = {0};
static EWRAM_DATA struct PartyMenuBox *sPartyMenuBoxes = NULL;
static EWRAM_DATA u8 *sPartyBgGfxTilemap = NULL;
static EWRAM_DATA u8 *sPartyBgTilemapBuffer = NULL;
EWRAM_DATA bool8 gPartyMenuUseExitCallback = 0;
EWRAM_DATA u8 gSelectedMonPartyId = 0;
EWRAM_DATA MainCallback gPostMenuFieldCallback = NULL;
static EWRAM_DATA u16 *sSlot1TilemapBuffer = 0; // for switching party slots
static EWRAM_DATA u16 *sSlot2TilemapBuffer = 0; //
EWRAM_DATA u8 gSelectedOrderFromParty[MAX_FRONTIER_PARTY_SIZE] = {0};
static EWRAM_DATA u16 sPartyMenuItemId = 0;
EWRAM_DATA u8 gBattlePartyCurrentOrder[PARTY_SIZE / 2] = {0}; // bits 0-3 are the current pos of Slot 1, 4-7 are Slot 2, and so on
static EWRAM_DATA u8 sInitialLevel = 0;
static EWRAM_DATA u8 sFinalLevel = 0;

// IWRAM common
COMMON_DATA void (*gItemUseCB)(u8, TaskFunc) = NULL;

static void ResetPartyMenu(void);
static void CB2_InitPartyMenu(void);
static void CB2_ReloadPartyMenu(void);
static bool8 ShowPartyMenu(void);
static bool8 ReloadPartyMenu(void);
static void SetPartyMonsAllowedInMinigame(void);
static void ExitPartyMenu(void);
static bool8 AllocPartyMenuBg(void);
static bool8 AllocPartyMenuBgGfx(void);
static void InitPartyMenuWindows(enum PartyMenuLayout);
static void LoadPartyMenuWindows(void);
static void InitPartyMenuBoxes(enum PartyMenuLayout);
static void LoadPartyMenuBoxes(enum PartyMenuLayout);
static void LoadPartyMenuPokeballGfx(void);
static bool8 CreatePartyMonSpritesLoop(void);
static bool8 RenderPartyMenuBoxes(void);
static void CreateCancelConfirmPokeballSprites(void);
static void CreateCancelConfirmWindows(u8);
static void Task_ExitPartyMenu(u8);
static void FreePartyPointers(void);
static void PartyPaletteBufferCopy(u8);
static void DisplayPartyPokemonDataForMultiBattle(u8);
static void LoadPartyBoxPalette(struct PartyMenuBox *, u8);
static void DrawEmptySlot(u8 windowId);
static void DisplayPartyPokemonDataForRelearner(u8);
static void DisplayPartyPokemonDataForContest(u8);
static void DisplayPartyPokemonDataForChooseHalf(u8);
static void DisplayPartyPokemonDataForWirelessMinigame(u8);
static void DisplayPartyPokemonDataForBattlePyramidHeldItem(u8);
static bool8 DisplayPartyPokemonDataForMoveTutorOrEvolutionItem(u8);
static void DisplayPartyPokemonData(u8);
static void DisplayPartyPokemonNickname(struct Pokemon *, struct PartyMenuBox *, u8);
static void DisplayPartyPokemonLevelCheck(struct Pokemon *, struct PartyMenuBox *, u8);
static void DisplayPartyPokemonGenderNidoranCheck(struct Pokemon *, struct PartyMenuBox *, u8);
static void DisplayPartyPokemonHPCheck(struct Pokemon *, struct PartyMenuBox *, u8);
static void DisplayPartyPokemonMaxHPCheck(struct Pokemon *, struct PartyMenuBox *, u8);
static void DisplayPartyPokemonHPBarCheck(struct Pokemon *, struct PartyMenuBox *);
static void DisplayPartyPokemonDescriptionText(u8, struct PartyMenuBox *, u8);
static bool8 IsMonAllowedInMinigame(u8);
static void DisplayPartyPokemonDataToTeachMove(u8, enum Move);
static enum CanMoveBeLearned CanTeachMove(struct Pokemon *, enum Move);
static void DisplayPartyPokemonBarDetail(u8, const u8 *, u8, const u8 *);
static void DisplayPartyPokemonBarDetailToFit(u8 windowId, const u8 *str, u8 color, const u8 *align, u32 width);
static void DisplayPartyPokemonLevel(u8, struct PartyMenuBox *);
static void DisplayPartyPokemonGender(u8, enum Species, u8 *, struct PartyMenuBox *);
static void DisplayPartyPokemonHP(u16 hp, u16 maxHp, struct PartyMenuBox *menuBox);
static void DisplayPartyPokemonMaxHP(u16, struct PartyMenuBox *);
static void DisplayPartyPokemonHPBar(u16, u16, struct PartyMenuBox *);
static void CreatePartyMonIconSpriteParameterized(enum Species species, u32 pid, bool32 isEgg, struct PartyMenuBox *menuBox, u8 priority);
static void CreatePartyMonHeldItemSpriteParameterized(enum Species, enum Item, struct PartyMenuBox *);
static void CreatePartyMonPokeballSpriteParameterized(enum Species, struct PartyMenuBox *);
static void CreatePartyMonStatusSpriteParameterized(enum Species, u8, struct PartyMenuBox *);
// These next 4 functions are essentially redundant with the above 4
// The only difference is that rather than receive the data directly they retrieve it from the mon struct
static void CreatePartyMonHeldItemSprite(struct Pokemon *, struct PartyMenuBox *);
static void CreatePartyMonPokeballSprite(struct Pokemon *, struct PartyMenuBox *);
static void CreatePartyMonIconSprite(struct Pokemon *, struct PartyMenuBox *);
static void CreatePartyMonStatusSprite(struct Pokemon *, struct PartyMenuBox *);
static u8 CreateSmallPokeballButtonSprite(u8, u8);
static void DrawCancelConfirmButtons(void);
static u8 CreatePokeballButtonSprite(u8, u8);
static void AnimateSelectedPartyIcon(u8, u8);
static void PartyMenuStartSpriteAnim(u8, u8);
static u8 GetPartyBoxPaletteFlags(u8, u8);
static bool8 PartyBoxPal_ParnterOrDisqualifiedInArena(u8);
static u8 GetPartyIdFromBattleSlot(u8);
static void Task_ClosePartyMenuAndSetCB2(u8);
static void UpdatePartyToFieldOrder(void);
static void MoveCursorToConfirm(void);
static void HandleChooseMonCancel(u8, s8 *);
static void HandleChooseMonSelection(u8, s8 *);
static u16 PartyMenuButtonHandler(s8 *);
static s8 *GetCurrentPartySlotPtr(void);
static bool8 IsSelectedMonNotEgg(u8 *);
static bool8 DoesSelectedMonKnowHM(u8 *);
static void PartyMenuRemoveWindow(u8 *);
static void CB2_SetUpExitToBattleScreen(void);
static void Task_ClosePartyMenuAfterText(u8);
static void TryTutorSelectedMon(u8);
static void TryGiveMailToSelectedMon(u8);
static void TryGiveItemOrMailToSelectedMon(u8);
static void SwitchSelectedMons(u8);
static void TryEnterMonForMinigame(u8, u8);
static void Task_TryCreateSelectionWindow(u8);
static void FinishTwoMonAction(u8);
static void CancelParticipationPrompt(u8);
static bool8 DisplayCancelChooseMonYesNo(u8);
static const u8 *GetFacilityCancelString(void);
static void Task_CancelChooseMonYesNo(u8);
static void PartyMenuDisplayYesNoMenu(void);
static void Task_HandleCancelChooseMonYesNoInput(u8);
static void Task_ReturnToChooseMonAfterText(u8);
static void UpdateCurrentPartySelection(s8 *, s8);
static void UpdatePartySelectionSingleLayout(s8 *, s8);
static void UpdatePartySelectionDoubleLayout(s8 *, s8);
static s8 GetNewSlotDoubleLayout(s8, s8);
static void PrintMessage(const u8 *);
static void Task_PrintAndWaitForText(u8);
static bool16 IsMonAllowedInPokemonJump(struct Pokemon *);
static bool16 IsMonAllowedInDodrioBerryPicking(struct Pokemon *);
static void Task_CancelParticipationYesNo(u8);
static void Task_HandleCancelParticipationYesNoInput(u8);
static bool8 ShouldUseChooseMonText(void);
static void SetPartyMonFieldSelectionActions(struct Pokemon *, u8);
static u8 GetPartyMenuActionsTypeInBattle(struct Pokemon *);
static u8 GetPartySlotEntryStatus(s8);
static void Task_UpdateHeldItemSprite(u8);
static void Task_HandleSelectionMenuInput(u8);
static void CB2_ShowPokemonSummaryScreen(void);
static void UpdatePartyToBattleOrder(void);
static void SlidePartyMenuBoxOneStep(u8);
static void Task_SlideSelectedSlotsOffscreen(u8);
static void SwitchPartyMon(void);
static void Task_SlideSelectedSlotsOnscreen(u8);
static void CB2_SelectBagItemToGive(void);
static void CB2_GiveHoldItem(void);
static void CB2_WriteMailToGiveMon(void);
static void Task_SwitchHoldItemsPrompt(u8);
static void Task_GiveHoldItem(u8);
static void Task_SwitchItemsYesNo(u8);
static void Task_HandleSwitchItemsYesNoInput(u8);
static void Task_WriteMailToGiveMonAfterText(u8);
static void CB2_ReturnToPartyMenuFromWritingMail(void);
static void Task_DisplayGaveMailFromPartyMessage(u8);
static void UpdatePartyMonHeldItemSprite(struct Pokemon *, struct PartyMenuBox *);
static void Task_TossHeldItemYesNo(u8 taskId);
static void Task_HandleTossHeldItemYesNoInput(u8);
static void Task_TossHeldItem(u8);
static void CB2_ReadHeldMail(void);
static void CB2_ReturnToPartyMenuFromReadingMail(void);
static void Task_SendMailToPCYesNo(u8);
static void Task_HandleSendMailToPCYesNoInput(u8);
static void Task_LoseMailMessageYesNo(u8);
static void Task_HandleLoseMailMessageYesNoInput(u8);
static bool8 TrySwitchInPokemon(void);
static void Task_SpinTradeYesNo(u8);
static void Task_HandleSpinTradeYesNoInput(u8);
static void Task_CancelAfterAorBPress(u8);
static void DisplayFieldMoveExitAreaMessage(u8);
static void DisplayCantUseFlashMessage(void);
static void DisplayCantUseSurfMessage(void);
static void Task_FieldMoveExitAreaYesNo(u8);
static void Task_HandleFieldMoveExitAreaYesNoInput(u8);
static void Task_FieldMoveWaitForFade(u8);
static enum Species GetFieldMoveMonSpecies(void);
static void UpdatePartyMonHPBar(u8, struct Pokemon*);
static void SpriteCB_UpdatePartyMonIcon(struct Sprite*);
static void SpriteCB_BouncePartyMonIcon(struct Sprite*);
static void ShowOrHideHeldItemSprite(enum Item, struct PartyMenuBox*);
static void CreateHeldItemSpriteForTrade(u8, bool8);
static void SpriteCB_HeldItem(struct Sprite*);
static void SetPartyMonAilmentGfx(struct Pokemon*, struct PartyMenuBox*);
static void UpdatePartyMonAilmentGfx(struct Pokemon*, struct PartyMenuBox*);
static u8 GetPartyLayoutFromBattleType(void);

// NEU: Beide Prototypen sauber am Ende der Liste platziert
bool8 CanPokemonEvolveFromMenu(struct Pokemon* mon);
void PartyMenuAction_Evolve(u8 taskId);

static void Task_SetSacredAshCB(u8);
static void CB2_ReturnToBagMenu(void);
static void Task_DisplayHPRestoredMessage(u8);
static u16 ItemEffectToMonEv(struct Pokemon *, u8);
static void ItemEffectToStatString(u8, u8 *);
static void ReturnToUseOnWhichMon(u8);
static void SetSelectedMoveForItem(u8);
static void TryUseItemOnMove(u8);
static void Task_LearnedMove(u8);
static void Task_ReplaceMoveYesNo(u8);
static void Task_DoLearnedMoveFanfareAfterText(u8);
static void Task_LearnNextMoveOrClosePartyMenu(u8);
static void Task_TryLearningNextMove(u8);
static void Task_HandleReplaceMoveYesNoInput(u8);
static void Task_ShowSummaryScreenToForgetMove(u8);
static void StopLearningMovePrompt(u8);
static void CB2_ShowSummaryScreenToForgetMove(void);
static void CB2_ReturnToPartyMenuWhileLearningMove(void);
static void Task_ReturnToPartyMenuWhileLearningMove(u8);
static void DisplayPartyMenuForgotMoveMessage(u8);
static void Task_PartyMenuReplaceMove(u8);
static void Task_HandleStopLearningMove(u8 taskId);
static void Task_StopLearningMoveYesNo(u8);
static void Task_HandleStopLearningMoveYesNoInput(u8);
static void Task_TryLearningNextMoveAfterText(u8);
static void BufferMonStatsToTaskData(struct Pokemon *, s16 *);
static void UpdateMonDisplayInfoAfterRareCandy(u8, struct Pokemon *);
static void Task_DisplayLevelUpStatsPg1(u8);
static void DisplayLevelUpStatsPg1(u8);
static void Task_DisplayLevelUpStatsPg2(u8);
static void DisplayLevelUpStatsPg2(u8);
static void Task_TryLearnNewMoves(u8);
static void PartyMenuTryEvolution(u8);
static void DisplayMonNeedsToReplaceMove(u8);
static void DisplayMonLearnedMove(u8, u16);
static void UseSacredAsh(u8);
static void Task_SacredAshLoop(u8);
static void Task_SacredAshDisplayHPRestored(u8);
static void GiveItemOrMailToSelectedMon(u8);
static void DisplayItemMustBeRemovedFirstMessage(u8);
static void Task_SwitchItemsFromBagYesNo(u8);
static void CB2_WriteMailToGiveMonFromBag(void);
static void GiveItemToSelectedMon(u8);
static void Task_UpdateHeldItemSpriteAndClosePartyMenu(u8);
static void CB2_ReturnToPartyOrBagMenuFromWritingMail(void);
static bool8 ReturnGiveItemToBagOrPC(enum Item);
static void Task_DisplayGaveMailFromBagMessage(u8);
static void Task_HandleSwitchItemsFromBagYesNoInput(u8);
static void Task_ValidateChosenHalfParty(u8);
static bool8 GetBattleEntryEligibility(struct Pokemon *);
static bool8 HasPartySlotAlreadyBeenSelected(u8);
static u8 GetBattleEntryLevelCap(void);
static u8 GetMaxBattleEntries(void);
static u8 GetMinBattleEntries(void);
static void Task_ContinueChoosingHalfParty(u8);
static void BufferBattlePartyOrder(u8 *, bool8);
static void BufferBattlePartyOrderBySide(u8 *, u8, enum BattlerId);
static void Task_InitMultiPartnerPartySlideIn(u8);
static void Task_MultiPartnerPartySlideIn(u8);
static void SlideMultiPartyMenuBoxSpritesOneStep(u8);
static void Task_WaitAfterMultiPartnerPartySlideIn(u8);
static void Task_WaitBeforeMultiPartnerFullParty(u8);
static void Task_WaitAfterMultiPartnerFullParty(u8);
static void BufferMonSelection(void);
static void Task_PartyMenuWaitForFade(u8 taskId);
static void Task_ChooseContestMon(u8 taskId);
static void CB2_ChooseContestMon(void);
static void Task_ChoosePartyMon(u8 taskId);
static void Task_ChooseMonForMoveRelearner(u8 taskId);
static void CB2_ChooseMonForMoveRelearner(void);
static void Task_BattlePyramidChooseMonHeldItems(u8);
static void ShiftMoveSlot(struct BoxPokemon *, u8, u8);
static void BlitBitmapToPartyWindow_LeftColumn(u8, u8, u8, u8, u8, bool8);
static void BlitBitmapToPartyWindow_RightColumn(u8, u8, u8, u8, u8, bool8);
static void CursorCb_Summary(u8);
static void CursorCb_StatEditor(u8);
static void CursorCb_Switch(u8);
static void CursorCb_Cancel1(u8);
static void CursorCb_Item(u8);
static void CursorCb_Give(u8);
static void CursorCb_TakeItem(u8);
static void CursorCb_MoveItem(u8);
static void CursorCb_Mail(u8);
static void CursorCb_Read(u8);
static void CursorCb_TakeMail(u8);
static void CursorCb_Cancel2(u8);
static void CursorCb_SendMon(u8);
static void CursorCb_Enter(u8);
static void CursorCb_NoEntry(u8);
static void CursorCb_Store(u8);
static void CursorCb_Register(u8);
static void CursorCb_Trade1(u8);
static void CursorCb_Trade2(u8);
static void CursorCb_Toss(u8);
static void CursorCb_FieldMove(u8);
static void CursorCb_CatalogBulb(u8);
static void CursorCb_CatalogOven(u8);
static void CursorCb_CatalogWashing(u8);
static void CursorCb_CatalogFridge(u8);
static void CursorCb_CatalogFan(u8);
static void CursorCb_CatalogMower(u8);
static void CursorCb_ChangeForm(u8);
static void CursorCb_ChangeAbility(u8);
void TryItemHoldFormChange(struct Pokemon *mon, s8 slotId, enum BattleTrainer trainer);
static void ShowMoveSelectWindow(u8 slot);
static void Task_HandleWhichMoveInput(u8 taskId);
static void Task_HideFollowerNPCForTeleport(u8);
static void FieldCallback_RockClimb(void);
static void GetPartyAndSlotFromPartyMenuId(s8 menuId, struct Pokemon **party, s8 *partySlot);
static struct Pokemon *GetPartyMonFromPartyMenuId(s8 menuId);
static void GetMultiPartyForSummaryScreen(void);
static void RestoreMultiPartyFromSummaryScreen(void);
static void Task_FirstBattleEnterParty_WaitFadeIn(u8 taskId);
static void Task_FirstBattleEnterParty_DarkenScreen(u8 taskId);
static void Task_FirstBattleEnterParty_WaitDarken(u8 taskId);
static void Task_FirstBattleEnterParty_CreatePrinter(u8 taskId);
static void Task_FirstBattleEnterParty_RunPrinterMsg1(u8 taskId);
static void Task_FirstBattleEnterParty_LightenFirstMonIcon(u8 taskId);
static void Task_FirstBattleEnterParty_WaitLightenFirstMonIcon(u8 taskId);
static void Task_FirstBattleEnterParty_StartPrintMsg2(u8 taskId);
static void Task_FirstBattleEnterParty_RunPrinterMsg2(u8 taskId);
static void Task_FirstBattleEnterParty_FadeNormal(u8 taskId);
static void Task_FirstBattleEnterParty_WaitFadeNormal(u8 taskId);
static u8 CombinedToIndividualPartyId(u8 index);
static u8 IndividualToCombinedPartyId(u8 index, enum BattlerId battler);
bool8 CanPokemonEvolveFromMenu(struct Pokemon* mon);
static const u8 sText_askText[] = _("Would you like to change {STR_VAR_1}'s\nability to {STR_VAR_2}?");
static const u8 sText_doneText[] = _("{STR_VAR_1}'s ability became\n{STR_VAR_2}!{PAUSE_UNTIL_PRESS}");
static const u8 sText_BasePointsResetToZero[] = _("{STR_VAR_1}'s base points\nwere all reset to zero!{PAUSE_UNTIL_PRESS}");
static const u8 sText_CannotSendMonToBoxHM[] = _("Cannot send that mon to the box,\nbecause it knows a HM move.{PAUSE_UNTIL_PRESS}");
static const u8 sText_CannotSendMonToBoxPartner[] = _("Cannot send a mon that doesn't\nbelong to you to the box.{PAUSE_UNTIL_PRESS}");
static void CursorCb_Follower(u8);
static void CursorCb_FollowerSet(u8);
static void CursorCb_FollowerUnset(u8);
static void CursorCb_FollowerReturn(u8);

// static const data
#include "data/party_menu.h"

// code
static void InitPartyMenu(enum PartyMenuType menuType, enum PartyMenuLayout layout, u8 partyAction, bool8 keepCursorPos, u8 messageId, TaskFunc task, MainCallback callback)
{
    u16 i;

    ResetPartyMenu();
    sPartyMenuInternal = Alloc(sizeof(struct PartyMenuInternal));
    if (sPartyMenuInternal == NULL)
    {
        SetMainCallback2(callback);
    }
    else
    {
        gPartyMenu.menuType = menuType;
        gPartyMenu.exitCallback = callback;
        gPartyMenu.action = partyAction;
        sPartyMenuInternal->messageId = messageId;
        sPartyMenuInternal->task = task;
        sPartyMenuInternal->exitCallback = NULL;
        sPartyMenuInternal->lastSelectedSlot = 0;
        sPartyMenuInternal->spriteIdConfirmPokeball = 0x7F;
        sPartyMenuInternal->spriteIdCancelPokeball = 0x7F;

        if (menuType == PARTY_MENU_TYPE_CHOOSE_HALF)
            sPartyMenuInternal->chooseHalf = TRUE;
        else
            sPartyMenuInternal->chooseHalf = FALSE;

        if (layout != KEEP_PARTY_LAYOUT)
            gPartyMenu.layout = layout;

        for (i = 0; i < ARRAY_COUNT(sPartyMenuInternal->data); i++)
            sPartyMenuInternal->data[i] = 0;
        for (i = 0; i < ARRAY_COUNT(sPartyMenuInternal->windowId); i++)
            sPartyMenuInternal->windowId[i] = WINDOW_NONE;

        if (!keepCursorPos)
        {
            gPartyMenu.slotId = 0;
        }
        else if (gPartyMenu.slotId > PARTY_SIZE - 1
         || GetMonData(GetPartyMonFromPartyMenuId(gPartyMenu.slotId), MON_DATA_SPECIES) == SPECIES_NONE)
        {
            gPartyMenu.slotId = 0;
        }

        if (gPartiesCount[B_TRAINER_PLAYER] == 0)
            gPartyMenu.slotId = PARTY_SIZE + 1; // Cancel

        gTextFlags.autoScroll = 0;
        CalculatePlayerPartyCount();
        SetMainCallback2(CB2_InitPartyMenu);
    }
}

static void LoadBattlePartyCurrentOrderForLayout(void)
{
    if (!gMain.inBattle || !IsMultiBattle())
        return;

    enum BattlerId battler = gBattlerInMenuId;

    if (gPartyMenu.layout == PARTY_LAYOUT_MULTI_FULL_PARTNER || gPartyMenu.layout == PARTY_LAYOUT_MULTI_FULL_SHOWCASE_PARTNER)
        battler = B_BATTLER_2;

    if (battler < gBattlersCount) // Including check given recent cases where animations set battlers OOB
        memcpy(gBattlePartyCurrentOrder, gBattleStruct->battlerPartyOrders[battler], sizeof(gBattlePartyCurrentOrder));
}

static void RefreshPartyMenu(void) //Refreshes the party menu without restarting tasks
{
    u16 i;
    for (i = 0; i < ARRAY_COUNT(sPartyMenuInternal->data); i++)
        sPartyMenuInternal->data[i] = 0;
    for (i = 0; i < ARRAY_COUNT(sPartyMenuInternal->windowId); i++)
        sPartyMenuInternal->windowId[i] = WINDOW_NONE;
    gTextFlags.autoScroll = 0;
    CalculatePlayerPartyCount();
    gMain.state = 0;
    SetMainCallback2(CB2_ReloadPartyMenu);
}

static void CB2_UpdatePartyMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void VBlankCB_PartyMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_InitPartyMenu(void)
{
    while (TRUE)
    {
        if (MenuHelpers_ShouldWaitForLinkRecv() == TRUE || ShowPartyMenu() == TRUE || MenuHelpers_IsLinkActive() == TRUE)
            break;
    }
}

static void CB2_ReloadPartyMenu(void)
{
    while (TRUE)
    {
        if (MenuHelpers_ShouldWaitForLinkRecv() == TRUE || ReloadPartyMenu() == TRUE || MenuHelpers_IsLinkActive() == TRUE)
            break;
    }
}

static bool8 ShowPartyMenu(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ResetVramOamAndBgCntRegs();
        ClearScheduledBgCopiesToVram();
        gMain.state++;
        break;
    case 1:
        ScanlineEffect_Stop();
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        gPaletteFade.bufferTransferDisabled = TRUE;
        gMain.state++;
        break;
    case 3:
        ResetSpriteData();
        gMain.state++;
        break;
    case 4:
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 5:
        if (!MenuHelpers_IsLinkActive())
            ResetTasks();
        gMain.state++;
        break;
    case 6:
        SetPartyMonsAllowedInMinigame();
        gMain.state++;
        break;
    case 7:
        if (!AllocPartyMenuBg())
        {
            ExitPartyMenu();
            return TRUE;
        }
        else
        {
            sPartyMenuInternal->data[0] = 0;
            gMain.state++;
        }
        break;
    case 8:
        if (AllocPartyMenuBgGfx())
            gMain.state++;
        break;
    case 9:
        InitPartyMenuWindows(gPartyMenu.layout);
        gMain.state++;
        break;
    case 10:
        InitPartyMenuBoxes(gPartyMenu.layout);
        sPartyMenuInternal->data[0] = 0;
        gMain.state++;
        break;
    case 11:
        LoadHeldItemIcons();
        gMain.state++;
        break;
    case 12:
        LoadPartyMenuPokeballGfx();
        gMain.state++;
        break;
    case 13:
        LoadPartyMenuAilmentGfx();
        gMain.state++;
        break;
    case 14:
        LoadMonIconPalettes();
        gMain.state++;
        break;
    case 15:
        if (CreatePartyMonSpritesLoop())
        {
            sPartyMenuInternal->data[0] = 0;
            gMain.state++;
        }
        break;
    case 16:
        if (RenderPartyMenuBoxes())
        {
            sPartyMenuInternal->data[0] = 0;
            gMain.state++;
        }
        break;
    case 17:
        CreateCancelConfirmPokeballSprites();
        gMain.state++;
        break;
    case 18:
        CreateCancelConfirmWindows(sPartyMenuInternal->chooseHalf);
        gMain.state++;
        break;
    case 19:
        gMain.state++;
        break;
    case 20:
        CreateTask(sPartyMenuInternal->task, 0);
        DisplayPartyMenuStdMessage(sPartyMenuInternal->messageId);
        gMain.state++;
        break;
    case 21:
        BlendPalettes(PALETTES_ALL, 16, 0);
        gPaletteFade.bufferTransferDisabled = FALSE;
        gMain.state++;
        break;
    case 22:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gMain.state++;
        break;
    default:
        SetVBlankCallback(VBlankCB_PartyMenu);
        SetMainCallback2(CB2_UpdatePartyMenu);
        return TRUE;
    }
    return FALSE;
}

static bool8 ReloadPartyMenu(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ClearScheduledBgCopiesToVram();
        gMain.state++;
        break;
    case 1:
        ScanlineEffect_Stop();
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        gPaletteFade.bufferTransferDisabled = TRUE;
        gMain.state++;
        break;
    case 3:
        ResetSpriteData();
        gMain.state++;
        break;
    case 4:
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 5:
        SetPartyMonsAllowedInMinigame();
        gMain.state++;
        break;
    case 6:
        sPartyMenuInternal->data[0] = 0;
        gMain.state++;
        break;
    case 7:
        LoadPartyMenuWindows();
        gMain.state++;
        break;
    case 8:
        LoadPartyMenuBoxes(gPartyMenu.layout);
        sPartyMenuInternal->data[0] = 0;
        gMain.state++;
        break;
    case 9:
        LoadHeldItemIcons();
        gMain.state++;
        break;
    case 10:
        LoadPartyMenuPokeballGfx();
        gMain.state++;
        break;
    case 11:
        LoadPartyMenuAilmentGfx();
        gMain.state++;
        break;
    case 12:
        LoadMonIconPalettes();
        gMain.state++;
        break;
    case 13:
        if (CreatePartyMonSpritesLoop())
        {
            sPartyMenuInternal->data[0] = 0;
            gMain.state++;
        }
        break;
    case 14:
        if (RenderPartyMenuBoxes())
        {
            sPartyMenuInternal->data[0] = 0;
            gMain.state++;
        }
        break;
    case 15:
        CreateCancelConfirmPokeballSprites();
        gMain.state++;
        break;
    case 16:
        CreateCancelConfirmWindows(sPartyMenuInternal->chooseHalf);
        gMain.state++;
        break;
    case 17:
        BlendPalettes(PALETTES_ALL, 16, RGB_WHITEALPHA);
        gPaletteFade.bufferTransferDisabled = FALSE;
        gMain.state++;
        break;
    case 18:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_WHITEALPHA);
        gMain.state++;
        break;
    default:
        SetVBlankCallback(VBlankCB_PartyMenu);
        SetMainCallback2(CB2_UpdatePartyMenu);
        return TRUE;
    }
    return FALSE;
}

static void ExitPartyMenu(void)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_ExitPartyMenu, 0);
    SetVBlankCallback(VBlankCB_PartyMenu);
    SetMainCallback2(CB2_UpdatePartyMenu);
}

static void Task_ExitPartyMenu(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(gPartyMenu.exitCallback);
        FreePartyPointers();
        DestroyTask(taskId);
    }
}

static void ResetPartyMenu(void)
{
    sPartyMenuInternal = NULL;
    sPartyBgTilemapBuffer = NULL;
    sPartyMenuBoxes = NULL;
    sPartyBgGfxTilemap = NULL;
}

static bool8 AllocPartyMenuBg(void)
{
    sPartyBgTilemapBuffer = Alloc(0x800);
    if (sPartyBgTilemapBuffer == NULL)
        return FALSE;

    memset(sPartyBgTilemapBuffer, 0, 0x800);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sPartyMenuBgTemplates, ARRAY_COUNT(sPartyMenuBgTemplates));
    SetBgTilemapBuffer(1, sPartyBgTilemapBuffer);
    ResetAllBgsCoordinates();
    ScheduleBgCopyTilemapToVram(1);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
    return TRUE;
}

static bool8 AllocPartyMenuBgGfx(void)
{
    u32 sizeout;

    switch (sPartyMenuInternal->data[0])
    {
    case 0:
        sPartyBgGfxTilemap = malloc_and_decompress(gPartyMenuBg_Gfx, &sizeout);
        LoadBgTiles(1, sPartyBgGfxTilemap, sizeout, 0);
        sPartyMenuInternal->data[0]++;
        break;
    case 1:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            DecompressDataWithHeaderWram(gPartyMenuBg_Tilemap, sPartyBgTilemapBuffer);
            sPartyMenuInternal->data[0]++;
        }
        break;
    case 2:
        LoadPalette(gPartyMenuBg_Pal, BG_PLTT_ID(0), 11 * PLTT_SIZE_4BPP);
        CpuCopy16(gPlttBufferUnfaded, sPartyMenuInternal->palBuffer, 11 * PLTT_SIZE_4BPP);
        sPartyMenuInternal->data[0]++;
        break;
    case 3:
        PartyPaletteBufferCopy(4);
        sPartyMenuInternal->data[0]++;
        break;
    case 4:
        PartyPaletteBufferCopy(5);
        sPartyMenuInternal->data[0]++;
        break;
    case 5:
        PartyPaletteBufferCopy(6);
        sPartyMenuInternal->data[0]++;
        break;
    case 6:
        PartyPaletteBufferCopy(7);
        sPartyMenuInternal->data[0]++;
        break;
    case 7:
        PartyPaletteBufferCopy(8);
        sPartyMenuInternal->data[0]++;
        break;
    default:
        return TRUE;
    }
    return FALSE;
}

static void PartyPaletteBufferCopy(u8 palNum)
{
    u8 offset = PLTT_ID(palNum);
    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(3)], &gPlttBufferUnfaded[offset], PLTT_SIZE_4BPP);
    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(3)], &gPlttBufferFaded[offset], PLTT_SIZE_4BPP);
}

static void FreePartyPointers(void)
{
    if (sPartyMenuInternal)
        Free(sPartyMenuInternal);
    if (sPartyBgTilemapBuffer)
        Free(sPartyBgTilemapBuffer);
    if (sPartyBgGfxTilemap)
        Free(sPartyBgGfxTilemap);
    if (sPartyMenuBoxes)
        Free(sPartyMenuBoxes);
    FreeAllWindowBuffers();
}

static void InitPartyMenuBoxes(enum PartyMenuLayout layout)
{
    sPartyMenuBoxes = Alloc(sizeof(struct PartyMenuBox[PARTY_SIZE]));
    LoadPartyMenuBoxes(layout);
}

static void LoadPartyMenuBoxes(enum PartyMenuLayout layout)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        sPartyMenuBoxes[i].infoRects = &sPartyBoxInfoRects[PARTY_BOX_RIGHT_COLUMN];
        sPartyMenuBoxes[i].spriteCoords = sPartyMenuSpriteCoords[layout][i];
        sPartyMenuBoxes[i].windowId = i;
        sPartyMenuBoxes[i].monSpriteId = SPRITE_NONE;
        sPartyMenuBoxes[i].itemSpriteId = SPRITE_NONE;
        sPartyMenuBoxes[i].pokeballSpriteId = SPRITE_NONE;
        sPartyMenuBoxes[i].statusSpriteId = SPRITE_NONE;
    }

    // The first party mon goes in the left column
    sPartyMenuBoxes[0].infoRects = &sPartyBoxInfoRects[PARTY_BOX_LEFT_COLUMN];

    if (layout == PARTY_LAYOUT_MULTI_SHOWCASE)
        sPartyMenuBoxes[3].infoRects = &sPartyBoxInfoRects[PARTY_BOX_LEFT_COLUMN];
    else if (layout == PARTY_LAYOUT_DOUBLE || layout == PARTY_LAYOUT_MULTI)
        sPartyMenuBoxes[1].infoRects = &sPartyBoxInfoRects[PARTY_BOX_LEFT_COLUMN];
}

static void RenderPartyMenuBox(u8 slot)
{
    if (gPartyMenu.menuType == PARTY_MENU_TYPE_MULTI_FULL_SHOWCASE && gPartyMenu.layout == PARTY_LAYOUT_MULTI_FULL_SHOWCASE_PARTNER)
    {
        DisplayPartyPokemonDataForMultiBattle(slot);
        if (gMultiPartnerParty[slot].species == SPECIES_NONE)
            LoadPartyBoxPalette(&sPartyMenuBoxes[slot], PARTY_PAL_NO_MON);
        else
            LoadPartyBoxPalette(&sPartyMenuBoxes[slot], PARTY_PAL_MULTI_ALT);
        CopyWindowToVram(sPartyMenuBoxes[slot].windowId, COPYWIN_GFX);
        PutWindowTilemap(sPartyMenuBoxes[slot].windowId);
        ScheduleBgCopyTilemapToVram(2);
    }
    else if (gPartyMenu.menuType == PARTY_MENU_TYPE_MULTI_SHOWCASE && slot >= MULTI_PARTY_SIZE)
    {
        DisplayPartyPokemonDataForMultiBattle(slot);
        if (gMultiPartnerParty[slot - MULTI_PARTY_SIZE].species == SPECIES_NONE)
            LoadPartyBoxPalette(&sPartyMenuBoxes[slot], PARTY_PAL_NO_MON);
        else
            LoadPartyBoxPalette(&sPartyMenuBoxes[slot], PARTY_PAL_MULTI_ALT);
        CopyWindowToVram(sPartyMenuBoxes[slot].windowId, COPYWIN_GFX);
        PutWindowTilemap(sPartyMenuBoxes[slot].windowId);
        ScheduleBgCopyTilemapToVram(2);
    }
    else if (gPartiesCount[B_TRAINER_PLAYER] != 0)
    {
        if (GetMonData(GetPartyMonFromPartyMenuId(slot), MON_DATA_SPECIES) == SPECIES_NONE)
        {
            DrawEmptySlot(sPartyMenuBoxes[slot].windowId);
            LoadPartyBoxPalette(&sPartyMenuBoxes[slot], PARTY_PAL_NO_MON);
            CopyWindowToVram(sPartyMenuBoxes[slot].windowId, COPYWIN_GFX);
        }
        else
        {
            if (gPartyMenu.menuType == PARTY_MENU_TYPE_MOVE_RELEARNER)
                DisplayPartyPokemonDataForRelearner(slot);
            else if (gPartyMenu.menuType == PARTY_MENU_TYPE_CONTEST)
                DisplayPartyPokemonDataForContest(slot);
            else if (gPartyMenu.menuType == PARTY_MENU_TYPE_CHOOSE_HALF)
                DisplayPartyPokemonDataForChooseHalf(slot);
            else if (gPartyMenu.menuType == PARTY_MENU_TYPE_MINIGAME)
                DisplayPartyPokemonDataForWirelessMinigame(slot);
            else if (gPartyMenu.menuType == PARTY_MENU_TYPE_STORE_PYRAMID_HELD_ITEMS)
                DisplayPartyPokemonDataForBattlePyramidHeldItem(slot);
            else if (!DisplayPartyPokemonDataForMoveTutorOrEvolutionItem(slot))
                DisplayPartyPokemonData(slot);

            if (gPartyMenu.menuType == PARTY_MENU_TYPE_MULTI_SHOWCASE || gPartyMenu.menuType == PARTY_MENU_TYPE_MULTI_FULL_SHOWCASE)
                AnimatePartySlot(slot, 0);
            else if (gPartyMenu.slotId == slot)
                AnimatePartySlot(slot, 1);
            else
                AnimatePartySlot(slot, 0);
        }
        PutWindowTilemap(sPartyMenuBoxes[slot].windowId);
        ScheduleBgCopyTilemapToVram(0);
    }
}

static void DisplayPartyPokemonData(u8 slot)
{
    struct Pokemon *mon = GetPartyMonFromPartyMenuId(slot);
    if (GetMonData(mon, MON_DATA_IS_EGG))
    {
        sPartyMenuBoxes[slot].infoRects->blitFunc(sPartyMenuBoxes[slot].windowId, 0, 0, 0, 0, TRUE);
        DisplayPartyPokemonNickname(mon, &sPartyMenuBoxes[slot], 0);
    }
    else
    {
        sPartyMenuBoxes[slot].infoRects->blitFunc(sPartyMenuBoxes[slot].windowId, 0, 0, 0, 0, FALSE);
        DisplayPartyPokemonNickname(mon, &sPartyMenuBoxes[slot], 0);
        DisplayPartyPokemonLevelCheck(mon, &sPartyMenuBoxes[slot], 0);
        DisplayPartyPokemonGenderNidoranCheck(mon, &sPartyMenuBoxes[slot], 0);
        DisplayPartyPokemonHPCheck(mon, &sPartyMenuBoxes[slot], 0);
        DisplayPartyPokemonMaxHPCheck(mon, &sPartyMenuBoxes[slot], 0);
        DisplayPartyPokemonHPBarCheck(mon, &sPartyMenuBoxes[slot]);
    }
}

static void DisplayPartyPokemonDescriptionData(u8 slot, u8 stringID)
{
    struct Pokemon *mon = GetPartyMonFromPartyMenuId(slot);

    sPartyMenuBoxes[slot].infoRects->blitFunc(sPartyMenuBoxes[slot].windowId, 0, 0, 0, 0, TRUE);
    DisplayPartyPokemonNickname(mon, &sPartyMenuBoxes[slot], 0);
    if (!GetMonData(mon, MON_DATA_IS_EGG))
    {
        DisplayPartyPokemonLevelCheck(mon, &sPartyMenuBoxes[slot], 0);
        DisplayPartyPokemonGenderNidoranCheck(mon, &sPartyMenuBoxes[slot], 0);
    }
    DisplayPartyPokemonDescriptionText(stringID, &sPartyMenuBoxes[slot], 0);
}

static void DisplayPartyPokemonDataForChooseHalf(u8 slot)
{
    u8 i;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot];
    u8 *order = gSelectedOrderFromParty;

    if (!GetBattleEntryEligibility(mon))
    {
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_NOT_ABLE);
        return;
    }
    else
    {
        for (i = 0; i < GetMaxBattleEntries(); i++)
        {
            if (order[i] != 0 && (order[i] - 1) == slot)
            {
                DisplayPartyPokemonDescriptionData(slot, i + PARTYBOX_DESC_FIRST);
                return;
            }
        }
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_ABLE_3);
    }
}

static void DisplayPartyPokemonDataForContest(u8 slot)
{
    switch (GetContestEntryEligibility(&gParties[B_TRAINER_PLAYER][slot]))
    {
    case CANT_ENTER_CONTEST:
    case CANT_ENTER_CONTEST_EGG:
    case CANT_ENTER_CONTEST_FAINTED:
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_NOT_ABLE);
        break;
    case CAN_ENTER_CONTEST_EQUAL_RANK:
    case CAN_ENTER_CONTEST_HIGH_RANK:
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_ABLE);
        break;
    }
}

static void DisplayPartyPokemonDataForRelearner(u8 slot)
{
    bool32 hasMoves = FALSE;
    if (!GetBoxMonData(&gParties[B_TRAINER_PLAYER][slot].box, MON_DATA_IS_EGG) && HasMoveToRelearn(&gParties[B_TRAINER_PLAYER][slot].box, gMoveRelearnerState))
        hasMoves = TRUE;
    u32 desc = (hasMoves ? PARTYBOX_DESC_ABLE_2 : PARTYBOX_DESC_NOT_ABLE_2);
    DisplayPartyPokemonDescriptionData(slot, desc);
}

static void DisplayPartyPokemonDataForWirelessMinigame(u8 slot)
{
    if (IsMonAllowedInMinigame(slot) == TRUE)
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_ABLE);
    else
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_NOT_ABLE);
}

static void DisplayPartyPokemonDataForBattlePyramidHeldItem(u8 slot)
{
    if (GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_HELD_ITEM))
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_HAVE);
    else
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_DONT_HAVE);
}

// Returns TRUE if teaching move or can't evolve with item (i.e. description data is shown), FALSE otherwise
static bool8 DisplayPartyPokemonDataForMoveTutorOrEvolutionItem(u8 slot)
{
    struct Pokemon *currentPokemon = &gParties[B_TRAINER_PLAYER][slot];
    enum Item item = gSpecialVar_ItemId;

    if (gPartyMenu.action == PARTY_ACTION_MOVE_TUTOR)
    {
        gSpecialVar_Result = FALSE;
        DisplayPartyPokemonDataToTeachMove(slot, gSpecialVar_0x8005);
    }
    else
    {
        if (gPartyMenu.action != PARTY_ACTION_USE_ITEM)
            return FALSE;

        switch (CheckIfItemIsTMHMOrEvolutionStone(item))
        {
        default:
            return FALSE;
        case ITEM_IS_TM_HM: // TM/HM
            DisplayPartyPokemonDataToTeachMove(slot, ItemIdToBattleMoveId(item));
            break;
        case ITEM_IS_EVOLUTION_STONE: // Evolution stone
            if (!GetMonData(currentPokemon, MON_DATA_IS_EGG) && GetEvolutionTargetSpecies(currentPokemon, EVO_MODE_ITEM_CHECK, item, NULL, NULL, CHECK_EVO) != SPECIES_NONE)
                return FALSE;
            DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_NO_USE);
            break;
        }
    }
    return TRUE;
}

static void DisplayPartyPokemonDataToTeachMove(u8 slot, enum Move move)
{
    switch (CanTeachMove(&gParties[B_TRAINER_PLAYER][slot], move))
    {
    case CANNOT_LEARN_MOVE:
    case CANNOT_LEARN_MOVE_IS_EGG:
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_NOT_ABLE_2);
        break;
    case ALREADY_KNOWS_MOVE:
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_LEARNED);
        break;
    default:
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_ABLE_2);
        break;
    }
}

static void DisplayPartyPokemonDataForMultiBattle(u8 slot)
{
    struct PartyMenuBox *menuBox = &sPartyMenuBoxes[slot];
    u8 actualSlot = slot - MULTI_PARTY_SIZE;

    if (gPartyMenu.layout == PARTY_LAYOUT_MULTI_FULL_SHOWCASE_PARTNER)
        actualSlot = slot;

    if (gMultiPartnerParty[actualSlot].species == SPECIES_NONE)
    {
        DrawEmptySlot(menuBox->windowId);
    }
    else
    {
        menuBox->infoRects->blitFunc(menuBox->windowId, 0, 0, 0, 0, FALSE);
        StringCopy(gStringVar1, gMultiPartnerParty[actualSlot].nickname);
        StringGet_Nickname(gStringVar1);
        ConvertInternationalPlayerName(gStringVar1);
        DisplayPartyPokemonBarDetailToFit(menuBox->windowId, gStringVar1, 0, menuBox->infoRects->dimensions, 50);
        DisplayPartyPokemonLevel(gMultiPartnerParty[actualSlot].level, menuBox);
        DisplayPartyPokemonGender(gMultiPartnerParty[actualSlot].gender, gMultiPartnerParty[actualSlot].species, gMultiPartnerParty[actualSlot].nickname, menuBox);
        DisplayPartyPokemonHP(gMultiPartnerParty[actualSlot].hp, gMultiPartnerParty[actualSlot].maxhp, menuBox);
        DisplayPartyPokemonMaxHP(gMultiPartnerParty[actualSlot].maxhp, menuBox);
        DisplayPartyPokemonHPBar(gMultiPartnerParty[actualSlot].hp, gMultiPartnerParty[actualSlot].maxhp, menuBox);
    }
}

static bool8 RenderPartyMenuBoxes(void)
{
    RenderPartyMenuBox(sPartyMenuInternal->data[0]);
    if (++sPartyMenuInternal->data[0] == PARTY_SIZE)
        return TRUE;
    else
        return FALSE;
}

static u8 *GetPartyMenuBgTile(u16 tileId)
{
    return &sPartyBgGfxTilemap[tileId << 5];
}

static void CreatePartyMonSprites(u8 slot)
{
    struct Pokemon *party = NULL;
    s8 partySlot = 0;
    GetPartyAndSlotFromPartyMenuId(slot, &party, &partySlot);

    if (gPartyMenu.menuType == PARTY_MENU_TYPE_MULTI_FULL_SHOWCASE && gPartyMenu.layout == PARTY_LAYOUT_MULTI_FULL_SHOWCASE_PARTNER)
    {
        u8 status;

        if (gMultiPartnerParty[partySlot].species != SPECIES_NONE)
        {
            CreatePartyMonIconSpriteParameterized(gMultiPartnerParty[partySlot].species, gMultiPartnerParty[partySlot].personality, FALSE, &sPartyMenuBoxes[slot], 0);
            CreatePartyMonHeldItemSpriteParameterized(gMultiPartnerParty[partySlot].species, gMultiPartnerParty[partySlot].heldItem, &sPartyMenuBoxes[slot]);
            CreatePartyMonPokeballSpriteParameterized(gMultiPartnerParty[partySlot].species, &sPartyMenuBoxes[slot]);
            if (gMultiPartnerParty[partySlot].hp == 0)
                status = AILMENT_FNT;
            else
                status = GetAilmentFromStatus(gMultiPartnerParty[partySlot].status);
            CreatePartyMonStatusSpriteParameterized(gMultiPartnerParty[partySlot].species, status, &sPartyMenuBoxes[slot]);
        }
    }
    else if (gPartyMenu.menuType == PARTY_MENU_TYPE_MULTI_SHOWCASE && slot >= MULTI_PARTY_SIZE)
    {
        u8 status;

        if (gMultiPartnerParty[partySlot].species != SPECIES_NONE)
        {
            CreatePartyMonIconSpriteParameterized(gMultiPartnerParty[partySlot].species, gMultiPartnerParty[partySlot].personality, FALSE, &sPartyMenuBoxes[slot], 0);
            CreatePartyMonHeldItemSpriteParameterized(gMultiPartnerParty[partySlot].species, gMultiPartnerParty[partySlot].heldItem, &sPartyMenuBoxes[slot]);
            CreatePartyMonPokeballSpriteParameterized(gMultiPartnerParty[partySlot].species, &sPartyMenuBoxes[slot]);
            if (gMultiPartnerParty[partySlot].hp == 0)
                status = AILMENT_FNT;
            else
                status = GetAilmentFromStatus(gMultiPartnerParty[partySlot].status);
            CreatePartyMonStatusSpriteParameterized(gMultiPartnerParty[partySlot].species, status, &sPartyMenuBoxes[slot]);
        }
    }
    else if (GetMonData(&party[partySlot], MON_DATA_SPECIES) != SPECIES_NONE)
    {
        CreatePartyMonIconSprite(&party[partySlot], &sPartyMenuBoxes[slot]);
        CreatePartyMonHeldItemSprite(&party[partySlot], &sPartyMenuBoxes[slot]);
        CreatePartyMonPokeballSprite(&party[partySlot], &sPartyMenuBoxes[slot]);
        CreatePartyMonStatusSprite(&party[partySlot], &sPartyMenuBoxes[slot]);
    }
}

static bool8 CreatePartyMonSpritesLoop(void)
{
    CreatePartyMonSprites(sPartyMenuInternal->data[0]);
    if (++sPartyMenuInternal->data[0] == PARTY_SIZE)
        return TRUE;
    else
        return FALSE;
}

static void CreateCancelConfirmPokeballSprites(void)
{
    if (gPartyMenu.menuType == PARTY_MENU_TYPE_MULTI_SHOWCASE
     || gPartyMenu.menuType == PARTY_MENU_TYPE_MULTI_FULL_SHOWCASE)
    {
        // The showcase has no Cancel/Confirm buttons
        FillBgTilemapBufferRect(1, 14, 23, 17, 7, 2, 1);
    }
    else
    {
        if (sPartyMenuInternal->chooseHalf)
        {
            sPartyMenuInternal->spriteIdConfirmPokeball = CreateSmallPokeballButtonSprite(0xBF, 0x88);
            DrawCancelConfirmButtons();
            sPartyMenuInternal->spriteIdCancelPokeball = CreateSmallPokeballButtonSprite(0xBF, 0x98);
        }
        else
        {
            sPartyMenuInternal->spriteIdCancelPokeball = CreatePokeballButtonSprite(198, 148);
        }
        AnimatePartySlot(gPartyMenu.slotId, 1);
    }
}

void AnimatePartySlot(u8 slot, u8 animNum)
{
    u8 spriteId;

    switch (slot)
    {
    default:
        if (GetMonData(GetPartyMonFromPartyMenuId(slot), MON_DATA_SPECIES) != SPECIES_NONE)
        {
            LoadPartyBoxPalette(&sPartyMenuBoxes[slot], GetPartyBoxPaletteFlags(slot, animNum));
            AnimateSelectedPartyIcon(sPartyMenuBoxes[slot].monSpriteId, animNum);
            PartyMenuStartSpriteAnim(sPartyMenuBoxes[slot].pokeballSpriteId, animNum);
        }
        return;
    case PARTY_SIZE: // Confirm
        if (animNum == 0)
            SetBgTilemapPalette(1, 23, 16, 7, 2, 1);
        else
            SetBgTilemapPalette(1, 23, 16, 7, 2, 2);
        spriteId = sPartyMenuInternal->spriteIdConfirmPokeball;
        break;
    case PARTY_SIZE + 1: // Cancel
        // The position of the Cancel button changes if Confirm is present
        if (!sPartyMenuInternal->chooseHalf)
        {
            if (animNum == 0)
                SetBgTilemapPalette(1, 23, 17, 7, 2, 1);
            else
                SetBgTilemapPalette(1, 23, 17, 7, 2, 2);
        }
        else if (animNum == 0)
        {
            SetBgTilemapPalette(1, 23, 18, 7, 2, 1);
        }
        else
        {
            SetBgTilemapPalette(1, 23, 18, 7, 2, 2);
        }
        spriteId = sPartyMenuInternal->spriteIdCancelPokeball;
        break;
    }
    PartyMenuStartSpriteAnim(spriteId, animNum);
    ScheduleBgCopyTilemapToVram(1);
}

static u8 GetPartyBoxPaletteFlags(u8 slot, u8 animNum)
{
    u8 palFlags = 0;
    struct Pokemon *mon = GetPartyMonFromPartyMenuId(slot);

    if (animNum == 1)
        palFlags |= PARTY_PAL_SELECTED;
    if (GetMonData(mon, MON_DATA_HP) == 0)
        palFlags |= PARTY_PAL_FAINTED;
    if (PartyBoxPal_ParnterOrDisqualifiedInArena(slot) == TRUE)
        palFlags |= PARTY_PAL_MULTI_ALT;
    if (gPartyMenu.action == PARTY_ACTION_SWITCHING)
        palFlags |= PARTY_PAL_SWITCHING;
    if (gPartyMenu.action == PARTY_ACTION_SWITCH)
    {
        if (slot == gPartyMenu.slotId || slot == gPartyMenu.slotId2)
            palFlags |= PARTY_PAL_TO_SWITCH;
    }
    if (gPartyMenu.action == PARTY_ACTION_SOFTBOILED && slot == gPartyMenu.slotId )
        palFlags |= PARTY_PAL_TO_SOFTBOIL;

    return palFlags;
}

static bool8 PartyBoxPal_ParnterOrDisqualifiedInArena(u8 slot)
{
    if (gPartyMenu.layout == PARTY_LAYOUT_MULTI_FULL_PARTNER || gPartyMenu.layout == PARTY_LAYOUT_MULTI_FULL_SHOWCASE_PARTNER)
        return TRUE;

    if (gPartyMenu.layout == PARTY_LAYOUT_MULTI && (slot == 1 || slot == 4 || slot == 5))
        return TRUE;

    if (slot < MULTI_PARTY_SIZE && (gBattleTypeFlags & BATTLE_TYPE_ARENA) && gMain.inBattle && (gBattleStruct->arenaLostPlayerMons >> GetPartyIdFromBattleSlot(slot) & 1))
        return TRUE;

    return FALSE;
}

static void DrawCancelConfirmButtons(void)
{
    CopyToBgTilemapBufferRect_ChangePalette(1, sConfirmButton_Tilemap, 23, 16, 7, 2, 17);
    CopyToBgTilemapBufferRect_ChangePalette(1, sCancelButton_Tilemap, 23, 18, 7, 2, 17);
    ScheduleBgCopyTilemapToVram(1);
}

bool8 IsMultiBattle(void)
{
    if (gBattleTypeFlags & BATTLE_TYPE_MULTI && IsDoubleBattle() && gMain.inBattle)
        return TRUE;
    else
        return FALSE;
}

static void SwapPartyPokemon(struct Pokemon *mon1, struct Pokemon *mon2)
{
    struct Pokemon *temp = Alloc(sizeof(struct Pokemon));

    *temp = *mon1;
    *mon1 = *mon2;
    *mon2 = *temp;

    Free(temp);
}

static void Task_ClosePartyMenu(u8 taskId)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_ClosePartyMenuAndSetCB2;
}

static void Task_ClosePartyMenuAndSetCB2(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        if (gPartyMenu.menuType == PARTY_MENU_TYPE_IN_BATTLE)
            UpdatePartyToFieldOrder();

        if (sPartyMenuInternal->exitCallback != NULL)
            SetMainCallback2(sPartyMenuInternal->exitCallback);
        else
            SetMainCallback2(gPartyMenu.exitCallback);

        ResetSpriteData();
        FreePartyPointers();
        DestroyTask(taskId);
    }
}

u8 GetCursorSelectionMonId(void)
{
    return gPartyMenu.slotId;
}

u8 GetPartyMenuType(void)
{
    return gPartyMenu.menuType;
}

void Task_HandleChooseMonInput(u8 taskId)
{
    if (!gPaletteFade.active && MenuHelpers_ShouldWaitForLinkRecv() != TRUE)
    {
        s8 *slotPtr = GetCurrentPartySlotPtr();

        switch (PartyMenuButtonHandler(slotPtr))
        {
        case A_BUTTON: // Selected mon
            HandleChooseMonSelection(taskId, slotPtr);
            break;
        case B_BUTTON: // Selected Cancel / pressed B
            HandleChooseMonCancel(taskId, slotPtr);
            break;
        case START_BUTTON:
            if (sPartyMenuInternal->chooseHalf)
            {
                PlaySE(SE_SELECT);
                MoveCursorToConfirm();
            }
            break;
        case R_BUTTON: // Only used in full-team multis to cycle player/partner parties
            PlaySE(SE_M_HARDEN);
            UpdatePartyToFieldOrder();

            if (gPartyMenu.layout == PARTY_LAYOUT_MULTI_FULL)
                gPartyMenu.layout = PARTY_LAYOUT_MULTI_FULL_PARTNER;
            else
                gPartyMenu.layout = PARTY_LAYOUT_MULTI_FULL;

            gPartyMenu.slotId = 0;
            sPartyMenuInternal->lastSelectedSlot = 0;

            LoadBattlePartyCurrentOrderForLayout();
            UpdatePartyToBattleOrder();
            RefreshPartyMenu();
            break;
        }
    }
}

static s8 *GetCurrentPartySlotPtr(void)
{
    if (gPartyMenu.action == PARTY_ACTION_SWITCH || gPartyMenu.action == PARTY_ACTION_SOFTBOILED)
        return &gPartyMenu.slotId2;
    else
        return &gPartyMenu.slotId;
}

static void HandleChooseMonSelection(u8 taskId, s8 *slotPtr)
{
    if (*slotPtr == PARTY_SIZE)
    {
        gPartyMenu.task(taskId);
    }
    else
    {
        switch (gPartyMenu.action)
        {
        case PARTY_ACTION_SOFTBOILED:
            if (IsSelectedMonNotEgg((u8 *)slotPtr))
            {
                PartyMenuRemoveWindow(&sPartyMenuInternal->windowId[1]);
                Task_TryUseSoftboiledOnPartyMon(taskId);
            }
            break;
        case PARTY_ACTION_USE_ITEM:
            if (IsSelectedMonNotEgg((u8 *)slotPtr))
            {
                if (gPartyMenu.menuType == PARTY_MENU_TYPE_IN_BATTLE)
                    sPartyMenuInternal->exitCallback = CB2_SetUpExitToBattleScreen;

                PartyMenuRemoveWindow(&sPartyMenuInternal->windowId[1]);
                gItemUseCB(taskId, Task_ClosePartyMenuAfterText);
            }
            break;
        case PARTY_ACTION_MOVE_TUTOR:
            if (IsSelectedMonNotEgg((u8 *)slotPtr))
            {
                PlaySE(SE_SELECT);
                PartyMenuRemoveWindow(&sPartyMenuInternal->windowId[1]);
                TryTutorSelectedMon(taskId);
            }
            break;
        case PARTY_ACTION_GIVE_MAILBOX_MAIL:
            if (IsSelectedMonNotEgg((u8 *)slotPtr))
            {
                PlaySE(SE_SELECT);
                PartyMenuRemoveWindow(&sPartyMenuInternal->windowId[1]);
                TryGiveMailToSelectedMon(taskId);
            }
            break;
        case PARTY_ACTION_GIVE_ITEM:
        case PARTY_ACTION_GIVE_PC_ITEM:
            if (IsSelectedMonNotEgg((u8 *)slotPtr))
            {
                PlaySE(SE_SELECT);
                PartyMenuRemoveWindow(&sPartyMenuInternal->windowId[1]);
                TryGiveItemOrMailToSelectedMon(taskId);
            }
            break;
        case PARTY_ACTION_SWITCH:
            PlaySE(SE_SELECT);
            SwitchSelectedMons(taskId);
            break;
        case PARTY_ACTION_CHOOSE_AND_CLOSE:
            PlaySE(SE_SELECT);
            Task_ClosePartyMenu(taskId);
            break;
        case PARTY_ACTION_MINIGAME:
            if (IsSelectedMonNotEgg((u8 *)slotPtr))
            {
                TryEnterMonForMinigame(taskId, (u8)*slotPtr);
            }
            break;
        case PARTY_ACTION_CHOOSE_FAINTED_MON:
        {
            u8 partyId = GetPartyIdFromBattleSlot((u8)*slotPtr);
            struct Pokemon *party = NULL;
            s8 partySlot = 0;
            GetPartyAndSlotFromPartyMenuId(*slotPtr, &party, &partySlot);

            if (GetMonData(&party[partySlot], MON_DATA_HP) > 0
             || GetMonData(&party[partySlot], MON_DATA_SPECIES_OR_EGG) == SPECIES_EGG
             || ((gBattleTypeFlags & BATTLE_TYPE_MULTI) && partyId >= (PARTY_SIZE / 2)))
            {
                // Can't select if egg, alive, or doesn't belong to you
                PlaySE(SE_FAILURE);
            }
            else
            {
                PlaySE(SE_SELECT);
                gSelectedMonPartyId = CombinedToIndividualPartyId(partyId);
                Task_ClosePartyMenu(taskId);
            }
            break;
        }
        case PARTY_ACTION_SEND_MON_TO_BOX:
        {
            u8 partyId = (u8)*slotPtr;
            if ((gBattleTypeFlags & BATTLE_TYPE_MULTI) && partyId >= (PARTY_SIZE / 2))
            {
                // Can't select if mon doesn't belong to you
                PlaySE(SE_FAILURE);
                DisplayPartyMenuMessage(sText_CannotSendMonToBoxPartner, FALSE);
                ScheduleBgCopyTilemapToVram(2);
                gTasks[taskId].func = Task_ReturnToChooseMonAfterText;
            }
            else if (DoesSelectedMonKnowHM((u8 *)slotPtr))
            {
                PlaySE(SE_FAILURE);
                DisplayPartyMenuMessage(sText_CannotSendMonToBoxHM, FALSE);
                ScheduleBgCopyTilemapToVram(2);
                gTasks[taskId].func = Task_ReturnToChooseMonAfterText;
            }
            else
            {
                PlaySE(SE_SELECT);
                gSelectedMonPartyId = GetPartyIdFromBattleSlot(partyId);
                Task_ClosePartyMenu(taskId);
            }
            break;
        }
        default:
        case PARTY_ACTION_ABILITY_PREVENTS:
        case PARTY_ACTION_SWITCHING:
            PlaySE(SE_SELECT);
            Task_TryCreateSelectionWindow(taskId);
            break;
        }
    }
}

static bool8 SpeciesHasTradeEvolution(enum Species species)
{
    const struct Evolution *evolutions = GetSpeciesEvolutions(species);
    int i;

    if (evolutions == NULL)
        return FALSE;

    for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        if (SanitizeSpeciesId(evolutions[i].targetSpecies) == SPECIES_NONE)
            continue;
        if (evolutions[i].method == EVO_TRADE)
            return TRUE;
    }

    return FALSE;
}

void ItemUseCB_LevelCapCandy(u8 taskId, TaskFunc task)
{
    u8 i;
    const u8 defaultLevel = 20;
    const u8 tradeLevel = 40;
    bool8 anyChanged = FALSE;
    struct Pokemon *mon;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            continue;
        if (GetMonData(mon, MON_DATA_IS_EGG))
            continue;

        u8 curLevel = GetMonData(mon, MON_DATA_LEVEL);
        enum Species species = GetMonData(mon, MON_DATA_SPECIES);
        u8 targetLevel = SpeciesHasTradeEvolution(species) ? tradeLevel : defaultLevel;

        if (targetLevel < 1)
            targetLevel = 1;
        if (targetLevel > MAX_LEVEL)
            targetLevel = MAX_LEVEL;

        if (curLevel >= targetLevel)
            continue;

        // Gesamt-EXP für das Ziel-Level entsprechend Wachstumstabelle
        u32 newExp = gExperienceTables[gSpeciesInfo[species].growthRate][targetLevel];

        // EXP setzen und Stats/Level neu berechnen
        SetMonData(mon, MON_DATA_EXP, &newExp);
        CalculateMonStats(mon);

        // Vollheilen auf Max-HP
        u16 maxHp = GetMonData(mon, MON_DATA_MAX_HP);
        SetMonData(mon, MON_DATA_HP, &maxHp);

        // Freundschaft auf 0 setzen, damit Freundschafts‑Entwicklung nicht auslöst
        u8 zeroFriendship = 0;
        SetMonData(mon, MON_DATA_FRIENDSHIP, &zeroFriendship);

        // UI aktualisieren
        UpdateMonDisplayInfoAfterRareCandy(i, mon);

        anyChanged = TRUE;
    }

    if (!anyChanged)
    {
        gPartyMenuUseExitCallback = FALSE;
        PlaySE(SE_SELECT);
        DisplayPartyMenuMessage(gText_WontHaveEffect, TRUE);
        ScheduleBgCopyTilemapToVram(2);
        gTasks[taskId].func = task;
    }
    else
    {
        gPartyMenuUseExitCallback = TRUE;
        PlaySE(SE_USE_ITEM);

        // Wir nutzen hier direkt die feste ID deines Items, statt uns auf die unzuverlässige SpecialVar zu verlassen
        RemoveBagItem(ITEM_LEVEL_CAP_CANDY, 1);

        // Das _("...")-Makro ist hier vollkommen ausreichend und sicher
        StringExpandPlaceholders(gStringVar4, _("All POKéMON were raised to the target levels!{PAUSE_UNTIL_PRESS}"));
        DisplayPartyMenuMessage(gStringVar4, TRUE);

        // Übergibt die Kontrolle zurück an die Menü-Task, um den Text anzuzeigen und das Menü sauber zu schließen
        gTasks[taskId].func = Task_PartyMenu_Action_Cancel; // Oder der Name deiner spezifischen 'task'-Variable
    }

}

void PartyMenuAction_Evolve(u8 taskId)
{
    PlaySE(SE_SELECT);
    u16 targetSpecies = GetEvolutionTargetSpecies(&gPlayerParty[gSelectedMonPartyId], EVO_MODE_LEVEL, 0);

    if (targetSpecies != SPECIES_NONE)
    {
        gCB2_AfterEvolution = CB2_ReturnToPartyMenu;
        BeginEvolutionScene(&gPlayerParty[gSelectedMonPartyId], targetSpecies, TRUE, gSelectedMonPartyId);
    }
    else
    {
        Task_ExitPartyMenu(taskId);
    }
}

#endif // <-- DIESE ZEILE AM ABSOLUTEN DATEIENDE ERGÄNZEN!
