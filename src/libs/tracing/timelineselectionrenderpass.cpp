// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "timelineselectionrenderpass.h"
#include "timelineitemsrenderpass.h"
#include <QtMath>
#include <QSGSimpleRectNode>

namespace Timeline {

QSGGeometryNode *createSelectionNode(QSGMaterial *material)
{
    QSGGeometryNode *selectionNode = new QSGGeometryNode;
    selectionNode->setMaterial(material);
    selectionNode->setFlag(QSGNode::OwnsMaterial, false);
    QSGGeometry *geometry = new QSGGeometry(OpaqueColoredPoint2DWithSize::attributes(), 4);
    Q_ASSERT(geometry->vertexData());
    geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
    OpaqueColoredPoint2DWithSize *v = OpaqueColoredPoint2DWithSize::fromVertexData(geometry);
    for (int i = 0; i < 4; ++i)
        v[i].set(0, 0, 0, 0, 0, 0, 0, 0, 0);
    selectionNode->setGeometry(geometry);
    selectionNode->setFlag(QSGNode::OwnsGeometry, true);
    selectionNode->setFlag(QSGNode::OwnedByParent, false);
    return selectionNode;
}

class TimelineSelectionRenderPassState : public TimelineRenderPass::State {
public:
    TimelineSelectionRenderPassState();
    ~TimelineSelectionRenderPassState();

    QSGNode *expandedOverlay() const override { return m_expandedOverlay; }
    QSGNode *collapsedOverlay() const override { return m_collapsedOverlay; }
    TimelineItemsMaterial *material() { return &m_material; }

private:
    QSGNode *m_expandedOverlay;
    QSGNode *m_collapsedOverlay;
    TimelineItemsMaterial m_material;
};

TimelineRenderPass::State *TimelineSelectionRenderPass::update(
        const TimelineAbstractRenderer *renderer, const TimelineRenderState *parentState,
        State *oldState, int firstIndex, int lastIndex, bool stateChanged, float spacing) const
{
    Q_UNUSED(stateChanged)

    const TimelineModel *model = renderer->model();
    if (!model || model->isEmpty())
        return oldState;

    TimelineSelectionRenderPassState *state;

    if (oldState == nullptr)
        state = new TimelineSelectionRenderPassState;
    else
        state = static_cast<TimelineSelectionRenderPassState *>(oldState);

    int selectedItem = renderer->selectedItem();
    QSGGeometryNode *node = static_cast<QSGGeometryNode *>(model->expanded() ?
                                                               state->expandedOverlay() :
                                                               state->collapsedOverlay());

    if (selectedItem != -1 && selectedItem >= firstIndex && selectedItem < lastIndex) {
        qreal top = 0;
        qreal height = 0;
        if (model->expanded()) {
            int row = model->expandedRow(selectedItem);
            int rowHeight = model->expandedRowHeight(row);
            height = rowHeight * model->relativeHeight(selectedItem);
            top = (model->expandedRowOffset(row) + rowHeight) - height;
        } else {
            int row = model->collapsedRow(selectedItem);
            height = TimelineModel::defaultRowHeight() * model->relativeHeight(selectedItem);
            top = TimelineModel::defaultRowHeight() * (row + 1) - height;
        }

        qint64 startTime = qBound(parentState->start(), model->startTime(selectedItem),
                                  parentState->end());
        qint64 endTime = qBound(parentState->start(), model->endTime(selectedItem),
                                parentState->end());
        qint64 left = startTime - parentState->start();
        qint64 width = endTime - startTime;

        // Construct from upper left and lower right for better precision. When constructing from
        // left and width the error on the left border is inherited by the right border. Like this
        // they're independent.

        QRectF position(left * parentState->scale(), top, width * parentState->scale(),  height);

        QColor itemColor = model->color(selectedItem);
        uchar red = itemColor.red();
        uchar green = itemColor.green();
        uchar blue = itemColor.blue();
        int selectionId = model->selectionId(selectedItem);

        OpaqueColoredPoint2DWithSize *v = OpaqueColoredPoint2DWithSize::fromVertexData(
                    node->geometry());
        v[0].set(position.left(), position.bottom(), -position.width(), -position.height(),
                 selectionId, red, green, blue, 255);
        v[1].set(position.right(), position.bottom(), position.width(), -position.height(),
                 selectionId, red, green, blue, 255);
        v[2].set(position.left(), position.top(), -position.width(), position.height(),
                 selectionId, red, green, blue, 255);
        v[3].set(position.right(), position.top(), position.width(), position.height(),
                 selectionId, red, green, blue, 255);
        state->material()->setSelectionColor(renderer->selectionLocked() ? QColor(96,0,255) :
                                                                           Qt::blue);
        state->material()->setSelectedItem(selectionId);
        state->material()->setScale(QVector2D(spacing / parentState->scale(), 1));
        node->markDirty(QSGNode::DirtyMaterial | QSGNode::DirtyGeometry);
    } else {
        OpaqueColoredPoint2DWithSize *v = OpaqueColoredPoint2DWithSize::fromVertexData(
                    node->geometry());
        for (int i = 0; i < 4; ++i)
            v[i].set(0, 0, 0, 0, 0, 0, 0, 0, 0);
        node->markDirty(QSGNode::DirtyGeometry);
    }
    return state;
}

const TimelineSelectionRenderPass *TimelineSelectionRenderPass::instance()
{
    static const TimelineSelectionRenderPass pass;
    return &pass;
}

TimelineSelectionRenderPass::TimelineSelectionRenderPass()
{
}

TimelineSelectionRenderPassState::TimelineSelectionRenderPassState() :
    m_expandedOverlay(nullptr), m_collapsedOverlay(nullptr)
{
    m_expandedOverlay = createSelectionNode(&m_material);
    m_collapsedOverlay = createSelectionNode(&m_material);
}

TimelineSelectionRenderPassState::~TimelineSelectionRenderPassState()
{
    delete m_collapsedOverlay;
    delete m_expandedOverlay;
}

} // namespace Timeline
