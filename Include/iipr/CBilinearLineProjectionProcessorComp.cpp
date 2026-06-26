// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iipr/CBilinearLineProjectionProcessorComp.h>


// ACF includes
#include <istd/CChangeNotifier.h>
#include <istd/TSmartPtr.h>
#include <i2d/CRectangle.h>
#include <iimg/TPixelConversion.h>
#include <iprm/TParamsPtr.h>
#include <iimg/IBitmap.h>
#include <iipr/TImagePixelInterpolator.h>


// ACF-Solutions includes
#include <imeas/IDataSequence.h>
#include <imeas/CSamplesInfo.h>
#include <imeas/INumericValue.h>


namespace iipr
{

// public methods

template <typename InputPixelType>
bool CBilinearLineProjectionProcessorComp::DoAutosizeProjection(
            const iimg::IBitmap& bitmap,
            const i2d::CLine2d& bitmapLine,
            imeas::IDataSequence& results) const
{
    istd::CIndex2d imageSize = bitmap.GetImageSize();
    if (imageSize.IsSizeEmpty()){
        return false;
    }

    const i2d::CVector2d diffVector = bitmapLine.GetDiffVector();
    const int projectionSize = 1*diffVector.GetLength();
    const double step = 1.0/projectionSize;

    results.CreateSequence(projectionSize);

    for (int i = 0; i < projectionSize; i++){
        const i2d::CVector2d pos = bitmapLine.GetPositionFromAlpha(i*step);

        const double x = pos.GetX() - 0.5;
        const double y = pos.GetY() - 0.5;

        if (x < 0 || y < 0 ||
            x >= imageSize.GetX() - 1 || y >= imageSize.GetY() - 1
        ) {
            results.SetSample(i, 0, NAN);
            continue;
        }

        const int sourceX = int(x);
        const double alphaX = x - sourceX;
        const int sourceY = int(y);
        const double alphaY = y - sourceY;

        const InputPixelType* linePtr = (const InputPixelType*)bitmap.GetLinePtr(sourceY);
        const InputPixelType* line1Ptr = (const InputPixelType*)bitmap.GetLinePtr(sourceY + 1);

        double pixel11Value = double(linePtr[sourceX]);
        double pixel12Value = double(linePtr[sourceX + 1]);
        double pixel21Value = double(line1Ptr[sourceX]);
        double pixel22Value = double(line1Ptr[sourceX + 1]);

        if (std::isnan(pixel11Value) || std::isnan(pixel12Value) || std::isnan(pixel21Value) || std::isnan(pixel22Value)) {
            results.SetSample(i, 0, NAN);
            continue;
        }

        pixel11Value *= (1 - alphaX);
        pixel12Value *= alphaX;
        pixel12Value += pixel11Value;
        pixel12Value *= (1 - alphaY);
        pixel21Value *= (1 - alphaX);
        pixel22Value *= alphaX;
        pixel22Value += pixel21Value;
        pixel22Value *= alphaY;
        pixel22Value += pixel12Value;

        results.SetSample(i, 0, pixel22Value);
    }

    return true;

}


// reimplemented (iipr::IFeatureToImageMapper)

bool CBilinearLineProjectionProcessorComp::GetImagePosition(
            const imeas::INumericValue& feature,
            const iprm::IParamsSet* paramsPtr,
            i2d::CVector2d& result) const
{
    if (m_featureMapperCompPtr.IsValid() && (paramsPtr != NULL)){
        iprm::TParamsPtr<i2d::CLine2d> linePtr(paramsPtr, *m_lineParamIdAttrPtr);
        double position;
        if (		(linePtr.IsValid()) &&
                    m_featureMapperCompPtr->GetProjectionPosition(feature, paramsPtr, position)){
            // TODO: correct exactness of this mapping: DoAutosizeProjection return rough line exactness!
            result = linePtr->GetPositionFromAlpha(position);

            return true;
        }
    }

    return false;
}


// reimplemented (iipr::ILineProjectionProcessor)

bool CBilinearLineProjectionProcessorComp::DoProjection(
            const iimg::IBitmap& bitmap,
            const i2d::CLine2d& projectionLine,
            const iipr::IProjectionParams* /*paramsPtr*/,
            imeas::IDataSequence& results)
{
    i2d::CLine2d bitmapLine;
    bitmapLine.CopyFrom(projectionLine, istd::IChangeable::CM_CONVERT);

    int retVal;

    switch (bitmap.GetPixelFormat()){
        case  iimg::IBitmap::PF_FLOAT32:
            retVal = DoAutosizeProjection<float>(bitmap, bitmapLine, results);
            break;

        case iimg::IBitmap::PF_FLOAT64:
            retVal = DoAutosizeProjection<double>(bitmap, bitmapLine, results);
            break;

        case iimg::IBitmap::PF_GRAY:
            retVal = DoAutosizeProjection<quint8>(bitmap, bitmapLine, results);
            break;

        case iimg::IBitmap::PF_GRAY16:
            retVal = DoAutosizeProjection<quint16>(bitmap, bitmapLine, results);
            break;

        case iimg::IBitmap::PF_GRAY32:
            retVal = DoAutosizeProjection<quint32>(bitmap, bitmapLine, results);
            break;

        default:
            SendErrorMessage(0, QString("Unsupported image format"));

            retVal = false;
            break;
    }

    return retVal;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CBilinearLineProjectionProcessorComp::DoProcessing(
            const iprm::IParamsSet* paramsPtr,
            const istd::IPolymorphic* inputPtr,
            istd::IChangeable* outputPtr,
            ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
    if (outputPtr == NULL){
        return TS_OK;
    }

    const iimg::IBitmap* bitmapPtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
    imeas::IDataSequence* projectionPtr = dynamic_cast<imeas::IDataSequence*>(outputPtr);

    if (		(bitmapPtr == NULL) ||
                (projectionPtr == NULL) ||
                (paramsPtr == NULL)){
        return TS_INVALID;
    }

    iprm::TParamsPtr<i2d::CLine2d> linePtr(paramsPtr, *m_lineParamIdAttrPtr);
    if (!linePtr.IsValid()){
        SendErrorMessage(0, QString("Projection line parameter (Parameter ID = %1) is not defined inside of parameter set").arg((*m_lineParamIdAttrPtr).constData()));

        return TS_INVALID;
    }

    return DoProjection(*bitmapPtr, *linePtr, NULL, *projectionPtr)? TS_OK: TS_INVALID;
}


// reimplemented (iipr::IProjectionConstraints)

istd::CRange CBilinearLineProjectionProcessorComp::GetLineWidthRange() const
{
    return istd::CRange(-1, -1);
}


int CBilinearLineProjectionProcessorComp::GetMinProjectionSize() const
{
    return -1;
}


int CBilinearLineProjectionProcessorComp::GetMaxProjectionSize() const
{
    return -1;
}


bool CBilinearLineProjectionProcessorComp::IsAutoProjectionSizeSupported() const
{
    return true;
}


// protected methods

// reimplemented (icomp::CComponentBase)

void CBilinearLineProjectionProcessorComp::OnComponentCreated()
{
    BaseClass::OnComponentCreated();

    m_featureMapperCompPtr.EnsureInitialized();
}


} // namespace iipr


