#ifndef iipr_CGeometryMessage_included
#define iipr_CGeometryMessage_included


// ACF includes
#include <ilog/CExtMessage.h>
#include <i2d/CObject2dFactory.h>


namespace iipr
{


/**
	Message implementation for transport of 2D-objects.
*/
class CGeometryMessage: public ilog::CExtMessage
{
public:
	typedef ilog::CExtMessage BaseClass;

	static QByteArray GetTypeName();

	CGeometryMessage();
	CGeometryMessage(istd::IInformationProvider::InformationCategory category,
				int id,
				const QString& text,
				const QString& source,
				int flags = 0,
				const QDateTime* timeStampPtr = nullptr);

	// reimplemented (iser::IObject)
	virtual QByteArray GetFactoryId() const override;

	// reimplemented (iser::IChangeable)
	virtual istd::IChangeableUniquePtr CloneMe(CompatibilityMode mode = CM_WITHOUT_REFS) const override;

private:
	static i2d::CObject2dFactory s_factory;
};


} // namespace iipr


#endif // !iipr_CGeometryMessage_included


