// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iedge_CFastEdgesExtractorComp_included
#define iedge_CFastEdgesExtractorComp_included


// Qt includes
#include <QtCore/QMutex>

// ACF includes
#include <iimg/IBitmap.h>
#include <iimg/CScanlineMask.h>
#include <iproc/TSyncProcessorCompBase.h>

// ACF-Solutions includes
#include <imeas/INumericValue.h>
#include <imeas/INumericConstraints.h>

#include <iedge/IEdgesExtractor.h>


namespace iedge
{


class CFastEdgesExtractorComp:
			public iproc::TSyncProcessorCompBase<IEdgesExtractor>,
			virtual public imeas::INumericConstraints,
			virtual protected iprm::IOptionsList
{
public:
	typedef iproc::TSyncProcessorCompBase<IEdgesExtractor> BaseClass;

	I_BEGIN_COMPONENT(CFastEdgesExtractorComp);
		I_REGISTER_INTERFACE(imeas::INumericConstraints);
		I_ASSIGN(m_defaultThresholdParamCompPtr, "DefaultThresholdParam", "Default threshold parameter, if not taken from parameters", false, "DefaultThresholdParam");
		I_ASSIGN(m_thresholdParamIdAttrPtr, "ThresholdParamId", "Threshold parameter ID (imeas::INumericValue)", false, "EdgeThreshold");
		I_ASSIGN(m_defaultAoiCompPtr, "DefaultAoi", "Area of interest used if not specified in parameters", false, "DefaultAoi");
		I_ASSIGN(m_aoiParamIdAttrPtr, "AoiParamId", "ID of area of interest in parameter set", false, "AoiParams");
		I_ASSIGN(m_maxNodesCountAttrPtr, "MaxNodesCount", "Maximal number of nodes in container", true, 100000);
		I_ASSIGN(m_keepSingletonsAttrPtr, "KeepSingletons", "If enabled lines containing only single isolated edge point will be also created", true, false);
	I_END_COMPONENT;

	//	reimplemented (iedge::IEdgesExtractor)
	virtual bool DoContourExtraction(
				const iprm::IParamsSet* paramsPtr,
				const iimg::IBitmap& bitmap,
				CEdgeLineContainer& result,
				istd::IChangeable* processingReportPtr = NULL) const override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

	// reimplemented (imeas::INumericConstraints)
	virtual const iprm::IOptionsList& GetValueListInfo() const override;
	virtual const imath::IUnitInfo* GetNumericValueUnitInfo(int index) const override;

protected:
	// reimplemented (iprm::IOptionsList)
	virtual int GetOptionsFlags() const override;
	virtual int GetOptionsCount() const override;
	virtual QString GetOptionName(int index) const override;
	virtual QString GetOptionDescription(int index) const override;
	virtual QByteArray GetOptionId(int index) const override;
	virtual bool IsOptionEnabled(int index) const override;

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private:
	enum
	{
		THRESHOLD_FACTOR = 510 // Maximum value of derivative strenghtness
	};


	struct ExtNode
	{
		i2d::CVector2d position;
		i2d::CVector2d derivative;

		ExtNode* prevPtr;
		ExtNode* nextPtr;

		double rawWeight;
		double prevWeight;
		double nextWeight;

		bool isHorizontal;
		mutable bool isExtracted;
	};


	struct PixelDescriptor
	{
		quint32 dirLength2;
		qint16 dx, dy;
		qint16 brightness;

		ExtNode* listReference;
	};


	class MemoryPoolManager
	{
	public:
		~MemoryPoolManager();

		void* GetMemory(int bytes);
		bool ReleaseMemory(void* ptr);

	private:
		void Dispose();

		struct ChunkInfo
		{
			int bytes;
			void* ptr;
		};

		typedef QList<ChunkInfo> ChunkList;
		ChunkList m_freeList;
		ChunkList m_usedList;

		QMutex m_lock;
	};

	class InternalContainer
	{
	public:
		InternalContainer(int size);
		~InternalContainer();

		bool IsContainerFull() const;

		void ExtractLines(
					double weightScale,
					CEdgeLineContainer& result,
					bool keepSingletons);

		ExtNode* AddElementToList();

	private:
		ExtNode* m_buffer;
		int m_bufferSize;
		int m_freeIndex;

		bool m_isContainerFull;

		static MemoryPoolManager s_memoryManager;
	};

	// static methods
	static void TryConnectElements(
				const PixelDescriptor& neightborPixel,
				PixelDescriptor& pixel);
	static void CalcDerivative(
				const quint8* prevSourceLine,
				const quint8* sourceLine,
				int x,
				PixelDescriptor& pixelDescriptor);
	static void CalcDerivativeLine(
				const quint8* prevSourceLine,
				const quint8* sourceLine,
				int inputBeginX,
				int inputEndX,
				PixelDescriptor* destLine);
	static void CalcPoint(
				int x,
				int y,
				quint32 threshold2Factor,
				PixelDescriptor* prevPrevDestLine,
				PixelDescriptor* prevDestLine,
				PixelDescriptor* destLine,
				InternalContainer& container);
	static void CalcOutputLine(
				int outputBeginX,
				int outputEndX,
				int y,
				quint32 threshold2Factor,
				PixelDescriptor* prevPrevDestLine,
				PixelDescriptor* prevDestLine,
				PixelDescriptor* destLine,
				InternalContainer& container);

	I_REF(imeas::INumericValue, m_defaultThresholdParamCompPtr);
	I_ATTR(QByteArray, m_thresholdParamIdAttrPtr);
	I_REF(i2d::IObject2d, m_defaultAoiCompPtr);
	I_ATTR(QByteArray, m_aoiParamIdAttrPtr);
	I_ATTR(int, m_maxNodesCountAttrPtr);
	I_ATTR(bool, m_keepSingletonsAttrPtr);
};


} // namespace iedge


#endif // !iedge_CFastEdgesExtractorComp_included


