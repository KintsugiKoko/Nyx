# Nyx Core Technical Design Document

Date: 2026-05-02

Status: Work-in-progress source design

## Purpose

This TDD documents the current Unreal C++ gameplay system architecture for Nyx and the next intended technical direction. It is meant to help future implementation passes stay focused, reviewable, and honest about what exists.

## Source Scope

Primary source folder:

```text
Source/Nyx
```

Current system areas:

- Fishing: `UFishingComponent`, `UFishDataAsset`
- Starwell: `AStarwell`, `FStarwellOfferingThreshold`
- Economy: `UEconomyComponent`, `ENyxResourceType`
- Upgrades: `UUpgradeDataAsset`, `ENyxUpgradePrototypeEffect`
- Deck: `UDeckComponent`, `UCardDataAsset`
- Save/load: `UNyxSaveGame`, `UNyxSaveGameLibrary`
- Player base: `ANyxCatPlayerCharacter`
- Debug: `ANyxGameplayDebugActor`
- Validation: `UNyxGameplayValidationLibrary`
- FTUE: `UNyxFTUEComponent`
- Merchant: `ATravelingMerchant`
- Koi skills: `UKoiSkillDataAsset`, `UKoiSkillTreeComponent`

No binary Unreal assets are required for these systems to compile.

## Architecture Principles

- Keep core gameplay rules in C++.
- Expose safe Blueprint hooks for UI, VFX, audio, and animation.
- Use Data Assets for designer-tunable content.
- Store durable state in SaveGame structs, not runtime Actor references.
- Use stable IDs for save keys.
- Keep validation helpers source-only and asset-light.
- Avoid creating or modifying `.uasset` files during source system work.

## Fishing Technical Design

Files:

- `Source/Nyx/FishingComponent.h`
- `Source/Nyx/FishingComponent.cpp`

Responsibilities:

- Track `EFishingState`.
- Select fish from `AvailableFish`.
- Generate deterministic bite time from `RandomSeed` and `CastIndex`.
- Manage cast, bite, reel, catch, fail, tension, and reward handoff.
- Track durable collection progress in `FNyxFishingProgressData`.
- Clear transient runtime state on restore.

Progression hooks:

- `FishingPowerMultiplier`
- `BiteTimeMultiplier`
- `BonusFishPullsOnCatch`
- `OnBonusFishPullResolved`

Technical notes:

- `BiteTimeMultiplier` currently scales generated bite time.
- `FishingPowerMultiplier` reduces initial reel tension derived from fish difficulty.
- `BonusFishPullsOnCatch` repeats catch progress and reward processing for extra pull moments.
- Bonus pull behavior is intentionally simple and may need later balancing to avoid over-rewarding rare fish.

Blueprint API:

- Existing dynamic multicast events drive presentation.
- FTUE and validation can observe fishing events without forcing actions.

## Starwell Technical Design

Files:

- `Source/Nyx/Starwell.h`
- `Source/Nyx/Starwell.cpp`

Responsibilities:

- Accept fish offerings.
- Convert fish value to Echo Scales.
- Track `OfferingProgress`, `TotalFishAccepted`, and `TotalEchoScalesGenerated`.
- Track `ReachedStoryUnlockIds`.
- Broadcast gameplay and restoration events.

Progression hook:

- `TurnInMultiplier`

Technical notes:

- `CalculateEchoScalesForFish()` applies `TurnInMultiplier`.
- Threshold save identity uses `StoryUnlockId`.
- `OnOfferingThresholdReached` should only fire for new threshold claims.
- `OnStarwellStateRestored` refreshes presentation after load and should not grant rewards.

## Economy Technical Design

Files:

- `Source/Nyx/EconomyComponent.h`
- `Source/Nyx/EconomyComponent.cpp`
- `Source/Nyx/UpgradeDataAsset.h`

Resources:

- `Stardust`
- `MoonPearls`
- `EchoScales`
- `Koi`

Responsibilities:

- Add and spend resources.
- Check affordability.
- Apply `UUpgradeDataAsset`.
- Track `AppliedUpgradeCounts`.
- Restore saved resources and applied upgrade state.

Technical notes:

- `Koi` is a talent-point style resource.
- Upgrade costs use `FNyxResourceAmount`, so merchant and skill systems can share resource concepts where useful.
- `UUpgradeDataAsset` includes prototype effect enums for future routing.

## Traveling Merchant Technical Design

Files:

- `Source/Nyx/TravelingMerchant.h`
- `Source/Nyx/TravelingMerchant.cpp`

Responsibilities:

- Hold `UpgradeInventory`.
- Check purchase eligibility through `UEconomyComponent`.
- Apply upgrades through `UEconomyComponent::ApplyUpgrade()`.
- Broadcast `OnUpgradePurchased`.

Current limitations:

