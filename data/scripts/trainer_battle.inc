@ --- GLOBALE ENGINE ENTRY POINTS FÜR BATTLE_SETUP.C ---
gTrainerBattleOpponentA::
EventScript_TryDoNormalTrainerBattle::
	lockall
	trainerbattle_single VAR_0x8000, NULL, NULL
	releaseall
	end

EventScript_TryDoDoubleTrainerBattle::
	lockall
	trainerbattle_double VAR_0x8000, NULL, NULL, NULL
	releaseall
	end

EventScript_StartTrainerApproach::
EventScript_TrainerApproach::
	lockall
	releaseall
	end

EventScript_TryDoDoubleRematchBattle::
	lockall
	releaseall
	end

EventScript_DoNoIntroTrainerBattle::
	lockall
	releaseall
	end

EventScript_TryDoRematchBattle::
	lockall
	releaseall
	end
