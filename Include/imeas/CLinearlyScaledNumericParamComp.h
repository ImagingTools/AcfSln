#pragma once


// ACF includes
#include <icomp/CComponentBase.h>
#include <imod/CModelUpdateBridge.h>
#include <imod/IModel.h>

// ACF-Solutions includes
#include <imeas/INumericConstraints.h>
#include <imeas/INumericValue.h>


namespace imeas
{


/**
	Component implementing imeas::INumericValue with linear scaling from a source value.

	Each element is scaled between own constraints range and source constraints range:
		scaled[i] = ownMin + (source[i] - srcMin) * (ownMax - ownMin) / (srcMax - srcMin)
		source[i] = srcMin + (scaled[i] - ownMin) * (srcMax - srcMin) / (ownMax - ownMin)

	Ranges are taken from INumericConstraints of own and source components respectively.
*/
class CLinearlyScaledNumericParamComp : public icomp::CComponentBase, virtual public imeas::INumericValue
{
public:
	typedef icomp::CComponentBase BaseClass;

	I_BEGIN_COMPONENT(CLinearlyScaledNumericParamComp)
		I_REGISTER_INTERFACE(istd::IChangeable);
		I_REGISTER_INTERFACE(iser::ISerializable);
		I_REGISTER_INTERFACE(imeas::INumericValue);
		I_ASSIGN(m_sourceValueCompPtr, "SourceValue", "Source numeric value to scale from", true, "SourceValue");
		I_ASSIGN_TO(m_sourceModelCompPtr, m_sourceValueCompPtr, true);
		I_ASSIGN(m_constraintsCompPtr, "Constraints", "Own constraints describing value ranges", false, "Constraints");
	I_END_COMPONENT;

	CLinearlyScaledNumericParamComp();

	// reimplemented (imeas::INumericValue)
	bool IsValueTypeSupported(ValueTypeId valueTypeId) const override;
	const imeas::INumericConstraints* GetNumericConstraints() const override;
	imath::CVarVector GetComponentValue(ValueTypeId valueTypeId) const override;
	imath::CVarVector GetValues() const override;
	bool SetValues(const imath::CVarVector& values) override;

	// reimplemented (iser::ISerializable)
	bool Serialize(iser::IArchive& archive) override;

	// reimplemented (istd::IChangeable)
	bool CopyFrom(const IChangeable& object, CompatibilityMode mode = CM_WITHOUT_REFS) override;

protected:
	// reimplemented (icomp::CComponentBase)
	void OnComponentCreated() override;
	void OnComponentDestroyed() override;

private:
	double SourceToOwn(double sourceValue, int index) const;
	double OwnToSource(double ownValue, int index) const;

private:
	I_REF(imeas::INumericValue, m_sourceValueCompPtr);
	I_REF(imod::IModel, m_sourceModelCompPtr);
	I_REF(imeas::INumericConstraints, m_constraintsCompPtr);

	imod::CModelUpdateBridge m_updateBridge;
};


} // namespace imeas
