OldaleTown_Mart_MapScripts::
	.byte 0

OldaleTown_Mart_EventScript_Clerk::
	lock
	faceplayer
	message OldaleTown_Mart_Welcome       @ FIX: Auf das korrekte, existierende Label umgestellt!
	waitmessage
	goto_if_unset FLAG_BADGE01_GET, OldaleTown_Mart_EventScript_Basic
	pokemart OldaleTown_Mart_Pokemart_Expanded
	msgbox OldaleTown_Mart_Goodbye, MSGBOX_DEFAULT
	release
	end

OldaleTown_Mart_EventScript_Basic:
	pokemart OldaleTown_Mart_Pokemart_Basic
	msgbox OldaleTown_Mart_Goodbye, MSGBOX_DEFAULT
	release
	end

	.align 2
OldaleTown_Mart_Pokemart_Basic:
	.2byte ITEM_LEAF_STONE
	.2byte ITEM_MOON_STONE
	.2byte ITEM_SUN_STONE
	.2byte ITEM_LINKING_CORD 
	.2byte ITEM_NONE

	.align 2
OldaleTown_Mart_Pokemart_Expanded:
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_LIFE_ORB
	.2byte ITEM_CHOICE_BAND
	.2byte ITEM_EVIOLITE
	.2byte ITEM_CHARIZARDITE_X 
	.2byte ITEM_LUCARIONITE 
	.2byte ITEM_DYNAMAX_BAND
	.2byte ITEM_WISHING_PIECE
	.2byte ITEM_NONE

OldaleTown_Mart_EventScript_Woman::
	lock
	faceplayer
	goto_if_set FLAG_ADVENTURE_STARTED, OldaleTown_Mart_EventScript_PokeBallsInStock
	msgbox OldaleTown_Mart_Text_PokeBallsAreSoldOut, MSGBOX_DEFAULT
	release
	end

OldaleTown_Mart_EventScript_PokeBallsInStock::
	msgbox OldaleTown_Mart_Text_ImGoingToBuyPokeBalls, MSGBOX_DEFAULT
	release
	end

OldaleTown_Mart_EventScript_Boy::
	msgbox OldaleTown_Mart_Text_RestoreHPWithPotion, MSGBOX_NPC
	end

OldaleTown_Mart_Text_PokeBallsAreSoldOut:
	.string "The clerk says they're all sold out.\n"
	.string "I can't buy any POKé BALLS.$"

OldaleTown_Mart_Text_ImGoingToBuyPokeBalls:
	.string "I'm going to buy a bunch of POKé BALLS\n"
	.string "and catch a bunch of POKéMON!$"

OldaleTown_Mart_Text_RestoreHPWithPotion:
	.string "If a POKéMON gets hurt and loses its HP\n"
	.string "and faints, it won't be able to battle.\p"
	.string "To prevent your POKéMON from fainting,\n"
	.string "restore its HP with a POTION.$"

OldaleTown_Mart_Welcome:
	.string "Hi! How may I serve you today?$"

OldaleTown_Mart_Goodbye:
	.string "Please come again!$"
