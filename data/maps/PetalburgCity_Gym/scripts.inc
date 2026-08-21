PetalburgCity_Gym_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, PetalburgCity_Gym_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, PetalburgCity_Gym_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, PetalburgCity_Gym_OnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, PetalburgCity_Gym_OnFrame
	.byte 0

PetalburgCity_Gym_OnLoad:
	goto_if_eq VAR_PETALBURG_GYM_STATE, 6, PetalburgCity_Gym_EventScript_OpenUnlockedZimmer
	call_if_ge VAR_PETALBURG_GYM_STATE, 7, PetalburgCity_Gym_EventScript_UnlockHaus
	end

PetalburgCity_Gym_EventScript_OpenUnlockedZimmer::
	setvar VAR_0x8005, 1
	call PetalburgCity_Gym_EventScript_OpenGymEntranceDoors
	call_if_defeated TRAINER_RANDALL, PetalburgCity_Gym_EventScript_OpenKueche
	call_if_defeated TRAINER_MARY, PetalburgCity_Gym_EventScript_OpenWohnzimmer
	call_if_defeated TRAINER_PARKER, PetalburgCity_Gym_EventScript_OpenBadezimmer
	call_if_defeated TRAINER_ALEXIA, PetalburgCity_Gym_EventScript_OpenSchlafzimmer
	call_if_defeated TRAINER_GEORGE, PetalburgCity_Gym_EventScript_OpenKeller
	call_if_defeated TRAINER_JODY, PetalburgCity_Gym_EventScript_OpenGarten
	call_if_defeated TRAINER_BERKE, PetalburgCity_Gym_EventScript_OpenDachboden
	end

PetalburgCity_Gym_EventScript_UnlockHaus::
	setvar VAR_0x8005, 1
	call PetalburgCity_Gym_EventScript_OpenGymEntranceDoors
	call PetalburgCity_Gym_EventScript_OpenKueche
	call PetalburgCity_Gym_EventScript_OpenWohnzimmer
	call PetalburgCity_Gym_EventScript_OpenBadezimmer
	call PetalburgCity_Gym_EventScript_OpenSchlafzimmer
	call PetalburgCity_Gym_EventScript_OpenKeller
	call PetalburgCity_Gym_EventScript_OpenGarten
	call PetalburgCity_Gym_EventScript_OpenDachboden
	return

PetalburgCity_Gym_EventScript_OpenKueche:
	call PetalburgCity_Gym_EventScript_OpenSpeedRoomDoors
	return

PetalburgCity_Gym_EventScript_OpenWohnzimmer:
	call PetalburgCity_Gym_EventScript_OpenAccuracyRoomDoors
	return

PetalburgCity_Gym_EventScript_OpenBadezimmer:
	call PetalburgCity_Gym_EventScript_OpenConfusionRoomDoors
	return

PetalburgCity_Gym_EventScript_OpenSchlafzimmer:
	call PetalburgCity_Gym_EventScript_OpenDefenseRoomDoors
	return

PetalburgCity_Gym_EventScript_OpenKeller:
	call PetalburgCity_Gym_EventScript_OpenRecoveryRoomDoors
	return

PetalburgCity_Gym_EventScript_OpenGarten:
	call PetalburgCity_Gym_EventScript_OpenStrengthRoomDoors
	return

PetalburgCity_Gym_EventScript_OpenDachboden:
	call PetalburgCity_Gym_EventScript_OpenOHKORoomDoors
	return

PetalburgCity_Gym_OnTransition:
	call_if_eq VAR_PETALBURG_GYM_STATE, 1, PetalburgCity_Gym_EventScript_MoveWallyToEntrance
	call_if_lt VAR_PETALBURG_GYM_STATE, 6, PetalburgCity_Gym_EventScript_MoveNormanToEntrance
	end

PetalburgCity_Gym_EventScript_MoveWallyToEntrance::
	setobjectxyperm LOCALID_PETALBURG_GYM_WALLY, 5, 108
	return

