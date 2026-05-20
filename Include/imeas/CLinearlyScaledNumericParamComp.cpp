#include "CLinearlyScaledNumericParamComp.h"


namespace gmg::meas2
{


// public methods

CLinearlyScaledNumericParamComp::CLinearlyScaledNumericParamComp()
	: m_updateBridge(this, imod::CModelUpdateBridge::UF_DELEGATED)
{
}


// reimplemented (imeas::INumericValue)

bool CLinearlyScaledNumericParamComp::IsValueTypeSupported(ValueTypeId valueTypeId) const
{
	if (!m_sourceValueCompPtr.IsValid()) {
		return false;
	}

	return m_sourceValueCompPtr->IsValueTypeSupported(valueTypeId);
}


const imeas::INumericConstraints* CLinearlyScaledNumericParamComp::GetNumericConstraints() const
{
	if (m_constraintsCompPtr.IsValid()) {
		return m_constraintsCompPtr.GetPtr();
	}

	return nullptr;
}


imath::CVarVector CLinearlyScaledNumericParamComp::GetComponentValue(ValueTypeId valueTypeId) const
{
	Q_ASSERT(valueTypeId == VTI_AUTO);

	if (valueTypeId != VTI_AUTO) {
		return imath::CVarVector();
	}

	return GetValues();
}


imath::CVarVector CLinearlyScaledNumericParamComp::GetValues() const
{
	if (!m_sourceValueCompPtr.IsValid()) {
		return imath::CVarVector();
	}

	auto sourceValues = m_sourceValueCompPtr->GetValues();
	int count = sourceValues.GetElementsCount();

	imath::CVarVector retVal(count);
	for (int i = 0; i < count; ++i) {
		retVal.SetElement(i, SourceToOwn(sourceValues.GetElement(i), i));
	}

	return retVal;
}


bool CLinearlyScaledNumericParamComp::SetValues(const imath::CVarVector& values)
{
	if (!m_sourceValueCompPtr.IsValid()) {
		return false;
	}

	int count = values.GetElementsCount();

	imath::CVarVector sourceValues(count);
	for (int i = 0; i < count; ++i) {
		sourceValues.SetElement(i, OwnToSource(values.GetElement(i), i));
	}

	bool retVal = m_sourceValueCompPtr->SetValues(sourceValues);

	return retVal;
}


// reimplemented (iser::ISerializable)

bool CLinearlyScaledNumericParamComp::Serialize(iser::IArchive& /*archive*/)
{
	return false;
}


// reimplemented (istd::IChangeable)

bool CLinearlyScaledNumericParamComp::CopyFrom(const IChangeable& /*object*/, CompatibilityMode /*mode*/)
{
	return false;
}


// protected methods

// reimplemented (icomp::CComponentBase)

void CLinearlyScaledNumericParamComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_sourceModelCompPtr.IsValid()) {
		m_sourceModelCompPtr->AttachObserver(&m_updateBridge);
	}
}


void CLinearlyScaledNumericParamComp::OnComponentDestroyed()
{
	m_updateBridge.EnsureModelsDetached();

	BaseClass::OnComponentDestroyed();
}


// private methods

double CLinearlyScaledNumericParamComp::SourceToOwn(double sourceValue, int index) const
{
	const imeas::INumericConstraints* ownConstraints = GetNumericConstraints();
	const imeas::INumericConstraints* sourceConstraints =
		m_sourceValueCompPtr.IsValid() ? m_sourceValueCompPtr->GetNumericConstraints() : nullptr;

	if (ownConstraints == nullptr || sourceConstraints == nullptr) {
		Q_ASSERT_X(false, "CLinearlyScaledNumericParamComp::SourceToOwn", "Invalid constraints");

		return sourceValue;
	}

	const imath::IUnitInfo* ownUnitInfo = ownConstraints->GetNumericValueUnitInfo(index);
	const imath::IUnitInfo* srcUnitInfo = sourceConstraints->GetNumericValueUnitInfo(index);

	if (ownUnitInfo == nullptr || srcUnitInfo == nullptr) {
		Q_ASSERT_X(false, "CLinearlyScaledNumericParamComp::SourceToOwn", "Invalid unit info");

		return sourceValue;
	}

	istd::CRange ownRange = ownUnitInfo->GetValueRange();
	istd::CRange srcRange = srcUnitInfo->GetValueRange();

	if (!ownRange.IsValid() || !srcRange.IsValid()) {
		Q_ASSERT_X(false, "CLinearlyScaledNumericParamComp::SourceToOwn", "Invalid value range");

		return sourceValue;
	}

	double srcSpan = srcRange.GetMaxValue() - srcRange.GetMinValue();
	if (qFuzzyIsNull(srcSpan)) {
		return ownRange.GetMinValue();
	}

	double ownSpan = ownRange.GetMaxValue() - ownRange.GetMinValue();
	double normalized = (sourceValue - srcRange.GetMinValue()) / srcSpan;

	auto retVal = ownRange.GetMinValue() + normalized * ownSpan;

	return retVal;
}


double CLinearlyScaledNumericParamComp::OwnToSource(double ownValue, int index) const
{
	const imeas::INumericConstraints* ownConstraints = GetNumericConstraints();
	const imeas::INumericConstraints* sourceConstraints =
		m_sourceValueCompPtr.IsValid() ? m_sourceValueCompPtr->GetNumericConstraints() : nullptr;

	if (ownConstraints == nullptr || sourceConstraints == nullptr) {
		Q_ASSERT_X(false, "CLinearlyScaledNumericParamComp::OwnToSource", "Invalid constraints");

		return ownValue;
	}

	const imath::IUnitInfo* ownUnitInfo = ownConstraints->GetNumericValueUnitInfo(index);
	const imath::IUnitInfo* srcUnitInfo = sourceConstraints->GetNumericValueUnitInfo(index);

	if (ownUnitInfo == nullptr || srcUnitInfo == nullptr) {
		Q_ASSERT_X(false, "CLinearlyScaledNumericParamComp::OwnToSource", "Invalid unit info");

		return ownValue;
	}

	istd::CRange ownRange = ownUnitInfo->GetValueRange();
	istd::CRange srcRange = srcUnitInfo->GetValueRange();

	if (!ownRange.IsValid() || !srcRange.IsValid()) {
		Q_ASSERT_X(false, "CLinearlyScaledNumericParamComp::OwnToSource", "Invalid value range");

		return ownValue;
	}

	double ownSpan = ownRange.GetMaxValue() - ownRange.GetMinValue();
	if (qFuzzyIsNull(ownSpan)) {
		return srcRange.GetMinValue();
	}

	double srcSpan = srcRange.GetMaxValue() - srcRange.GetMinValue();
	double normalized = (ownValue - ownRange.GetMinValue()) / ownSpan;

	auto retVal = srcRange.GetMinValue() + normalized * srcSpan;

	return retVal;
}


} // namespace gmg::meas2
