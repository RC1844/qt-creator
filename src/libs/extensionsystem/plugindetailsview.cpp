// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "plugindetailsview.h"
#include "ui_plugindetailsview.h"

#include "pluginmanager.h"
#include "pluginspec.h"

#include <utils/algorithm.h>

#include <QDir>
#include <QJsonObject>
#include <QJsonValue>
#include <QRegularExpression>

/*!
    \class ExtensionSystem::PluginDetailsView
    \inheaderfile extensionsystem/plugindetailsview.h
    \inmodule QtCreator

    \brief The PluginDetailsView class implements a widget that displays the
    contents of a PluginSpec.

    Can be used for integration in the application that
    uses the plugin manager.

    \sa ExtensionSystem::PluginView
*/

using namespace ExtensionSystem;

/*!
    Constructs a new view with given \a parent widget.
*/
PluginDetailsView::PluginDetailsView(QWidget *parent)
	: QWidget(parent),
          m_ui(new Internal::Ui::PluginDetailsView())
{
    m_ui->setupUi(this);
}

/*!
    \internal
*/
PluginDetailsView::~PluginDetailsView()
{
    delete m_ui;
}

/*!
    Reads the given \a spec and displays its values
    in this PluginDetailsView.
*/
void PluginDetailsView::update(PluginSpec *spec)
{
    m_ui->name->setText(spec->name());
    const QString revision = spec->revision();
    const QString versionString = spec->version() + (revision.isEmpty() ? QString()
                                                                        : " (" + revision + ")");
    m_ui->version->setText(versionString);
    m_ui->compatVersion->setText(spec->compatVersion());
    m_ui->vendor->setText(spec->vendor());
    const QString link = QString::fromLatin1("<a href=\"%1\">%1</a>").arg(spec->url());
    m_ui->url->setText(link);
    QString component = tr("None");
    if (!spec->category().isEmpty())
        component = spec->category();
    m_ui->component->setText(component);
    m_ui->location->setText(QDir::toNativeSeparators(spec->filePath()));
    m_ui->description->setText(spec->description());
    m_ui->copyright->setText(spec->copyright());
    m_ui->license->setText(spec->license());
    const QRegularExpression platforms = spec->platformSpecification();
    const QString pluginPlatformString = platforms.pattern().isEmpty() ? tr("All") : platforms.pattern();
    const QString platformString = tr("%1 (current: \"%2\")").arg(pluginPlatformString,
                                                                  PluginManager::platformName());
    m_ui->platforms->setText(platformString);
    const QStringList depStrings = Utils::transform<QList>(spec->dependencies(), &PluginDependency::toString);
    m_ui->dependencies->addItems(depStrings);
}
