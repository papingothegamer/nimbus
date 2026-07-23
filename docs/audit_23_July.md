# Nimbus Project Audit - July 23

## Work Accomplished So Far
- Implemented core Timeline logic including basic clip instantiation, UI tracking, and component lifecycle events.
- Created `GroupTrackHeaderComponent` for visually distinguishing grouped tracks, rendering collapsible fold buttons, and customized styling to differentiate from standard tracks.
- Successfully implemented recursive/nested `groupTracks` routing in the `TimelineProject` data model, shifting track locations properly and mapping `trackClips` correctly.
- Addressed multiple synchronization issues between `TimelineProject` events and `TimelineComponent` lane rendering to prevent index out-of-bounds rendering mismatches.
- Resolved an issue in `GroupTrackHeaderComponent` where aggressive width subtraction resulted in a negative `bounds.width`, causing the track name label to be fully hidden.

## Current Problem
The key outstanding issue is visually rendering tracks appropriately when tracks are duplicated *within* a folded or unfolded track group. 
- **The Context**: Duplicating a track within an existing track group triggers `insertTrack` followed by a sequence of `addClipToTrack` loops via `clone()`.
- **The Bug**: Duplicating a track within an existing track group causes the duplicated clip to not render in the timeline UI. The audio engine successfully picks up the cloned `trackClips` and plays audio, but the visual `ClipComponent` instances are not rendered correctly in their lanes.
- **Root Cause & Fix Strategy**: The `TrackLaneComponent` UI logic relies heavily on `trackClipsChanged` firing sequentially after track insertion, but asynchronous `resized()` calls on `TimelineComponent` can temporarily set the new lane's bounds before clips are fully registered, or `trackClips` size mismatches occur before `updateClips` fetches them. We added a defensive render loop via `getNumClipComponents()` mapped directly into the `resized()` listener. When the main Timeline invokes `resized()`, it will verify that the number of `ClipComponent` instances matches the data model. If it doesn't, it forces a safe layout `updateClips()` trigger.

## Next Steps
1. Verify the `updateClipsSafe` fallback fixes the group duplication missing-clip issue natively.
2. Polish any remaining rough edges with `muteButton` mapping, as we have dual logic handling the track number `powerToggle` and explicit `muteButton` controls.
3. Ensure the newly grouped tracks appropriately map to Mixer routing.
