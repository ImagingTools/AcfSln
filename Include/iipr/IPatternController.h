#ifndef iipr_IPatternController_included
#define iipr_IPatternController_included


#include <istd/IChangeable.h>


namespace iipr
{


class IPatternController: virtual public istd::IChangeable
{
public:
	/**
		Data model change notification flags.
	*/
	enum ChangeFlags
	{
		/**
			Supplier result changed.
		*/
		CF_PATTERN_LEARNED = 0x456d447
	};

	/**
		Do teaching of the pattern.
	*/
	virtual bool TeachPattern(const istd::IChangeable* sourceObjectPtr = nullptr) = 0;

	/**
		Clear all pattern features.
	*/
	virtual void ResetPattern() = 0;

	/**
		Get \c true of the pattern is valid
	*/
	virtual bool IsPatternValid() const = 0;

	/**
		Get pattern object.
	*/
	virtual const iser::ISerializable* GetPatternObject() const = 0;
};


} // namespace iipr


#endif // !iipr_IPatternController_included
