// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include "MeasPck.h"


#include <icomp/export.h>


namespace MeasPck
{


I_EXPORT_PACKAGE(
			"AcfSln/Measure",
			"System-undependent package for sampling, measurement acquisition and related thema",
			"Measurement Acquisition Undependend" IM_TAG("Sampling") IM_PROJECT("ACF-Solutions") IM_COMPANY("ImagingTools"));

I_EXPORT_COMPONENT(
			GeneralBitmap,
			"General, system undependent bitmap implementation",
			"Bitmap Picture Acquisition Simple General" IM_TAG("Image") IM_AUTHOR("\"Witold Gantzke\" \"Kirill Lepskiy\""));

I_EXPORT_COMPONENT(
			GeneralDataSequence,
			"General, system undependent sample sequence implementation",
			"Samples Sequence Acquisition" IM_AUTHOR("\"Witold Gantzke\""));

I_EXPORT_COMPONENT(
			SimpleSamplesSequence8,
			"Simple sample sequence storing 8-bit samples, system undependent",
			"Samples Sequence Measurement" IM_AUTHOR("\"Witold Gantzke\" \"Kirill Lepskiy\""));

I_EXPORT_COMPONENT(
			SimpleSamplesSequence16,
			"Simple sample sequence storing 16-bit samples, system undependent",
			"Samples Sequence Measurement" IM_AUTHOR("\"Witold Gantzke\" \"Kirill Lepskiy\""));

I_EXPORT_COMPONENT(
			SimpleSamplesSequence32,
			"Simple sample sequence storing 32-bit samples, system undependent",
			"Samples Sequence Measurement" IM_AUTHOR("\"Witold Gantzke\" \"Kirill Lepskiy\""));

I_EXPORT_COMPONENT(
			WavSamplesLoader,
			"Loader for loading and saving sample sequence as Audio WAV file",
			"Audio WAV Loder Serializer Persistence Samples Sequence Measurement" IM_TAG("Loader") IM_AUTHOR("\"Witold Gantzke\""));

I_EXPORT_COMPONENT(
			SimpleDataStatistics,
			"Basic statistics of a numerical data",
			"Data" IM_TAG("Statistics") IM_AUTHOR("\"Kirill Lepskiy\""));

I_EXPORT_COMPONENT(
			DataSequenceStatistics,
			"Data sequence statistics",
			"Sequence Data" IM_TAG("Statistics") IM_AUTHOR("\"Kirill Lepskiy\""));

I_EXPORT_COMPONENT(
			HistogramStatisticsProcessor,
			"Processor for calculation of statistics",
			"Histogram Statistics" IM_TAG("Processing") IM_AUTHOR("\"Kirill Lepskiy\""));

I_EXPORT_COMPONENT(
			NumericParams,
			"Set of numeric values used for filter values, channel values etc.",
			"Numeric Values Parameter" IM_AUTHOR("\"Witold Gantzke\""));

I_EXPORT_COMPONENT(
			NumericConstraints,
			"Defines simple constraints for numeric parameters with the same units and ranges for all values",
			"Numeric Values Parameter Constraints" IM_AUTHOR("\"Witold Gantzke\""));

I_EXPORT_COMPONENT(
			NumericParamsProxy,
			"Simple proxy of numeric parameters",
			"Numeric Values Parameter Proxy" IM_AUTHOR("\"DHDAM\""));

I_EXPORT_COMPONENT(
			LinearAdjustParams,
			"Parameter with scale and offset for simple linear transformations",
			"Parameter Scale Offset Shift Contrast Brightness Linear Power" IM_CATEGORY(I_DATA_MODEL) IM_TAG("Model Parameter") IM_AUTHOR("\"Witold Gantzke\""));

I_EXPORT_COMPONENT(
			CrossCorrelationProcessor,
			"Calculate the cross correlation of two data sequences",
			"Correlation Processor Comparison" IM_AUTHOR("\"Kirill Lepskiy\""));

I_EXPORT_COMPONENT(
			RelativeUnitSelector,
			"Selector of unit info for relative values",
			"Correlation Processor Comparison" IM_AUTHOR("\"Kirill Lepskiy\""));

I_EXPORT_COMPONENT(
			LinearlyScaledNumericParam,
			"Numeric parameter with linear scaling from a source value",
			"Numeric Values Parameter Scale Linear" IM_AUTHOR("\"Witold Gantzke\""));


} // namespace MeasPck


