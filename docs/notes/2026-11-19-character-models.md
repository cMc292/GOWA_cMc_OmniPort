# Character models: the mesh and skeleton formats, written back and checked in the game (2026-11-19)

**Correction, later the same day.** The first version of this note took `R_PRISONER00.WAD` for the player's body. It
holds the prisoners behind the bars of the first level; Kratos's body is in `R_HERO00.AS` (below). The format findings
and the tests stand: they were made on the prisoner's mesh, which has the same format, and were repeated on Kratos's.

## Goal

Can a mod replace a character's model? Map the mesh and skeleton formats far enough to write a mesh the game takes,
and check that against the game itself.

## Method

- **The files:** the WAD-format files of `RESOURCES.PSARC`, read with `scripts/gametext.py`'s PSARC and WAD readers.
- **The streams:** Python ports of the EDGE decoders in `native/` ([index decompression](../../native/spu_edge_idx.c),
  [vertex unpacking](../../native/spu_edge_vtx.c), [skinning](../../native/spu_edge_skin.c) and its
  [palette layout](../../native/spu_edge_skinmat.c)), and encoders written as their inverses.
- **The game:** a measurement build (`gowa.py build --jobcap`, in its own tree through `GOWA_BUILD_DIR`) on the fight
  replay of the [parasite-fight probes](2026-11-18-parasite-fight-probes.md), `SPU_JOB_CAPTURE` of w14-16 from 70 s on,
  450 EDGE jobs a run. A job's own inputs are in its local store at its start: the 22-slot DMA list of the
  [output contract](2026-11-04-edge-output-contract.md). A slot's EA, less the mesh's base in memory, is an offset in
  the mesh entry. Screenshots (F12, as `gowa.py bench` takes them) for what the SPU never reads (UVs).
- The study's scripts are kept outside the tree.

## Where the bodies are

- **Kratos: `R_HERO00.AS`**, a WAD-format file of 127 MB (not only animations): 22 costumes or states, each a mesh
  `MG_<hash>_30000_0`, a skeleton named `<hash>`, materials and textures; `R_HERO00.WAD` has a small game object per
  hash. In the first level's fight the game draws parts of three body meshes in the same frame, all three with the same
  18-20 parts (`chest` in 16 segments, `hands`, `legs`, `feet`, `armArmor`, `legArmor`, `Belt`, `beard`, `eyebrows`,
  eyes, teeth, tongue): `kratosBodyDirty` (`81BC3F6F`), `kratosBodyOlive` (`4E90421D`), `kratosBodyDirtyInk`
  (`34AE727F`, two ink parts more); and two skirts with their own skeletons (`kratosSkirtTattered` `FF05E07`,
  `kratosSkirtPristine` `993B9D89`). About 20,300 vertices a body.
- **The prisoners: `R_PRISONER00.WAD`**, `MG_7b771621_0`: 10 parts, 47 segments, about 20,800 vertices; the figures in
  the cells.
- `R_HERO00.WAD` also holds the hero's shared rigid parts (25 meshes), a template skeleton (`heroA`, 261 nodes), the
  animation entry `ANM_heroa` and sound banks. The Blades' chains and other forms are WADs of their own
  (`R_KRATOSCHAINLEFT00`, `R_KRATOSCHAINRIGHT00`, `R_FURYKRATOS00`, `R_KRATOSOLIVEWT00`, `R_KRATOSMP`).

## The WAD

As the [game-text note](2026-11-11-game-text.md) has it: a run of entries, each a 64-byte header (u16 tag, u16 flags,
u32 size, a 56-byte name), the data, padding to 16. Tag 1 is a resource, and its flags field is the resource's type;
tags 2 and 3 open and close a group. The types here:

| type | what |
|---|---|
| 0x75 | mesh (`MG_...`) |
| 0x83 | model (`MDL_...`) |
| 0x77 | material (`MAT_...`): a shader id and its textures' names |
| 0x16 | texture: a DDS |
| 0x2D | game object; the skeleton is one |
| 0x58 | animation (`ANM_...`) |
| 0x13, 0x42, 0x1F, 0x14 | script components, effects, lights, sound emitters |

## A mesh (type 0x75)

- **Header:** `0x0001000D` (in every mesh seen), the size, 0, 0; word 4 the parts' count (high half) and a count of
  bound records (low half); from word 8 the parts' offsets, then the bound records (`0xFFFF0000` and four floats each).
- **A part:** a head (word 5 the skeleton node it belongs to: a mesh node such as `chest` or a joint; bounds as floats;
  a run of `0x0D` bytes), then one or more **sub-blocks**: a run of 0x70-byte segment records, each sub-block with its own
  groups and its own blocks of streams in this order: weights, culling entries, vertices, the RSX-only block, indices
  (each index stream 128-aligned). A sub-block after the first opens with a head of its own.