PetalburgCity_Gym_EventScript_MoveNormanToEntrance::
	setobjectxyperm LOCALID_PETALBURG_GYM_NORMAN, 4, 107
	return

PetalburgCity_Gym_OnWarp:
	map_script_2 VAR_PETALBURG_GYM_STATE, 1, PetalburgCity_Gym_EventScript_TurnPlayerNorth
	.2byte 0

PetalburgCity_Gym_EventScript_TurnPlayerNorth::
	turnobject LOCALID_PLAYER, DIR_NORTH
	end

PetalburgCity_Gym_OnFrame:
	map_script_2 VAR_PETALBURG_GYM_STATE, 1, PetalburgCity_Gym_EventScript_ReturnFromWallyTutorial
	.2byte 0

PetalburgCity_Gym_EventScript_ReturnFromWallyTutorial::
	lockall
	msgbox PetalburgCity_Gym_Text_HausvaterSoDidItWorkOut, MSGBOX_DEFAULT
	msgbox PetalburgCity_Gym_Text_WallyThankYouBye, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	applymovement LOCALID_PETALBURG_GYM_WALLY, PetalburgCity_Gym_Movement_WallyExitGym
	waitmovement 0
	playse SE_EXIT
	removeobject LOCALID_PETALBURG_GYM_WALLY
	setflag FLAG_HIDE_PETALBURG_CITY_WALLY
	delay 30
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	msgbox PetalburgCity_Gym_Text_HausvaterGoCollectBadges, MSGBOX_DEFAULT
	setvar VAR_PETALBURG_GYM_STATE, 2
	releaseall
	end

PetalburgCity_Gym_Movement_WallyExitGym:
	walk_down
	walk_down
	walk_down
	delay_16
	step_end

PetalburgCity_Gym_EventScript_Norman::
	lock
	faceplayer
	switch VAR_PETALBURG_GYM_STATE
	case 2, PetalburgCity_Gym_EventScript_NormanNoBadges
	case 3, PetalburgCity_Gym_EventScript_NormanNoBadges
	case 4, PetalburgCity_Gym_EventScript_NormanNoBadges
	case 5, PetalburgCity_Gym_EventScript_NormanNoBadges
	case 6, PetalburgCity_Gym_EventScript_NormanBattle
	case 7, PetalburgCity_Gym_EventScript_NormanPostBattle
	end

PetalburgCity_Gym_EventScript_NormanNoBadges:
	msgbox PetalburgCity_Gym_Text_DadNoBadges, MSGBOX_NPC
	end

PetalburgCity_Gym_EventScript_NormanPostBattle:
	msgbox PetalburgCity_Gym_Text_MomPostBattle, MSGBOX_NPC
	end

PetalburgCity_Gym_EventScript_NormanBattle::
	msgbox PetalburgCity_Gym_Text_MomWelcome, MSGBOX_DEFAULT
	closemessage
	playse SE_DOOR
	msgbox PetalburgCity_Gym_Text_MistyInterrupts, MSGBOX_DEFAULT
	closemessage
	
	@ Der Doppelkampf mit korrekten String-Referenzen
	trainerbattle_two_trainers TRAINER_NORMAN_1, PetalburgCity_Gym_Text_MomDefeat, TRAINER_MISTY, PetalburgCity_Gym_Text_MistyDefeat
	
	setflag FLAG_FINISHED_HAUS_BATTLE
	msgbox PetalburgCity_Gym_Text_MomRewardSpeech, MSGBOX_DEFAULT
	giveitem ITEM_TM01, 1
	goto_if_eq VAR_RESULT, FALSE, PetalburgCity_Gym_EventScript_BagIsFull
	msgbox PetalburgCity_Gym_Text_MomPostBattle, MSGBOX_DEFAULT
	setvar VAR_PETALBURG_GYM_STATE, 7
	release
	end

