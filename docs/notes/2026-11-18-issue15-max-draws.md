# Issue #15: the parasitic hand's frames outgrew MAX_DRAWS (2026-11-18)

The reporter's alpha.15 report (`report-20261117-114455`, Ryzen 5 5600X, RTX 5060, render scale 2, SMAA) ended right
as the parasitic hand comes up at the end of the first level, with "a microscopic performance dip as it emerges" and
no shader notice.

## What the report shows

- **Two frames past the batch's room.** `[D3D12] a batch outgrew MAX_DRAWS (8192): 247 draws dropped` and `171 draws
  dropped`, one flip each, over 40 and 39 ms: frames of 8,439 and 8,363 draw records, the first to reach the limit in
  a report since alpha.15 says so. The draws past 8,192 were left out of those two frames.
- **Where.** The log's file opens line up one to one with the pad recording's (3,226 each), which has the time: the
  two batches were 99.88-99.89 minutes into the session, its last seven seconds, after `R_HECAHANDL00` and
  `HECAAI_BOSS01` were read and a cinematic had played at 64 flips a second. Those seconds ran at 29-55 flips a
  second, the lowest where the two batches were. No shader was built there.
- Not this: the shader waits of that session (512 by 4.5 minutes, 15.3 s in all) were all in its first minutes, in
  the first place played.

## Why not only a larger MAX_DRAWS

The per-draw vertex constants (vp_c[512], the viewport epilogue, the raw-fetch table) are a slot of 8,960 bytes in an
upload heap, twice over for the two parities, and a record took the slot at its own index: 140 MB at 8,192 records,
280 MB at 16,384. But a draw already shares the last written slot when its constants are the same (`vp_cb_commit`,
five records in six at the gate), so most of those slots were never written.

## What changed

- **MAX_DRAWS 16384.** A record is 1,560 bytes: the batch's records, the texture view keys and the SRV heap's draw
  windows (262,144 descriptors) grow with it, and are touched only as far as a batch goes.
- **Constant slots of their own, VP_CB_SLOTS 8192 a half.** `vp_cb_commit` takes the half's next slot (`s_cb_next`,
  from the first again at each batch, `vpass_reset`) only for constants that differ from the last written ones; the
  root CBV and the debug readers address `parity * VP_CB_SLOTS + cb_slot`. The buffer stays 140 MB.
- **b1 stays 16 MB a half** (`FPCB_HALF`, now `MAX_DRAWS * 1024`): 512 bytes a slot at least, a full half says so.
- **A batch out of slots** drops the draw (its FG record too), clears the merge anchor so the next draw does not
  extend another record, and the batch's end says `[D3D12] a batch ran out of vertex constant slots`.
- **A batch past 8,192 records** says `[D3D12] the largest batch so far: <draws>, <slots> of the 8192 vertex constant
  slots, ...` at each new most, so the next report from that scene shows how big it gets.
- Knobs: `D3D12_BATCH_NOTE=<records>` sets that line's threshold; `D3D12_CB_SLOTS=<n>` takes fewer slots, to try the
  end of them.

## How it was tried

The parasite-fight replay (`build\probes\fight.pad` from `saves-start`, 125 s, windowed, as in
`2026-11-18-parasite-fight-probes.md`), one run each:

- **The new build, `D3D12_BATCH_NOTE=0`:** `[VPCB] per-draw constant buffer: 140 MB (8192 slots a batch, 16384
  draws)`. The largest batch was 4,565 draws in 355 slots, so a batch of 16,384 records would need about 1,300 at that
  rate. No draw dropped, b1 never full; three screenshots at 70, 85 and 100 s drew the street, the HUD and the
  courtyard as before.
- **`D3D12_CB_SLOTS=200`, to run out of them:** from the street on, over 512 batches did, each dropping 50-325 of its
  2,300-3,400 records, and the game played on to the replay's end and closed as usual. A batch drops its end, the
  composite and the HUD among it: one screenshot was black, the other the blurred scene with nothing over it. No draw
  came out with another's constants. So running out of slots costs frames, as running out of records does; at 355
  slots for the heaviest batch, 8,192 are not met.
- A frame that outgrew MAX_DRAWS dropped its end the same way, so the reporter's two frames most likely lost their
  composite and HUD, not some objects.
