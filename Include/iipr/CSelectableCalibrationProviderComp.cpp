#include <iipr/CSelectableCalibrationProviderComp.h>


namespace iipr
{


// public methods

// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CSelectableCalibrationProviderComp::GetCalibration() const
{
	if (m_multiCalibrationProviderCompPtr.IsValid() && m_calibrationSelectionCompPtr.IsValid()){
		int selectedIndex = m_calibrationSelectionCompPtr->GetSelectedOptionIndex();
		if (selectedIndex >= 0){
			// Try to select a calibration by unique ID:
			const iprm::IOptionsList* calibrationListPtr = m_calibrationSelectionCompPtr->GetSelectionConstraints();
			const iprm::IOptionsList* foundCalibrationListPtr = m_multiCalibrationProviderCompPtr->GetCalibrationSelectionContraints();
			if ((calibrationListPtr != nullptr) && (foundCalibrationListPtr != nullptr)){
				QByteArray referenceId = calibrationListPtr->GetOptionId(selectedIndex);
				for (int foundCalibrationIndex = 0; foundCalibrationIndex < foundCalibrationListPtr->GetOptionsCount(); ++foundCalibrationIndex){
					QByteArray calibrationId = foundCalibrationListPtr->GetOptionId(foundCalibrationIndex);
					if (calibrationId == referenceId){
						return m_multiCalibrationProviderCompPtr->GetCalibration(foundCalibrationIndex);
					}
				}
			}
			else{
				// Try to select a calibration using simple index:
				int calibrationsCount = m_multiCalibrationProviderCompPtr->GetCalibrationsCount();
				if (selectedIndex < calibrationsCount){
					return m_multiCalibrationProviderCompPtr->GetCalibration(selectedIndex);
				}
			}
		}
	}

	if (m_defaultCalibrationProviderCompPtr.IsValid()){
		return m_defaultCalibrationProviderCompPtr->GetCalibration();
	}

	return nullptr;
}


} // namespace iipr


