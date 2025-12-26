#include <iipr/CConvolutionKernel2d.h>


// Qt includes
#include <QtCore/QVector>

// ACF includes
#include <istd/CChangeNotifier.h>

#include <iser/IArchive.h>
#include <iser/CArchiveTag.h>

// IACF includes
#include <iipr/iipr.h>


namespace iipr
{


// static constants
static const iser::CArchiveTag sizeTag("KernelSize", "Size of convolution kernel", iser::CArchiveTag::TT_GROUP);
static const iser::CArchiveTag widthTag("Width", "Width", iser::CArchiveTag::TT_LEAF, &sizeTag);
static const iser::CArchiveTag heightTag("Height", "Height", iser::CArchiveTag::TT_LEAF, &sizeTag);
static const iser::CArchiveTag valuesTag("Values", "values of convolution kernel", iser::CArchiveTag::TT_GROUP);
static const iser::CArchiveTag offsetValueTag("Offset", "Offset value", iser::CArchiveTag::TT_LEAF);


CConvolutionKernel2d::CConvolutionKernel2d()
:	m_valueOffset(0)
{
}


CConvolutionKernel2d::CConvolutionKernel2d(const CConvolutionKernel2d& kernel)
:	BaseClass(kernel),
	m_valueOffset(kernel.m_valueOffset)
{
}


CConvolutionKernel2d::CConvolutionKernel2d(const imath::ISampledFunction2d& function2d)
:	BaseClass(function2d),
	m_valueOffset(0)
{
}


CConvolutionKernel2d::CConvolutionKernel2d(const istd::CIndex2d& kernelSize, double value)
:	BaseClass(kernelSize, value),
	m_valueOffset(0)
{
}


double CConvolutionKernel2d::GetKernelDistance(const IConvolutionKernel2d& kernel) const
{
	double norm2 = 0;

	istd::CIndex2d size = GetKernelSize();
	istd::CIndex2d kernelSize = kernel.GetKernelSize();
	istd::CIndex2d commonSize(qMax(size.GetX(), kernelSize.GetX()), qMax(size.GetY(), kernelSize.GetY()));

	istd::CIndex2d index = istd::CIndex2d::GetZero();
	if (index.IsInside(commonSize)){
		do{
			double element = 0;
			if (index.IsInside(size)){
				element = GetKernelElement(index);
			}

			double kernelElement = 0;
			if (index.IsInside(kernelSize)){
				kernelElement = kernel.GetKernelElement(index);
			}

			norm2 += (element - kernelElement) * (element - kernelElement);
		} while (index.Increase(commonSize));
	}

	return qSqrt(norm2);
}


// reimplemented (iipr::IConvolutionKernel2d)

istd::CIndex2d CConvolutionKernel2d::GetKernelSize() const
{
	return GetGridSize2d();
}


void CConvolutionKernel2d::SetKernelSize(const istd::CIndex2d& kernelSize, double value)
{
	CreateGrid2d(kernelSize, value);
}


double CConvolutionKernel2d::GetKernelElement(const istd::CIndex2d& index) const
{
	return GetSampleValue(index);
}


bool CConvolutionKernel2d::SetKernelElement(const istd::CIndex2d& index, double value)
{
	SetSampleValue(index, value);

	return true;
}


double CConvolutionKernel2d::GetOffsetValue() const
{
	return m_valueOffset;
}


bool CConvolutionKernel2d::SetOffsetValue(double offset)
{
	if (offset != m_valueOffset){
		istd::CChangeNotifier notifier(this);
		Q_UNUSED(notifier);

		m_valueOffset = offset;
	}

	return true;
}


bool CConvolutionKernel2d::GetCombinedKernel(const IConvolutionKernel2d& kernel, IConvolutionKernel2d& result) const
{
	istd::CIndex2d size = GetKernelSize();
	if (size.IsSizeEmpty()){
		result = kernel;
		return true;
	}

	istd::CIndex2d kernelSize = kernel.GetKernelSize();
	if (kernelSize.IsSizeEmpty()){
		result = *this;
		return true;
	}

	istd::CIndex2d resultSize(size.GetX() + kernelSize.GetX() - 1, size.GetY() + kernelSize.GetY() - 1);
	result.SetKernelSize(resultSize, 0);

	istd::CIndex2d index = istd::CIndex2d::GetZero();
	Q_ASSERT(index.IsInside(size));
	Q_ASSERT(index.IsInside(kernelSize));

	do{
		double element = GetKernelElement(index);
		if (element != 0){
			istd::CIndex2d kernelIndex = istd::CIndex2d::GetZero();
			do{
				double kernelElement = kernel.GetKernelElement(kernelIndex);
				istd::CIndex2d resultIndex = index + kernelIndex;

				result.SetKernelElement(resultIndex, result.GetKernelElement(resultIndex) + element * kernelElement);
			} while (kernelIndex.Increase(kernelSize));
		}
	} while (index.Increase(size));

	result.SetOffsetValue(GetOffsetValue() + kernel.GetOffsetValue());

	return true;
}


bool CConvolutionKernel2d::TrySeparateKernels(IConvolutionKernel2d& result1, IConvolutionKernel2d& result2, int separationType, double tolerance) const
{
	if ((separationType != ST_AUTO) && (separationType != ST_HOR_VERT)){
		return false;
	}

	istd::CIndex2d kernelSize = GetKernelSize();

	QVector<double> squaresX(kernelSize.GetX(), 0);
	QVector<double> squaresY(kernelSize.GetY(), 0);
	double norm2 = 0;

	istd::CIndex2d kernelIndex;
	for (kernelIndex[1] = 0; kernelIndex[1] < kernelSize.GetY(); ++kernelIndex[1]){
		for (kernelIndex[0] = 0; kernelIndex[0] < kernelSize.GetX(); ++kernelIndex[0]){
			double kernelElement = GetKernelElement(kernelIndex);

			double elementSquare = kernelElement * kernelElement;

			squaresX[kernelIndex[0]] += elementSquare;
			squaresY[kernelIndex[1]] += elementSquare;
			norm2 += elementSquare;
		}
	}

	// find maximal X and Y
	double maxX = 0;
	int maxXIndex = -1;
	for (int i = 0; i < kernelSize.GetX(); ++i){
		double value = squaresX[i];
		if (value > maxX){
			maxXIndex = i;
			maxX = value;
		}
	}
	double maxY = 0;
	int maxYIndex = -1;
	for (int i = 0; i < kernelSize.GetY(); ++i){
		double value = squaresY[i];
		if (value > maxY){
			maxYIndex = i;
			maxY = value;
		}
	}

	result1.SetKernelSize(istd::CIndex2d(kernelSize.GetX(), 1), 0);
	result1.SetOffsetValue(0);

	result2.SetKernelSize(istd::CIndex2d(1, kernelSize.GetY()), 0);
	result2.SetOffsetValue(m_valueOffset);

	if ((maxXIndex < 0) || (maxYIndex < 0)){
		return true;	// nothing to do, kernel contains only zeroes
	}

	double divideFactorX = qSqrt(qSqrt(norm2));

	for (kernelIndex = istd::CIndex2d::GetZero(); kernelIndex[0] < kernelSize.GetX(); ++kernelIndex[0]){
		double value = qSqrt(squaresX[kernelIndex[0]]) / divideFactorX;

		if (GetKernelElement(istd::CIndex2d(kernelIndex[0], maxYIndex)) < 0){
			value = -value;
		}

		result1.SetKernelElement(kernelIndex, value);
	}

	double divideFactorY = (GetKernelElement(istd::CIndex2d(maxXIndex, maxYIndex)) > 0)? divideFactorX: -divideFactorX;
	for (kernelIndex = istd::CIndex2d::GetZero(); kernelIndex[1] < kernelSize.GetY(); ++kernelIndex[1]){
		double value = qSqrt(squaresY[kernelIndex[1]]) / divideFactorY;

		if (GetKernelElement(istd::CIndex2d(maxXIndex, kernelIndex[1])) < 0){
			value = -value;
		}

		result2.SetKernelElement(kernelIndex, value);
	}

	CConvolutionKernel2d reconstructed;

	return result1.GetCombinedKernel(result2, reconstructed) && (qFabs(GetKernelDistance(reconstructed)) < tolerance);
}

	
// reimplemented (iser::ISerializable)

bool CConvolutionKernel2d::Serialize(iser::IArchive& archive)
{
	bool retVal = true;

	bool isStoring = archive.IsStoring();
	istd::CChangeNotifier notifier(isStoring? nullptr: this);
	Q_UNUSED(notifier);

	if (isStoring){
		istd::CIndex2d size = GetGridSize2d();

		retVal = retVal && archive.BeginTag(sizeTag);

		retVal = retVal && archive.BeginTag(widthTag);
		retVal = retVal && archive.Process(size[0]);
		retVal = retVal && archive.EndTag(widthTag);

		retVal = retVal && archive.BeginTag(heightTag);
		retVal = retVal && archive.Process(size[1]);
		retVal = retVal && archive.EndTag(heightTag);

		retVal = retVal && archive.EndTag(sizeTag);

		retVal = retVal && archive.BeginTag(valuesTag);

		istd::CIndex2d index;
		for (index[0] = 0; index[0] < size[0]; ++index[0]){
			for (index[1] = 0; index[1] < size[1]; ++index[1]){
				double value = GetSampleValue(index);
				retVal = retVal && archive.Process(value);
			}
		}

		retVal = retVal && archive.EndTag(valuesTag);
	}
	else{
		istd::CIndex2d size(0, 0);

		retVal = retVal && archive.BeginTag(sizeTag);

		retVal = retVal && archive.BeginTag(widthTag);
		retVal = retVal && archive.Process(size[0]);
		retVal = retVal && archive.EndTag(widthTag);

		retVal = retVal && archive.BeginTag(heightTag);
		retVal = retVal && archive.Process(size[1]);
		retVal = retVal && archive.EndTag(heightTag);

		retVal = retVal && archive.EndTag(sizeTag);

		if (!CreateGrid2d(size)){
			return false;
		}

		retVal = retVal && archive.BeginTag(valuesTag);

		istd::CIndex2d index;
		for (index[0] = 0; index[0] < size[0]; ++index[0]){
			for (index[1] = 0; index[1] < size[1]; ++index[1]){
				double value = 0;
				retVal = retVal && archive.Process(value);
				if (!retVal){
					return false;
				}

				SetSampleValue(index, value);
			}
		}

		retVal = retVal && archive.EndTag(valuesTag);
	}

	quint32 versionNumber = 0;
	if (!archive.GetVersionInfo().GetVersionNumber(1, versionNumber) || (versionNumber >= 1621)){
		retVal = retVal && archive.BeginTag(offsetValueTag);
		retVal = retVal && archive.Process(m_valueOffset);
		retVal = retVal && archive.EndTag(offsetValueTag);
	}
	else if (!isStoring){
		m_valueOffset = 0;
	}

	return retVal;
}


quint32 CConvolutionKernel2d::GetMinimalVersion(int versionId) const
{
	if (versionId == AcfSlnVersionId){
		if (m_valueOffset != 0){
			return 1621;
		}
	}

	return 0;
}


} // namespace iipr


