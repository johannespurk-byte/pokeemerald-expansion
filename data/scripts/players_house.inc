@ =========================================================================
@ RADIKAL BEREINIGTE HAUS-LOGIK (Verhindert jegliche .if-Makro-Abstuerze)
@ =========================================================================

PlayersHouse_1F_EventScript_EnterHouseMovingIn::
	lockall
	msgbox CustomHouse_Text_OurNewHomeLetsGoInside, MSGBOX_DEFAULT
	closemessage
	setvar VAR_LITTLEROOT_INTRO_STATE, 7
	releaseall
	end

PlayersHouse_1F_EventScript_PetalburgGymReportMale::
PlayersHouse_1F_EventScript_PetalburgGymReportFemale::
	lockall
	msgbox LittlerootTown_Text_BirchSomethingToShowYouAtLab, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

PlayersHouse_1F_EventScript_MomGoSeeRoom::
	lockall
	msgbox LittlerootTown_Text_SomethingIsHappeningOutThere, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

PlayersHouse_1F_EventScript_GetSSTicketAndSeeLatiTV::
	lockall
	releaseall
	end

PlayersHouse_1F_EventScript_Mom::
	lock
	faceplayer
	msgbox CustomHouse_Text_OurNewHomeLetsGoInside, MSGBOX_DEFAULT
	release
	end

PlayersHouse_1F_EventScript_Vigoroth1::
PlayersHouse_1F_EventScript_Vigoroth2::
	lock
	faceplayer
	msgbox LittlerootTown_Text_SomethingIsHappeningOutThere, MSGBOX_DEFAULT
	release
	end

LittlerootTown_BrendansHouse_2F_EventScript_WallClock::
LittlerootTown_MaysHouse_2F_EventScript_WallClock::
	lockall
	msgbox PlayersHouse_2F_Text_JustANormalClock, MSGBOX_DEFAULT
	releaseall
	end

@ --- ERZWUNGENE SYSTEM-PLATZHALTER GEGEN UNDEFINED REFERENCE ---
PlayersHouse_2F_EventScript_CheckWallClock::
PlayersHouse_2F_EventScript_SetWallClock::
LittlerootTown_MaysHouse_2F_EventScript_PC::
LittlerootTown_BrendansHouse_2F_EventScript_PC::
LittlerootTown_BrendansHouse_2F_EventScript_TurnOffPlayerPC::
LittlerootTown_MaysHouse_2F_EventScript_TurnOffPlayerPC::
PlayersHouse_2F_EventScript_Notebook::
PlayersHouse_2F_EventScript_GameCube::
EventScript_RunningShoesManual::
	lockall
	releaseall
	end

@ Fehlende Texte und Hilfs-Dummys deklarieren
PlayersHouse_2F_Text_JustANormalClock:
	.string "Es ist eine schicke Wanduhr. Sie laeuft\n"
	.string "voellig automatisch.$"

Common_Text_LookCloserAtMap:
	.string "Es ist eine detailreiche Regionskarte.$"

LittlerootTown_Text_SomethingIsHappeningOutThere:
	.string "Draussen vor der Stadt ist etwas los...$"

CustomHouse_Text_OurNewHomeLetsGoInside:
	.string "MUTTER: Lass uns reingehn!$"


