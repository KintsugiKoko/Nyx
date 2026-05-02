# Nyx

## Overview

Nyx is a work-in-progress Unreal Engine C++ gameplay systems prototype with a cozy cosmic fishing theme. The concept premise centers on a tuxedo-pattern cat catching celestial fish, offering them to a mysterious Starwell, earning economy rewards, unlocking progression thresholds, and saving persistent progress.

This is not a finished commercial game. It is a learning-focused prototype for exploring Unreal C++ architecture, data-driven gameplay, Blueprint-facing events, save/load boundaries, and small testable gameplay systems.

The current source foundation focuses on the gameplay loop and supporting architecture more than character implementation, final art, UI, levels, screenshots, or shipped content.

## Visual Direction

Nyx is adopting a simple, bubbly, low-poly visual direction for its first visual prototype pass. The target feel is cozy cosmic and toy-like: rounded shapes, chunky silhouettes, soft cosmic colors, readable rarity accents, and squishy animation timing that supports the fishing and offering loop.

This is work-in-progress art direction, not final production art. The goal is to make the prototype easier to read, easier to animate, and more portfolio-friendly while the C++ gameplay systems continue to mature.

Key visual notes:

- The player character is a simplified low-poly tuxedo-pattern cat with natural black-and-white markings. The cat is not wearing a tuxedo.
- `AStarwell` should read as a chunky, round, cosmic basin with clear idle pulse, offering accepted, threshold reached, and post-load restored presentation states.
- `UFishDataAsset` now keeps gameplay tuning separate from optional presentation references such as fish mesh, material, and icon.
- Fish should use simple rounded silhouettes with rarity differences that are readable at a glance.
- The first prototype asset pass should focus on the cat blockout, Starwell blockout, one Glow Minnow fish, a small dock platform, a simple fishing rod, and one offering animation.

See `docs/ArtPipeline.md` for practical Blender-to-Unreal guidance.

## Current Status

Status: work in progress gameplay systems prototype.

Implemented or present in `Source/Nyx`:

- Core fishing flow through `UFishingComponent`
- World-facing Starwell offering actor through `AStarwell`
- Economy resources and upgrade application through `UEconomyComponent`
- Deck draw, play, discard, and deterministic reshuffle behavior through `UDeckComponent`
- Fish, card, upgrade, and Starwell threshold definitions through C++ data structures and Data Assets
- Save/load capture and restore helpers through `UNyxSaveGame` and `UNyxSaveGameLibrary`
- Blueprint event hooks for presentation layers
- PIE-facing gameplay test harness through `ANyxGameplayDebugActor`
- Source-only validation helpers and automation test hooks through `UNyxGameplayValidationLibrary`
- First-pass visual hookup points through `ANyxCatPlayerCharacter`, `AStarwell` scene components, and fish presentation references

Prototype or early foundation:

- Deckbuilding and incremental progression foundations
- Upgrade effect routing through prototype enums and gameplay tags
- Starwell story threshold unlock data
- Deterministic fishing and deck state for repeatable testing

Planned or not yet represented as complete gameplay:

- Final player-facing UI
- Final VFX, audio, animation, and presentation
- A complete inventory screen or dedicated `UInventoryComponent`
- Finished content tuning, levels, assets, screenshots, or release packaging

## Core Gameplay Loop

The intended loop is:

```text
Catch celestial fish -> offer fish to the Starwell -> earn rewards -> unlock progress -> save/load state
```

In the current C++ prototype:

1. `UFishDataAsset` defines fish data such as display name, rarity, base value, bite timing, reel difficulty, and card interaction tags.
2. `UFishingComponent` selects a fish, manages cast/bite/reel/catch states, tracks tension, and can offer completed catches to a Starwell target.
3. `AStarwell` accepts caught fish, converts fish value into Echo Scales, tracks offering progress, and fires threshold/story unlock events.
4. `UEconomyComponent` stores resources such as Stardust, Moon Pearls, and Echo Scales, and applies upgrade data when affordable.
5. `UDeckComponent` supports early deckbuilding foundations with draw pile, hand, discard pile, play, and shuffle behavior.
6. `UNyxSaveGame` captures stable save data for fishing, economy, deck, and Starwell progress.

