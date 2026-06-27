// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iipr/CCheckerboardPointGridExtractorComp.h>

// ACF includes
#include <i2d/CLine2d.h>
#include <i2d/CPosition2d.h>
#include <ilog/CExtMessage.h>

// QsfCore includes
#include <iipr/IFeaturesConsumer.h>
#include <iipr/CPointGridFeature.h>

namespace iipr
{

struct LinesConsumer: virtual public iipr::IFeaturesConsumer
{
	struct LineInfo
	{
		double weight;
		i2d::CLine2d line;
	};

	// reimplemented (iipr::IFeaturesConsumer)
	virtual void ResetFeatures() override
	{
		lines.clear();
	}

	virtual bool AddFeature(const imeas::INumericValue* featurePtr, bool* /*isFullPtr*/) override
	{
		if (featurePtr != NULL){
			const i2d::CLine2d* linePtr = dynamic_cast<const i2d::CLine2d*>(featurePtr);
			LineInfo info;
			if (linePtr != NULL){
				info.line = *linePtr;
				info.weight = 1;
			}
			else {
				imath::CVarVector vector = featurePtr->GetValues();
				if (vector.GetElementsCount() >= 4){
					info.line = i2d::CLine2d(vector[0], vector[1], vector[2], vector[3]);
					if (vector.GetElementsCount() > 4){
						info.weight = vector[4];
					}
					else {
						info.weight = 1;
					}
				}
			}

			if (!info.line.IsNull()){
				lines.push_back(info);
			}
		}

		delete featurePtr;

		return true;
	}

