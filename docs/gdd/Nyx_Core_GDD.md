# Nyx Core Game Design Document

Date: 2026-05-02

Status: Work-in-progress prototype design

## Overview

Nyx is a cozy cosmic fishing prototype about a tuxedo-pattern cat catching celestial fish, offering them to the Starwell, earning resources, and gradually becoming better at the loop through upgrades, tools, cards, and Koi skill investment.

This document describes the intended design direction. It does not claim finished gameplay, final balance, final art, UI, audio, VFX, or shipped content.

## Design Pillars

- Cozy cosmic routine: the player should feel pulled into a gentle loop of fishing, offering, upgrading, and returning stronger.
- Small upgrades, big feeling: upgrades should be gradual but satisfying, with clear feedback when the player gets stronger.
- Readable systems: fish, resources, cards, tools, and skills should be easy to understand without heavy UI.
- Source-first prototype: C++ systems define the core rules; Blueprint presentation can make them feel good later.
- Honest progression: save/load, upgrades, and skill investment should protect player progress and avoid replaying one-time rewards.

## Player Fantasy

The player is a simplified tuxedo-pattern cat exploring a soft cosmic fishing space. The cat is not wearing a tuxedo; the markings are natural black-and-white fur.

The player fantasy is:

```text
I fish in a strange moonlit place, offer catches to a cosmic basin, earn rewards, buy charming upgrades from a traveling cat merchant, and slowly make each run feel better.
```

## Core Loop

Current intended loop:

```text
Cast -> wait for bite -> reel -> catch fish -> offer to Starwell -> earn Echo Scales/Koi/progress -> buy upgrades or invest skills -> fish better next time
```

Implemented C++ foundations include:

- `UFishingComponent`
- `UFishDataAsset`
- `AStarwell`
- `UEconomyComponent`
- `UUpgradeDataAsset`
- `UDeckComponent`
- `UNyxSaveGame`
- `UNyxFTUEComponent`
- `ATravelingMerchant`
- `UKoiSkillDataAsset`
- `UKoiSkillTreeComponent`

## FTUE Goals

The first-time user experience should be short, readable, and non-invasive. It should guide the player through the loop without locking input or becoming a long tutorial.

Current FTUE step targets:

- Cast line
- Start reel
- Complete catch
- Offer catch
- Draw card
- Play card
- Continue exploring

Implementation foundation:

- `UNyxFTUEComponent` owns the step definitions.
- It listens to fishing, deck, and Starwell events.
- Blueprint UI should display it as a small prompt or toast, not a modal blocker.

Design intent:

- Use plain verbs.
- Keep copy short.
- Let the player act naturally.
- Advance from observed gameplay events, not from forced tutorial commands.

## Fishing Design

Fishing should feel calm, clear, and increasingly satisfying as upgrades come online.

Current prototype concepts:

- Fish are defined by `UFishDataAsset`.
- `UFishingComponent` controls cast, bite, reel, catch, fail, and tension state.
- Bite timing should become faster through Koi skills and upgrades.
- Player power should make reel difficulty feel more manageable.
- Bonus fish pulls should create occasional satisfying extra rewards after successful catches.

Future tuning questions:

- How long should the first bite take?
- How quickly should bite time upgrades ramp?
- Should bonus pulls be guaranteed, chance-based, or tied to perfect reels?
- Should rare fish interact with card tags or specific tools?

## Starwell Design

The Starwell is the central conversion and progression object.

Player-facing purpose:

- Give caught fish a meaningful destination.
- Convert catches into resources.
- Track offering progress.
- Trigger threshold moments.
- Support future story unlocks.

Current implementation:

- `AStarwell` accepts caught fish.
- Fish convert into Echo Scales.
- Offering progress is tracked.
- Threshold IDs are stable for save/load.
- Turn-in multipliers can increase reward value.

Presentation goals:

- Idle pulse
- Offering accepted pulse
- Threshold reached burst
- Post-load restored shimmer that refreshes state without implying a new reward

