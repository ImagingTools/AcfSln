#ifndef iipr_IFeaturesConsumer_included
#define iipr_IFeaturesConsumer_included


#include <istd/IChangeable.h>

#include <imeas/INumericValue.h>


namespace iipr
{


/**
	Common interface for a features container consuming feature objects.
*/
class IFeaturesConsumer: virtual public istd::IChangeable
{
public:
	/**
		Remove all features from this container.
	*/
	virtual void ResetFeatures() = 0;

	/**
		Add new feature to the set of features.
		\param	featurePtr	instance of feature object.
							Please note, that the container takes controll over this object, 
							so you cannot destroy this object manually.
							It cannot be nullptr.
		\param isFullPtr	Optional parameter pointing at flag idicating that container is full.
							This flag will be set to true, if this container is full and it will not collect additional features.
							In other case the flag will be not changed.
		\return	true if the pushed feature is valid for this container and was inserted correctly.
	*/
	virtual bool AddFeature(const imeas::INumericValue* featurePtr, bool* isFullPtr = nullptr) = 0;
};


} // namespace iipr


#endif // !iipr_IFeaturesConsumer_included

