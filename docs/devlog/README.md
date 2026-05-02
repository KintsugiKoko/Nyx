# Nyx Devlog Workflow

This folder tracks short, portfolio-friendly development notes for Nyx. Each devlog should explain real progress, real decisions, real testing, or real confusion from the project without making the prototype sound finished.

The goal is to turn work into clear evidence: what changed, why it mattered, how it was checked, and what still needs attention.

## When To Write A Devlog

Write a devlog when there is a useful learning or portfolio signal, such as:

- a commit that changes gameplay systems, architecture, validation, or documentation
- a successful or failed PIE test pass
- a bug found through testing or review
- a save/load, UI, Blueprint, data asset, or tuning decision
- a visual direction or art pipeline decision that affects the prototype
- a learning milestone that explains how the project is becoming easier to build or test

Do not write a devlog just to make the project look busier. A small honest update is stronger than a large vague one.

## Evidence To Check First

Before drafting, review real project evidence:

- `git log --oneline` for recent commits
- `git diff --stat` and `git diff --name-only` for uncommitted work
- relevant source files in `Source/Nyx/`
- existing devlogs in `docs/devlog/`
- README sections that may need a link
- any test, PIE, bug, or build notes from the session

If the evidence is only planned work, say it is planned. If the work is untested, say it is untested. If a system is a foundation, prototype, or WIP, use that wording.

## Suggested Devlog Shape

Use this structure for most entries:

```text
# Short Title

Date: YYYY-MM-DD

Status: Work-in-progress prototype documentation

## Overview
What changed or what was learned?

## What Changed
What files, systems, or notes were updated?

## Why It Matters
Why does this matter for Nyx as an Unreal C++ learning project?

## Testing Or Validation
What was checked? If nothing was checked, say that honestly.

## Risks Or Open Questions
What could still break, change, or need review?

## Portfolio Takeaway
What does this show about systems thinking, QA, Unreal C++, documentation, or learning?
```

## Honesty Rules

- Do not claim Nyx is finished.
- Do not claim features are complete unless the repository clearly supports that.
- Do not invent screenshots, videos, tests, assets, or playtest results.
- Do not turn planned systems into implemented systems.
- Use actual class names only when they exist in `Source/Nyx/`.
- Keep Blueprint, UI, VFX, audio, and art language WIP unless finished assets exist.
- Keep test language precise: "manual PIE checklist," "debug validation," "planned smoke test," or "not yet tested" are all acceptable when true.

## Tone

Use a tone that is:

- professional
- honest
- concise
- learning-focused
- portfolio-ready
- specific enough to show real work

Avoid dramatic language, vague hype, or claims that make the prototype sound shipped.

## File Naming

Use this format:

```text
YYYY-MM-DD-short-topic.md
```

Examples:

- `2026-05-02-save-load-reliability-pass.md`
- `2026-05-02-save-load-low-poly-pass.md`
- `2026-05-08-pie-smoke-test-notes.md`

## README Links

After adding a devlog, update the root `README.md` only if it has an appropriate `Devlog`, `Learning Journey`, or `Project Updates` section.

Keep README links short:

```markdown
- [Entry Title](docs/devlog/file-name.md) - one sentence explaining the update.
```

## Weekly Automation Guidance

The weekly devlog assistant should draft updates, not silently publish them. Treat it as a regular learning checkpoint that helps turn real Nyx work into a short portfolio note.

Recommended behavior:

- summarize real commits since the latest devlog
- call out uncommitted changes separately
- suggest a devlog draft only when there is enough evidence
- suggest a commit message
- avoid pushing unless the user explicitly approves the scoped change
- include one clear next step so the project keeps moving

This keeps the devlog useful as a learning record instead of turning it into automatic noise.
