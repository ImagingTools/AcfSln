// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef icmpstr_TObjectShapeBase_included
#define icmpstr_TObjectShapeBase_included


// Qt includes
#include <QtCore/QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QGraphicsSceneMouseEvent>
#else
#include <QtGui/QGraphicsSceneMouseEvent>
#endif

// ACF includes
#include <istd/CChangeGroup.h>
#include <istd/TDelPtr.h>
#include <imod/TSingleModelObserverBase.h>
#include <i2d/IObject2d.h>

// ACF-Solutions includes
#include <icmpstr/TShapeBase.h>


namespace icmpstr
{


template <class GraphicsItemClass, class ObjectClass>
class TObjectShapeBase: 
			public TShapeBase<GraphicsItemClass>,
			public imod::TSingleModelObserverBase<ObjectClass>
{
public:
	typedef TShapeBase<GraphicsItemClass> BaseClass;
	typedef imod::TSingleModelObserverBase<ObjectClass> BaseClass2;

	TObjectShapeBase(bool isEditable = false, const ISceneProvider* providerPtr = NULL);

protected:
	// reimplemented (TShapeBase)
	virtual void OnSelectionChanged(bool isSelected) override;
	virtual void OnPositionChanged(const QPointF& position) override;

	// reimplemented (QGraphicsItem) 
	virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* eventPtr) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* eventPtr) override;
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* eventPtr) override;

	// reimplemented (imod::IObserver)
	virtual void AfterUpdate(imod::IModel* modelPtr, const istd::IChangeable::ChangeSet& changeSet) override;

	// abstract methods
	virtual void UpdateGraphicsItem(const ObjectClass& object) = 0;

private:
	bool m_isShapeUpdateBlocked;

	istd::TDelPtr<istd::CChangeGroup> m_dragNotifierPtr;

	static const istd::IChangeable::ChangeSet s_moveObjectChangeSet;
};


// public methods

template <class GraphicsItemClass, class ObjectClass>
TObjectShapeBase<GraphicsItemClass, ObjectClass>::TObjectShapeBase(bool isEditable, const ISceneProvider* providerPtr)
:	BaseClass(isEditable, providerPtr),
	m_isShapeUpdateBlocked(false)
{
	BaseClass::SetPen(BaseClass::InactiveColor, QPen(Qt::darkGreen, 0));
	BaseClass::SetPen(BaseClass::EditableColor, QPen(Qt::green, 0));
	BaseClass::SetPen(BaseClass::SelectedColor, QPen(Qt::yellow, 0));

	if (isEditable){
		BaseClass::setFlags(BaseClass::ItemIsMovable | BaseClass::ItemIsSelectable | BaseClass::ItemSendsGeometryChanges);
		BaseClass::setCursor(QCursor(Qt::ArrowCursor)); 

		BaseClass::SwitchColorSheme(BaseClass::EditableColor);
	}
	else{
		BaseClass::SwitchColorSheme(BaseClass::InactiveColor);
	}
}


// protected methods

// reimplemented (TShapeBase<GraphicsItemClass, ObjectClass>) 

template <class GraphicsItemClass, class ObjectClass>
void TObjectShapeBase<GraphicsItemClass, ObjectClass>::OnSelectionChanged(bool isSelected)
{
	if (isSelected){
		BaseClass::SwitchColorSheme(BaseClass::SelectedColor);
	}
	else{
		if (BaseClass::IsEditable()){
			BaseClass::SwitchColorSheme(BaseClass::EditableColor);
		}
		else{
			BaseClass::SwitchColorSheme(BaseClass::InactiveColor);		
		}
	}
}


template <class GraphicsItemClass, class ObjectClass>
void TObjectShapeBase<GraphicsItemClass, ObjectClass>::OnPositionChanged(const QPointF& position)
{
	i2d::IObject2d* objectPtr = BaseClass2::GetObservedObject();
	if (objectPtr != NULL){
		objectPtr->MoveCenterTo(i2d::CVector2d(position));
	}

	BaseClass::OnPositionChanged(position);
}


template <class GraphicsItemClass, class ObjectClass>
QVariant TObjectShapeBase<GraphicsItemClass, ObjectClass>::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
	if (!m_isShapeUpdateBlocked){
		return BaseClass::itemChange(change, value);
	}

	return value;
}


template <class GraphicsItemClass, class ObjectClass>
void TObjectShapeBase<GraphicsItemClass, ObjectClass>::mousePressEvent(QGraphicsSceneMouseEvent* eventPtr)
{
	BaseClass::mousePressEvent(eventPtr);

	if (BaseClass::IsEditable() && (eventPtr->button() == Qt::LeftButton)){
		m_dragNotifierPtr.SetPtr(new istd::CChangeGroup(BaseClass2::GetObservedObject(), &s_moveObjectChangeSet));
	}
}


template <class GraphicsItemClass, class ObjectClass>
void TObjectShapeBase<GraphicsItemClass, ObjectClass>::mouseReleaseEvent(QGraphicsSceneMouseEvent* eventPtr)
{
	BaseClass::mouseReleaseEvent(eventPtr);

	m_dragNotifierPtr.Reset();
}


template <class GraphicsItemClass, class ObjectClass>
void TObjectShapeBase<GraphicsItemClass, ObjectClass>::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* eventPtr)
{
	BaseClass::mouseDoubleClickEvent(eventPtr);

	m_dragNotifierPtr.Reset();
}


// reimplemented (imod::IObserver)

template <class GraphicsItemClass, class ObjectClass>
void TObjectShapeBase<GraphicsItemClass, ObjectClass>::AfterUpdate(imod::IModel* /*modelPtr*/, const istd::IChangeable::ChangeSet& changeSet)
{
	Q_ASSERT(!m_isShapeUpdateBlocked);

	ObjectClass* objectPtr = BaseClass2::GetObservedObject();
	if ((objectPtr != NULL) && !changeSet.ContainsExplicit(istd::IChangeable::CF_DESTROYING)){
		m_isShapeUpdateBlocked = true;

		UpdateGraphicsItem(*objectPtr);

		m_isShapeUpdateBlocked = false;

		BaseClass::update();
	}
}


// static attributes

template <class GraphicsItemClass, class ObjectClass>
const istd::IChangeable::ChangeSet TObjectShapeBase<GraphicsItemClass, ObjectClass>::s_moveObjectChangeSet(i2d::IObject2d::CF_OBJECT_POSITION, QObject::tr("Move component"));


} // namespace icmpstr


#endif // !icmpstr_TObjectShapeBase_included