## Gameplay Architecture

Nyx is organized around a simple Unreal gameplay separation:

| Layer | Role | Examples |
| --- | --- | --- |
| Actors | World-facing objects placed or spawned in the level | `AStarwell`, `ANyxGameplayDebugActor` |
| Actor Components | Reusable gameplay logic attached to actors | `UFishingComponent`, `UEconomyComponent`, `UDeckComponent`, `UHealthComponent` |
| Data Assets | Designer-tunable content definitions | `UFishDataAsset`, `UCardDataAsset`, `UUpgradeDataAsset` |
| SaveGame structs | Stable persisted state | `FNyxFishingSaveData`, `FNyxEconomySaveData`, `FNyxDeckSaveData`, `FNyxStarwellSaveData` |
| Blueprint events | Presentation-facing hooks | fishing events, deck events, economy events, Starwell threshold events |
| Validation helpers | Lightweight test coverage for gameplay behavior | `UNyxGameplayValidationLibrary`, `Nyx.Gameplay.*` automation tests |

The important architectural idea is that runtime actors and components can change, but save data should remain stable and predictable. `UNyxSaveGame` stores simple resource values, progression numbers, stable IDs, and soft references to data assets rather than depending on direct Actor references.

## Systems Breakdown

### Fishing

Files:

- `Source/Nyx/FishingComponent.h`
- `Source/Nyx/FishingComponent.cpp`

`UFishingComponent` owns the current fishing state. It supports casting, deterministic fish selection, bite timing, reeling, catch completion, failure, tension changes, and optional automatic offering to `AStarwell`.

Blueprint events such as `OnCastStarted`, `OnFishBite`, `OnReelStarted`, `OnCatchCompleted`, `OnCatchFailed`, and `OnCatchOfferedToStarwell` are intended for UI, VFX, audio, and player feedback.

Save/load deliberately restores fishing to a clean idle state. Durable progress such as fish discovery, catch counts, and perfect catch counts lives in `FNyxFishingProgressData`; transient state such as active bite timers, reel tension, and partial catches is cleared on restore.

### Starwell

Files:

- `Source/Nyx/Starwell.h`
- `Source/Nyx/Starwell.cpp`

`AStarwell` is the world-facing offering object. It accepts caught fish, calculates Echo Scales from fish value, adds rewards through `UEconomyComponent`, tracks offering progress, and broadcasts threshold/story unlock events.

`FStarwellOfferingThreshold` defines threshold content with required progress, stable story unlock IDs, and gameplay tags for future story or world-state routing.

For the first visual prototype pass, `AStarwell` exposes source-created components for a visual mesh, interaction collision, offering point, and VFX spawn point. Gameplay events such as `OnFishAccepted` and `OnOfferingThresholdReached` stay separate from restoration events such as `OnStarwellStateRestored`, so loading a save can refresh presentation without replaying rewards.

### Economy and Upgrades

Files:

- `Source/Nyx/EconomyComponent.h`
- `Source/Nyx/EconomyComponent.cpp`
- `Source/Nyx/UpgradeDataAsset.h`
- `Source/Nyx/UpgradeDataAsset.cpp`

`UEconomyComponent` tracks Stardust, Moon Pearls, Echo Scales, and applied upgrade counts. `UUpgradeDataAsset` defines upgrade IDs, display text, resource costs, max applications, prototype effect hooks, and affected gameplay tags.

Upgrade behavior is still prototype-level. The current system demonstrates cost checks, resource spending, stable upgrade IDs, and save-friendly applied upgrade counts.

### Deckbuilding Foundation

Files:

- `Source/Nyx/CardDataAsset.h`
- `Source/Nyx/CardDataAsset.cpp`
- `Source/Nyx/DeckComponent.h`
- `Source/Nyx/DeckComponent.cpp`

