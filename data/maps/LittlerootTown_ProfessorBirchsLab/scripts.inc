LittlerootTown_ProfessorBirchsLab_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, LittlerootTown_ProfessorBirchsLab_OnTransition
	.byte 0

LittlerootTown_ProfessorBirchsLab_OnTransition:
	call_if_eq VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 1, LittlerootTown_ProfessorBirchsLab_EventScript_MoveBirchToLeft
	end

LittlerootTown_ProfessorBirchsLab_EventScript_MoveBirchToLeft::
	setobjectxyperm 1, 6, 4
	setobjectmovementtype 1, MOVEMENT_TYPE_FACE_RIGHT
	return

LittlerootTown_ProfessorBirchsLab_EventScript_Birch::
	lock
	faceplayer
	msgbox LittlerootTown_ProfessorBirchsLab_Text_BirchWelcome, MSGBOX_DEFAULT
	release
	end

@ ==========================================================
@ REINES GBA-STANDARD MENÜ FÜR DIE STARTER BIS GEN 9
@ ==========================================================
LittlerootTown_ProfessorBirchsLab_EventScript_Assistant::
	lock
	faceplayer
	msgbox LittlerootTown_ProfessorBirchsLab_Text_AssistantIntro, MSGBOX_DEFAULT

LittlerootTown_ProfessorBirchsLab_EventScript_Assistant_GroupMenu::
	message LittlerootTown_ProfessorBirchsLab_Text_WhichGroup
	waitmessage
	multichoice 20, 2, 0, FALSE @ Nutzt das Standard 4er-Fenster (Kanto, Johto, Hoenn, Cancel oder Äquivalent)
	@ Falls 0 bei dir anders heißt, nutzen wir hier das allgegenwärtige Standard-Fenster
	switch VAR_RESULT
	case 0, LittlerootTown_ProfessorBirchsLab_EventScript_Group_1_3
	case 1, LittlerootTown_ProfessorBirchsLab_EventScript_Group_4_6
	case 2, LittlerootTown_ProfessorBirchsLab_EventScript_Group_7_9
	case 3, LittlerootTown_ProfessorBirchsLab_EventScript_ExitMenu
	case MULTI_B_PRESSED, LittlerootTown_ProfessorBirchsLab_EventScript_ExitMenu
	end

@ --- GRUPPE 1: GEN 1 BIS GEN 3 ---
LittlerootTown_ProfessorBirchsLab_EventScript_Group_1_3::
	message LittlerootTown_ProfessorBirchsLab_Text_WhichGeneration
	waitmessage
	multichoice 20, 2, 1, FALSE
	switch VAR_RESULT
	case 0, LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen1
	case 1, LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen2
	case 2, LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen3
	case 3, LittlerootTown_ProfessorBirchsLab_EventScript_Assistant_GroupMenu
	case MULTI_B_PRESSED, LittlerootTown_ProfessorBirchsLab_EventScript_Assistant_GroupMenu
	end

@ --- GRUPPE 2: GEN 4 BIS GEN 6 ---
LittlerootTown_ProfessorBirchsLab_EventScript_Group_4_6::
	message LittlerootTown_ProfessorBirchsLab_Text_WhichGeneration
	waitmessage
	multichoice 20, 2, 1, FALSE
	switch VAR_RESULT
	case 0, LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen4
	case 1, LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen5
	case 2, LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen6
	case 3, LittlerootTown_ProfessorBirchsLab_EventScript_Assistant_GroupMenu
	case MULTI_B_PRESSED, LittlerootTown_ProfessorBirchsLab_EventScript_Assistant_GroupMenu
	end

@ --- GRUPPE 3: GEN 7 BIS GEN 9 ---
LittlerootTown_ProfessorBirchsLab_EventScript_Group_7_9::
	message LittlerootTown_ProfessorBirchsLab_Text_WhichGeneration
	waitmessage
	multichoice 20, 2, 1, FALSE
	switch VAR_RESULT
	case 0, LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen7
	case 1, LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen8
	case 2, LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen9
	case 3, LittlerootTown_ProfessorBirchsLab_EventScript_Assistant_GroupMenu
	case MULTI_B_PRESSED, LittlerootTown_ProfessorBirchsLab_EventScript_Assistant_GroupMenu
	end

@ --- DIE AUSWERTUNGEN ---
LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen1::
	msgbox LittlerootTown_ProfessorBirchsLab_Text_KantoSelected, MSGBOX_DEFAULT
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen2::
	msgbox LittlerootTown_ProfessorBirchsLab_Text_JohtoSelected, MSGBOX_DEFAULT
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen3::
	msgbox LittlerootTown_ProfessorBirchsLab_Text_HoennSelected, MSGBOX_DEFAULT
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen4::
	msgbox LittlerootTown_ProfessorBirchsLab_Text_SinnohSelected, MSGBOX_DEFAULT
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen5::
	msgbox LittlerootTown_ProfessorBirchsLab_Text_UnovaSelected, MSGBOX_DEFAULT
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen6::
	msgbox LittlerootTown_ProfessorBirchsLab_Text_KalosSelected, MSGBOX_DEFAULT
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen7::
	msgbox LittlerootTown_ProfessorBirchsLab_Text_AlolaSelected, MSGBOX_DEFAULT
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen8::
	msgbox LittlerootTown_ProfessorBirchsLab_Text_GalarSelected, MSGBOX_DEFAULT
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_ChooseGen9::
	msgbox LittlerootTown_ProfessorBirchsLab_Text_PaldeaSelected, MSGBOX_DEFAULT
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_ExitMenu::
	msgbox LittlerootTown_ProfessorBirchsLab_Text_AssistantGoodLuck, MSGBOX_DEFAULT
	release
	end