PetalburgCity_Gym_EventScript_BagIsFull:
	msgbox PetalburgCity_Gym_Text_BagIsFull, MSGBOX_DEFAULT
	release
	end

PetalburgCity_Gym_EventScript_Randall::
	trainerbattle_single TRAINER_RANDALL, PetalburgCity_Gym_Text_RandallIntro, PetalburgCity_Gym_Text_RandallDefeat, PetalburgCity_Gym_EventScript_RandallPost
	msgbox PetalburgCity_Gym_Text_RandallPostBattle, MSGBOX_AUTOCLOSE
	end
PetalburgCity_Gym_EventScript_RandallPost::
	setvar VAR_0x8005, 1
	call PetalburgCity_Gym_EventScript_OpenKueche
	special DrawWholeMapView
	end

PetalburgCity_Gym_EventScript_Mary::
	trainerbattle_single TRAINER_MARY, PetalburgCity_Gym_Text_MaryIntro, PetalburgCity_Gym_Text_MaryDefeat, PetalburgCity_Gym_EventScript_MaryPost
	msgbox PetalburgCity_Gym_Text_MaryPostBattle, MSGBOX_AUTOCLOSE
	end
PetalburgCity_Gym_EventScript_MaryPost::
	setvar VAR_0x8005, 1
	call PetalburgCity_Gym_EventScript_OpenWohnzimmer
	special DrawWholeMapView
	end

PetalburgCity_Gym_EventScript_Parker::
	trainerbattle_single TRAINER_PARKER, PetalburgCity_Gym_Text_ParkerIntro, PetalburgCity_Gym_Text_ParkerDefeat, PetalburgCity_Gym_EventScript_ParkerPost
	msgbox PetalburgCity_Gym_Text_ParkerPostBattle, MSGBOX_AUTOCLOSE
	end
PetalburgCity_Gym_EventScript_ParkerPost::
	setvar VAR_0x8005, 1
	call PetalburgCity_Gym_EventScript_OpenBadezimmer
	special DrawWholeMapView
	end

PetalburgCity_Gym_EventScript_Alexia::
	trainerbattle_single TRAINER_ALEXIA, PetalburgCity_Gym_Text_AlexiaIntro, PetalburgCity_Gym_Text_AlexiaDefeat, PetalburgCity_Gym_EventScript_AlexiaPost
	msgbox PetalburgCity_Gym_Text_AlexiaPostBattle, MSGBOX_AUTOCLOSE
	end
PetalburgCity_Gym_EventScript_AlexiaPost::
	setvar VAR_0x8005, 1
	call PetalburgCity_Gym_EventScript_OpenSchlafzimmer
	special DrawWholeMapView
	end

PetalburgCity_Gym_EventScript_George::
	trainerbattle_single TRAINER_GEORGE, PetalburgCity_Gym_Text_GeorgeIntro, PetalburgCity_Gym_Text_GeorgeDefeat, PetalburgCity_Gym_EventScript_GeorgePost
	msgbox PetalburgCity_Gym_Text_GeorgePostBattle, MSGBOX_AUTOCLOSE
	end
PetalburgCity_Gym_EventScript_GeorgePost::
	setvar VAR_0x8005, 1
	call PetalburgCity_Gym_EventScript_OpenKeller
	special DrawWholeMapView
	end

PetalburgCity_Gym_EventScript_Jody::
	trainerbattle_single TRAINER_JODY, PetalburgCity_Gym_Text_JodyIntro, PetalburgCity_Gym_Text_JodyDefeat, PetalburgCity_Gym_EventScript_JodyPost
	msgbox PetalburgCity_Gym_Text_JodyPostBattle, MSGBOX_AUTOCLOSE
	end
PetalburgCity_Gym_EventScript_JodyPost::
	setvar VAR_0x8005, 1
	call PetalburgCity_Gym_EventScript_OpenGarten
	special DrawWholeMapView
	end

