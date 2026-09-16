# Pyra performance optimizations

This build includes the earlier Pyra SDL hot-path optimizations plus additional low-overhead changes aimed at Cortex-A15 + PowerVR SGX GLES2.

## SDL/window path
- Relative mouse mode is changed only when the requested state changes.
- Mouse focus is cached from SDL window enter/leave events instead of querying window flags every gameplay frame.
- Resize handling only triggers expensive resolution setup when the effective window/drawable size actually changes.
- Consecutive SDL_GetTicks() calls in the frame timing path are collapsed to one timestamp read.

## GLES2 render path
- Removed three per-draw `glGetBufferParameteriv(GL_BUFFER_SIZE)` queries. The element count is already known as `GL_tmpTrisAmt * 3`; querying the driver for the buffer size can introduce a synchronous driver round-trip.
- Removed a duplicate set of three `glEnableVertexAttribArray()` calls in the post-processing/simple-texture draw path.

## Deliberately unchanged
- Gameplay still has one present in `rdFinishFrame()` via `Window_SdlVblank()`. `Window_SdlUpdate()` only swaps in the menu/non-DDraw path, so there is no duplicate gameplay swap to remove.
- The SDL_Delay frame limiter is menu-only in the normal SDL update path and therefore was not removed from gameplay.
- No broad GL state cache was added: blindly caching texture/program state across the existing multi-texture renderer risks stale state and rendering regressions. Only demonstrably redundant state/query calls were removed.

## v3: remaining hot-path work
- Consecutive `SDL_MOUSEMOTION` events are coalesced before legacy `WM_MOUSEMOVE` translation. Relative x/y deltas are summed exactly; the final absolute position and timestamp are retained. Event ordering is preserved by flushing before every non-motion event.
- Replaced the oscillating `SDL_Delay` feedback controller with a deterministic remaining-time delay. This removes the per-loop `sampleTime_delay++/--` jitter mechanism.
- Added a render-list-local SGX state cache for texture-unit selection, texture binding, and blend-function changes. The cache is invalidated at the beginning of every world render-list pass, so menu/post-process GL state cannot make it stale.
- The existing renderer already batches adjacent triangles with identical texture/flags into one `glDrawElements` call. v3 keeps this ordering-safe batching. Reordering further across texture/flag boundaries is intentionally not done because transparent/additive surfaces are order-dependent and would change rendering semantics.
- Menu/overlay rendering remains conditional on the existing menu/non-DDraw paths; no gameplay menu draw was found that could safely be deleted.

## v4: additional optimizations beyond the original seven points

- Release optimization level raised from `-O2` to `-O3`. With `-mcpu=cortex-a15 -mfpu=neon-vfpv4`, this gives GCC more scope for Cortex-A15/NEON loop and inlining optimizations. `-ffast-math` is deliberately not enabled because game/physics floating-point semantics should remain intact.
- Extended the world-pass GLES2 state cache to depth function, depth write mask, cull face, `uniform_tex_mode`, and `uniform_blend_mode`. Redundant driver calls are suppressed within the render-list pass.
- Added a material-uniform cache for emissive/albedo/displacement parameters and texture-filter/bloom dependent texture mode. Repeated batches using the same material no longer resend the same material uniform block.
- Removed a no-op forward triangle scan in `std3D_DoTex()`. The loop walked following triangles for emissive textures even though its only mutation had already been commented out; this was pure CPU overhead in a render hot path.
- Existing v2/v3 optimizations remain intact: mouse-state transition caching, resize de-duplication, mouse-motion coalescing, frame pacing cleanup, texture/blend state caching, redundant GL query removal, and redundant vertex-attrib state removal.

The state caches remain deliberately scoped to `std3D_DrawRenderList()` and are invalidated at the start of each world render-list pass. This avoids assuming state ownership across menu/post-processing code.

## v5: CPU/memory/audio hot-path pass

This pass continues outside the original seven-point list while preserving v4 behavior.

* `rdMatrix_TransformPoint34Acc()` no longer copies its input vector through `stdPlatform_Memcpy32()` before every in-place transform. The three source components are captured as scalars so GCC can keep them in Cortex-A15 registers and optimize the multiply/add sequence directly.
* `rdClip_SphereInFrustum()` caches the camera projection type once per sphere test instead of repeatedly dereferencing the current camera for each frustum plane.
* OpenAL 24-bit PCM conversion in `stdSound_BufferUnlock()` is now performed in-place, backwards. This removes a full-size temporary allocation, `memcpy`, `memset`, and `free` for every converted sound buffer and also removes the strict-aliasing-unsafe integer reinterpretation in the old converter.
* The existing v2-v4 GLES synchronization/state reductions remain in place.

### Areas deliberately not claimed as complete

* Dynamic VBO/IBO streaming still uses `glBufferData(..., GL_STREAM_DRAW)`. On the SGX544 driver this already gives the driver an orphan/reallocate opportunity. Replacing it blindly with `glBufferSubData` can introduce GPU/CPU stalls, so a persistent capacity/ring-buffer rewrite needs measurement on the Pyra before being considered an optimization.
* Visibility/culling received a safe hot-path reduction, but the portal/frustum architecture has not been redesigned.
* General `memcpy`/`memset` calls used to copy clipping/output arrays remain where the copies are semantically required.
* OpenAL runtime state queries remain where their answers are needed to maintain queue/playback correctness; the large avoidable PCM conversion allocation/copy path is removed.

## v6: GLES2 dynamic VBO/IBO multi-buffer streaming

The dynamic world and menu/UI vertex/index streams now use a three-slot VBO/IBO ring instead of repeatedly overwriting one GL buffer object.

- Three world VBOs and three world IBOs are generated and rotated once per rendered scene.
- Three menu/UI VBOs and three menu/UI IBOs are rotated for dynamic submissions.
- The previous one-vertex dummy `glBufferData()` in `std3D_StartScene()` was removed.
- Vertex attribute pointers are established while the currently selected ring VBO is bound, preserving GLES2 buffer-object semantics.
- All ring objects, including the menu IBOs, are deleted during resource teardown.
- Uploads remain `GL_STREAM_DRAW` + `glBufferData()`: this intentionally lets the SGX driver choose storage placement while the ring avoids immediately redefining the object used by the most recent GPU submission.

This is GLES2-core compatible and does not require `glMapBuffer`, sync objects, persistent mapping, or GLES3 extensions.
