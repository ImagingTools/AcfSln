# IIPR Library - Actionable Issues and Improvements

This document provides specific, actionable issues identified during the IIPR library review, organized by priority.

## Critical Priority Issues

None identified. The library is in good working condition.

## High Priority Issues

### Issue 1: Hardcoded Threshold in Adaptive Binarization

**File:** `Include/iipr/CAdaptiveImageBinarizeProcessorComp.cpp:70`

**Problem:**
```cpp
int minContrast = 0.05 * 255; // TODO: Use parameterization!
```

The minimum contrast threshold is hardcoded at 5% (12/255), making it impossible to adjust for different image types or use cases.

**Impact:**
- Limits flexibility for different image types
- May produce suboptimal results for low-contrast or high-contrast images
- Users cannot tune the algorithm for their specific needs

**Recommended Solution:**
1. Add a new parameter to `CAdaptiveImageBinarizeProcessorComp` for `minContrastThreshold`
2. Default value: 0.05 (maintains backward compatibility)
3. Valid range: 0.0 to 1.0
4. Update component parameter interface to expose this setting

**Estimated Effort:** 2-4 hours

---

### Issue 2: Projection Line Exactness Problem

**Files:** 
- `Include/iipr/CBilinearLineProjectionProcessorComp.cpp`
- `Include/iipr/CLineProjectionProcessorComp.cpp`

**Problem:**
```cpp
// TODO: correct exactness of this mapping: DoAutosizeProjection return rough line exactness!
```

The projection calculation returns approximate rather than exact line positions, which may affect measurement accuracy.

**Impact:**
- Reduced accuracy in caliper measurements
- Potential precision loss in geometric calculations
- May accumulate errors in multi-step processing

**Recommended Solution:**
1. Investigate the source of approximation in DoAutosizeProjection
2. Implement exact projection calculation
3. Add unit tests to verify precision
4. Document the expected precision level

**Estimated Effort:** 8-16 hours (requires investigation)

---

### Issue 3: Qt Framework Dependency in Image Flip

**File:** `Include/iipr/CImageFlipProcessorComp.cpp:64`

**Problem:**
```cpp
// TODO: Implement it more general and independent from QImage
```

The image flip operation is implemented using Qt's QImage, creating a platform dependency.

**Impact:**
- Reduces portability to non-Qt environments
- Creates dependency on Qt GUI module
- May be less efficient than direct pixel manipulation

**Recommended Solution:**
1. Implement platform-independent pixel-level flip operations
2. Create separate implementations for horizontal and vertical flipping
3. Keep Qt version as an optional optimization
4. Add performance comparison tests

**Implementation Approach:**
```cpp
// Horizontal flip: swap pixels in each row
for (int y = 0; y < height; ++y) {
    PixelType* line = GetLinePtr(y);
    std::reverse(line, line + width);
}

// Vertical flip: swap entire rows
for (int y = 0; y < height/2; ++y) {
    std::swap_ranges(GetLinePtr(y), GetLinePtr(y) + width,
                     GetLinePtr(height - 1 - y));
}
```

**Estimated Effort:** 4-8 hours

---

## Medium Priority Issues

### Issue 4: Replace NULL with nullptr

**Impact:** Throughout codebase (20+ occurrences in sample)

**Problem:**
Use of C-style NULL macro instead of C++11 nullptr keyword.

**Impact:**
- Less type-safe (NULL is just 0)
- Can cause ambiguity in overload resolution
- Not following modern C++ best practices

**Recommended Solution:**
- Global find and replace: `NULL` → `nullptr`
- Test thoroughly after change
- Use automated tools if available

**Estimated Effort:** 1-2 hours (mostly testing)

---

### Issue 5: Manual Memory Management with new[]/delete[]

**Locations:**
```
- CImagePolarTransformProcessorComp.cpp:251-252 (cosTable, sinTable)
- CCheckerboardPointGridExtractorComp.cpp:56
- CPerspectiveCalibrationSupplierComp.cpp:35
- CLensCorrFindSupplierComp.cpp:486
- CCheckboardCalibSupplierComp.cpp:393
- CSingleFeatureConsumer.cpp:89
```

**Problem:**
Raw arrays allocated with `new[]` and manually deleted with `delete[]`.

**Impact:**
- Risk of memory leaks if exceptions occur
- Manual management is error-prone
- Not following RAII principles

**Recommended Solution:**

For arrays:
```cpp
// Current:
double* cosTable = new double[alphaSize];
// ... use ...
delete [] cosTable;

// Better:
std::vector<double> cosTable(alphaSize);
// automatic cleanup
```

For single objects:
```cpp
// Current:
delete featurePtr;

// Better:
std::unique_ptr<FeatureType> featurePtr;
// automatic cleanup
```

**Estimated Effort:** 4-6 hours (with testing)

