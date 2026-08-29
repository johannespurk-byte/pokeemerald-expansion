LittlerootTown_MapScripts::
.byte 0

PlayersHouse_2F_EventScript_BlockStairsUntilClockIsSet::
end

LittlerootTown_EventScript_Twin::
lock
faceplayer
loadword 0, LittlerootTown_Text_Twin
callstd 2
release
end

LittlerootTown_EventScript_FatMan::
lock
faceplayer
loadword 0, LittlerootTown_Text_FatMan
callstd 2
release
end

LittlerootTown_EventScript_Boy::
lock
faceplayer
loadword 0, LittlerootTown_Text_Boy
callstd 2
release
end

LittlerootTown_EventScript_Mom::
lock
faceplayer
loadword 0, LittlerootTown_Text_Mom
callstd 2
release
end

LittlerootTown_EventScript_StyleChanger::
lock
faceplayer
loadword 0, LittlerootTown_Text_StyleChanger
callstd 2
release
end

LittlerootTown_EventScript_Healer::
	lock
	faceplayer
	special HealPlayerParty
	
	@ Wir umgehen das automatische Makro und sprechen den NPC-Kanal 2 direkt an:
	loadword 0, LittlerootTown_Text_Healer
	callstd 2
	
	release
	end


LittlerootTown_EventScript_NeedPokemonTriggerLeft::
end

LittlerootTown_EventScript_NeedPokemonTriggerRight::
end

LittlerootTown_EventScript_GoSaveBirchTrigger::
end

LittlerootTown_EventScript_GiveRunningShoesTrigger0::
end

LittlerootTown_EventScript_GiveRunningShoesTrigger1::
end

LittlerootTown_EventScript_GiveRunningShoesTrigger2::
end

LittlerootTown_EventScript_GiveRunningShoesTrigger3::
end

LittlerootTown_EventScript_GiveRunningShoesTrigger4::
end

LittlerootTown_EventScript_GiveRunningShoesTrigger5::
end

LittlerootTown_EventScript_TownSign::
loadword 0, LittlerootTown_Text_TownSign
callstd 3
end

LittlerootTown_EventScript_BirchsLabSign::
loadword 0, LittlerootTown_Text_BirchsLabSign
callstd 3
end

LittlerootTown_EventScript_BrendansHouseSign::
loadword 0, LittlerootTown_Text_BrendansHouseSign
callstd 3
end

LittlerootTown_EventScript_MaysHouseSign::
loadword 0, LittlerootTown_Text_MaysHouseSign
callstd 3
end

LittlerootTown_Text_BirchSomethingToShowYouAtLab:
.string "Professor Birk wartet im Labor auf dich.$"

LittlerootTown_Text_Twin:
.string "Schau dir unsere neue Stadt an! Ist sie\nnicht viel schoener geworden?$"

LittlerootTown_Text_FatMan:
.string "Technologie ist einfach unglaublich!\nMan kann alles in Porymap umbauen!$"

LittlerootTown_Text_Boy:
.string "Ich mag den neuen grossen Platz hier in\nWurzelheim.$"

LittlerootTown_Text_Mom:
.string "Hallo mein Schatz! Viel Spass beim Erkun-\nden deiner neuen Region!$"

LittlerootTown_Text_StyleChanger:
.string "Willkommen beim Style Changer!$"

LittlerootTown_Text_Healer:
.string "Deine Pokemon wurden komplett geheilt!$"

LittlerootTown_Text_TownSign:
.string "Wurzelheim\nDas Dorf, das sich neu erfunden hat.$"

LittlerootTown_Text_BirchsLabSign:
.string "Prof. Birks Pokemon-Labor$"

LittlerootTown_Text_BrendansHouseSign:
.string "Haus von Brendan$"

LittlerootTown_Text_MaysHouseSign:
.string "Haus von May$"

