#include <iipr/CGeometryMessage.h>


// ACF includes
#include <ilog/CMessageContainer.h>


namespace iipr
{


// public methods

CGeometryMessage::CGeometryMessage()
	:BaseClass(&s_factory)
{
}


CGeometryMessage::CGeometryMessage(
			istd::IInformationProvider::InformationCategory category,
			int id,
			const QString& text,
			const QString& source,
			int flags,
			const QDateTime* timeStampPtr)
	:BaseClass(category, id, text, source, flags, timeStampPtr, &s_factory)
{
}


QByteArray CGeometryMessage::GetFactoryId() const
{
	return GetTypeName();
}


// reimplemented (iser::IChangeable)

istd::IChangeableUniquePtr CGeometryMessage::CloneMe(CompatibilityMode mode) const
{
	istd::IChangeableUniquePtr clonedPtr(new CGeometryMessage);
	if (clonedPtr->CopyFrom(*this, mode)){
		return clonedPtr;
	}

	return nullptr;
}


QByteArray CGeometryMessage::GetTypeName()
{
	return istd::CClassInfo::GetName<CGeometryMessage>();
}


// private static members

i2d::CObject2dFactory CGeometryMessage::s_factory;


I_REGISTER_MESSAGE_TYPE(CGeometryMessage, CGeometryMessage::GetTypeName());


} // namespace iipr


