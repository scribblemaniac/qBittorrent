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

#ifndef SPEEDPLOTVIEW_H
#define SPEEDPLOTVIEW_H

#include <QGraphicsView>
#include <QMap>
#include <QQueue>
class QPen;

class SpeedPlotView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit SpeedPlotView(QWidget *parent = 0);

    void setGraphEnable(int id, bool enable);

    void pushXPoint(double x);
    void pushYPoint(int id, double y);

    void setViewableLastPoints(int period);

    void replot();

    enum GraphID
    {
        UP = 0,
        DOWN,
        PAYLOAD_UP,
        PAYLOAD_DOWN,
        OVERHEAD_UP,
        OVERHEAD_DOWN,
        DHT_UP,
        DHT_DOWN,
        TRACKER_UP,
        TRACKER_DOWN,
        NB_GRAPHS
    };

    enum TimePeriod
    {
        MIN1 = 0,
        MIN5,
        MIN30,
        HOUR6
    };

    enum PeriodInSeconds
    {
        MIN1_SEC = 60,
        MIN5_SEC = 300,
        MIN30_SEC = 1800,
        HOUR6_SEC = 21600
    };

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    struct GraphProperties
    {
        GraphProperties();
        GraphProperties(const QString &_name, const QPen &_pen, bool _enable = false);

        QString name;
        QPen pen;
        bool enable;
    };

    QQueue<double> m_xData;
    QMap<int, QQueue<double> > m_yData;
    QMap<int, GraphProperties> m_properties;

    int m_viewablePointsCount;
    int m_maxCapacity;

    double maxYValue();
};

#endif // SPEEDPLOTVIEW_H
