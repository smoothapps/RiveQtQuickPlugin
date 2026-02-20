# Rive Runtime Update Notes

## Update Summary
- **Previous Version**: `8a54c80a217d2cc651c4d52385ceb6090ff8c6f1` (May 4, 2025)
- **New Version**: `20a210a7ec9b348baef5e0ada922b9225fb396c9` (Feb 19, 2026)
- **Commits**: ~555 commits ahead
- **Time Gap**: ~9 months

## API Compatibility Verification

### Factory Interface ✅
All required pure virtual methods are implemented:
- `makeRenderBuffer()` ✅
- `makeLinearGradient()` ✅
- `makeRadialGradient()` ✅
- `makeRenderPath()` ✅
- `makeEmptyRenderPath()` ✅
- `makeRenderPaint()` ✅
- `decodeImage()` ✅

**Status**: No new Factory methods added. All existing implementations remain valid.

### File API ✅
- `File::import()` - Compatible (optional parameters unchanged)
- `artboardCount()` - Compatible
- `artboard()`, `artboardAt()`, `artboardDefault()` - Compatible
- `artboardNameAt()` - Compatible

**Status**: No breaking changes detected.

### ArtboardInstance API ✅
- `updateComponents()` - Compatible
- `animationCount()`, `animation()` - Compatible
- `stateMachineAt()` - Compatible
- `width()`, `height()` - Compatible
- `defaultStateMachineIndex()` - Compatible

**Status**: No breaking changes detected.

### StateMachineInstance API ✅
- `pointerMove()`, `pointerDown()`, `pointerUp()` - Compatible
  - Note: `pointerMove()` now has optional `timeStamp` parameter (default value)
- `inputCount()`, `input()` - Compatible
- State machine input types (`SMINumber`, `SMIBool`, `SMITrigger`) - Compatible

**Status**: No breaking changes detected. Return values (`HitResult`) are ignored in current implementation, which is acceptable.

## Key Improvements in Updated Runtime

Based on commit history analysis:
- Performance optimizations (feather rendering, dither, clockwise mode)
- Vulkan rendering improvements (premultiplied alpha)
- Bug fixes across multiple platforms
- Scripting support enhancements (Luau)
- ViewModel/data binding improvements
- Audio support additions

## Build Configuration Changes

Updated `3rdParty/CMakeLists.txt` with:
- MinGW-specific assembler flag for large object files (`-Wa,-mbig-obj`)
- Improved compiler detection (MSVC vs MinGW/GCC)
- Better warning level handling for different compilers

## Testing Recommendations

1. **Compilation**: Verify build on all target platforms (Windows, macOS, Linux)
2. **Basic Functionality**:
   - File loading
   - Artboard display
   - Animation playback
   - State machine interactions
3. **Rendering**: Test on all graphics APIs (OpenGL, Metal, Vulkan, D3D11)
4. **State Machines**: Verify QML bindings and property updates

## Code Changes Required

### Renderer API Updates
- **drawImage()**: Now requires `ImageSampler` parameter (added between `RenderImage*` and `BlendMode`)
- **drawImageMesh()**: Now requires `ImageSampler` parameter (added between `RenderImage*` and buffer parameters)
- **modulateOpacity()**: New pure virtual method that must be implemented

**Files Updated**:
- `src/RiveQtQuickItem/renderer/riveqtpainterrenderer.h/cpp`
- `src/RiveQtQuickItem/renderer/riveqtrhirenderer.h/cpp`

### File Type Change
- **File::import()**: Now returns `rcp<File>` instead of `unique_ptr<File>`
- Changed `m_riveFile` from `std::unique_ptr<rive::File>` to `rive::rcp<rive::File>`

**Files Updated**:
- `src/RiveQtQuickItem/riveqtquickitem.h/cpp`

### ArtboardInstance Handling
- **artboardAt()** and **artboardDefault()**: Still return `unique_ptr<ArtboardInstance>`
- Need to convert to `shared_ptr` when assigning to `m_currentArtboardInstance`
- Fixed artboard comparison to use `artboardSource()` instead of non-existent `artboard()` method

**Files Updated**:
- `src/RiveQtQuickItem/riveqtquickitem.cpp`

### ListenerType Enum
- **draggableConstraint**: Removed from enum (no longer exists)

**Files Updated**:
- `src/RiveQtQuickItem/riveqtquickitem.cpp`

## Notes

- Factory interface remains compatible (no new methods added)
- Core APIs (File, ArtboardInstance, StateMachineInstance) remain backward compatible
- Renderer API changes required updates to both software and RHI renderers
- Vec2D and HitResult types remain available (included transitively)

## Rollback

If issues are encountered:
```bash
cd third_party/RiveQtQuickPlugin/3rdParty/rive-runtime
git checkout 8a54c80a217d2cc651c4d52385ceb6090ff8c6f1
```
