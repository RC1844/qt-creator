// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "outputtabwidget.h"
#include "outputpane.h"

#include <utils/qtcassert.h>

#include <QLayout>
#include <QPainter>
#include <QStackedWidget>
#include <QToolBar>

using namespace ScxmlEditor::OutputPane;

PaneTitleButton::PaneTitleButton(OutputPane *pane, QWidget *parent)
    : QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    animator.setPropertyName("colorOpacity");
    animator.setTargetObject(this);

    setObjectName("PanePushButton");
    setCheckable(true);

    setText(pane->title());
    setIcon(pane->icon());

    connect(this, &PaneTitleButton::toggled, this, [this](bool toggled) {
        if (toggled)
            stopAlert();
    });

    connect(&animator, &QAbstractAnimation::finished, this, [this] {
        m_animCounter++;
        if (m_animCounter < 8) {
            if (m_animCounter % 2 == 1)
                fadeOut();
            else
                fadeIn();
        }
    });

    connect(pane, &OutputPane::titleChanged, this, [=] {
        setText(pane->title());
    });

    connect(pane, &OutputPane::iconChanged, this, [=] {
        setIcon(pane->icon());
    });
}

void PaneTitleButton::startAlert(const QColor &color)
{
    m_color = color;
    m_animCounter = 0;
    fadeIn();
}

void PaneTitleButton::stopAlert()
{
    animator.stop();
}

void PaneTitleButton::fadeIn()
{
    animator.stop();
    animator.setDuration(300);
    animator.setStartValue(0);
    animator.setEndValue(80);
    animator.start();
}

void PaneTitleButton::fadeOut()
{
    animator.stop();
    animator.setDuration(300);
    animator.setStartValue(80);
    animator.setEndValue(0);
    animator.start();
}

void PaneTitleButton::setColorOpacity(int value)
{
    m_colorOpacity = value;
    update();
}

void PaneTitleButton::paintEvent(QPaintEvent *e)
{
    QToolButton::paintEvent(e);

    QPainter p(this);
    p.save();
    if (animator.state() != QAbstractAnimation::Stopped) {
        QRect r = rect();
        m_color.setAlpha(m_colorOpacity);
        p.setBrush(QBrush(m_color));
        p.setPen(Qt::NoPen);
        p.drawRect(r);
    }
    p.restore();
}

OutputTabWidget::OutputTabWidget(QWidget *parent)
    : QFrame(parent)
{
    createUi();
    close();
}

OutputTabWidget::~OutputTabWidget() = default;

int OutputTabWidget::addPane(OutputPane *pane)
{
    if (pane) {
        auto button = new PaneTitleButton(pane, this);
        connect(button, &PaneTitleButton::clicked, this, [this, button](bool checked) {
            buttonClicked(button, checked);
        });
        connect(pane, &OutputPane::dataChanged, this, [this, pane] { showAlert(pane); });

        m_toolBar->addWidget(button);
        m_stackedWidget->addWidget(pane);

        m_buttons << button;
        m_pages << pane;

        return m_pages.count() - 1;
    }

    return -1;
}

void OutputTabWidget::showPane(OutputPane *pane)
{
    QTC_ASSERT(pane, return);

    m_stackedWidget->setCurrentWidget(pane);
    m_buttons[m_pages.indexOf(pane)]->setChecked(true);
    pane->setPaneFocus();
    if (!m_stackedWidget->isVisible()) {
        m_stackedWidget->setVisible(true);
        emit visibilityChanged(true);
    }
}

void OutputTabWidget::showPane(int index)
{
    showPane(static_cast<OutputPane*>(m_stackedWidget->widget(index)));
}

void OutputTabWidget::createUi()
{
    m_toolBar = new QToolBar;
    m_stackedWidget = new QStackedWidget;

    setLayout(new QVBoxLayout);
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(m_toolBar);
    layout()->addWidget(m_stackedWidget);
}

void OutputTabWidget::close()
{
    m_stackedWidget->setVisible(false);
    emit visibilityChanged(false);
}

void OutputTabWidget::showAlert(OutputPane *pane)
{
    const int index = m_pages.indexOf(pane);
    if (index >= 0 && !m_buttons[index]->isChecked())
        m_buttons[index]->startAlert(m_pages[index]->alertColor());
}

void OutputTabWidget::buttonClicked(PaneTitleButton *button, bool checked)
{
    const int index = m_buttons.indexOf(button);
    if (index < 0)
        return;

    if (checked) {
        for (int i = 0; i < m_buttons.count(); ++i) {
            if (i != index)
                m_buttons[i]->setChecked(false);
        }
        showPane(index);
    } else {
        close();
    }
}
