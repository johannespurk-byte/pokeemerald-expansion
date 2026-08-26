LittlerootTown_ProfessorBirchsLab_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, LittlerootTown_ProfessorBirchsLab_OnTransition
	.byte 0

LittlerootTown_ProfessorBirchsLab_OnTransition::
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

LittlerootTown_ProfessorBirchsLab_EventScript_Assistant::
	lock
	faceplayer
	
	@ Prüft, ob der Spieler die Schuhe bereits erhalten hat
	checkflag FLAG_SYS_B_DASH
	goto_if_eq VAR_RESULT, TRUE, Lab_Assistant_EventScript_AlreadyHaveShoes
	
	@ Dialog und korrekte Freischaltung der Turbotreter per Flag
	msgbox Lab_Assistant_Text_GiveShoesIntro, MSGBOX_DEFAULT
	setflag FLAG_SYS_B_DASH @ Schaltet das Rennen im Spiel systemweit frei
	msgbox Lab_Assistant_Text_ExplainShoes, MSGBOX_DEFAULT
	release
	end

Lab_Assistant_EventScript_AlreadyHaveShoes::
	msgbox Lab_Assistant_Text_HowAreShoes, MSGBOX_DEFAULT
	release
	end

@ --- TEXTE FÜR DEN ASSISTENTEN ---
Lab_Assistant_Text_GiveShoesIntro::
	.string "ASSISTENT: Ah, {PLAYER}!\n"
	.string "Prof. Birk hat mich gebeten, dir das\p"
	.string "hier für dein Abenteuer zu geben!$"

Lab_Assistant_Text_ExplainShoes::
	.string "Das sind die TURBOTRETER!\n"
	.string "Halte einfach den B-Knopf gedrückt,\p"
	.string "um unglaublich schnell zu rennen!$"

Lab_Assistant_Text_HowAreShoes::
	.string "ASSISTENT: Kommst du mit den\n"
	.string "Schuhen gut voran?$"

LittlerootTown_ProfessorBirchsLab_EventScript_Bookshelf::
	lockall
	releaseall
	end

LittlerootTown_ProfessorBirchsLab_EventScript_PC::
	lockall
	releaseall
	end

LittlerootTown_ProfessorBirchsLab_EventScript_Cyndaquil::
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_Totodile::
	release
	end

LittlerootTown_ProfessorBirchsLab_EventScript_Chikorita::
	release
	end

LittlerootTown_EventScript_TownSign::
	msgbox LittlerootTown_Text_TownSign, MSGBOX_SIGN
	end

LittlerootTown_ProfessorBirchsLab_EventScript_Rival::
	lock
	faceplayer
	release
	end

LittlerootTown_Text_BirchSomethingToShowYouAtLab::
	.string "PROF. BIRCH has something to show you\n"
	.string "at his POKéMON LAB.$"

LittlerootTown_ProfessorBirchsLab_EventScript_ScottAboardSSTidalCall::
	lockall
	releaseall
	end

LittlerootTown_ProfessorBirchsLab_Text_BirchWelcome::
	.string "PROF. BIRK: Hallo! Welcome to\n"
	.string "my POKéMON LAB!$"
