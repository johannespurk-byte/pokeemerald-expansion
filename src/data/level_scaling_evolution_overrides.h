void BeginEvolutionScene(struct Pokemon *mon, enum Species postEvoSpecies, bool32 canStopEvo, u8 partyId)
{
    u16 newSpecies = (u16)postEvoSpecies;

    // 1) Setze die Species in den Box-Daten (persistente Daten)
    SetBoxMonData(&mon->box, MON_DATA_SPECIES, &newSpecies);

    // 2) Kopiere Box -> aktives struct Pokemon und berechne Stats neu
    BoxMonToMon(&mon->box, mon);
    CalculateMonStats(mon);

    // 3) KP auf das neue Maximum setzen
    u16 maxHp = GetMonData(mon, MON_DATA_MAX_HP);
    SetMonData(mon, MON_DATA_HP, &maxHp);

    // 4) Spiele den Cry des neuen Species
    PlayCry_NormalNoDucking(newSpecies, 0, CRY_VOLUME_RS, CRY_VOLUME_RS);

    // 5) UI aktualisieren, falls Party‑Menu offen (partyId = Slot 0..5)
    if (partyId < PARTY_SIZE)
    {
        // Aktualisiert Icon / Held‑Item / Statusanzeige ähnlich wie RareCandy-Flow
        UpdateMonDisplayInfoAfterRareCandy(partyId, mon);
    }

    // 6) Keine Scene starten – Funktion endet hier (headless evolution).
    // Hinweis: Falls du Pokedex, Mail, Evolutions‑Tracker, Itementfernung, gelehrte Moves etc.
    // brauchst, füge die entsprechenden Aufrufe hier ein.
    return;
}