- **A draw group** (in a part's head or a sub-block's): a header of 0x24 bytes (u16 attribute records, u16 segments,
  u16 1, u16 culling entries, a mask, a word, then self-relative offsets of the joint list, of the attribute records,
  the record size 12, of the first segment record and of the culling entries), one 16-byte entry a segment (u32 vertex
  count, u32 triangles << 8 | hole size, the index stream self-relative, u16 first and u16 count of its culling
  entries), 9 or 10 attribute records (12 bytes: a self-relative offset, a format word, a size) and a u16 list of the
  joints the group uses.
  - The first three attribute records point into the first segment's vertex stream (position +0, normal +6, tangent +9
    in a 12-byte vertex).
  - The others with data form the **RSX-only block**, which the SPU never reads: interleaved records of a stride
    given by the format word's third byte (stride x 2): UVs (`0x0A...`, two F32) alone (stride 8), or with a second UV
    (`0x0B...`, stride 16), with three F16 (`0x11...`, stride 14), or behind four bytes of something else (stride 12, the
    UV at +4). A segment's records start after the earlier segments' vertex counts x the stride. Groups without such
    records (teeth, eyes) have no UVs.
  - A culling entry: 12 bytes not decoded (a quantized box, it seems), the node (its low byte: nodes past 255 keep only
    that), `0x7F`, 0.
- **A segment:** a 0x70-byte record, then its streams. Every field the EDGE job reads is mapped, each one against its
  DMA slot in all captured segments:

| record | what | slot |
|---|---|---|
| +0x00 | EDGE's segment header: flags (0x83-0x88), hole size in 16 bytes (35-44), the formats' bytes (0xFF: the job's), index kind 2 with no skinning flavour (0x20), matrix format 2, vertex count, index count, 0xFFFFFFFF | 13 |
| +0x10, +0x14 | the index stream, self-relative (both the same); +0x18 its size | 5 |
| +0x20 | the vertex stream, self-relative; +0x28 its size (count x stride, to 16) | 11 |
| +0x34, +0x36 | byte offsets of the two palette pieces in the character's joint matrix array (64 bytes a matrix); +0x38, +0x3A their sizes | 6, 7 |
| +0x3C, +0x40 | size and self-relative offset of the skin weights; 0 for a rigid segment | 10 |
| +0x44, +0x4C | the input and output vertex formats, self-relative; +0x50, +0x54 their sizes | 14, 16 |
| +0x58 | the job's buffer size, at most 0xC000 (the job manager's buffer set) | |
| +0x60..+0x68 | three int32: the fixed-point positions' offsets | |

### The streams

- **Indices:** EDGE's compression ([format](2026-11-03-edge-stages.md)). Encoding it again takes the decoder's rules
  backwards, and they are the original tool's:
  - each triangle's code against the one before it: 0 for (a, c, next), 1 for (c, b, next), 2 for (b, a, next), else 3;
  - a 0 bit for a vertex used for the first time (vertices are numbered in first-use order), a 1 bit and its value
    for any other;
  - the values as eight running sums of their differences, h1 = minus the smallest difference (0 if none is below 0),
    k = the bits of the largest difference + h1;
  - h2 = the bitmask's bits in bytes, rounded up; byte 7 = 0; zero padding to 16.
