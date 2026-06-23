// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <imeas/CWavSamplesLoaderComp.h>


// STL includes
#include <cstring>

// Qt includes
#include <QtCore/QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtCore/QtMath>
#else
#include <QtCore/qmath.h>
#endif
#include <QtCore/QStringList>
#include <QtCore/QFile>

// ACF includes
#include <istd/TRange.h>

// ACF-Solutions
#include <imeas/IDataSequence.h>
#include <imeas/CSamplesInfo.h>


namespace imeas
{


#pragma pack(push, 1)


struct PrefixHeader
{
	char code[4];
	quint32 size;
};

struct WavHeader: public PrefixHeader
{
	char wave[4];
};

struct FmtHeader: public PrefixHeader
{
	quint16 formatTag;
	quint16 channelsCount;
	quint32 samplesPerSecond;
	quint32 bytesPerSecond;
	quint16 blockAlign;
};

struct PcmFmtHeader: public FmtHeader
{
	quint16 bitsPerSample;
};


#pragma pack(pop)


// reimplemented (ifile::IFilePersistence)

bool CWavSamplesLoaderComp::IsOperationSupported(
			const istd::IChangeable* dataObjectPtr,
			const QString* filePathPtr,
			int flags,
			bool /*beQuiet*/) const
{
	if ((filePathPtr != NULL) && filePathPtr->isEmpty()){
		return false;
	}

	return		((dataObjectPtr == NULL) || (dynamic_cast<const IDataSequence*>(dataObjectPtr) != NULL)) &&
				((flags & QF_SAVE) != 0) &&
				((flags & QF_FILE) != 0);
}


ifile::IFilePersistence::OperationState CWavSamplesLoaderComp::LoadFromFile(
			istd::IChangeable& /*data*/,
			const QString& /*filePath*/,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
	return OS_FAILED;
}


ifile::IFilePersistence::OperationState CWavSamplesLoaderComp::SaveToFile(
			const istd::IChangeable& data,
			const QString& filePath,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
	if (!IsOperationSupported(&data, &filePath, QF_SAVE | QF_FILE, false)){
		return OS_FAILED;
	}

	QFile fileStream(filePath);
	if (!fileStream.open(QIODevice::WriteOnly | QIODevice::Truncate)){
		return OS_FAILED;
	}

	ifile::IFilePersistence::OperationState retVal = OS_FAILED;

	const IDataSequence* sequencePtr = dynamic_cast<const IDataSequence*>(&data);
	Q_ASSERT(sequencePtr != NULL);

	if (sequencePtr != NULL){
		int samplesCount = sequencePtr->GetSamplesCount();
		int channelsCount = sequencePtr->GetChannelsCount();

		double samplingPeriod = 44000;

		const CSamplesInfo* infoPtr = dynamic_cast<const CSamplesInfo*>(sequencePtr->GetSequenceInfo());
		if (infoPtr != NULL){
			const istd::CRange& timeRange = infoPtr->GetLogicalSamplesRange();
			if (timeRange.IsValid()){
				samplingPeriod = samplesCount / timeRange.GetLength();
			}
		}

		WavHeader header;
		std::memcpy(header.code, "RIFF", 4);
		std::memcpy(header.wave, "WAVE", 4);
		header.size = quint32(sizeof(WavHeader) + sizeof(PcmFmtHeader) + sizeof(PrefixHeader) + WC_BYTES_PER_SAMPLE * samplesCount * channelsCount) - 8;

		fileStream.write((const char*)&header, sizeof(header));

		PcmFmtHeader pcmFormat;
		std::memcpy(pcmFormat.code, "fmt ", 4);
		pcmFormat.size = sizeof(pcmFormat) - 8;
		pcmFormat.formatTag = WC_WAVE_FORMAT_PCM;
		pcmFormat.channelsCount = quint16(channelsCount);
		pcmFormat.samplesPerSecond = (samplingPeriod > 0)? quint32(1 / samplingPeriod): 1000;
		pcmFormat.bytesPerSecond = quint32(pcmFormat.samplesPerSecond * WC_BYTES_PER_SAMPLE * channelsCount);
		pcmFormat.blockAlign = quint16(channelsCount * WC_BYTES_PER_SAMPLE);
		pcmFormat.bitsPerSample = WC_BITS_PER_SAMPLE;

		fileStream.write((const char*)&pcmFormat, sizeof(pcmFormat));

		PrefixHeader dataPrefix;
		std::memcpy(dataPrefix.code, "data", 4);
		dataPrefix.size = quint32(WC_BYTES_PER_SAMPLE * samplesCount * channelsCount);

		fileStream.write((const char*)&dataPrefix, sizeof(dataPrefix));

		istd::CRange range = istd::CRange::GetInvalid();
		for (int sampleIndex = 0; sampleIndex < samplesCount; ++sampleIndex){
			for (int channelIndex = 0; channelIndex < channelsCount; ++channelIndex){
				double sample = sequencePtr->GetSample(sampleIndex, channelIndex);

				range.Unite(sample);
			}
		}

		double offset = qPow(2.0, WC_BITS_PER_SAMPLE - 1);
		double amplitude = qPow(2.0, WC_BITS_PER_SAMPLE - 1) - I_BIG_EPSILON;
		for (int sampleIndex = 0; sampleIndex < samplesCount; ++sampleIndex){
			for (int channelIndex = 0; channelIndex < channelsCount; ++channelIndex){
				double sample = sequencePtr->GetSample(sampleIndex, channelIndex);

				qint32 rawSample = qint32(range.GetAlphaFromValue(sample) * amplitude + offset);

				fileStream.write((const char*)&rawSample, WC_BYTES_PER_SAMPLE);
			}
		}

		retVal = (fileStream.error() == QFile::NoError)? OS_OK : OS_FAILED;
	}

	fileStream.close();

	return retVal;
}


// reimplemented (ifile::IFileTypeInfo)

bool CWavSamplesLoaderComp::GetFileExtensions(QStringList& result, const istd::IChangeable* /*dataObjectPtr*/, int flags, bool doAppend) const
{
	if (!doAppend){
		result.clear();
	}

	if ((flags & QF_SAVE) != 0){
		result.push_back("wav");
	}

	return true;
}


QString CWavSamplesLoaderComp::GetTypeDescription(const QString* extensionPtr) const
{
	if ((extensionPtr != NULL) && (*extensionPtr != "wav")){
		return "";
	}

	return "Sound file";
}


} // namespace imeas


