# IIPR Library - Build and Test Notes

## Build System

### CMake Configuration

The IIPR library is integrated into a larger CMake build system:

**Main Build File:** `/Build/CMake/CMakeLists.txt`  
**Library CMake:** `/Include/iipr/CMake/CMakeLists.txt`

### Dependencies

The library requires:
1. **ACF Framework** - Core framework (ACFDIR environment variable or sibling directory)
   - Location: `../../../Acf` (relative to build directory)
   - Required modules: istd, iser, icmm, iproc, imeas, iimg, iprm, i2d, ibase

2. **Qt Framework** - Qt 5 or Qt 6
   - Required components: Core, Widgets, Gui, Xml, Network, Svg, Concurrent
   - Used primarily for QImage operations

3. **C++ Compiler** - CMake 3.26 or higher required

### Build Instructions

To build the IIPR library:

```bash
# Set environment variables
export ACFDIR=/path/to/Acf
export ACFSLNDIR=/path/to/AcfSln

# Create build directory
mkdir -p build
cd build

# Configure
cmake ../Build/CMake

# Build IIPR library specifically
cmake --build . --target iipr

# Build implementation package
cmake --build . --target IprPck
```

### Build Verification Status

**Status:** Not tested in this review

**Reason:** ACF framework dependencies not available in the review environment

**Recommendation:** 
- Set up CI/CD pipeline to automatically build on commits
- Add build status badge to documentation
- Verify all 75+ exported components build successfully
- Check for compiler warnings and address them

## Test Infrastructure

### Current Status

**Unit Tests:** Not found in `/Include/iipr/` directory  
**Integration Tests:** Not found  
**Test Framework:** Unknown (possibly in ACF framework)

### Test Recommendations

#### 1. Unit Test Structure

Recommended directory structure:
```
Tests/iipr/
├── CMakeLists.txt
├── TestMain.cpp
├── Processors/
│   ├── TestImageFlipProcessor.cpp
│   ├── TestBinarizeProcessor.cpp
│   ├── TestCaliperProcessor.cpp
│   ├── TestProjectionProcessor.cpp
│   └── ...
├── Features/
│   ├── TestFeaturesContainer.cpp
│   ├── TestFeatureMapper.cpp
│   └── ...
└── Utilities/
    ├── TestBitmapOperations.cpp
    └── ...
```

#### 2. Suggested Test Framework

**Recommended:** Google Test (gtest) or Catch2

**Rationale:**
- Industry standard
- Good documentation
- Easy integration with CMake
- Supports parameterized tests (useful for image processing)

#### 3. Priority Test Cases

**High Priority:**
1. `CAdaptiveImageBinarizeProcessorComp` - especially edge cases
2. `CImageFlipProcessorComp` - verify correctness of Qt-based implementation
3. `CBilinearLineProjectionProcessorComp` - verify projection accuracy
4. `CLineProjectionProcessorComp` - verify projection accuracy
5. `CFeaturesContainer` - test feature addition and retrieval

**Medium Priority:**
6. All processors with dynamic_cast validation
7. Memory management in feature deletion paths
8. Edge cases for empty or invalid images
9. Parameter validation in all components

**Low Priority:**
10. Performance benchmarks
11. Integration tests for processor chains
12. Qt-specific vs platform-independent paths

#### 4. Test Coverage Goals

- **Initial Target:** 50% line coverage
- **Short-term Goal:** 70% line coverage
- **Long-term Goal:** 85%+ line coverage for critical paths

#### 5. Example Test Case

