# GDD and TDD Foundations

Date: 2026-05-02

Status: Work-in-progress prototype documentation

## Overview

Nyx now has first-pass Game Design Document and Technical Design Document folders. This gives the project two different kinds of memory: one for the intended player experience, and one for how the Unreal C++ systems are actually being built.

The goal is not to make the prototype look finished. The goal is to make the work easier to explain, easier to review, and easier to continue without losing the thread.

## What Changed

- Added `docs/gdd/` for player-facing design notes.
- Added `docs/tdd/` for implementation and architecture notes.
- Added a core GDD covering the fishing loop, Starwell, FTUE, traveling merchant, Koi skill tree, economy, and progression direction.
- Added a core TDD covering the matching C++ systems, save/load boundaries, Blueprint handoff, validation helpers, and open technical questions.
- Linked the new docs from the root `README.md`.

## Why Both Matter

The GDD helps answer: what should the player feel and do?

The TDD helps answer: how are we building that safely in Unreal C++?

Keeping both matters because Nyx is becoming a systems-heavy prototype. Features like save/load reliability, Koi skill investment, merchant upgrades, FTUE prompts, and Starwell rewards need design intent and technical boundaries. The GDD keeps the loop cozy and readable. The TDD keeps the implementation reviewable and testable.

## Testing Or Validation

This devlog documents Markdown-only changes. No Unreal build was required for the documentation commit.

The source work described by the docs was previously validated with `NyxEditor` build and `Automation RunTests Nyx.Gameplay`.

## Portfolio Takeaway

This update shows a more professional project habit: pairing gameplay ideas with technical notes so the repository explains both the creative direction and the engineering approach. It also keeps the WIP status clear instead of pretending the design, tuning, UI, or art are final.