PetalburgCity_Gym_EventScript_Berke::
	trainerbattle_single TRAINER_BERKE, PetalburgCity_Gym_Text_BerkeIntro, PetalburgCity_Gym_Text_BerkeDefeat, PetalburgCity_Gym_EventScript_BerkePost
	msgbox PetalburgCity_Gym_Text_BerkePostBattle, MSGBOX_AUTOCLOSE
	end
PetalburgCity_Gym_EventScript_BerkePost::
	setvar VAR_0x8005, 1
	call PetalburgCity_Gym_EventScript_OpenDachboden
	special DrawWholeMapView
	end

PetalburgCity_Gym_EventScript_GeheimWarpTrigger::
	lockall
	goto_if_set FLAG_FINISHED_HAUS_BATTLE, PetalburgCity_Gym_EventScript_DoGeheimWarp
	msgbox PetalburgCity_Gym_Text_WarpIsLocked, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, PetalburgCity_Gym_Movement_PushPlayerBack
	waitmovement 0
	releaseall
	end

PetalburgCity_Gym_EventScript_DoGeheimWarp:
	warp MAP_LITTLEROOT_TOWN, 14, 11
	waitstate
	end

PetalburgCity_Gym_Movement_PushPlayerBack:
	walk_down
	step_end

PetalburgCity_Gym_Text_WarpIsLocked:
	.string "Hier steht Text.\n"
	.string "Erst wenn du bereit bist...$"

PetalburgCity_Gym_Text_HausvaterSoDidItWorkOut:
	.string "Und, hat alles geklappt mit deinem ersten POKéMON?$"

PetalburgCity_Gym_Text_WallyThankYouBye:
	.string "Ja, vielen Dank! Ich mache mich auf den Weg!$"

PetalburgCity_Gym_Text_HausvaterGoCollectBadges:
	.string "Fordere die Zimmerbewohner heraus!$"

PetalburgCity_Gym_Text_DadNoBadges:
	.string "MOM: Besiege erst alle Mitbewohner!$"

PetalburgCity_Gym_Text_MomWelcome:
	.string "MOM: Testen wir dein Team! Ich kaempfe aber nicht allein...$"

PetalburgCity_Gym_Text_MistyInterrupts:
	.string "MISTY: Huhu! Ich helfe dir, Mom!$"

PetalburgCity_Gym_Text_MomDefeat:
	.string "MOM: Unglaublich... du hast uns besiegt!$"

PetalburgCity_Gym_Text_MistyDefeat:
	.string "MISTY: Oh nein! Mein Wasser ist verdampft...$"

PetalburgCity_Gym_Text_MomRewardSpeech:
@ =========================================================================
@ TEXTE FÜR PETALBURG CITY ARENA
@ =========================================================================

PetalburgCity_Gym_Text_MomRewardSpeech::
	.string "MOM: Nimm stattdessen diese TM von\n"
	.string "mir!$"

PetalburgCity_Gym_Text_MomPostBattle::
	.string "MOM: Zeig allen, was du gelernt\n"
	.string "hast!$"

PetalburgCity_Gym_Text_BagIsFull::
	.string "MOM: Dein Rucksack ist randvoll!$"

PetalburgCity_Gym_Text_RandallIntro::
	.string "Wer die Kueche betritt, muss\n"
	.string "kaempfen!$"

PetalburgCity_Gym_Text_RandallDefeat::
	.string "Mein Essen brennt an...$"

PetalburgCity_Gym_Text_RandallPostBattle::
	.string "Du darfst weiter ins Wohnzimmer.$"

PetalburgCity_Gym_Text_MaryIntro::
	.string "Stoer mich nicht beim Faulenzen!$"

PetalburgCity_Gym_Text_MaryDefeat::
	.string "Jetzt habe ich das Finale verpasst!$"

PetalburgCity_Gym_Text_MaryPostBattle::
	.string "Geh ruhig weiter.$"

