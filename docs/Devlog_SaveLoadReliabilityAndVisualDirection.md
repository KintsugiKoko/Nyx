# Devlog: Save/Load Reliability And Visual Direction

Nyx is still a work-in-progress Unreal C++ learning project, but this pass moved it closer to a presentable prototype foundation.

## Save/Load Reliability

The save/load flow now draws a clearer line between durable progress and transient runtime state.

Durable fishing progress is tracked through `FNyxFishingProgressData`, including discovered fish IDs, catch counts, and perfect catch counts. Active fishing state does not try to survive loading. When `UFishingComponent` restores save data, it clears active cast runtime data such as selected fish, bite timing, and tension, then returns to `Idle`.

This matters because timer handles, reel tension, and partially resolved catch moments are gameplay runtime state, not stable save data. Loading should refresh the game state without pretending a half-finished cast can continue safely.

## Blueprint Restoration Events

The project now separates gameplay action events from restoration events.

Examples:

- `AStarwell::OnFishAccepted` means a fish was offered during gameplay.
- `AStarwell::OnOfferingThresholdReached` means a new threshold was reached for the first time.
- `AStarwell::OnStarwellStateRestored` means saved Starwell state was restored and presentation can refresh.
- `UFishingComponent::OnFishingStateRestored` means fishing state loaded and UI can redraw.

This keeps UI, VFX, and audio responsive after loading without replaying one-time rewards.

## Starwell Threshold IDs

Starwell thresholds use stable `StoryUnlockId` values for save tracking. The validation helper can report missing or duplicate IDs, which helps prevent accidental reward duplication after content changes.

Claimed threshold IDs are saved in `AStarwell::ReachedStoryUnlockIds`, so loading progress does not grant threshold rewards again.

## Debug Validation

`ANyxGameplayDebugActor::DebugValidateSaveLoadReliability()` now exercises the save/load edge cases that matter most for this phase:

- active fishing state restores to `Idle`
- fish discovery and catch counts survive restore
- Starwell progress restores correctly
- claimed thresholds stay claimed
- threshold events do not duplicate after load
- restoration events can be observed and logged

## Visual Direction

Nyx is adopting a simple, bubbly, low-poly visual direction for the first prototype art pass. The target is cozy cosmic and toy-like: rounded forms, chunky silhouettes, soft colors, natural tuxedo-cat markings, readable fish rarity, and squishy animation.

This is not final production art. The first pass is about proving the look and connecting art to the C++ gameplay hooks:

- `ANyxCatPlayerCharacter` for cat mesh, rod sockets, and animation cue events
- `AStarwell` for mesh, interaction collision, offering point, and VFX spawn point
- `UFishDataAsset` for optional mesh/material/icon presentation references

The next visual prototype should focus on the cat blockout, Starwell blockout, one Glow Minnow fish, a dock platform, a simple fishing rod, and one offering animation.
