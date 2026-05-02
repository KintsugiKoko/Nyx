# Nyx Gameplay Systems Developer Note

This folder contains the source-only gameplay foundation for the Nyx Unreal C++ prototype. The systems are intentionally modular: data assets define tunable content, actor components hold runtime state, and lightweight validation helpers make the loop testable without creating binary assets.

## Current Gameplay Loop

Nyx currently supports this prototype loop:

1. Fish definitions are authored as `UFishDataAsset`.
2. `UFishingComponent` selects a fish from available fish data and drives cast, bite, reel, catch, and fail states.
3. Completed catches can be offered to `AStarwell`.
4. `AStarwell` converts fish into Echo Scales and tracks offering progress.
5. `UEconomyComponent` stores Stardust, Moon Pearls, Echo Scales, and applied upgrades.
6. `UDeckComponent` manages draw pile, hand, discard pile, and deterministic reshuffling.
7. `UNyxSaveGame` captures and restores fishing, economy, deck, and Starwell progress.

## Visual Prototype Readiness

The first low-poly visual pass is supported by source-only hooks:

- `ANyxCatPlayerCharacter` provides a native cat player base with `UFishingComponent`, `UEconomyComponent`, `UDeckComponent`, rod/hold/offering socket names, and animation cue events.
- `AStarwell` owns `VisualMesh`, `InteractionCollision`, `OfferingPoint`, and `VfxSpawnPoint` components for Blueprint art hookup.
- `UFishDataAsset` separates gameplay tuning from optional presentation references such as `PresentationMesh`, `PresentationMaterial`, and `IconTexture`.

The style target is simple, bubbly, low-poly, cozy cosmic, and toy-like. These are hookup points only; no final meshes, materials, VFX, animation assets, or binary content are created in source.

## Gameplay Systems

### Health

Files:
- `HealthComponent.h`
- `HealthComponent.cpp`

`UHealthComponent` provides reusable health state with damage, healing, health changed events, death events, and server-authoritative behavior for networked play.

Why it exists: gameplay actors need a shared health model that can be reused by players, enemies, interactables, and future encounter prototypes.

### Fish Data

Files:
- `FishDataAsset.h`
- `FishDataAsset.cpp`

`UFishDataAsset` defines fish content:
- stable fish ID for save/load tracking
- display name
- description
- base Stardust value
- rarity
- bite time range
- reel difficulty
- gameplay tags for card interactions
- optional soft presentation references for mesh, material, and icon

Why it exists: fish tuning should live in designer-friendly data, not hard-coded fishing logic.

### Fishing

Files:
- `FishingComponent.h`
- `FishingComponent.cpp`

`UFishingComponent` supports:
- `StartCast()`
- `CancelCast()`
- `StartReel()`
- `CompleteCatch()`
- `FailCatch()`
- deterministic fish selection using `RandomSeed + CastIndex`
- basic tension tracking
- durable fishing progress through `FNyxFishingProgressData`
- Blueprint events for bite, reel, catch, failure, tension, and state changes
- post-load restoration events through `OnFishingSaveApplied` and `OnFishingStateRestored`
- optional catch offering to `AStarwell`

Why it exists: fishing is the runtime bridge between fish data, player interaction, economy rewards, and story progress.

Save/load note: active casts, bite timers, temporary reel tension, and partial catch results are transient runtime state. Loading a save restores durable progress and then returns fishing to `Idle`.

### Cards And Decks

Files:
- `CardDataAsset.h`
- `CardDataAsset.cpp`
- `DeckComponent.h`
- `DeckComponent.cpp`

`UCardDataAsset` defines card content:
- name
- cost
- description
- type: Lure, Tide, Ritual, Tool, Companion, Oddity
- affected gameplay tags
- prototype effect enum

`UDeckComponent` manages:
- draw pile
- hand
- discard pile
- draw
- play
- shuffle discard into draw pile
- deterministic shuffle state

Why it exists: card effects can eventually modify fishing, Starwell offerings, economy rewards, and fish filtering through gameplay tags.

### Economy And Upgrades

Files:
- `EconomyComponent.h`
- `EconomyComponent.cpp`
- `UpgradeDataAsset.h`
- `UpgradeDataAsset.cpp`

`UEconomyComponent` stores:
- Stardust
- Moon Pearls
- Echo Scales
- applied upgrade counts

It supports:
- `AddResource()`
- `SpendResource()`
- `CanAfford()`
- `ApplyUpgrade()`

`UUpgradeDataAsset` defines upgrade costs, max applications, prototype effects, and affected tags.

Why it exists: rewards and upgrades need one shared source of truth that can be saved, tested, and connected to UI.

### Starwell

Files:
- `Starwell.h`
- `Starwell.cpp`

`AStarwell` accepts caught fish, converts them into Echo Scales, tracks offering progress, and fires Blueprint events when story thresholds are reached.

`AStarwell` also exposes source-created art hookup components:
- `VisualMesh`
- `InteractionCollision`
- `OfferingPoint`
- `VfxSpawnPoint`

Post-load presentation should listen to `OnStarwellStateRestored`. Gameplay reward moments should listen to action events such as `OnFishAccepted`, `OnOfferingThresholdReached`, or `OnStoryUnlockAvailable`.

Why it exists: the Starwell turns fishing rewards into long-term progression and future story unlocks.

### Save Game

Files:
- `NyxSaveGame.h`
- `NyxSaveGame.cpp`

`UNyxSaveGame` stores save data for:
- fishing
- economy
- deck
- Starwell

