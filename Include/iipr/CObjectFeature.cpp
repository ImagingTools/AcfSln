#include <iipr/CObjectFeature.h>


// ACF includes
#include <istd/TDelPtr.h>
#include <istd/CChangeNotifier.h>
#include <iser/IArchive.h>
#include <iser/CArchiveTag.h>


namespace iipr
{


// public methods

CObjectFeature::CObjectFeature()
	:m_angle(0.0),
	m_scale(1.0, 1.0)
{
	BaseClass2::SetPosition(i2d::CVector2d(0.0, 0.0));

	imath::CVarVector values(6, 0.0);
	values[0] = 0;
	values[1] = 0.0;
	values[2] = 0.0;
	values[3] = 1.0;
	values[4] = 1.0;
	values[5] = 0.0;

	SetValues(values);
}


CObjectFeature::CObjectFeature(
			double weight,
			const i2d::CVector2d& position,
			double angle,
			const i2d::CVector2d& scale)
	:BaseClass(weight),
	m_angle(angle),
	m_scale(scale)
{
	BaseClass2::SetPosition(position);

	imath::CVarVector values(6, 0.0);

	values[0] = weight;
	values[1] = position.GetX();
	values[2] = position.GetY();
	values[3] = scale.GetX();
	values[4] = scale.GetY();
	values[5] = angle;

	SetValues(values);
}


double CObjectFeature::GetAngle() const
{
	return m_angle;
}


i2d::CVector2d CObjectFeature::GetScale() const
{
	return m_scale;
}


void CObjectFeature::SetScale(const i2d::CVector2d& scale)
{
	m_scale = scale;
}


QByteArray CObjectFeature::GetObjectId() const
{
	return m_objectId;
}


void CObjectFeature::SetObjectId(const QByteArray& objectId)
{
	if (m_objectId != objectId)
	{
		istd::CChangeNotifier changeNotifier(this);

		m_objectId = objectId;
	}
}


const i2d::ITransformation2d& CObjectFeature::GetTransformation() const
{
	m_transformation.Reset(GetPosition(), m_angle);

	return m_transformation;
}


// reimplemented (imeas::INumericValue)

bool CObjectFeature::IsValueTypeSupported(CObjectFeature::ValueTypeId valueTypeId) const
{
	switch (valueTypeId){
	case VTI_AUTO:
	case VTI_POSITION:
	case VTI_ANGLE:
	case VTI_WEIGHT:
	case VTI_2D_TRANSFORM:
		return true;

	default:
		return false;
	}
}


imath::CVarVector CObjectFeature::GetComponentValue(CObjectFeature::ValueTypeId valueTypeId) const
{
	imath::CVarVector result;

	switch (valueTypeId){
	case VTI_AUTO:
		return BaseClass::GetComponentValue(VTI_AUTO);

	case VTI_POSITION:
		result.SetElementsCount(2, 0);
		result[0] = GetPosition().GetX();
		result[1] = GetPosition().GetY();
		break;

	case VTI_ANGLE:
		result.SetElementsCount(1, m_angle);
		break;

	case VTI_WEIGHT:
		result.SetElementsCount(1, m_weight);
		break;

	case VTI_2D_TRANSFORM:
		result.SetElementsCount(6);
		m_transformation.Reset(GetPosition(), m_angle);
		{
			const i2d::CMatrix2d& matrix = m_transformation.GetTransformation().GetDeformMatrix();
			result.SetElement(0, matrix.GetAt(0,0));
			result.SetElement(1, matrix.GetAt(0,1));
			result.SetElement(2, matrix.GetAt(1,0));
			result.SetElement(3, matrix.GetAt(1,1));

			const i2d::CVector2d& translation = m_transformation.GetTransformation().GetTranslation();
			result.SetElement(4, translation.GetX());
			result.SetElement(5, translation.GetY());
		}
		break;

	default:
		break;
	}

	return result;
}


// reimplemented (iser::ISerializable)

bool CObjectFeature::Serialize(iser::IArchive& archive)
{
	bool retVal = true;

	retVal = retVal && BaseClass::Serialize(archive);

	static iser::CArchiveTag angleTag("Angle", "Angle of found model", iser::CArchiveTag::TT_LEAF);
	static iser::CArchiveTag scaleTag("Scale", "Scale of found model", iser::CArchiveTag::TT_LEAF);
	static iser::CArchiveTag objectIdTag("ObjectId", "ID of the found object", iser::CArchiveTag::TT_LEAF);

	retVal = retVal && archive.BeginTag(angleTag);
	retVal = retVal && archive.Process(m_angle);
	retVal = retVal && archive.EndTag(angleTag);

	retVal = retVal && archive.BeginTag(scaleTag);
	retVal = retVal && m_scale.Serialize(archive);
	retVal = retVal && archive.EndTag(scaleTag);

	retVal = retVal && archive.BeginTag(objectIdTag);
	retVal = retVal && archive.Process(m_objectId);
	retVal = retVal && archive.EndTag(objectIdTag);

	return retVal;
}


// reimplemented (istd::IChangeable)

bool CObjectFeature::CopyFrom(const IChangeable& object, CompatibilityMode mode)
{
	const CObjectFeature* objectPtr = dynamic_cast<const CObjectFeature*>(&object);

	if (objectPtr != nullptr){
		istd::CChangeNotifier notifier(this);

		m_angle = objectPtr->m_angle;
		m_scale = objectPtr->m_scale;
		m_objectId = objectPtr->m_objectId;

		BaseClass::CopyFrom(object, mode);

		return true;
	}

	return false;
}


istd::IChangeableUniquePtr CObjectFeature::CloneMe(CompatibilityMode mode) const
{
	istd::TDelPtr<CObjectFeature> retVal(new CObjectFeature);

	if (retVal->CopyFrom(*this, mode)){
		return retVal.PopPtr();
	}

	return nullptr;
}


} // namespace iipr


