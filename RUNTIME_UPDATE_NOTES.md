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

## Notes

- No code changes required in RiveQtQuickPlugin implementation
- All APIs remain backward compatible
- Factory abstraction successfully shields plugin from internal rendering changes
- Vec2D and HitResult types remain available (included transitively)

## Rollback

If issues are encountered:
```bash
cd third_party/RiveQtQuickPlugin/3rdParty/rive-runtime
git checkout 8a54c80a217d2cc651c4d52385ceb6090ff8c6f1
```
