#ifndef iipr_CSingleFeatureConsumer_included
#define iipr_CSingleFeatureConsumer_included


// ACF includes
#include <istd/TDelPtr.h>
#include <iipr/IFeaturesConsumer.h>
#include <iipr/IFeaturesProvider.h>


namespace iipr
{


/**
	Feature consumer implmentation storing one feature only.
*/
class CSingleFeatureConsumer:
			virtual public IFeaturesConsumer,
			virtual public IFeaturesProvider
{
public:

	/**
		Control storing of the features in the container.
	*/
	enum FeaturePolicy
	{
		/**
			Only first feature is stored in the container.
		*/
		FP_FIRST,

		/**
			Only last feature is stored in the container.
		*/
		FP_LAST,

		/**
			Only heaviest feature is stored in the container
		*/
		FP_HEAVIEST
	};

	CSingleFeatureConsumer(int featurePolicy = FP_FIRST);

	/**
		Get feature adding policy.
		\sa FeaturePolicy
	*/
	int GetFeaturePolicy() const;

	/**
		Set feature adding policy. 
		\note Call of this function removes the current feature.
	*/
	void SetFeaturePolicy(int featurePolicy);

	// reimplemented (iipr::IFeaturesConsumer)
	virtual void ResetFeatures() override;
	virtual bool AddFeature(const imeas::INumericValue* featurePtr, bool* isFullPtr = nullptr) override;

	// reimplemented (iipr::IFeaturesProvider)
	virtual int GetFeaturesCount() const override;
	virtual const imeas::INumericValue& GetFeature(int index) const override;

private:
	istd::TDelPtr<const imeas::INumericValue> m_featurePtr;

	int m_featurePolicy;
};


} // namespace iipr


#endif // !iipr_CSingleFeatureConsumer_included