## Economy Design

Current resources:

- Stardust: general value and future soft currency.
- Moon Pearls: rarer progression or purchase currency.
- Echo Scales: Starwell offering reward currency.
- Koi: talent-point style resource for skill tree investment.

Design intent:

- Echo Scales support merchant purchases and upgrade pacing.
- Koi supports longer-term player power choices.
- Resource gains should be clear and paired with small feedback moments.
- Spending should feel like progress, not loss.

## Traveling Merchant Design

The traveling merchant is a tortoiseshell cat who sells upgrades and tools.

Design purpose:

- Provide a cozy shop layer between fishing runs.
- Give Echo Scales and other currencies a satisfying use.
- Introduce tools that improve collection pacing.
- Make upgrades feel gradual and rewarding.

Current implementation foundation:

- `ATravelingMerchant`
- `UpgradeInventory`
- `PurchaseUpgrade()`
- `OnUpgradePurchased`

Visual identity:

- Tortoiseshell cat markings: patchy black, orange, cream, and brown.
- Cozy, toy-like, simple low-poly form.
- Should feel like a friendly recurring visitor, not a final polished NPC yet.

Upgrade feeling goals:

- Clear purchase confirmation.
- Small reward burst.
- Immediate readable benefit where possible.
- Stronger upgrades should feel earned over time.

Tool direction:

- Tools can gather or pull fish more efficiently over time.
- Early tool effects should be simple prototype hooks.
- Later passes can add visuals, animations, and stronger feedback moments.

## Koi Skill Tree Design

Koi is a talent-point style resource used to invest in player growth.

Current skill effect categories:

- Player power
- Faster bites
- Bonus fish pulls
- Turn-in multiplier

Design intent:

- Let players choose what kind of improvement they want first.
- Make early investments cheap and understandable.
- Keep effects gradual so the fishing loop has room to grow.
- Avoid overwhelming the player with a large tree too early.

Potential early skill examples:

- Patient Paw: slightly reduces bite times.
- Strong Reel: increases player power against reel difficulty.
- Lucky Splash: adds a bonus fish pull on successful catches.
- Starwell Favor: increases turn-in rewards.

Open questions:

- Should Koi come from Starwell thresholds, rare fish, merchant purchases, or milestone rewards?
- Should skills have prerequisites or simple rows?
- Should Koi be refundable during prototype tuning?

## Deckbuilding Design

The deck system is an early foundation, not a finished card game.

Current role:

- Draw cards.
- Play cards.
- Shuffle discard into deck.
- Support future modifiers through tags and prototype effects.

Design direction:

- Cards should create small, readable changes to fishing, offering, or rewards.
- Early cards should be simple enough to test manually.
- Cards should eventually reinforce build identity without overwhelming the cozy loop.

## Save/Load Design

Save/load should preserve durable progress and clear transient gameplay state.

Durable examples:

- Discovered fish and catch counts.
- Resources.
- Applied upgrades.
- Starwell progress.
- Claimed threshold IDs.
- Skill ranks.

Transient examples:

- Active cast timers.
- Bite timers.
- Temporary reel tension.
- Partially resolved catch results.

Design rule:

Loading a save should refresh presentation, not replay one-time rewards.

## First Playable Goal

A first playable should prove:

- The player can fish.
- A fish can be offered.
- The Starwell gives rewards.
- A merchant can sell an upgrade.
- Koi can be invested in one or two skills.
- The next fishing loop feels slightly better.
- Save/load keeps progress without replaying rewards.

## Current Non-Goals

- Final balance.
- Final UI.
- Final art, animation, VFX, or audio.
- Large content volume.
- Complex NPC schedules.
- Full card effect library.
- Full skill tree layout.

## Next GDD Passes

- First 10-minute player journey.
- Merchant upgrade curve.
- Koi earning and spending curve.
- First five fish definitions.
- First five cards.
- First three tools.
- Starwell threshold reward plan.
