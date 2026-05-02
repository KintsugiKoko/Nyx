# Nyx First-Pass Low-Poly Art Pipeline

Nyx is using a simple, bubbly, low-poly visual style for the first visual prototype pass. This is work-in-progress art direction for learning, iteration, and portfolio presentation; it is not final production art.

The goal is to make assets readable in Unreal while staying small enough to model, import, test, and revise quickly.

## 1. Visual Style Pillars

- Rounded shapes: avoid sharp realism; favor softened cubes, spheres, capsules, and bevelled forms.
- Chunky silhouettes: assets should read from a gameplay camera before details matter.
- Toy-like materials: use simple rough materials, soft gradients, and broad color zones.
- Cozy cosmic palette: deep blue, soft violet, moonlit teal, warm cream, pearl, and small star-glow accents.
- Squishy motion: animations should feel light and responsive, with gentle squash/stretch where appropriate.
- Honest prototype finish: blockouts can be charming without pretending to be final art.

Important identity note: the player is a tuxedo-pattern cat with natural black-and-white markings, not a cat wearing a tuxedo.

## 2. Blender Modeling Guidelines

General setup:

- Use meters in Blender and export FBX at scale 1.0.
- Model around Unreal scale: 1 Unreal unit = 1 centimeter.
- Keep geometry low-poly but intentionally shaped. Use bevels or extra loops only where they improve silhouette.
- Apply transforms before export: `Ctrl+A > Rotation & Scale`.
- Keep origins/pivots deliberate before export.
- Name objects clearly before export so Unreal imports are readable.

Asset plans:

### Tuxedo-Pattern Cat Blockout

- Shape breakdown: rounded head, bean-shaped torso, short chunky legs, simple tapered tail, triangular ears, large readable paws.
- Approximate proportions: 90-110 cm tall if bipedal, or 45-60 cm shoulder height if quadruped. Head should be slightly oversized for charm.
- Material/color zones: black back/head patches, white muzzle/chest/paws/belly, soft pink nose/inner ears, warm eye color.
- Pivot placement: root at ground center between feet or under body center.
- Unreal scale notes: import as skeletal mesh if animated; keep socket names aligned with `ANyxCatPlayerCharacter`.
- Collision notes: player collision should come from the Character capsule, not detailed mesh collision.
- Animation needs: idle breathing, walk/run, cast anticipation, reel loop, catch celebration, offering handoff.
- Export notes: export mesh and armature together for skeletal tests; keep material slots named `M_Cat_Black`, `M_Cat_White`, `M_Cat_Accent`.

### Starwell Blockout

- Shape breakdown: chunky round basin, thick rim, squat pedestal, shallow glowing interior, small moon/star motifs.
- Approximate proportions: 180-220 cm diameter, 90-130 cm tall.
- Material/color zones: stone or ceramic outer shell, pearly inner basin, glowing teal/violet center, small gold or moonlit accents.
- Pivot placement: bottom center of the basin pedestal.
- Unreal scale notes: assign the mesh to `AStarwell::VisualMesh`.
- Collision notes: use `AStarwell::InteractionCollision` for interaction; mesh can keep simple/no collision.
- Animation needs: idle pulse, offering accepted pulse, threshold reached burst, post-load restored shimmer.
- Export notes: export static mesh as `SM_Starwell_Blockout`; keep VFX timing in Blueprint, not baked into mesh.

### Glow Minnow Fish

- Shape breakdown: rounded capsule body, simple tail fin, two side fins, tiny dorsal fin, large readable eyes.
- Approximate proportions: 25-40 cm long.
- Material/color zones: soft teal body, pale belly, glowing stripe or dots, rarity accent color.
- Pivot placement: body center for swim wiggle and catch presentation.
- Unreal scale notes: assign as `UFishDataAsset::PresentationMesh`; gameplay tuning stays in the Data Asset fields.
- Collision notes: no detailed collision needed for first pass; use simple sphere/capsule if spawned interactively.
- Animation needs: idle wiggle, caught bob, offering float.
- Export notes: export as `SM_Fish_GlowMinnow_Blockout`; material slots can be `M_Fish_Body`, `M_Fish_Glow`, `M_Fish_Eye`.

### Small Dock Platform

- Shape breakdown: chunky planks, rounded posts, simple rope loops, soft bevels.
- Approximate proportions: 400-600 cm long, 250-350 cm wide.
- Material/color zones: warm wood, slightly darker plank sides, soft rope tan, subtle moonlit edge tint.
- Pivot placement: center of the platform footprint at floor height.
- Unreal scale notes: build as modular pieces if the dock will expand later; one combined blockout is fine for the first pass.
- Collision notes: use simple box collision for walking surface.
- Animation needs: optional soft bobbing if it becomes a floating dock.
- Export notes: export as `SM_Dock_Platform_Blockout`; keep plank modules separate if iteration is easier.

### Simple Fishing Rod

