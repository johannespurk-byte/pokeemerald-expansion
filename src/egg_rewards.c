#include "global.h"
#include "script_pokemon_util.h"
#include "constants/trainers.h"
#include "trainer.h"
#include "pokemon.h"
#include "random.h"

// TODO: Implementiere nach Wunsch: Pool / Gewichtungen / Ausnahmen.
// Platzhalter: diese Funktion muss eine gültige Species zurückgeben.
static u16 GetRandomEggSpecies(void)
{
    // Beispiel: einfache Zufallsauswahl aus kleinem Pool — passe an.
    static const u16 sEggPool[] = {
        SPECIES_PICHU, SPECIES_IGGLYBUFF, SPECIES_TOGEPI, SPECIES_MAREEP
    };
    return sEggPool[Random() % ARRAY_COUNT(sEggPool)];
}

// Verteile ein zufälliges Ei NUR, wenn es kein Boss/Leader/etc. war.
// Rückgabe: TRUE = Ei erfolgreich übergeben, FALSE = kein Ei gegeben (z. B. FullParty / geblockt)
bool8 TryGiveNormalTrainerEgg(u16 trainerId)
{
    u8 trainerClass;

    if (trainerId >= TRAINER_FRONTIER_BRAIN) // safety: out-of-range trainerId check (anpassen falls nötig)
        return FALSE;

    // Versuche Trainerklasse zu ermitteln. Falls gTrainers verfügbar ist, nutze sie;
    // andernfalls musst du hier eine passende API verwenden.
#if defined(gTrainers)
    trainerClass = gTrainers[trainerId].trainerClass;
#else
    // Fallback: wenn keine direkte Tabelle verfügbar ist, versuche alternative Methode
    // oder return FALSE, bis eine korrekte Abfrage implementiert ist.
    return FALSE;
#endif

    // Blockieren-Liste: wichtige Trainergruppen erhalten kein Ei
    if (trainerClass == TRAINER_CLASS_LEADER
     || trainerClass == TRAINER_CLASS_RIVAL
     || trainerClass == TRAINER_CLASS_CHAMPION
     || trainerClass == TRAINER_CLASS_ELITE_FOUR
     || trainerClass == TRAINER_CLASS_MAGMA_LEADER
     || trainerClass == TRAINER_CLASS_AQUA_LEADER
     || trainerClass == TRAINER_CLASS_MAGMA_ADMIN
     || trainerClass == TRAINER_CLASS_AQUA_ADMIN
     || trainerClass == TRAINER_CLASS_TEAM_ROCKET_FRLG)
    {
        return FALSE;
    }

    // Prüfen, ob Party noch Platz hat (gPartiesCount[B_TRAINER_PLAYER] ist die aktuelle Player-Party-Größe)
    if (gPartiesCount[B_TRAINER_PLAYER] >= PARTY_SIZE)
        return FALSE;

    // Bestimme Species und übergebe das Ei (ScriptGiveEgg kümmert sich um Party/PC-Logik)
    u16 species = GetRandomEggSpecies();
    if (species == SPECIES_NONE)
        return FALSE;

    if (ScriptGiveEgg(species))
        return TRUE;

    return FALSE;
}