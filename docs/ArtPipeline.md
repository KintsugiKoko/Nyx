# Nyx Art Pipeline

Nyx is a work-in-progress Unreal Engine C++ cozy cosmic fishing prototype. The first visual pass uses a simple, bubbly, low-poly style so assets can be built, imported, tested, and revised quickly while still looking intentional in a portfolio.

This document describes the current art direction and first-pass production rules. It is not a final art bible.

## Visual Direction

Nyx should feel like a small toy world under soft cosmic light.

Style pillars:

- Simple: readable forms, low detail density, clean silhouettes.
- Bubbly: rounded edges, soft corners, inflated forms, friendly proportions.
- Low-poly: visible broad planes are welcome, but shapes should still feel cared for.
- Cozy cosmic: moonlit blues, soft teals, gentle violets, pearl whites, warm wood, and small glow accents.
- Toy-like: rough plastic, painted wood, soft ceramic, felt-like cloth, and gentle emissive details.
- Gameplay-first: every object should read clearly from the third-person camera.

Avoid realistic detail, noisy textures, horror-heavy shapes, sharp sci-fi plating, and overly complex materials during this pass.

## Shape Language

Use chunky silhouettes and broad forms before adding detail.

Good base shapes:

- Squashed spheres
- Capsules
- Beveled cubes
- Rounded cones
- Thick cylinders
- Bean-shaped bodies
- Oversized paws, fins, rims, handles, and props

Readable proportions matter more than anatomical accuracy. A fish, cat, Starwell, dock plank, or fishing rod should be identifiable in grayscale before color or glow is added.

## Color And Material Goals

Use a soft cosmic palette with warm cozy anchors.

Suggested color zones:

- Night sky blue and soft violet for cosmic surfaces.
- Moonlit teal for magical glow and Starwell energy.
- Pearl, cream, and pale blue for highlights.
- Warm wood for dock pieces and handmade tools.
- Soft black and warm white for the tuxedo-pattern cat.
- Rarity accents for fish, used sparingly and consistently.

Material goals:

- Prefer simple material instances over complex shader work.
- Use broad color regions instead of tiny texture detail.
- Keep roughness high for most toy-like surfaces.
- Use emissive only for important read points: Starwell basin, fish glow marks, rarity accents, magical offering moments.
- Do not let glow replace silhouette. The object should still read when emissive is disabled.

## Tuxedo-Pattern Cat Guidelines

The player character is a cat with natural tuxedo-pattern fur, not a cat wearing a tuxedo.

Design goals:

- Simplified low-poly cat with a rounded head, bean body, chunky paws, and expressive tail.
- Natural black-and-white markings: black head/back/tail areas, white muzzle, chest, belly, and paws.
- Avoid suit collars, bow ties, shirt fronts, lapels, buttons, or clothing cues unless they are added later as separate optional cosmetics.
- Keep the head slightly oversized for charm and readability.
- Make eyes large enough to read from the gameplay camera.
- Preserve clean material slots, such as `M_Cat_Black`, `M_Cat_White`, and `M_Cat_Accent`.

Unreal hookup notes:

- Use `ANyxCatPlayerCharacter` as the source-code base for future player mesh, fishing rod socket, offering socket, and animation cue integration.
- Player collision should come from the Character capsule, not detailed mesh collision.
- Fishing rod and offering props should attach through named sockets rather than being baked into the mesh.

## Starwell Guidelines

The Starwell should be a chunky, round, cosmic basin that feels ancient but friendly.

Shape goals:

- Wide rounded basin with a thick rim.
- Squat pedestal or soft stone base.
- Shallow glowing interior.
- Simple moon, star, or carved groove details.
- Strong circular silhouette that reads from above and at third-person camera distance.

Visual states to support:

- Idle pulse: slow basin glow or gentle breathing light.
- Offering accepted: short pulse from the basin center or `OfferingPoint`.
- Threshold reached: stronger burst, upward sparkle, or ring pulse.
- Post-load restored: subtle shimmer that refreshes presentation without implying a new reward.

Unreal hookup notes:

- `AStarwell::VisualMesh` receives the mesh in Blueprint or a child class.
- `AStarwell::InteractionCollision` handles player interaction overlap.
- `AStarwell::OfferingPoint` marks where offered fish or props move before conversion.
- `AStarwell::VfxSpawnPoint` marks idle, offering, threshold, and restore VFX placement.
- Keep gameplay events separate from restoration presentation events:
  - Gameplay reward: `OnFishAccepted`, `OnOfferingThresholdReached`
  - Presentation refresh after load: `OnStarwellStateRestored`

## Fish Guidelines

Fish should be simple, rounded, and instantly readable.

Silhouette rules:

- Use capsule or bean bodies.
- Use large simple tail fins.
- Keep side fins and dorsal fins broad and low-detail.
- Use oversized eyes or face marks for charm.
- Make rarity readable through shape and accent color, not text alone.
- Avoid thin fins, noisy scales, or realistic anatomy in the first pass.