- Shape breakdown: tapered curved rod, chunky handle, simple reel cylinder, short line guide shapes.
- Approximate proportions: 120-160 cm long for a small stylized cat.
- Material/color zones: dark rod, warm handle, small metallic reel, optional glowing lure tip.
- Pivot placement: handle grip point where it attaches to `FishingRodSocketName`.
- Unreal scale notes: test attached to `ANyxCatPlayerCharacter::FishingRodSocketName`.
- Collision notes: no collision needed for first pass unless used as a world pickup.
- Animation needs: follows cat skeletal animation; optional bend via material, control rig, or simple skeletal setup later.
- Export notes: export as `SM_FishingRod_Blockout`; keep handle aligned with +X or the project’s chosen socket-forward convention.

### Card/Deck Table

- Shape breakdown: small round or square table, chunky legs, simple card tray, cosmic cloth inset.
- Approximate proportions: 150-220 cm wide depending on camera framing.
- Material/color zones: dark wood or blue-painted surface, soft cloth center, moon/star trim.
- Pivot placement: floor center.
- Unreal scale notes: can become a UI diorama prop for `UDeckComponent` later.
- Collision notes: simple box collision if placed in the world.
- Animation needs: card flip, draw pulse, discard swirl later.
- Export notes: export as `SM_CardDeck_Table_Blockout`.

## 3. Unreal Import Guidelines

- Import FBX files into a clearly named prototype folder, for example `Content/Art/Prototype/`.
- Keep source Blender files outside Unreal or in a source-art folder only if the repo policy allows it.
- Use soft references in data assets where possible. `UFishDataAsset` presentation mesh/material/icon fields are optional and should not drive gameplay.
- Assign Starwell meshes in a Blueprint child of `AStarwell`, not in C++.
- Use `ANyxCatPlayerCharacter` as the future native base for cat mesh, sockets, and animation cue hookup.

## 4. Naming Conventions

Suggested prefixes:

- Static mesh: `SM_`
- Skeletal mesh: `SK_`
- Skeleton: `SKEL_`
- Physics asset: `PHYS_`
- Material: `M_`
- Material instance: `MI_`
- Texture: `T_`
- Animation sequence: `A_`
- Blueprint: `BP_`
- Data Asset: `DA_`

Examples:

- `SK_Cat_Tuxedo_Blockout`
- `SM_Starwell_Blockout`
- `SM_Fish_GlowMinnow_Blockout`
- `SM_Dock_Platform_Blockout`
- `SM_FishingRod_Blockout`
- `DA_Fish_GlowMinnow`

## 5. Collision And Pivots

- Use simple collision for blockouts. Prefer boxes, capsules, and spheres over generated complex collision.
- For the cat, use the Character capsule.
- For `AStarwell`, use `InteractionCollision` for player overlap and keep the visual mesh collision simple or disabled.
- Pivot world props at floor center unless there is a strong reason not to.
- Pivot held props, such as the fishing rod, at the grip point.
- Pivot fish at body center for bobbing, swimming, and offering animations.

## 6. Materials

- Start with flat colors and rough surfaces.
- Use broad readable zones instead of tiny texture detail.
- Glow should be used sparingly: fish accents, Starwell basin, rarity cues, and offering moments.
- Use material instances in Unreal for quick color tuning.
- Keep gameplay rarity readable without relying only on text.

## 7. Animation Expectations

First-pass animation should prioritize readable state changes:

- Cat: idle breathing, cast anticipation, reel loop, catch success, offering handoff.
- Starwell: idle pulse, offering accepted pulse, threshold reached burst, post-load restored shimmer.
- Glow Minnow: swim wiggle, caught bob, offering float.
- Dock: optional gentle bob if floating.
- Fishing rod: attach to cat socket first; rod bending can come later.

Animation events can call `ANyxCatPlayerCharacter` cue functions or bind to gameplay events from `UFishingComponent` and `AStarwell`.

## 8. Actor Integration Notes

- `ANyxCatPlayerCharacter` owns `UFishingComponent`, `UEconomyComponent`, and `UDeckComponent` and exposes socket names for rod and offering props.
- `AStarwell` owns `VisualMesh`, `InteractionCollision`, `OfferingPoint`, and `VfxSpawnPoint`.
- `UFishDataAsset` owns fish gameplay tuning plus optional presentation references.
- `ANyxGameplayDebugActor` can validate save/load and gameplay loops in PIE without final assets.
- Keep one-time gameplay events separate from restoration events:
  - Gameplay: `OnFishAccepted`, `OnOfferingThresholdReached`, `OnCatchCompleted`
  - Restore/presentation refresh: `OnFishingStateRestored`, `OnStarwellStateRestored`, economy/deck save-applied events

## 9. What Not To Commit Publicly

- Paid third-party assets unless the license explicitly allows redistribution.
- Marketplace source files or vendor content that should stay private.
- Binary Unreal assets that are not intended for public release.
- Personal machine paths, private credentials, API keys, or launcher cache files.
- Large generated folders such as `Binaries`, `Intermediate`, `Saved`, and `DerivedDataCache`.

Binary Unreal assets and paid third-party assets should be handled carefully. Public GitHub documentation can describe the pipeline and show screenshots later, but redistribution rights need to be checked before committing content.
