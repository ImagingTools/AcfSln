#ifndef iipr_CBitmapOperations_included
#define iipr_CBitmapOperations_included


// ACF includes
#include <istd/ILogger.h>
#include <ilog/IMessageConsumer.h>

// ACF-Solutions includes
#include <iimg/IBitmapProvider.h>


namespace iipr
{


/**
	Set of atmomic bitmap manipulations.
*/
class CBitmapOperations
{
public:
	/**
		Processing mode for join of more bitmaps.
	*/
	enum JoinMode
	{
		/**
			Bitmaps are placed vertically in the output image.
		*/
		JM_VERTICAL,

		/**
			Bitmaps are placed horizontally in the output image.
		*/
		JM_HORIZONTAL,
	};

	/**
		Processing mode for calculation of more bitmaps into a single one.
	*/
	enum CombineMode
	{
		/**
			Average value of all bitmaps will be used in the output.
			If this mode is active, an additional list of weights (for each input bitmap) can be set.
			In this case the averaging is weighted and calculated as outputValue = (input1 * weight1 + input2 * weight 2 + ... ) / Sum (weights)
		*/
		CM_AVERAGE,

		/**
			As result value of a bitmap pixel the maximum of all input bitmap values on the given position is taken.
		*/
		CM_MAX,
		/**
			As result value of a bitmap pixel the minimum of all input bitmap values on the given position is taken.
		*/
		CM_MIN
	};

	/**
		Definition of bitmap list.
	*/
	typedef QVector<const iimg::IBitmap*> BitmapList;

	/**
		Crop the bitmap to the bounding box of a given region.
	*/
	static bool ReduceBitmapToRegion(const iimg::IBitmap& inputBitmap, const i2d::IObject2d& region, iimg::IBitmap& outputBitmap);

	/**
		Join more bitmaps into a single one by placing of the input bitmaps into the output.
		\sa JoinMode
	*/
	static bool JoinBitmaps(
				const BitmapList& bitmapList,
				JoinMode joinMode,
				iimg::IBitmap& outputBitmap,
				istd::ILogger* operationLogPtr = nullptr);

	/**
		Join more bitmaps into a single one by calculation of the pixel values from the input bitmap list.
		\sa CombineMode
	*/
	static bool CombineBitmaps(
				const BitmapList& bitmapList,
				CombineMode combineMode,
				iimg::IBitmap& outputBitmap,
				imath::CVarVector* weghtsPtr = nullptr,
				istd::ILogger* operationLogPtr = nullptr);

	/**
		Calculate the difference of two bitmaps.
	*/
	static bool CaclulateBitmapDifference(
				const iimg::IBitmap& inputBitmap1,
				const iimg::IBitmap& inputBitmap2,
				iimg::IBitmap& outputBitmap,
				double offset = 0,
				ilog::IMessageConsumer* messageConsumerPtr = nullptr);
};


} // namespace iipr


#endif // !iipr_CBitmapOperations_included


