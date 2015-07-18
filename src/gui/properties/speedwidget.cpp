/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2015 Anton Lashkov <lenton_91@mail.ru>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#include "speedwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QToolButton>
#include <QMenu>
#include <QSignalMapper>

#include <libtorrent/session_status.hpp>

#include "propertieswidget.h"
#include "core/bittorrent/session.h"
#include "core/bittorrent/sessionstatus.h"
#include "core/preferences.h"
#include "core/utils/misc.h"

SpeedWidget::SpeedWidget(PropertiesWidget *parent)
    : QWidget(parent)
    , m_properties(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_hlayout = new QHBoxLayout();
    m_hlayout->setContentsMargins(0, 0, 0, 0);

    m_periodLabel = new QLabel("<b>" + tr("Period:") + "</b>");

    m_periodCombobox = new QComboBox();
    m_periodCombobox->addItem(tr("1 Minute"));
    m_periodCombobox->addItem(tr("5 Minutes"));
    m_periodCombobox->addItem(tr("30 Minutes"));
    m_periodCombobox->addItem(tr("6 Hours"));

    connect(m_periodCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(onPeriodChange(int)));

    m_graphsButton = new QToolButton();
    m_graphsButton->setText(tr("Select Graphs"));
    m_graphsButton->setPopupMode(QToolButton::InstantPopup);
    m_graphsButton->setAutoExclusive(true);

    m_graphsMenu = new QMenu();
    m_graphsMenu->addAction(tr("Total Upload"));
    m_graphsMenu->addAction(tr("Total Download"));
    m_graphsMenu->addAction(tr("Payload Upload"));
    m_graphsMenu->addAction(tr("Payload Download"));
    m_graphsMenu->addAction(tr("Overhead Upload"));
    m_graphsMenu->addAction(tr("Overhead Download"));
    m_graphsMenu->addAction(tr("DHT Upload"));
    m_graphsMenu->addAction(tr("DHT Download"));
    m_graphsMenu->addAction(tr("Tracker Upload"));
    m_graphsMenu->addAction(tr("Tracker Download"));

    m_graphsMenuActions = m_graphsMenu->actions();
    m_graphsSignalMapper = new QSignalMapper();

    for (int id = SpeedPlotView::UP; id < SpeedPlotView::NB_GRAPHS; ++id) {
        QAction *action = m_graphsMenuActions.at(id);
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(changed()), m_graphsSignalMapper, SLOT(map()));
        m_graphsSignalMapper->setMapping(action, id);
    }
    connect(m_graphsSignalMapper, SIGNAL(mapped(int)), this, SLOT(onGraphChange(int)));

    m_graphsButton->setMenu(m_graphsMenu);

    m_hlayout->addWidget(m_periodLabel);
    m_hlayout->addWidget(m_periodCombobox);
    m_hlayout->addStretch();
    m_hlayout->addWidget(m_graphsButton);

    m_plot = new SpeedPlotView(this);

    m_layout->addLayout(m_hlayout);
    m_layout->addWidget(m_plot);

    loadSettings();

    m_isUpdating = true;
    m_updateFuture = QtConcurrent::run(this, &SpeedWidget::update);

    m_plot->show();
}

SpeedWidget::~SpeedWidget()
{
    qDebug("SpeedWidget::~SpeedWidget() ENTER");

    m_isUpdating = false;
    m_updateFuture.waitForFinished();

    saveSettings();

    qDebug("SpeedWidget::~SpeedWidget() EXIT");
}

void SpeedWidget::update()
{
    while (m_isUpdating) {

        BitTorrent::SessionStatus btStatus = BitTorrent::Session::instance()->status();

        m_plot->pushXPoint(QDateTime::currentDateTime().toTime_t());
        m_plot->pushYPoint(SpeedPlotView::UP, btStatus.uploadRate());
        m_plot->pushYPoint(SpeedPlotView::DOWN, btStatus.downloadRate());
        m_plot->pushYPoint(SpeedPlotView::PAYLOAD_UP, btStatus.payloadUploadRate());
        m_plot->pushYPoint(SpeedPlotView::PAYLOAD_DOWN, btStatus.payloadDownloadRate());
        m_plot->pushYPoint(SpeedPlotView::OVERHEAD_UP, btStatus.ipOverheadUploadRate());
        m_plot->pushYPoint(SpeedPlotView::OVERHEAD_DOWN, btStatus.ipOverheadDownloadRate());
        m_plot->pushYPoint(SpeedPlotView::DHT_UP, btStatus.dhtUploadRate());
        m_plot->pushYPoint(SpeedPlotView::DHT_DOWN, btStatus.dhtDownloadRate());
        m_plot->pushYPoint(SpeedPlotView::TRACKER_UP, btStatus.trackerUploadRate());
        m_plot->pushYPoint(SpeedPlotView::TRACKER_DOWN, btStatus.trackerDownloadRate());

        QMetaObject::invokeMethod(this, "graphUpdate");
        Utils::Misc::msleep(1000);
    }
}

void SpeedWidget::graphUpdate()
{
    m_plot->replot();
}

void SpeedWidget::onPeriodChange(int period)
{
    switch (period) {
    case SpeedPlotView::MIN1:
        m_plot->setViewableLastPoints(SpeedPlotView::MIN1_SEC);
        break;
    case SpeedPlotView::MIN5:
        m_plot->setViewableLastPoints(SpeedPlotView::MIN5_SEC);
        break;
    case SpeedPlotView::MIN30:
        m_plot->setViewableLastPoints(SpeedPlotView::MIN30_SEC);
        break;
    case SpeedPlotView::HOUR6:
        m_plot->setViewableLastPoints(SpeedPlotView::HOUR6_SEC);
        break;
    default:
        break;
    }

    graphUpdate();
}

void SpeedWidget::onGraphChange(int id)
{
    QAction *action = m_graphsMenuActions.at(id);
    m_plot->setGraphEnable(id, action->isChecked());

    graphUpdate();
}

void SpeedWidget::loadSettings()
{
    Preferences *preferences = Preferences::instance();

    int periodIndex = preferences->getSpeedWidgetPeriod();
    m_periodCombobox->setCurrentIndex(periodIndex);
    onPeriodChange(periodIndex);

    for (int id = SpeedPlotView::UP; id < SpeedPlotView::NB_GRAPHS; ++id) {
        QAction *action = m_graphsMenuActions.at(id);

        if (preferences->getSpeedWidgetGraphEnable(id)) {
            action->setChecked(true);
            m_plot->setGraphEnable(id, true);
        } else {
            action->setChecked(false);
            m_plot->setGraphEnable(id, false);
        }
    }
}

void SpeedWidget::saveSettings() const
{
    Preferences *preferences = Preferences::instance();

    preferences->setSpeedWidgetPeriod(m_periodCombobox->currentIndex());

    for (int id = SpeedPlotView::UP; id < SpeedPlotView::NB_GRAPHS; ++id) {
        QAction *action = m_graphsMenuActions.at(id);
        preferences->setSpeedWidgetGraphEnable(id, action->isChecked());
    }
}

