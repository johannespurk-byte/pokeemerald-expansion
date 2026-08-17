PetalburgCity_Mart_MapScripts::
	.byte 0
	
PetalburgCity_Mart_EventScript_Clerk::
	lock
	faceplayer
	message PetalburgCity_Mart_Text_Welcome
	waitmessage
	goto_if_set FLAG_BADGE01_GET, PetalburgCity_Mart_EventScript_ExpandedItems
	pokemart PetalburgCity_Mart_Pokemart_Basic
	msgbox PetalburgCity_Mart_Text_Goodbye, MSGBOX_DEFAULT
	release
	end

PetalburgCity_Mart_EventScript_ExpandedItems::
	pokemart PetalburgCity_Mart_Pokemart_Expanded
	msgbox PetalburgCity_Mart_Text_Goodbye, MSGBOX_DEFAULT
	release
	end

	.align 2
PetalburgCity_Mart_Pokemart_Basic:
	.2byte ITEM_NORMAL_GEM
	.2byte ITEM_FIRE_GEM
	.2byte ITEM_WATER_GEM
	.2byte ITEM_TM01                @ FIX: Nutzt die native TM-ID deiner Expansion
	.2byte ITEM_TM02                @ FIX: Nutzt die native TM-ID deiner Expansion
	.2byte ITEM_NONE
	release
	end
	.align 2
PetalburgCity_Mart_Pokemart_Expanded:
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_TM01                @ FIX: Synchronisiert mit der Basic-Liste
	.2byte ITEM_GRASS_GEM
	.2byte ITEM_ELECTRIC_GEM
	.2byte ITEM_FIGHTING_GEM
	.2byte ITEM_FLYING_GEM
	.2byte ITEM_POISON_GEM
	.2byte ITEM_GROUND_GEM
	.2byte ITEM_ROCK_GEM
	.2byte ITEM_BUG_GEM
	.2byte ITEM_GHOST_GEM
	.2byte ITEM_STEEL_GEM
	.2byte ITEM_DRAGON_GEM
	.2byte ITEM_DARK_GEM
	.2byte ITEM_FAIRY_GEM
	.2byte ITEM_TM88
	.2byte ITEM_TM04
	.2byte ITEM_TM05
	.2byte ITEM_TM02
	.2byte ITEM_TM84
	.2byte ITEM_TM54
	.2byte ITEM_TM92	
	.2byte ITEM_X_SPEED
	.2byte ITEM_X_ATTACK
	.2byte ITEM_X_DEFENSE
	.2byte ITEM_ORANGE_MAIL
	.2byte ITEM_NONE

	pokemartlistend

PetalburgCity_Mart_EventScript_Woman::
	msgbox PetalburgCity_Mart_Text_WeakWillGrowStronger, MSGBOX_NPC
	end

PetalburgCity_Mart_EventScript_Boy::
	msgbox PetalburgCity_Mart_Text_RepelIsUseful, MSGBOX_NPC
	end

PetalburgCity_Mart_EventScript_Man::
	msgbox PetalburgCity_Mart_Text_TakeSomeAntidotesWithYou, MSGBOX_NPC
	end

PetalburgCity_Mart_Text_WeakWillGrowStronger:
	.string "Even if a POKéMON is weak now,\n"
	.string "it will grow stronger.\p"
	.string "The most important thing is love!\n"
	.string "Love for your POKéMON!$"

PetalburgCity_Mart_Text_RepelIsUseful:
	.string "Do you use REPEL?\n"
	.string "It keeps POKéMON away, so it's\l"
	.string "useful when you're in a hurry.$"

PetalburgCity_Mart_Text_TakeSomeAntidotesWithYou:
	.string "Do you have any ANTIDOTES with\n"
	.string "you?\p"
	.string "If you walk around with a poisoned\n"
	.string "POKéMON, it will lose HP until it faints.\l"
	.string "Take some ANTIDOTES with you.$"

PetalburgCity_Mart_Text_Welcome:
	.string "Hi! How may I serve you today?$"

PetalburgCity_Mart_Text_Goodbye:
	.string "Please come again!$"