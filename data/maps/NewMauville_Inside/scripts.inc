NewMauville_Inside_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, NewMauville_Inside_OnResume
	map_script MAP_SCRIPT_ON_TRANSITION, NewMauville_Inside_OnTransition
	.byte 0

NewMauville_Inside_OnResume:
	call_if_set FLAG_SYS_CTRL_OBJ_DELETE, NewMauville_Inside_EventScript_TryRemoveVoltorb
	end

NewMauville_Inside_EventScript_TryRemoveVoltorb::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, Common_EventScript_NopReturn
	removeobject VAR_LAST_TALKED
	return

NewMauville_Inside_OnTransition:
	end

NewMauville_Inside_EventScript_BlueButton::
	lockall
	playse SE_PIN
	special DrawWholeMapView
	releaseall
	end

NewMauville_Inside_EventScript_GreenButton::
	lockall
	playse SE_PIN
	special DrawWholeMapView
	releaseall
	end

NewMauville_Inside_EventScript_SetBarrierStateBlueButton::
	return

NewMauville_Inside_EventScript_SetBarrierStateGreenButton::
	return

NewMauville_Inside_EventScript_RedButton::
	lockall
	setvar VAR_NEW_MAUVILLE_STATE, 2
	releaseall
	end

NewMauville_Inside_EventScript_SetGeneratorOffMetatiles::
	return

NewMauville_Inside_EventScript_Generator::
	lockall
	releaseall
	end

NewMauville_Inside_EventScript_GeneratorOff::
	releaseall
	end

NewMauville_Inside_EventScript_Voltorb1::
	lock
	faceplayer
	setwildbattle SPECIES_VOLTORB, 25
	dowildbattle
	release
	end

NewMauville_Inside_EventScript_Voltorb2::
	lock
	faceplayer
	setwildbattle SPECIES_VOLTORB, 25
	dowildbattle
	release
	end

NewMauville_Inside_EventScript_Voltorb3::
	lock
	faceplayer
	setwildbattle SPECIES_ELECTRODE, 30
	dowildbattle
	release
	end

NewMauville_Inside_Text_SteppedOnSwitchGeneratorStopped:
	.string "PLAYER stepped on the switch.\nThe generator appears to have stopped.$"

NewMauville_Inside_Text_GeneratorRadiatingHeat:
	.string "The generator is radiating heat.\nIt is best not to touch it.$"

NewMauville_Inside_Text_GeneratorQuietedDown:
	.string "The generator quieted down.$"