	QList<LineInfo> lines;
};

// reimplemented (iipr::IImageToFeatureProcessor)

iproc::IProcessor::TaskState CCheckerboardPointGridExtractorComp::DoExtractFeatures(
			const iprm::IParamsSet* paramsPtr,
			const iimg::IBitmap& image,
			IFeaturesConsumer& results,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	LinesConsumer processedLines;
	if (m_lineFinderCompPtr->DoExtractFeatures(paramsPtr, image, processedLines) != iproc::IProcessor::TS_OK) {
		SendErrorMessage(0, QObject::tr("LineFinder is undefined"));
		return TS_INVALID;
	}

	//filter nearest parallel lines
	QMap<int, i2d::CLine2d> allLines;
	QSet<int> missedLines;
	for (int i = 0; i < processedLines.lines.size(); ++i){
		i2d::CLine2d line1 = processedLines.lines[i].line;

		//aggregate nearest lines
		for (int j = i + 1; j < processedLines.lines.size(); ++j){
			const i2d::CLine2d & line2 = processedLines.lines[j].line;
			double distance =
				line1.GetExtendedDistance(line2.GetPoint1()) +
				line1.GetExtendedDistance(line2.GetPoint2()) +
				line1.GetExtendedDistance(line1.GetPoint1()) +
				line1.GetExtendedDistance(line1.GetPoint2());

			if (distance < 8){
				const double a1 = (line1.GetPoint1() - line2.GetPoint1()).GetLength2();
				const double a2 = (line1.GetPoint1() - line2.GetPoint2()).GetLength2();
				const double a3 = (line1.GetPoint2() - line2.GetPoint1()).GetLength2();
				const double a4 = (line1.GetPoint2() - line2.GetPoint2()).GetLength2();

				if (a1 >= a2 && a1 >= a3 && a1 >= a4){
					line1.SetPoint1(line1.GetPoint1());
					line1.SetPoint2(line2.GetPoint1());
				}
				else if (a2 > a1 && a2 > a3 && a2 >= a4){
					line1.SetPoint1(line1.GetPoint1());
					line1.SetPoint2(line2.GetPoint2());
				}
				else if (a3 >= a1 && a3 >= a2 && a3 >= a4){
					line1.SetPoint1(line1.GetPoint2());
					line1.SetPoint2(line2.GetPoint1());
				}
				else {
					line1.SetPoint1(line1.GetPoint2());
					line1.SetPoint2(line2.GetPoint2());
				}
				missedLines.insert(j);
			}
		}

		allLines.insert(i, line1);
	}

	std::vector<i2d::CLine2d> filteredLines;
	for (QMap<int, i2d::CLine2d>::iterator it = allLines.begin(); it != allLines.end(); ++it){
		if (!missedLines.contains(it.key())){
			filteredLines.push_back(it.value());
		}
	}

	if (filteredLines.empty()){
		SendErrorMessage(0, QObject::tr("No enough lines found"));
		return TS_INVALID;
	}

	//clusterize lines per its angle
	//make 1D metric space with angle
	struct LineWithAngle {
		LineWithAngle() :  m_lineRef(NULL),m_angle(0){}
		LineWithAngle(const i2d::CLine2d & line) :
			m_lineRef(&line)
		{
			m_angle = line.GetDiffVector().GetAngle();
			// move angle to positie value
			if (m_angle < 0)
				m_angle += I_2PI;

			// move angle to first and second quartal
			if (m_angle > I_PI)
				m_angle -= I_PI;
		}
		const i2d::CLine2d * m_lineRef;
		double m_angle;
	};

	//calc cluster vertexes
	std::vector<LineWithAngle> vert;
	vert.reserve(filteredLines.size());
	for (std::vector<i2d::CLine2d>::iterator lineIt = filteredLines.begin(); lineIt != filteredLines.end(); ++lineIt){
		vert.push_back(LineWithAngle(*lineIt));
	}

	struct {
		bool operator()(const LineWithAngle & a, const LineWithAngle & b) const
		{
			return a.m_angle < b.m_angle;
		}
	} AngleLess;
	std::sort(vert.begin(), vert.end(), AngleLess);

	struct AngleEdge {
		AngleEdge() :
			m_leftLineIndex(0), m_angleDiff(0){
		}
		AngleEdge(size_t leftLineIndex, double angleDiff) :
			m_leftLineIndex(leftLineIndex), m_angleDiff(angleDiff){
		}

		size_t m_leftLineIndex;
		double m_angleDiff;
	};

	//calc cluster edges like angle for edge length
	std::vector<AngleEdge> edges;
	edges.reserve(vert.size() - 1);
	for (size_t i = 0; i < vert.size() - 1; ++i){
		edges.push_back(AngleEdge(i, std::abs(vert[i + 1].m_angle - vert[i].m_angle)));
	}

	struct {
		bool operator()(const AngleEdge & a, const AngleEdge & b) const
		{
			return a.m_angleDiff > b.m_angleDiff;
		}
	} AngleDiffLess;
	std::sort(edges.begin(), edges.end(), AngleDiffLess);

	if (edges.size() < 2){
		SendErrorMessage(0, QObject::tr("No enough lines found"));
		return TS_INVALID;
	}

	bool firstIsLeft = edges[0].m_leftLineIndex < edges[1].m_leftLineIndex;
	const AngleEdge & clusterEdge1 = firstIsLeft ? edges[0] : edges[1];
	const AngleEdge & clusterEdge2 = firstIsLeft ? edges[1] : edges[0];
	std::vector<const LineWithAngle*> cluster[3];

	for (size_t i = 0; i < clusterEdge1.m_leftLineIndex + 1; i++){
		cluster[0].push_back(&vert[i]);
	}
	for (size_t i = clusterEdge1.m_leftLineIndex + 1; i < clusterEdge2.m_leftLineIndex + 1; i++){
		cluster[1].push_back(&vert[i]);
	}
	for (size_t i = clusterEdge2.m_leftLineIndex + 1; i < vert.size(); i++){
		cluster[2].push_back(&vert[i]);
	}

	double angleCluster[3] = {0, 0, 0};
	for (int i = 0; i < 3; ++i){
		for (std::vector<const LineWithAngle*>::iterator it = cluster[i].begin(); it != cluster[i].end(); ++it){
			angleCluster[i] += (*it)->m_angle;
		}
		angleCluster[i] /= cluster[i].size();
	}

	//search minimal angle between 3 clusters
	double a1 = std::abs(angleCluster[0] - angleCluster[1]);
	double a2 = std::abs(angleCluster[1] - angleCluster[2]);
	double a3 = std::abs(angleCluster[0] - angleCluster[2]);

	a1 = a1 > 0.5 * I_PI ? std::abs(a1 - I_PI) : a1;
	a2 = a2 > 0.5 * I_PI ? std::abs(a2 - I_PI) : a2;
	a3 = a3 > 0.5 * I_PI ? std::abs(a3 - I_PI) : a3;

	std::vector<i2d::CLine2d> mergeLines[2];
	//merge 3 clusters to 2 clusters
	if (a1 < a2 && a1 < a3){
		for (std::vector<const LineWithAngle*>::iterator it = cluster[0].begin(); it != cluster[0].end(); ++it){
			mergeLines[0].push_back(*((*it)->m_lineRef));
		}
		for (std::vector<const LineWithAngle*>::iterator it = cluster[1].begin(); it != cluster[1].end(); ++it){
			mergeLines[0].push_back(*((*it)->m_lineRef));
		}
		for (std::vector<const LineWithAngle*>::iterator it = cluster[2].begin(); it != cluster[2].end(); ++it){
			mergeLines[1].push_back(*((*it)->m_lineRef));
		}
	}
	else if (a2 < a1 && a2 < a3){
		for (std::vector<const LineWithAngle*>::iterator it = cluster[0].begin(); it != cluster[0].end(); ++it){
			mergeLines[0].push_back(*((*it)->m_lineRef));
		}
		for (std::vector<const LineWithAngle*>::iterator it = cluster[1].begin(); it != cluster[1].end(); ++it){
			mergeLines[1].push_back(*((*it)->m_lineRef));
		}
		for (std::vector<const LineWithAngle*>::iterator it = cluster[2].begin(); it != cluster[2].end(); ++it){
			mergeLines[1].push_back(*((*it)->m_lineRef));
		}
	}
	else {
		for (std::vector<const LineWithAngle*>::iterator it = cluster[0].begin(); it != cluster[0].end(); ++it){
			mergeLines[0].push_back(*((*it)->m_lineRef));
		}
		for (std::vector<const LineWithAngle*>::iterator it = cluster[2].begin(); it != cluster[2].end(); ++it){
			mergeLines[0].push_back(*((*it)->m_lineRef));
		}
		for (std::vector<const LineWithAngle*>::iterator it = cluster[1].begin(); it != cluster[1].end(); ++it){
			mergeLines[1].push_back(*((*it)->m_lineRef));
		}
	}

	double vanAngle[2];
	for (int i = 0; i < 2; ++i){
		for (std::vector<i2d::CLine2d>::iterator lineIt = mergeLines[i].begin(); lineIt != mergeLines[i].end(); ++lineIt){
			double angle = lineIt->GetDiffVector().GetAngle();

			if (angle < 0){
				angle += I_2PI;
			}

			if (angle > I_PI){
				angle -= I_PI;
			}

			vanAngle[i] += angle;
		}
		vanAngle[i] /= mergeLines[i].size();
	}

	std::vector<i2d::CLine2d> vanLines[2];
	if (sin(vanAngle[0]) > sin(vanAngle[1])){ //0=X 1=Y
		vanLines[0] = mergeLines[0];
		vanLines[1] = mergeLines[1];
	}
	else {
		vanLines[0] = mergeLines[1];
		vanLines[1] = mergeLines[0];
	}

	// mapping of X and Y axis to valishing points
	// sort lines pointing at single vanishing point using its angle
	struct {
		bool operator()(const i2d::CLine2d & a, const i2d::CLine2d & b) const
		{
			return a.GetCenter().GetX() < b.GetCenter().GetX();
		}
	} LineXLess;
	std::sort(vanLines[0].begin(), vanLines[0].end(), LineXLess);

	struct {
		bool operator()(const i2d::CLine2d & a, const i2d::CLine2d & b) const
		{
			return a.GetCenter().GetY() > b.GetCenter().GetY(); //physical axis goes from bottom to top
		}
	} LineYLess;
	std::sort(vanLines[1].begin(), vanLines[1].end(), LineYLess);

	iipr::CPointGridFeature* crossPositions = new iipr::CPointGridFeature();
	crossPositions->SetSize(0, int(vanLines[0].size()));
	crossPositions->SetSize(1, int(vanLines[1].size()));

	for (int axisYLineIndex = 0; axisYLineIndex < int(vanLines[1].size()); ++axisYLineIndex){
		const i2d::CLine2d& axisYLine = vanLines[1][axisYLineIndex];

		for (int axisXLineIndex = 0; axisXLineIndex < int(vanLines[0].size()); ++axisXLineIndex){
			const i2d::CLine2d& axisXLine = vanLines[0][axisXLineIndex];

			i2d::CVector2d crossPoint;
			axisYLine.GetExtendedIntersection(axisXLine, crossPoint);
			i2d::CVector2d gridPos(crossPoint);
			istd::TIndex<2> ind;
			ind.SetAt(0, axisXLineIndex);
			ind.SetAt(1, axisYLineIndex);
			crossPositions->SetAt(ind, crossPoint);
		}
	}

	if (m_resultConsumerCompPtr.IsValid()){
		ilog::CExtMessage* messagePtr = new ilog::CExtMessage(
			istd::IInformationProvider::IC_INFO,
			0, QObject::tr("Found %1 point(s)").arg(crossPositions->GetElements().size()), "Chessboard Point Grid Extractor"
		);

		for (iipr::CPointGridFeature::Iterator iter = crossPositions->Begin(); iter != crossPositions->End(); ++iter){
			const i2d::CVector2d& p = *iter;

			i2d::CPosition2d* messagePointPtr = new i2d::CPosition2d(p);
			messagePtr->InsertAttachedObject(messagePointPtr);
		}

		m_resultConsumerCompPtr->AddMessage(ilog::IMessageConsumer::MessagePtr(messagePtr));
	}

	results.AddFeature(crossPositions);

	return TS_OK;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CCheckerboardPointGridExtractorComp::DoProcessing(
	const iprm::IParamsSet* paramsPtr,
	const istd::IPolymorphic* inputPtr,
	istd::IChangeable* outputPtr,
	ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* /*processingReportPtr*/)
{
	const iimg::IBitmap* inputBitmapPtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	if (inputBitmapPtr == NULL){
		SendCriticalMessage(0, "Input image is not defined");
		return TS_INVALID;
	}

	if (outputPtr == NULL){
		return TS_OK;
	}

	iipr::IFeaturesConsumer* outputConsumerPtr = dynamic_cast<iipr::IFeaturesConsumer*>(outputPtr);
	if (outputConsumerPtr == NULL){
		return TS_INVALID;
	}

	return DoExtractFeatures(paramsPtr, *inputBitmapPtr, *outputConsumerPtr, progressManagerPtr);
}


} // namespace iipr


