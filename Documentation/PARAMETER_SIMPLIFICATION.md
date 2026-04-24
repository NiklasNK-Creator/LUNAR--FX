# LUNAR.SNITCH Parameter Simplification

## Overview

LUNAR.SNITCH has been simplified to use only **1 essential parameter** instead of 4 separate controls. This makes the effect easier to use while maintaining all enhancement capabilities.

## Parameter Changes

### Before (4 Parameters)
1. **SNITCH Intensity** (0-100) - Main effect intensity
2. **Chromatic Shift** (0-50) - Chromatic aberration control
3. **Anime Contrast** (50-150) - Contrast enhancement
4. **Anime Glow** (0-100) - Glow effect

### After (1 Parameter)
1. **Intensity** (0-100) - Single control that scales all effects proportionally

## How It Works

The single **Intensity** parameter now automatically calculates:
- **Chromatic aberration**: Scaled at 30% of intensity
- **Contrast boost**: Calculated as `1.0 + (intensity * 0.4)`
- **Glow effect**: Scaled at 60% of intensity

This means:
- At 0% intensity: No effect
- At 30% intensity: Subtle enhancement (default)
- At 100% intensity: Maximum enhancement

## Benefits

1. **Simpler UI**: Only one slider to adjust
2. **Faster workflow**: No need to balance multiple parameters
3. **Consistent results**: All effects scale together proportionally
4. **User-friendly**: Easier for users to understand and use

## Code Changes

### Header File (LUNAR_SNITCH.h)
- Removed `SNITCH_CHROMATIC`, `SNITCH_CONTRAST`, `SNITCH_GLOW` enum values
- Removed chromatic, contrast, glow parameter ranges
- Removed chromatic, contrast, glow disk IDs
- Simplified `SnitchInfo` struct to only include `intensityF`

### Source File (LUNAR_SNITCH.cpp)
- Simplified `ParamsSetup()` to only add Intensity parameter
- Updated `ApplySnitchEffect8()` to calculate derived values from intensity
- Updated `Render()` to only read intensity parameter
- Updated `PreRender()` to only checkout intensity parameter
- Updated description to reflect "video editing" instead of "anime editing"

## Usage

Users now simply:
1. Apply LUNAR.SNITCH to footage
2. Adjust the single **Intensity** slider
3. Done - all enhancement effects scale together

## Versatility

The effect is now suitable for:
- Anime music videos (AMVs)
- General video editing
- Color grading
- Cinematic enhancement
- Any video content that benefits from subtle enhancement

## Technical Details

The derived calculations ensure that:
- Chromatic aberration is always subtle and proportional
- Contrast enhancement provides a natural boost without crushing blacks
- Glow effect enhances mid-tones without washing out highlights
- All effects work together harmoniously at any intensity level