`UCardDataAsset` defines card content, card types, prototype effects, and gameplay tags. `UDeckComponent` manages draw pile, hand, discard pile, card drawing, card playing, and deterministic discard reshuffling.

This is an early foundation for future card effects that may influence fishing, fish filtering, economy rewards, or Starwell offerings.

### Fish Data

Files:

- `Source/Nyx/FishDataAsset.h`
- `Source/Nyx/FishDataAsset.cpp`

`UFishDataAsset` keeps fish tuning out of hard-coded gameplay logic. It defines display text, rarity, base Stardust value, bite timing, reel difficulty, and card interaction tags.

Fish definitions also include stable `FishId` values for save/load tracking and optional soft presentation references for first-pass visual prototypes.

### Player Character Foundation

Files:

- `Source/Nyx/NyxCatPlayerCharacter.h`
- `Source/Nyx/NyxCatPlayerCharacter.cpp`

`ANyxCatPlayerCharacter` is a lightweight C++ base for the future tuxedo-pattern cat player. It owns the fishing, economy, and deck components, exposes socket names for fishing rod and offering props, and provides Blueprint-facing animation cue events. It does not include final mesh, animation, or asset content.

### Utility Components

Files:

- `Source/Nyx/HealthComponent.h`
- `Source/Nyx/HealthComponent.cpp`

`UHealthComponent` is a reusable health-state component with Blueprint events for health changes and death. It is present as a general gameplay utility, but it is not currently part of the main fishing-to-Starwell prototype loop.

### Save and Persistence

Files:

- `Source/Nyx/NyxSaveGame.h`
- `Source/Nyx/NyxSaveGame.cpp`

`UNyxSaveGame` stores stable progress data for fishing, economy, deck, and Starwell state. The save structs use values, stable IDs, and soft data asset references where appropriate.

This avoids saving direct runtime Actor references and keeps persistence explicit:

- `FNyxFishingSaveData`
- `FNyxEconomySaveData`
- `FNyxDeckSaveData`
- `FNyxStarwellSaveData`

`UNyxSaveGameLibrary` exposes Blueprint-callable helpers for capture, apply, save-to-slot, load-from-slot, and load-then-apply workflows.

### Debug and Validation

Files:

- `Source/Nyx/NyxGameplayDebugActor.h`
- `Source/Nyx/NyxGameplayDebugActor.cpp`
- `Source/Nyx/NyxGameplayValidation.h`
- `Source/Nyx/NyxGameplayValidation.cpp`

`ANyxGameplayDebugActor` is a developer-facing PIE test harness. It owns `UFishingComponent`, `UEconomyComponent`, and `UDeckComponent`, can use or spawn an `AStarwell`, and exposes Blueprint-callable debug actions for validating the loop in-editor.

Useful debug actions include:

- `DebugStartCast()`
- `DebugForceFishBite()`
- `DebugStartReel()`
- `DebugCompleteCatch()`
- `DebugOfferFishToStarwell()`
- `DebugAddResources()`
- `DebugDrawCard()`
- `DebugPlayFirstCardInHand()`
- `DebugSaveToSlot()`
- `DebugLoadFromSlot()`
- `DebugRunGameplayValidations()`
- `DebugValidateSaveLoadReliability()`

`UNyxGameplayValidationLibrary` provides lightweight validation helpers and automation test hooks for economy, deck, fishing, save game, and Starwell behavior.

## What This Project Demonstrates

Nyx is meant to show practical Unreal C++ systems thinking:

- Separating world-facing Actors from reusable Actor Components
- Keeping gameplay content data-driven with Data Assets
- Exposing C++ systems to Blueprint without putting core rules only in Blueprint
- Using multicast Blueprint events for UI, VFX, audio, and presentation feedback
- Designing SaveGame data around stable values and IDs instead of direct Actor references
- Creating a PIE debug harness for fast manual validation
- Using source-only validation helpers to reduce regression risk while systems are still changing
- Documenting unfinished work honestly for portfolio review

## Learning Journey