---

### Issue 6: C-Style Casts in Pixel Operations

**Locations:** Multiple files with pixel buffer access

**Problem:**
```cpp
quint8* inputImageBufferPtr = (quint8*)inputBitmap.GetLinePtr(y);
```

**Impact:**
- Less type-safe than C++ casts
- Harder to search and review
- May hide unintended conversions

**Recommended Solution:**
```cpp
// Use static_cast for known safe conversions
quint8* inputImageBufferPtr = static_cast<quint8*>(inputBitmap.GetLinePtr(y));

// Or reinterpret_cast for pointer type changes
quint8* inputImageBufferPtr = reinterpret_cast<quint8*>(inputBitmap.GetLinePtr(y));
```

**Estimated Effort:** 2-3 hours

---

## Low Priority Issues

### Issue 7: Missing Copyright and License Headers

**Problem:**
No copyright or license information in source files.

**Impact:**
- Unclear licensing terms
- Difficulty determining code ownership
- May cause issues for contributors

**Recommended Solution:**
Add standard header to all files:
```cpp
/*
 * Copyright (c) [YEAR] ImagingTools
 * 
 * This file is part of AcfSln.
 * 
 * [License terms]
 */
```

**Estimated Effort:** 1-2 hours (automated script)

---

### Issue 8: Missing Top-Level Documentation

**Problem:**
No README.md file explaining the library purpose, architecture, or usage.

**Impact:**
- Difficult for new developers to understand the library
- No clear entry point for documentation
- Usage examples not readily available

**Recommended Solution:**
Create `Include/iipr/README.md` with:
1. Library purpose and capabilities
2. Architecture overview
3. Component listing
4. Usage examples
5. Build instructions
6. Dependencies

**Estimated Effort:** 4-6 hours

---

### Issue 9: No Unit Tests Found

**Problem:**
No test directory found for IIPR library.

**Impact:**
- Difficult to verify correct behavior
- Risk of regressions when making changes
- No documentation of expected behavior

**Recommended Solution:**
1. Create test directory structure
2. Add unit tests for each processor component
3. Add integration tests for processor chains
4. Achieve minimum 70% code coverage

**Example Test Structure:**
```
Tests/iipr/
  - TestImageFlipProcessor.cpp
  - TestBinarizeProcessor.cpp
  - TestCaliperProcessor.cpp
  - TestProjectionProcessor.cpp
  ...
```

**Estimated Effort:** 40-80 hours (comprehensive test suite)

---

### Issue 10: Limited Input Validation

**Problem:**
Some functions don't validate input parameters thoroughly.

**Impact:**
- Potential crashes with invalid input
- Buffer overruns possible
- Integer overflow in size calculations

**Recommended Solution:**
Add validation for:
1. Image dimensions (max width/height)
2. Buffer sizes before memcpy operations
3. Array indices before access
4. Numerical parameter ranges

**Example:**
```cpp
bool ValidateImageSize(const istd::CIndex2d& size) {
    const int MAX_DIMENSION = 32768; // 32K pixels
    
    if (size.GetX() <= 0 || size.GetY() <= 0) {
        return false; // Invalid size
    }
    
    if (size.GetX() > MAX_DIMENSION || size.GetY() > MAX_DIMENSION) {
        return false; // Too large
    }
    
    // Check for multiplication overflow
    if (size.GetX() > INT_MAX / size.GetY()) {
        return false; // Would overflow
    }
    
    return true;
}
```

**Estimated Effort:** 8-12 hours

---

## Performance Improvement Opportunities

### Opportunity 1: SIMD Optimization for Pixel Operations

**Potential Files:**
- CAdaptiveImageBinarizeProcessorComp.cpp
- CImageCopyProcessorComp.cpp
- Morphological operations

**Benefit:**
- 2-4x speedup for pixel-level operations
- Better utilization of modern CPUs

**Estimated Effort:** 16-40 hours per operation

---

### Opportunity 2: Multi-threading for Large Images

**Potential Files:**
- Most image processors that work line-by-line

**Benefit:**
- Near-linear speedup with core count
- Better responsiveness for large images

**Estimated Effort:** 12-24 hours for framework + 2-4 hours per processor

---

## Summary

**Total High Priority Issues:** 3  
**Total Medium Priority Issues:** 6  
**Total Low Priority Issues:** 4  
**Performance Opportunities:** 2

**Recommended Sprint Plan:**

**Sprint 1 (1 week):** Issues 1, 4
**Sprint 2 (1 week):** Issue 2
**Sprint 3 (1 week):** Issues 3, 5, 6
**Sprint 4 (2 weeks):** Issues 7, 8, 10
**Sprint 5+ (ongoing):** Issue 9 (tests), Performance improvements

---

**Document Version:** 1.0  
**Last Updated:** December 26, 2025
