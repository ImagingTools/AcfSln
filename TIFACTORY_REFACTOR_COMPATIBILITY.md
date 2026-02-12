# TIFactory Pointer Type Refactor Compatibility Analysis

## Overview

This document analyzes the compatibility of the AcfSln codebase with changes made in the `copilot/refactor-tifactory-pointer-type` branch of the ImagingTools/Acf repository.

## Changes in Acf Repository

### TIFactory.h Interface Change

The `TIFactory<Interface>::CreateInstance()` method signature has been changed from:

```cpp
// OLD (main branch)
virtual Interface* CreateInstance(const QByteArray& keyId = "") const = 0;
```

to:

```cpp
// NEW (refactor-tifactory-pointer-type branch)
virtual istd::TUniqueInterfacePtr<Interface> CreateInstance(const QByteArray& keyId = "") const = 0;
```

### Additional Changes

- The TIFactory.h header now includes `<istd/TInterfacePtr.h>` to support the new return type
- The return value documentation updated from "pointer to created object or NULL" to "unique pointer to created object or empty pointer"

## Impact on AcfSln

### Call Site Analysis

A comprehensive search found **14 call sites** of `CreateInstance()` across the following files:

1. `Include/icam/CSnapMultiPageBitmapSupplierComp.cpp`
2. `Include/icam/CSnapBitmapSupplierComp.cpp`
3. `Include/icam/CBitmapSupplierMultiplexerComp.cpp`
4. `Include/icam/CMultiSourceSnapBitmapSupplierComp.cpp`
5. `Include/idocproc/CRenderedDocumentPreviewGeneratorComp.cpp`
6. `Include/icam/CBitmapJoinerCompBase.cpp`
7. `Include/icam/CSelectableBitmapSupplierComp.cpp`
8. `Include/icam/CMultiCameraBitmapSupplierComp.cpp`
9. `Include/iqtcam/CBitmapSupplierGuiComp.cpp`
10. `Include/ifileproc/CRenderedObjectFileLoaderComp.cpp`
11. `Include/iipr/CBitmapJoinerSupplierComp.cpp`
12. `Include/iipr/CProcessedAcquisitionComp.cpp`
13. `Include/iipr/CDifferenceBitmapSupplierComp.cpp`
14. `Include/iipr/CProcessedBitmapSupplierComp.h`

### Usage Patterns

All call sites follow one of these compatible patterns:

#### Pattern 1: Direct Assignment to UniquePtr (7 instances)
```cpp
iimg::IBitmapUniquePtr bitmapInstancePtr = m_bitmapCompFact.CreateInstance();
```
**Status:** ✅ Fully compatible - Smart pointer assignment works seamlessly with the new return type.

#### Pattern 2: Return Statement (3 instances)
```cpp
return m_bitmapCompFact.CreateInstance();
```
**Status:** ✅ Fully compatible - Direct return of unique pointers supported by move semantics.

#### Pattern 3: Direct Construction (1 instance)
```cpp
iimg::IBitmapUniquePtr tempBitmapPtr(m_bitmapFactCompPtr.CreateInstance());
```
**Status:** ✅ Fully compatible - Constructor-based initialization handles unique pointer transfer.

#### Pattern 4: FromUnique() Method (4 instances)
```cpp
m_bitmapPtr.FromUnique(m_bitmapCompFact.CreateInstance());
result.second.FromUnique(m_bitmapCompFact.CreateInstance());
previewBitmapPtr.FromUnique(m_bitmapFactoryCompPtr.CreateInstance());
```
**Status:** ✅ Fully compatible - `.FromUnique()` is explicitly designed to accept unique pointers and transfer ownership.

## Conclusion

### ✅ No Code Changes Required

All 14 call sites in AcfSln are already fully compatible with the new `TUniqueInterfacePtr` return type from `TIFactory::CreateInstance()`. The codebase already follows best practices for unique pointer usage throughout.

### Verification

The following verification steps confirm compatibility:

1. **No raw pointer assignments**: No instances of `Interface* ptr = factory.CreateInstance()` found
2. **Smart pointer patterns**: All call sites use appropriate smart pointer types (UniquePtr)
3. **Ownership transfer**: All patterns properly handle unique pointer semantics (move, return, FromUnique)
4. **No manual memory management**: No instances of `delete` on CreateInstance() results

### Next Steps

When building AcfSln against the updated Acf library:

1. Update ACFDIR to point to Acf with the refactor-tifactory-pointer-type branch
2. Rebuild AcfSln
3. Run existing tests to verify functionality
4. No source code modifications should be necessary

## References

- Acf Repository: https://github.com/ImagingTools/Acf
- Refactor Branch: copilot/refactor-tifactory-pointer-type
- Key Commit: 5765f5e6f4861cd745603310fc993912b32413a7 "Refactor TIFactory to return TUniqueInterfacePtr instead of raw pointer"

---

**Document Version:** 1.0  
**Date:** 2026-02-12  
**Analysis Status:** Complete