This project is part of my broader rebuild through code, GitHub, documentation, QA practice, and AI-assisted workflows.

With Nyx, I am practicing how Unreal C++ gameplay systems fit together:

- How Actors and Actor Components share responsibilities
- How Data Assets keep gameplay tuning editable and readable
- How Blueprint events bridge engineering logic and presentation
- How save/load boundaries shape system design
- How deterministic seeds make gameplay easier to test
- How debug actors and validation helpers support iteration before UI and content are final

The project is intentionally small in scope so the architecture can stay understandable while I keep learning.

## Devlog

Development notes capture specific learning passes while the prototype is still in progress:

- [Save/Load Reliability Pass](docs/devlog/2026-05-02-save-load-reliability-pass.md) - explains why Nyx separates durable player progress from transient fishing runtime state, and how restore events and stable Starwell IDs support safer save/load behavior.
- [Save/Load Reliability + Low-Poly Visual Direction Pass](docs/devlog/2026-05-02-save-load-low-poly-pass.md) - connects save/load boundaries with the first low-poly art direction notes for the tuxedo-pattern cat, Starwell, cosmic fish, and presentation goals.

## Work-in-Progress Scope

Nyx should be read as a prototype, not a finished game.

Currently in scope:

- Source-first gameplay systems
- Fishing, Starwell, economy, deck, upgrades, save/load, debug, and validation foundations
- Documentation that explains what exists and why it exists

Not currently claimed:

- Finished commercial gameplay
- Final art, sound, VFX, UI, or level design
- A complete inventory feature or dedicated inventory component
- Balanced content, full card effects, or final upgrade effects
- Shipped screenshots, trailers, or release builds

## Future Plans

Likely next steps:

- Add a dedicated inventory or catch collection layer once the fish storage rules are clearer
- Create or tune fish, card, upgrade, and Starwell threshold Data Assets in the editor
- Build a simple Blueprint UI around fishing, resources, deck state, and Starwell progress
- Add presentation feedback through Blueprint events for catch, offering, reward, threshold, and save/load moments
- Expand card and upgrade effects beyond prototype enums
- Add screenshots or clips only when there is real project content ready to show
- Revisit build instructions after local Windows SDK/platform support is fixed

## How to Explore the Code

Start here:

1. Read `Source/Nyx/README.md` for the lower-level developer note and validation checklist.
2. Open `Source/Nyx/FishingComponent.h` to see the fishing state machine and Blueprint events.
3. Open `Source/Nyx/Starwell.h` to see offering thresholds and progression events.
4. Open `Source/Nyx/EconomyComponent.h` and `Source/Nyx/UpgradeDataAsset.h` to see resources and upgrade data.
5. Open `Source/Nyx/DeckComponent.h` and `Source/Nyx/CardDataAsset.h` to see the deckbuilding foundation.
6. Open `Source/Nyx/NyxSaveGame.h` to see how progress is captured without relying on direct Actor references.
7. Open `Source/Nyx/NyxGameplayDebugActor.h` to see the PIE test harness for validating the loop.
8. Open `Source/Nyx/NyxGameplayValidation.cpp` to see source-only validation helpers and automation test hooks.

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
8. Change resources, deck, or Starwell progress.
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

## Known Local Build Note

Earlier command-line builds on this machine stopped before project code compiled because UnrealBuildTool reported `Win64` as not buildable due to missing SDK/toolchain support. That environment blocker was resolved by installing Visual Studio Build Tools 2022, the MSVC C++ toolchain, Windows SDK `10.0.22621.0`, and the .NET Framework Developer Pack.

Current validation flow:

1. Build `NyxEditor`.
2. Run PIE validation through `ANyxGameplayDebugActor`.
3. Run automation tests for the `Nyx.Gameplay.*` checks.

## Source-Only Constraint

The current gameplay foundation intentionally avoids creating or modifying:

- `.uasset`
- `.umap`
- other binary content files

Data assets, maps, UI, VFX, audio, screenshots, and final presentation content can be created later in the Unreal Editor after the C++ project compiles.