PetalburgCity_Gym_Text_ParkerIntro::
	.string "Besetzt! Hier kommt keiner ungefragt\n"
	.string "ins Bad!$"

PetalburgCity_Gym_Text_ParkerDefeat::
	.string "Die Zahnpasta ist im Auge gelandet!$"

PetalburgCity_Gym_Text_ParkerPostBattle::
	.string "Das Badezimmer steht dir jetzt frei.$"

PetalburgCity_Gym_Text_AlexiaIntro::
	.string "Ich wollte gerade einen\n"
	.string "Mittagsschlaf machen!$"

PetalburgCity_Gym_Text_AlexiaDefeat::
	.string "Jetzt bin ich hellwach...$"

PetalburgCity_Gym_Text_AlexiaPostBattle::
	.string "Suchst du den Keller?$"

PetalburgCity_Gym_Text_GeorgeIntro::
	.string "Ich repariere hier unten die\n"
	.string "Heizung.$"

PetalburgCity_Gym_Text_GeorgeDefeat::
	.string "Die Rohre brechen zusammen!$"

PetalburgCity_Gym_Text_GeorgePostBattle::
	.string "Der Weg zum Garten ist\n"
	.string "freigeschaltet.$"

PetalburgCity_Gym_Text_JodyIntro::
	.string "Ich giesse die Pflanzen im Garten.$"

PetalburgCity_Gym_Text_JodyDefeat::
	.string "Mein schoener Rasen ist\n"
	.string "ueberschwemmt!$"

PetalburgCity_Gym_Text_JodyPostBattle::
	.string "Geh hoch auf den Dachboden.$"

PetalburgCity_Gym_Text_BerkeIntro::
	.string "Hier oben auf dem Dachboden wird es\n"
	.string "heiss!$"

PetalburgCity_Gym_Text_BerkeDefeat::
	.string "Der Dachboden brennt lichterloh!$"

PetalburgCity_Gym_Text_BerkePostBattle::
	.string "Wahnsinn! Mom wartet in der\n"
	.string "Haupthalle!$"



PetalburgCity_Gym_EventScript_OpenGymEntranceDoors::
PetalburgCity_Gym_EventScript_OpenSpeedRoomDoors::
PetalburgCity_Gym_EventScript_OpenAccuracyRoomDoors::
PetalburgCity_Gym_EventScript_OpenConfusionRoomDoors::
PetalburgCity_Gym_EventScript_OpenDefenseRoomDoors::
PetalburgCity_Gym_EventScript_OpenRecoveryRoomDoors::
PetalburgCity_Gym_EventScript_OpenStrengthRoomDoors::
PetalburgCity_Gym_EventScript_OpenOHKORoomDoors::
PetalburgCity_Gym_EventScript_GymGuide::
PetalburgCity_Gym_EventScript_SpeedRoomDoor::
PetalburgCity_Gym_EventScript_AccuracyRoomDoor::
PetalburgCity_Gym_EventScript_ConfusionRoomDoor::
PetalburgCity_Gym_EventScript_LeftDefenseRoomDoor::
PetalburgCity_Gym_EventScript_RightDefenseRoomDoor::
PetalburgCity_Gym_EventScript_RecoveryRoomDoor::
PetalburgCity_Gym_EventScript_LeftStrengthRoomDoor::
PetalburgCity_Gym_EventScript_RightStrengthRoomDoor::
PetalburgCity_Gym_EventScript_LeftOHKORoomDoor::
PetalburgCity_Gym_EventScript_RightOHKORoomDoor::
PetalburgCity_Gym_EventScript_LeftGymLeadersRoomDoor::
PetalburgCity_Gym_EventScript_RightGymLeadersRoomDoor::
PetalburgCity_Gym_EventScript_LeftGymStatue::
PetalburgCity_Gym_EventScript_RightGymStatue::
NULLEventScript_CutTree::
	return
