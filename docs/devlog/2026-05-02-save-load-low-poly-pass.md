# Save/Load Reliability + Low-Poly Visual Direction Pass

Date: 2026-05-02

Status: Work-in-progress prototype documentation

## Overview

This pass connects two parts of Nyx that support the same portfolio goal: making the project easier to understand, test, and present honestly while it is still in development.

On the systems side, the focus is save/load reliability for the fishing, Starwell, economy, upgrade, and progression foundations. On the visual side, Nyx is moving toward a simple, bubbly, low-poly style that fits the cozy cosmic fishing concept without pretending the game is finished.

## Save/Load Reliability

Active fishing casts normalize back to `Idle` after loading. A live cast depends on transient runtime details such as active timers, reel tension, bite windows, selected interaction state, and temporary catch data. Those values are useful during play, but they are fragile to restore directly from a save file.

The safer boundary is to persist durable progress and reset temporary interaction state. For Nyx, durable progress can include discovered fish, resources, applied upgrades, Starwell progress, and claimed threshold IDs.

Blueprint post-load events provide a clean refresh point for UI, VFX, audio, and world presentation. They should update the world to match restored data without replaying one-time gameplay rewards, duplicating unlocks, or pretending the player just performed the action again.

Starwell thresholds also need stable unique IDs. A saved threshold should depend on a stable key such as `StoryUnlockId`, not display text, array position, or temporary asset names. That keeps progression safer as content is renamed, reordered, tuned, or expanded.

## Low-Poly Visual Direction

Nyx is adopting a simple, bubbly, low-poly visual direction for its first visual prototype pass. The target is cozy, readable, and toy-like rather than realistic or overly detailed.

The visual direction includes:

- a simplified tuxedo-pattern cat with natural black-and-white markings
- a chunky, round Starwell that can support clear offering and restore feedback
- simple cosmic fish with readable silhouettes
- soft colors and rarity accents
- toy-like materials that keep shapes friendly and clear
- squishy animation timing for casting, catching, offering, and response moments

This is a WIP visual foundation, not final production art. The goal is to make the prototype easier to read, easier to animate, and more understandable for portfolio review while the Unreal C++ systems continue to mature.

## Portfolio Takeaway

This pass shows two practical development habits:

- defining save/load boundaries so systems remain reliable as the prototype grows
- choosing a visual direction that supports readability, scope control, and clear presentation

Nyx is still a prototype, but these choices help it become easier to explain as an Unreal C++ gameplay systems project with a coherent creative direction.
