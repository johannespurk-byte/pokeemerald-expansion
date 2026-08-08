#include "global.h"
#include "party_menu.h"
#include "pokemon_icon.h"
#include "pokemon.h"
#include "sound.h"
#include "sprite.h"

void EvolutionWithoutScene(struct Pokemon *mon, u16 targetSpecies, s8 partySlot)
{
    // Aktualisiere die Box-Daten (sichert persistente Speicherung)
    SetBoxMonData(&mon->box, MON_DATA_SPECIES, &targetSpecies);

    // Kopiere die Box-Daten zurück in die aktive struct Pokemon und berechne Stats neu
    BoxMonToMon(&mon->box, mon);
    CalculateMonStats(mon);

    // Setze KP auf Maximum
    u16 maxHp = GetMonData(mon, MON_DATA_MAX_HP);
    SetMonData(mon, MON_DATA_HP, &maxHp);

    // Spiele den Cry des neuen Species
    PlayCry_NormalNoDucking(targetSpecies, 0, CRY_VOLUME_RS, CRY_VOLUME_RS);

    // Optional: Falls das Party‑Menu offen ist und du das Icon erneuern willst,
    // kannst du hier die Sprite aktualisieren (partySlot = 0..5, oder -1 zum Überspringen).
    if (partySlot >= 0)
    {
        extern struct PartyMenuBox *sPartyMenuBoxes; // falls vorhanden
        if (sPartyMenuBoxes != NULL)
        {
            // u8 spriteId = sPartyMenuBoxes[partySlot].monSpriteId; // <- Auskommentiert!
            // if (spriteId != SPRITE_NONE)                          // <- Auskommentiert!
            // {                                                     // <- Auskommentiert!
            //     FreeAndDestroyMonIconSprite(&gSprites[spriteId]); // <- Auskommentiert!
            // }                                                     // <- Auskommentiert!

            // CreatePartyMonIconSpriteParameterized(targetSpecies, GetMonData(mon, MON_DATA_PERSONALITY), // <- Auskommentiert!
            //                                       FALSE, &sPartyMenuBoxes[partySlot], 1);              // <- Auskommentiert!
            // Held-Item / Status-Icons ggf. ebenfalls aktualisieren (UpdatePartyMonHeldItemSprite etc.)
        }
    }

    // Hinweis: Pokedex, Mail, Evolution-Tracker, Item-Entfernung, spezielle Form-/Ability-Handling
    // und alle Events, die normalerweise die Evolutions‑Szene auslösen, müssen ggf. zusätzlich gesetzt werden.
}