@ ==========================================================
@ DIALOGE UND TEXTE (ASCII-KONFORM)
@ ==========================================================
LittlerootTown_ProfessorBirchsLab_Text_BirchWelcome:
	.string "BIRCH: Hello! Welcome to my POKéMON LAB!$"

LittlerootTown_ProfessorBirchsLab_Text_AssistantIntro:
	.string "ASSISTENT: Hey! Ich habe die Starter-Kapseln\n"
	.string "aller Regionen von Gen 1 bis Gen 9 sortiert!$"

LittlerootTown_ProfessorBirchsLab_Text_WhichGroup:
	.string "Welchen Block an Generationen moechtest\ndu dir ansehen?$"

LittlerootTown_ProfessorBirchsLab_Text_WhichGeneration:
	.string "Waehle eine Generation aus:$"

LittlerootTown_ProfessorBirchsLab_Text_KantoSelected:  .string "Du waehlst aus: Bisasam, Glumanda, Schiggy!$"
LittlerootTown_ProfessorBirchsLab_Text_JohtoSelected:  .string "Du waehlst aus: Endivie, Feurigel, Karnimani!$"
LittlerootTown_ProfessorBirchsLab_Text_HoennSelected:  .string "Du waehlst aus: Geckarbor, Flemmli, Hydropi!$"
LittlerootTown_ProfessorBirchsLab_Text_SinnohSelected: .string "Du waehlst aus: Chelast, Panflam, Plinfa!$"
LittlerootTown_ProfessorBirchsLab_Text_UnovaSelected:  .string "Du waehlst aus: Serpifeu, Floink, Ottaro!$"
LittlerootTown_ProfessorBirchsLab_Text_KalosSelected:  .string "Du waehlst aus: Igamaro, Fynx, Froxy!$"
LittlerootTown_ProfessorBirchsLab_Text_AlolaSelected:  .string "Du waehlst aus: Bauz, Flamiau, Robball!$"
LittlerootTown_ProfessorBirchsLab_Text_GalarSelected:  .string "Du waehlst aus: Chimpep, Hopplo, Memmeon!$"
LittlerootTown_ProfessorBirchsLab_Text_PaldeaSelected: .string "Du waehlst aus: Felori, Krokel, Kwaks!$"

LittlerootTown_ProfessorBirchsLab_Text_AssistantGoodLuck:
	.string "ASSISTENT: Lass dir ruhig Zeit bei\n"
	.string "deiner wichtigen Entscheidung!$"


@ --- ERZWUNGENE SYSTEM-PLATZHALTER GEGEN UNDEFINED REFERENCE ---
Common_EventScript_SaveGame::
LittlerootTown_ProfessorBirchsLab_EventScript_Rival::
LittlerootTown_ProfessorBirchsLab_EventScript_Cyndaquil::
LittlerootTown_ProfessorBirchsLab_EventScript_Totodile::
LittlerootTown_ProfessorBirchsLab_EventScript_Chikorita::
LittlerootTown_ProfessorBirchsLab_EventScript_Machine::
LittlerootTown_ProfessorBirchsLab_EventScript_Book::
LittlerootTown_ProfessorBirchsLab_EventScript_Bookshelf::
LittlerootTown_ProfessorBirchsLab_EventScript_PC::
	lockall
	msgbox LittlerootTown_ProfessorBirchsLab_Text_DefaultInteraction, MSGBOX_DEFAULT
	releaseall
	end

LittlerootTown_ProfessorBirchsLab_Text_DefaultInteraction:
	.string "Das Gehaeuse summt leise vor sich hin...$"


@ --- SYSTEM-PLATZHALTER GEGEN UNDEFINED REFERENCE ---
LittlerootTown_ProfessorBirchsLab_EventScript_ScottAboardSSTidalCall::
EventScript_RegisterProfBirch::
	return

LittlerootTown_Text_BirchSomethingToShowYouAtLab::
	.string "BIRCH: Ich habe etwas Neues im Labor für dich!$"

LittlerootTown_Text_ProfBirchsHouse::
	.string "PROFS. BIRCHs HAUS$"

LittlerootTown_ProfessorBirchsLab_Text_KantoSelected::
	.string "Du waehlst aus: Bisasam, Glumanda, Schiggy!$"
LittlerootTown_ProfessorBirchsLab_Text_JohtoSelected::
	.string "Du waehlst aus: Endivie, Feurigel, Karnimani!$"
LittlerootTown_ProfessorBirchsLab_Text_HoennSelected::
	.string "Du waehlst aus: Geckarbor, Flemmli, Hydropi!$"
LittlerootTown_ProfessorBirchsLab_Text_SinnohSelected::
	.string "Du waehlst aus: Chelast, Panflam, Plinfa!$"
LittlerootTown_ProfessorBirchsLab_Text_UnovaSelected::
	.string "Du waehlst aus: Serpifeu, Floink, Ottaro!$"
LittlerootTown_ProfessorBirchsLab_Text_KalosSelected::
	.string "Du waehlst aus: Igamaro, Fynx, Froxy!$"
LittlerootTown_ProfessorBirchsLab_Text_AlolaSelected::
	.string "Du waehlst aus: Bauz, Flamiau, Robball!$"
LittlerootTown_ProfessorBirchsLab_Text_GalarSelected::
	.string "Du waehlst aus: Chimpep, Hopplo, Memmeon!$"
LittlerootTown_ProfessorBirchsLab_Text_PaldeaSelected::
	.string "Du waehlst aus: Felori, Krokel, Kwaks!$"