- **Vertices:** EDGE's attribute blocks: the count, the stride and the blocks after the header; per attribute the
  offset, type, components, EDGE id, size, RSX slot and the fixed block's offset; a fixed block gives each component's
  integer and fraction bits (16 in all). The input types:
  - 2: F32;
  - 4: four 8-bit components (id 20, in one of Kratos's forms);
  - 8: 16-bit fixed point, `(raw + offset) / 2^fraction bits`, with the offsets of +0x60;
  - 9: a unit vector in 3 bytes. Two 10-bit components `A (1 + m 2^-23) + B` (A, B = `0x46352E7C`, `0xC6353150`: about
    -0.707 to 0.707), the left-out one rebuilt as `sqrt(1 - c1^2 - c2^2)`. A 4-bit pattern names its axis (pattern / 4),
    its sign (negative for an even pattern) and w (-1 for pattern % 4 < 2; the normals carry a sign there too).
  - The bodies: a type-8 position, a type-9 normal and a type-9 tangent, 12 bytes a vertex.
- **Weights:** 8 bytes a vertex, (weight, index) four times. The weight is /255 and the four sum to 255. The index
  points into the two palette pieces, the first then the second. Empty pairs are (0, 0), at the end.
- **The job's skinning:** `p' = sum of weight/255 x (x q0 + y q1 + z q2 + q3)` over the matrices' rows q0-q3, row
  vectors; a rigid segment goes through slot 18's matrix the same way.

## The skeleton (a type 0x2D entry)

- **Nodes:** the count at +8; from +0x24, 8 bytes a node: flags (0x8000; 0x8020 a joint), first child, next sibling,
  parent (0xFFFF: none). Then 56-byte names, `names[k]` for node k (node 0 the object).
- **Tables:** at +0x14 the offset of a table header (count, count, offset, kind). After the header, a 4x4 a node: a
  rest pose, not the bind pose. At header + offset, the **inverse bind matrices**: row vectors, rigid to 1e-6. Kratos's
  bodies have them with kind 0; the template `heroA` has none.
- **Joints no vertex uses keep an identity inverse bind** (35 in a Kratos body, 43 in the prisoner): the main upper arm
  and upper leg joints among them, whose skin hangs on twist joints (`JO*UpperArm1Twist0-3`, `JO*UpperLeg1Twist0-2`),
  and helpers, IK solvers and contact points. A bind pose for rigging takes those joints' places from their twist
  joints. The earlier "arm palettes keep no bone length" came from comparing against those identities.
- **Palette matrix k is node k:** of the prisoner's 14,830 vertices with two or more bones, 99.9% have their two
  heaviest bones next to each other in the tree (71% parent and child, 29% two steps). Shifting k by one breaks that.
- **The bind pose sits on the mesh:** the joints' positions from the inverse binds lie next to the centroids of the
  vertices weighted only to them; the bodies stand in a T pose (Kratos's head joint 2.165 high, wrists 0.987 out).
- Kratos's body skeletons have 174-176 nodes, the skirts' 298-329, and nodes past 255 do occur.

## Verification

| check | result |
|---|---|
| decode, the prisoner's and `R_HERO00.WAD`'s 167 segments | indices decode to exactly their vertices, none past them, no degenerate triangle; 66,988 unit vectors of length 1, the left-out component the largest but at 215 quantization ties (within 0.002); 20,352 skinned vertices: weights sum to 255, indices inside the palette, 1-4 bones |
| decode against the game (the capture) | positions from the file, skinned with the job's own palette and the file's weights, equal the job's PUTs within 0.00004 (SPU rounding); rigid segments through slot 18 within 0.00002 |
| streams byte for byte, every mesh of the level's WADs (the level, `R_PERM`, Megaera, the parasites, the prisoners, the chains, Kratos's other forms) | indices 2,103 of 2,103 streams (two with a degenerate triangle among them); vertices 2,103 of 2,103 (660,369 vertices); weights 249 of 249 |
| whole meshes rebuilt from their decoded parts (segments filled again, every block packed again, the records, group entries, attribute records and culling entries rewritten) | the prisoner's and Kratos's five (three bodies, two skirts): every triangle back with its positions, UVs and weights |
| written back, in the game | below |

The byte-for-byte vertex check carries each unit vector's stored pattern: an encoder that picks the largest component
itself writes 5,138 of 660,369 vertices differently, all at ties, and the decoder takes those as well. The weights keep
their stored order. That is heaviest first, but for 116 vertices whose two weights differ by 1 (127, 128): the rounding
remainder of a 50/50 split.

**The game tests** (local files, deleted after), all through the [mods folder](2026-11-18-mods-archive-and-text.md):
- The prisoner re-encoded in place, every position +0.05 in y, three segments' weights on one bone: as a whole WAD, as a
  one-step `.PATCH`, and with the FIOS cache (`vfs/cache/BCES01741/fios`, the game's 2 GB cache of raw disc-archive
  blocks) moved aside and put back. Every captured job that read the mesh (41-83 a run) read the written bytes and put
  out the positions computed from the test file within 0.000048.
- The prisoner rebuilt with new segments (the same triangles cut again): 49 jobs on 31 segments match the new layout
  within 0.000042, and the screenshots show it as before.
- A different mesh refilled into Kratos's three bodies in their original layout (each segment within its original
  vertex, index and palette capacity; palettes up to 100 matrices), the skirts emptied: the fight replay ran, the
  player drawn as the new shape and moved by Kratos's animations. Kratos's own textures on the new UVs show through
  their alpha (parts of it vanish), so textures are the next part of such a mod.

**Pitfalls:**
- Twin parts (the prisoner's `bodyMesh00` and `bodyMesh01and02`) have the same self-relative records, so a segment of
  one also "fits" the other at a base off by their distance. Matching jobs to segments needs the mesh's one base in
  memory, taken from a segment that occurs once.
- Kratos's three bodies share many identical parts; which mesh a job read needs the base too, not the first match.

## Laying a mesh out anew

A writer that builds every part from its pieces (part head, descriptors, groups, records, streams) rebuilds Kratos's
five meshes from their own streams **byte for byte**. What it takes, beyond the tables above:

- **The order in a part:** the head; per group (one a sub-block), a 0x38-byte descriptor (its last word the distance to
  the group header), `0x0D` bytes up to the header at a 128-byte boundary of the mesh; the header (0x24), entries,
  attribute records, joint list, zeros to 16; the output format, the input format; the segment records; the weights
  (8 bytes a vertex, to 16; skinned groups only); the culling entries; the vertices (to 16); the RSX-only block; zeros to
  128; the index streams, each at a 128-byte boundary. The next sub-block's descriptor follows the last index stream;
  the next part follows the last sub-block (parts are only 4-aligned).
- **The part head** (0x40 bytes and a table): word 7 the sub-block count (high half); a sphere for the part; for each
  sub-block after the first, a self-relative pointer to its descriptor and its sphere. The descriptor holds a box.
- **Attribute records:** the SPU's are the records that point into the first segment's vertex stream (+0, +6, +9 for a
  body's position, normal, tangent; the hair and eyebrows have only position and normal, so their UV record is the
  third), size the group's vertex bytes; the RSX block's have its size; a record of size 0 points at (the sub-block's
  end - the attribute table) past itself.
- **New segments** take the record of their group's largest segment (flags, hole size, the job's buffer size) and stay
  within its vertex and index counts; a group may have any number of them (three times the original count was tested).
- **The group header's u16 at +0x0E** is the most weights a vertex of the group may have (0 for a rigid group; 2 or 3
  for the feet, the leg armour and the belt; 4 for the rest): no original vertex has more. Vertices written with more
  are pulled out of shape in the game.
- **The joint list** may grow: segments that use joints of other parts' lists, added to their group's list, skin
  right. A weight on a joint that is not in the list and has no ancestor in it must go to a joint near the vertex: the
  lowest-numbered joint of the list can be the other foot's.
- **Materials** seen from the meshes: the beard and eyebrow material (`alpha_m`) samples only a strand mask, its colour
  is the shader's (its float parameters do not change it); the eye gasket material samples the face's colour map and
  the same mask, so geometry drawn with it is cut out and coloured.
- The prisoner's mesh puts its group headers off the 128-byte boundary; the writer does not cover it.

**Where a body is loaded from.** Each body or skirt is a *stream set*: a game object in `R_HERO00.WAD` named by its
hash (type 0x2D: `00040001`, the set's offset in `R_HERO00.AS`, its size, 5, its resident size, then the names
`KratosStreamSet`, `KratosBodySet` or `KratosSkirtSet`, and the body's name such as `kratosbodydirty`). The resident size
is about the set's entries other than textures. `GoStreamInfo` in the same WAD lists the two sets, eleven members each.
The game reads a set at that offset, so a set copied to the end of the `.AS`, with the object pointed there and its
resident size grown with the mesh, loads like the original.

**Limits, measured on the fight replay:**
- A body mesh of 0xB8000 bytes (736 KiB) loads; one of 0xBF000 does not, and the body is then not drawn, with nothing
  in the log. A resident size raised to 0x139370 with a small mesh loads, so the limit is on the mesh.
- The three bodies are not drawn whole: each part is drawn from one of them, and which one changes. The textures bound
  are the dirty body's for all three (only its textures are uploaded in the level).

## What an importer needs

Done, and shown to work in the game: the index, vertex and weight encoders; the record's fields; refilling a mesh's
original layout with new geometry (segments grown over shared vertices in strip order, the fixed-point formats widened
as the geometry needs, culling entries copied per node); the skeleton for rigging (names, tree, bind pose, palette
k = node k); getting the file in through the mods folder.

Done since: a mesh laid out anew (above), its stream set moved, up to the mesh limit.

Still open:
1. **Meshes past the limit:** what sets it (a pool, a buffer) is not found; a bigger figure would need it.
2. **The fields not derived:** the buffer size (+0x58), the hole size and the flags; new segments take their group's
   largest segment's, which hold as long as a segment stays within its counts.
3. **The culling entries' 12 bytes**: copied per node for now (a bigger original shape culls later, never earlier).
4. **Materials:** the original materials with new pictures through the [textures layer](2026-11-18-mods-textures.md),
   or new `MAT_` and texture entries, which are not mapped.
5. **Every form of the character:** one mesh for each body the game draws, in each place it draws them.
6. **Limits outside the files:** a new mesh moves with the character's skeleton and animations, so his proportions stay;
   pre-rendered movies keep the old model.

## Open points

- The part head's bound records and per-part floats, the group header's mask and word, and the RSX format words past
  their stride byte.
- The buffer size, hole size and flags formulas.
- Which material each part draws with.