Rarity direction:

- Common: clean shapes, one glow mark or simple color contrast.
- Uncommon: stronger accent color, extra fin shape, small pattern.
- Rare: more distinctive silhouette, brighter glow, unique tail or crest.
- Legendary or special: unusual silhouette, animated glow zones, stronger cosmic motifs.

Unreal hookup notes:

- Gameplay tuning stays in `UFishDataAsset`.
- Visual presentation references should stay optional and separate from gameplay values.
- Data assets can point to fish meshes, materials, or icons later, but the C++ gameplay systems should not require final art.

## Animation Style

Animation should feel soft, snappy, and a little squishy.

Principles:

- Use small anticipation before casts, offerings, and reward moments.
- Use squash and stretch lightly, especially for fish bobbing and cat celebration.
- Favor readable loops over realistic motion capture.
- Keep timing clear for gameplay: cast, bite, reel, catch, offer, reward.
- Restoration animations should look like UI or world refresh, not a second reward grant.

First-pass animation targets:

- Cat idle breathing, walk/run, cast anticipation, reel loop, catch success, offering handoff.
- Starwell idle pulse, offering accepted pulse, threshold reached burst, post-load restored shimmer.
- Fish idle wiggle, caught bob, offering float.
- Dock optional gentle bob if it becomes floating.
- Fishing rod socket-follow first, bend or secondary motion later.

## Unreal Import Notes

General import rules:

- Export FBX from Blender at scale 1.0.
- Work with Unreal scale in mind: 1 Unreal unit = 1 centimeter.
- Apply transforms before export.
- Keep pivots deliberate before import.
- Use simple material slots with clear names.
- Prefer simple collision for blockouts.
- Do not import paid or restricted assets into public project folders unless licensing is confirmed.

Recommended prototype content location:

- `Content/Art/Prototype/`

Recommended source-art location, if committed:

- `SourceArt/`

Only commit source-art files if they are yours, small enough for the repo, and intended for public sharing.

## Pivots And Collision

Pivot rules:

- Cat: root at ground center between feet or body center, depending on rig setup.
- Starwell: bottom center of the pedestal or basin footprint.
- Fish: body center for swimming, bobbing, and offering animations.
- Dock pieces: floor center or grid corner, based on modular assembly needs.
- Fishing rod: grip point where the cat socket attaches.
- Card/deck table props: floor center.

Collision rules:

- Cat uses Character capsule collision.
- Starwell interaction uses `AStarwell::InteractionCollision`.
- Starwell visual mesh can use no collision or simple collision.
- Dock uses simple box collision for walkable surfaces.
- Fish usually need no collision for the first pass unless spawned interactively.
- Fishing rod needs no collision unless it becomes a pickup.

## Naming Conventions

Use Unreal-friendly names with stable prefixes.

Common prefixes:

- `SM_` for static mesh
- `SK_` for skeletal mesh
- `SKEL_` for skeleton
- `PHYS_` for physics asset
- `M_` for material
- `MI_` for material instance
- `T_` for texture
- `A_` for animation sequence
- `ABP_` for animation Blueprint
- `BP_` for Blueprint
- `DA_` for Data Asset
- `NS_` for Niagara system

Example names:

- `SK_Cat_Tuxedo_Blockout`
- `SM_Starwell_Blockout`
- `SM_Fish_GlowMinnow_Blockout`
- `SM_Dock_Platform_Blockout`
- `SM_FishingRod_Blockout`
- `SM_CardDeck_Table_Blockout`
- `DA_Fish_GlowMinnow`
- `MI_Starwell_BasinGlow`
- `NS_Starwell_OfferingAccepted`

## First Prototype Asset Targets

Build only enough to prove the direction in-engine:

- Tuxedo-pattern cat blockout
- Starwell blockout
- Glow Minnow fish
- Small dock platform
- Simple fishing rod
- One offering animation or VFX beat

The first pass should answer: "Does Nyx read as a cozy cosmic fishing game from the gameplay camera?"

## What Not To Commit Publicly

Do not commit:

- Paid Marketplace assets unless the license explicitly allows public redistribution.
- Third-party Blender, texture, audio, or animation source files without redistribution rights.
- Private credentials, API keys, tokens, account IDs, or personal machine paths.
- Launcher cache files or generated folders such as `Binaries`, `Intermediate`, `Saved`, and `DerivedDataCache`.
- Temporary export files, backup files, or large unreviewed binaries.
- Binary Unreal assets that are not intended for the public repository.

Binary Unreal assets such as `.uasset` and `.umap` files should be handled carefully. For a public GitHub portfolio, prefer documenting the pipeline and showing screenshots or short clips unless the asset is original, lightweight, and safe to redistribute.
