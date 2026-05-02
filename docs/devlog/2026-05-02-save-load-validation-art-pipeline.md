# Save/Load Validation + Art Pipeline Notes

Date: 2026-05-02

Status: Work-in-progress prototype documentation

## Overview

This PR focused on making Nyx easier to validate and easier to explain as an Unreal C++ learning project. The work connects save/load reliability checks with clearer low-poly art pipeline documentation.

Nyx is still a prototype. This update does not claim finished gameplay, final visuals, or complete test coverage.

## What Changed

- Added native C++ event mirrors for save/load restoration hooks so debug validation can observe restored fishing, economy, deck, and Starwell state.
- Expanded `ANyxGameplayDebugActor` save/load reliability logging so PIE validation can report what restored correctly and what still needs attention.
- Strengthened checks around transient fishing state, durable collection progress, Starwell progress, claimed threshold IDs, and one-time threshold events.
- Expanded `docs/ArtPipeline.md` with a clearer bubbly low-poly direction for the tuxedo-pattern cat, chunky Starwell, simple cosmic fish, soft colors, toy-like materials, and squishy animation goals.

## Why It Matters

The save/load work practices an important Unreal C++ boundary: temporary runtime state should not be treated like durable player progress. Active casts, timers, reel tension, bite windows, and temporary catch data should reset safely after load, while discovered fish, resources, upgrades, Starwell progress, and claimed thresholds should persist.

The art pipeline notes help keep the project visually scoped. A simple low-poly direction makes the prototype easier to read, easier to animate, and easier to present without pretending it has final production art.

## Testing And Validation

The PR adds validation hooks and debug logging intended for PIE checks through `ANyxGameplayDebugActor`.

During this documentation pass, I reviewed the changed source and documentation split by commit. I did not run an Unreal build, PIE session, or automation test pass as part of this devlog.

## Portfolio Takeaway

This update shows how I am learning to connect Unreal C++ architecture, QA thinking, save/load reliability, and art direction into one clearer prototype story. The project is still in progress, but the work is becoming more testable, more readable, and easier to discuss honestly.