- No shop UI.
- No schedule, spawn rules, or level placement behavior.
- No final art or merchant animation.
- Tool behavior is represented through upgrade effect hooks, not final gameplay tools.

Blueprint integration:

- Blueprint can populate `UpgradeInventory`.
- Blueprint can show purchase feedback from `OnUpgradePurchased`.
- Merchant visual direction is stored as text notes only.

## Koi Skill Tree Technical Design

Files:

- `Source/Nyx/KoiSkillDataAsset.h`
- `Source/Nyx/KoiSkillDataAsset.cpp`
- `Source/Nyx/KoiSkillTreeComponent.h`
- `Source/Nyx/KoiSkillTreeComponent.cpp`

Responsibilities:

- Store available skill data.
- Track invested skill ranks by stable ID.
- Spend `Koi` through `UEconomyComponent`.
- Enforce max rank and prerequisite IDs.
- Calculate aggregate modifiers.
- Apply modifiers to `UFishingComponent` and `AStarwell`.

Skill effect categories:

- `PlayerPower`
- `FasterBites`
- `BonusFishPulls`
- `TurnInMultiplier`

Save/load:

- Skill ranks should persist through `FNyxSkillTreeSaveData`.
- After restore, skill modifiers should be re-applied to fishing and Starwell targets.

Current limitations:

- No visual skill tree UI.
- No final balance curve.
- No refund/respec support.
- No explicit tree layout metadata yet.

## FTUE Technical Design

Files:

- `Source/Nyx/NyxFTUEComponent.h`
- `Source/Nyx/NyxFTUEComponent.cpp`

Responsibilities:

- Store short prompt definitions.
- Track current FTUE step.
- Observe core loop events.
- Broadcast step changes for Blueprint UI.

Design constraints:

- Does not block input.
- Does not trigger gameplay actions.
- Does not spawn UI directly.
- Should be displayed by Blueprint as a small readable prompt.

Current steps:

- Cast line
- Start reel
- Complete catch
- Offer catch
- Draw card
- Play card
- Complete

## Save/Load Technical Design

Files:

- `Source/Nyx/NyxSaveGame.h`
- `Source/Nyx/NyxSaveGame.cpp`

Save structs:

- `FNyxFishingSaveData`
- `FNyxEconomySaveData`
- `FNyxDeckSaveData`
- `FNyxStarwellSaveData`
- `FNyxSkillTreeSaveData`

Rules:

- Save durable data only.
- Normalize active fishing state to idle on capture/restore.
- Do not persist active timers, temporary tension windows, animation locks, or partial catches.
- Use stable IDs for fish, upgrades, Starwell thresholds, and skills.
- Re-apply skill modifiers after restore.

Known technical risk:

- Function signatures in `UNyxSaveGameLibrary` now include `UKoiSkillTreeComponent*`. Existing Blueprint nodes may need refresh/recompile after this change.

## Debug And Validation Technical Design

Files:

- `Source/Nyx/NyxGameplayDebugActor.h`
- `Source/Nyx/NyxGameplayDebugActor.cpp`
- `Source/Nyx/NyxGameplayValidation.h`
- `Source/Nyx/NyxGameplayValidation.cpp`

Current validation coverage:

- Economy
- Deck
- Fishing
- SaveGame
- FTUE
- KoiSkillTree
- TravelingMerchant
- Starwell
- DebugSaveLoadReliability

Automation group:

```text
Nyx.Gameplay
```

Expected current count after this pass:

```text
8 tests
```

Validation goals:

- Catch compile regressions.
- Catch basic resource spend mistakes.
- Catch save/load restore mistakes.
- Confirm FTUE does not mutate gameplay on start.
- Confirm Koi skill investment, prerequisites, modifiers, and restore.
- Confirm merchant purchase path.

## Blueprint Handoff Notes

Blueprint should own:

- UI widgets.
- VFX/audio feedback.
- Character mesh assignment.
- Merchant presentation.
- Starwell visuals.
- Skill tree screen layout.
- FTUE prompt display.

C++ should own:

- Core state transitions.
- Resource math.
- Save/load data.
- Upgrade and skill application rules.
- Validation helpers.

## Open Technical Questions

- Should bonus fish pulls duplicate full rewards or generate separate bonus-only reward events?
- Should Koi be earned from Starwell thresholds, rare fish, perfect catches, or merchant bundles?
- Should skill tree layout data live in `UKoiSkillDataAsset` or a separate tree asset?
- Should merchant inventory rotate deterministically by day/run seed?
- Should tool upgrades become their own `UToolDataAsset` later?
- Should FTUE completion be saved in `UNyxSaveGame`?

## Next TDD Passes

- Merchant inventory rotation TDD.
- Koi skill layout and balance TDD.
- Tool/gathering behavior TDD.
- FTUE save/completion TDD.
- Bonus pull reward policy TDD.
- Blueprint UI handoff TDD.
