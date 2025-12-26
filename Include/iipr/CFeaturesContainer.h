#ifndef iipr_CFeaturesContainer_included
#define iipr_CFeaturesContainer_included


#include <istd/TPointerVector.h>

#include <iipr/IFeaturesConsumer.h>
#include <iipr/IFeaturesProvider.h>


namespace iipr
{


/**
	Basic implementation of a features container.
*/
class CFeaturesContainer:
			virtual public IFeaturesConsumer,
			virtual public IFeaturesProvider
{
public:
	/**
		Remove feature at selected index.
		The following elements will be shift, size of this container will be decreased.
	*/
	void RemoveFeature(int featureIndex);
	/**
		Get access to stored feature.
	*/
	imeas::INumericValue& GetFeatureRef(int index);

	// reimplemented (iipr::IFeaturesConsumer)
	virtual void ResetFeatures() override;
	virtual bool AddFeature(const imeas::INumericValue* featurePtr, bool* isFullPtr = nullptr) override;

	// reimplemented (iipr::IFeaturesProvider)
	virtual int GetFeaturesCount() const override;
	virtual const imeas::INumericValue& GetFeature(int index) const override;

	//reimplemented (istd::IChangeable)
	virtual bool ResetData(CompatibilityMode mode = CM_WITHOUT_REFS) override;

private:
	typedef istd::TPointerVector<imeas::INumericValue> FeaturesList;

	FeaturesList m_featuresList;
};


} // namespace iipr


#endif // !iipr_CFeaturesContainer_included