```cpp
#include <gtest/gtest.h>
#include <iipr/CImageFlipProcessorComp.h>
#include <iimg/CGeneralBitmap.h>

class ImageFlipProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        processor = std::make_unique<iipr::CImageFlipProcessorComp>();
    }

    std::unique_ptr<iipr::CImageFlipProcessorComp> processor;
};

TEST_F(ImageFlipProcessorTest, HorizontalFlipPreservesHeight) {
    // Create test image
    iimg::CGeneralBitmap input, output;
    input.CreateBitmap(iimg::IBitmap::PF_RGB, istd::CIndex2d(100, 50));
    
    // Set up parameters for horizontal flip
    // ... parameter setup ...
    
    // Process
    auto result = processor->DoProcessing(params, &input, &output, nullptr);
    
    // Verify
    EXPECT_EQ(result, iipr::CImageFlipProcessorComp::TS_OK);
    EXPECT_EQ(output.GetImageSize().GetY(), 50);
    EXPECT_EQ(output.GetImageSize().GetX(), 100);
}

TEST_F(ImageFlipProcessorTest, VerticalFlipPreservesWidth) {
    // Similar test for vertical flip
}

TEST_F(ImageFlipProcessorTest, RejectsNullInput) {
    iimg::CGeneralBitmap output;
    
    auto result = processor->DoProcessing(nullptr, nullptr, &output, nullptr);
    
    EXPECT_EQ(result, iipr::CImageFlipProcessorComp::TS_INVALID);
}

TEST_F(ImageFlipProcessorTest, RejectsEmptyInput) {
    iimg::CGeneralBitmap input, output;
    // input is empty (not initialized)
    
    auto result = processor->DoProcessing(nullptr, &input, &output, nullptr);
    
    EXPECT_EQ(result, iipr::CImageFlipProcessorComp::TS_INVALID);
}
```

## Continuous Integration

### Recommended CI/CD Pipeline

#### Stage 1: Build Verification
```yaml
build:
  - Check out code
  - Install dependencies (Qt, ACF)
  - Configure CMake
  - Build all components
  - Report build warnings
```

#### Stage 2: Static Analysis
```yaml
static-analysis:
  - Run cppcheck
  - Run clang-tidy
  - Check for common issues:
    - Memory leaks
    - Null pointer dereferences
    - Buffer overflows
    - Unused variables
```

#### Stage 3: Unit Tests
```yaml
test:
  - Run all unit tests
  - Generate coverage report
  - Fail if coverage drops below threshold
```

#### Stage 4: Integration Tests
```yaml
integration:
  - Run processor chain tests
  - Test component interactions
  - Verify end-to-end scenarios
```

#### Stage 5: Performance Tests
```yaml
performance:
  - Run benchmark suite
  - Compare against baseline
  - Alert on regressions > 10%
```

## Documentation Generation

### Doxygen Configuration

The code has Doxygen-style comments. Recommended setup:

```bash
# Generate Doxygen configuration
doxygen -g Doxyfile.iipr

# Configure for the library
# INPUT = Include/iipr
# RECURSIVE = YES
# EXTRACT_ALL = YES
# GENERATE_HTML = YES
# OUTPUT_DIRECTORY = docs/iipr

# Generate documentation
doxygen Doxyfile.iipr
```

### Publish Documentation

- Host on GitHub Pages
- Update on every commit to main branch
- Include class hierarchy diagrams
- Include collaboration diagrams
- Add usage examples

## Performance Profiling

### Recommended Tools

1. **Valgrind** - Memory profiling
   ```bash
   valgrind --tool=memcheck --leak-check=full ./test_iipr
   ```

2. **Perf** - CPU profiling
   ```bash
   perf record -g ./benchmark_iipr
   perf report
   ```

3. **gprof** - Function-level profiling
   ```bash
   # Compile with -pg flag
   # Run application
   gprof ./test_iipr gmon.out > analysis.txt
   ```

### Profile Priority

1. `CAdaptiveImageBinarizeProcessorComp::ConvertImage` - nested loops
2. Projection processors - used in measurements
3. Convolution operations - computationally intensive
4. Morphological operations - pixel-by-pixel processing

## Deployment Checklist

Before releasing IIPR library updates:

- [ ] All unit tests pass
- [ ] No build warnings
- [ ] Static analysis clean
- [ ] Code coverage ≥ 70%
- [ ] API documentation updated
- [ ] Release notes written
- [ ] Version number updated
- [ ] Performance benchmarks within acceptable range
- [ ] All TODOs addressed or documented as known issues
- [ ] Security review completed
- [ ] Backward compatibility verified

## Known Build Issues

None identified during review.

## Next Steps

1. **Set up ACF framework** in CI environment
2. **Configure and run build** to verify compilation
3. **Create test infrastructure** as outlined above
4. **Set up CI/CD pipeline** for automated testing
5. **Generate and publish documentation**

---

**Document Version:** 1.0  
**Last Updated:** December 26, 2025  
**Build Status:** Not verified (ACF dependencies unavailable)
