# Warming the driver's shader cache ahead of play: what it would take (2026-11-18)

With the vertex and pixel shaders built ahead (the precompile screen), what still costs on a first visit is the
driver: a pipeline the game has never made on this PC is compiled by it the first time. SCSKiller (a GPL-3.0 tool for
PC games, github.com/BlueHeisenberg/SCSKiller) warms the driver's cache before play by making each shader's pipelines
once, on the claim that the driver keeps a stage apart from the rest of a pipeline. Measured here, on the RX 6700 XT
(AMD's driver of 2026-11), with a temporary build (not kept) that makes each of a store's pipelines first in another
form, then as they are.

## The driver's cache on this PC

- Its files are `%LOCALAPPDATA%\AMD\DxcCache\<bucket>.<driver>.<kind>.*.parc`. Ascension-R.exe has a bucket of its own;
  copies of the exe under other names all shared one other bucket, not one a name: renaming the exe gives a cold cache
  once, not each time. The tests emptied that bucket before each run (its files moved aside, Ascension-R.exe's not
  touched).
- **A pipeline new to the driver costs some 60 ms**, not the 0.7 ms seen before (that was with its cache warm): the
  474 pipelines of the fight replay's store took 28,229 ms on six threads cold, 35 ms warm.
- **Its compiles run one at a time**: eight threads took 23.7 s where six took 25.7; two processes, half each, took
  41 s together. More threads or processes do not help.

## What warms it

The 474 pipelines (222 vertex programs, 284 pixel programs), built cold after each warm:

| warm first | the warm | the pipelines then |
| --- | --- | --- |
| none | - | 28,229 ms |
| each pipeline with other blend, depth and rasterizer state | 27,980 ms | 268 ms |
| the same, its input layout's formats and target formats changed too | 28,067 ms | 299 ms |
| each pixel program with one vertex program and state | 3,375 ms | 25,858 ms |
| each vertex program with one pixel program | 25,697 ms | 2,663 ms |
| each vertex and each pixel program, apart | 28,264 ms | 717 ms |

- The driver keeps each stage apart from the rest: neither the fixed-function state nor the formats are part of its
  key, and a stage built with any partner serves every pipeline that has it. Both stages warmed: 40 times faster.
- **The vertex programs are the cost**: some 107 ms each, against 12 ms for a pixel program. Warming them alone takes
  91% of the time away.
- The game uses both kinds of vertex program: of the 474 pipelines, 253 read the guest's vertex bytes (103 programs)
  and 221 have an input layout (119 programs).

## What warming the whole game would cost here

- The archives' 3,455 vertex programs, in both kinds, are 6,910 programs: some 12 minutes, one at a time. Their 39,598
  pixel programs: some 8 minutes. Some 20 minutes in all, or 12 for the vertex programs alone; 1 to 1.5 GB of driver
  cache (the bucket grew some 70 KB a stage).
- Again after every driver update, which throws the cache away.
- A vertex program with an input layout needs one that its input signature takes (read from its DXIL); one that
  reads the guest's bytes has none.
- A warm process must not be killed: the driver writes its cache behind the creates.

## Not known

- NVIDIA and Intel: SCSKiller says NVIDIA's cache is a stage's too, but how long its compiles take, and whether they
  run in parallel, is not measured (no NVIDIA GPU here). Issue #15's log had 7 ms a pipeline there.
- Why a vertex program costs the driver nine times a pixel program, though its DXIL is smaller: if a cheaper text
  compiled faster, every first visit would gain without any warming.