`UNyxSaveGameLibrary` exposes Blueprint-callable helpers:
- `CreateNyxSaveGame()`
- `CaptureNyxSaveGame()`
- `ApplyNyxSaveGame()`
- `SaveNyxGameToSlot()`
- `LoadNyxGameFromSlot()`
- `LoadNyxGameFromSlotAndApply()`

Save application is routed through component restore methods so Blueprint listeners can refresh after load:
- `UFishingComponent::RestoreSavedState()`
- `UEconomyComponent::RestoreSavedState()`
- `UDeckComponent::RestoreSavedState()`
- `AStarwell::RestoreSavedProgress()`

Active fishing casts are normalized to `Idle` when loaded because timer handles are runtime-only and are not serialized. This prevents a restored cast from getting stuck with no bite timer.

Fishing save data now preserves durable collection progress such as discovered fish IDs, catch counts, and perfect catch counts through `FNyxFishingProgressData`.

Why it exists: save/load is explicit and predictable. Runtime actor and component state is copied into a stable save object instead of depending on implicit actor serialization.

## Debug And Validation

### Debug Actor

Files:
- `NyxGameplayDebugActor.h`
- `NyxGameplayDebugActor.cpp`

`ANyxGameplayDebugActor` is a source-only PIE test harness. It owns:
- `UFishingComponent`
- `UEconomyComponent`
- `UDeckComponent`

It can use an assigned `AStarwell` or spawn one at runtime for testing.

Useful Blueprint-callable helpers include:
- `DebugAddResources()`
- `DebugDrawCard()`
- `DebugPlayFirstCardInHand()`
- `DebugShuffleDiscardIntoDeck()`
- `DebugStartCast()`
- `DebugForceFishBite()`
- `DebugStartReel()`
- `DebugCompleteCatch()`
- `DebugFailCatch()`
- `DebugSaveToSlot()`
- `DebugLoadFromSlot()`
- `DebugRunGameplayValidations()`
- `DebugValidateSaveLoadReliability()`

Why it exists: a single test actor makes it easier to prove the gameplay loop in PIE without building UI or creating extra assets.

### Validation Helpers

Files:
- `NyxGameplayValidation.h`
- `NyxGameplayValidation.cpp`

`UNyxGameplayValidationLibrary` exposes source-only validation helpers:
- `ValidateEconomySystem()`
- `ValidateDeckSystem()`
- `ValidateFishingSystem()`
- `ValidateStarwellSystem()`
- `ValidateSaveGameSystem()`
- `RunAllGameplayValidations()`

The validations create transient fish and card data in memory. They do not require binary assets.

Save validation includes an in-memory apply check and a primitive disk round trip through `SaveGameToSlot()` and `LoadGameFromSlot()`. Starwell validation also checks that offering thresholds use explicit, unique `StoryUnlockId` values.

`ANyxGameplayDebugActor::DebugValidateSaveLoadReliability()` adds a PIE-facing check for active fishing restore-to-Idle behavior, Starwell progress restoration, claimed threshold persistence, non-duplicated threshold rewards, and observable post-load restoration events.

Automation test names:
- `Nyx.Gameplay.Economy`
- `Nyx.Gameplay.Deck`
- `Nyx.Gameplay.Fishing`
- `Nyx.Gameplay.SaveGame`

Why it exists: early gameplay code changes fast. Lightweight validation catches simple regressions before the systems become harder to reason about.

## PIE Validation Checklist

After the project compiles:

1. Place `ANyxGameplayDebugActor` in a test level.
2. Optionally assign fish data assets to `FishingComponent.AvailableFish`.
3. Optionally assign card data assets to `DeckComponent.DrawPile`.
4. Press PIE.
5. Call `DebugRunGameplayValidations()`.
6. Check `bPassed`, `Summary`, and `Failures`.

Manual loop test:

1. Call `DebugStartCast()`.
2. Call `DebugForceFishBite()`.
3. Call `DebugStartReel()`.
4. Call `DebugCompleteCatch()`.
5. Confirm Echo Scales increased.
6. Confirm Starwell offering progress increased.
7. Call `DebugSaveToSlot()`.
8. Mutate resources, deck, or Starwell progress.
9. Call `DebugLoadFromSlot()`.
10. Confirm saved state is restored.

## Automation Checks

Once the project builds, open Unreal's automation tooling and search for:

- `Nyx.Gameplay.Economy`
- `Nyx.Gameplay.Deck`
- `Nyx.Gameplay.Fishing`
- `Nyx.Gameplay.SaveGame`

These tests are compiled behind `WITH_DEV_AUTOMATION_TESTS`.

The Starwell validation needs a valid world context, so it is exposed through the Blueprint validation helper and debug actor rather than the simple object-only automation tests.

## Source-Only Constraint

This gameplay foundation intentionally avoids creating or modifying:
- `.uasset`
- `.umap`
- other binary content files

Data assets can be created later in the Unreal Editor after the C++ project compiles.

## Local Build Note

The local command-line build was unblocked by installing Visual Studio Build Tools 2022, the MSVC C++ toolchain, Windows SDK `10.0.22621.0`, and the .NET Framework Developer Pack.

Current validation flow:

1. Build `NyxEditor`.
2. Run PIE validation through `ANyxGameplayDebugActor`.
3. Run automation tests for the `Nyx.Gameplay.*` checks.

## GitHub Showcase Summary

This source folder demonstrates a modular Unreal C++ gameplay prototype with:
- reusable actor components
- data-driven fish, card, and upgrade definitions
- deterministic fishing and deck state
- Blueprint event hooks
- source-only save/load support
- a gameplay debug actor
- lightweight validation helpers and automation-test hooks

This is portfolio-friendly because it shows not just features, but the engineering habits around them: separation of data from runtime behavior, testable systems, deterministic state, and clear Blueprint integration points.
