# Save/Load Reliability Pass

Date: 2026-05-02

Status: Work in progress gameplay systems prototype

## Overview

This devlog documents a save/load reliability pass for Nyx, an Unreal Engine C++ gameplay systems prototype. The goal of this pass was not to make the game feel finished. The goal was to make the core prototype safer to test, easier to reason about, and clearer as portfolio evidence.

The main lesson: durable player progress and transient gameplay state should not be treated the same way.

For Nyx, durable progress includes things like discovered fish, catch counts, economy resources, applied upgrades, deck state, and Starwell progress. Transient state includes an active fishing cast, bite timers, reel tension, bite windows, and partial catch state.

## Why Active Casts Normalize To Idle

`UFishingComponent` owns the fishing state machine for casts, bites, reeling, catch completion, and catch failure. During normal play, states such as `Casting`, `FishBiting`, and `Reeling` can depend on live timers, current input timing, selected fish data, tension values, and pending rewards.

Those details are useful while the interaction is happening, but they are fragile after a save file is loaded. Restoring a half-finished cast would require the game to rebuild runtime timing, animation, input, and feedback state exactly enough for the player to understand what is happening.

Instead, `UFishingComponent::RestoreSavedState()` clears transient runtime state and returns the fishing state to `EFishingState::Idle`. This keeps loaded saves understandable and avoids pretending an interrupted interaction can safely resume without the surrounding runtime context.

That choice protects the player experience: after load, the player should understand the state of the world and be able to start a fresh cast.

## Runtime State Versus Durable Progress

Active cast timers, reel tension, bite windows, and partial catch state are runtime details. They help the current interaction feel responsive, but they are not long-term progress.

Durable progress should survive save/load. In the current source, save data is split across structs such as:

- `FNyxFishingSaveData`
- `FNyxEconomySaveData`
- `FNyxDeckSaveData`
- `FNyxStarwellSaveData`

`UNyxSaveGame` collects those pieces, and `UNyxSaveGameLibrary` provides Blueprint-callable helpers for capture, apply, save-to-slot, load-from-slot, and load-then-apply workflows.

For the fishing system, durable progress can include collection progress such as discovered fish and catch counts. For the economy and Starwell, durable progress can include resources, applied upgrade counts, offering progress, total accepted fish, total Echo Scales generated, and reached story unlock IDs.

## Blueprint Post-Load Events

Save/load is not only a C++ data problem. After values are restored, Blueprint-driven presentation may need to refresh.

Nyx uses Blueprint-facing restore events such as:

- `OnFishingSaveApplied`
- `OnFishingStateRestored`
- `OnStarwellSaveApplied`
- `OnStarwellStateRestored`

These events give UI, VFX, audio, and world presentation a clean place to update after loading. The important part is that restoration should refresh the world to match saved data without replaying one-time gameplay rewards.

## Gameplay Events Versus Restoration Events

Gameplay events and restoration events need different meanings.

A gameplay event says, "the player just did something." For example, offering a fish to `AStarwell` can grant Echo Scales, increase Starwell progress, and trigger threshold logic.

A restoration event says, "the game state was loaded and presentation should catch up." It should not grant the reward again, replay an unlock as if it just happened, or duplicate progress.

Keeping those meanings separate makes the system easier to test. It also reduces the risk of load-related bugs such as duplicate rewards, repeated unlocks, or confusing UI state.

## Stable Starwell Threshold IDs

`AStarwell` tracks story threshold progress through `FStarwellOfferingThreshold` and saved `ReachedStoryUnlockIds`.

The important reliability choice is to use stable unique IDs, such as `StoryUnlockId`, rather than display text, array position, or temporary asset names.

Stable IDs matter because content changes over time. Threshold text may be rewritten, thresholds may be reordered, and asset names may change while the prototype grows. A save file should still know which Starwell thresholds were reached.

This keeps future narrative unlocks, world-state changes, card unlocks, or progression branches safer to expand later.

## Debug Validation

`ANyxGameplayDebugActor` provides developer-facing PIE tooling for testing the loop while the project is still in progress. The source includes save/load debug actions such as `DebugSaveToSlot()`, `DebugLoadFromSlot()`, and `DebugValidateSaveLoadReliability()`.

This does not mean the game has finished automated coverage or final gameplay. It means the prototype is being shaped so save/load behavior can be checked intentionally during editor testing.

## Portfolio Takeaway

This pass strengthens Nyx as a portfolio-ready Unreal C++ gameplay systems prototype because it shows more than feature writing. It shows system boundaries:

- runtime state is separated from durable progress
- C++ gameplay logic exposes safe Blueprint refresh points
- save data avoids direct Actor references
- Starwell progression uses stable IDs for future content changes
- debug tooling supports practical validation during iteration

The work is still in progress, but the architecture is moving toward something explainable, testable, and maintainable.
