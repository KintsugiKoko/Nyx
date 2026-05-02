# Nyx Technical Design Documents

This folder holds technical design documents for Nyx. These notes explain how gameplay systems are implemented or intended to be implemented in Unreal C++.

Nyx is still a work-in-progress prototype. TDD notes should describe actual source state, known gaps, validation coverage, and future implementation plans.

## Documents

- [Core Systems TDD](Nyx_Core_TDD.md) - technical overview for fishing, Starwell, economy, save/load, FTUE, merchant, and Koi skill systems.

## TDD Rules

- Use actual class and file names.
- Separate implemented behavior from planned behavior.
- Include save/load and Blueprint API implications.
- Include validation and test notes.
- Do not imply binary assets or Blueprint content exist unless they do.
