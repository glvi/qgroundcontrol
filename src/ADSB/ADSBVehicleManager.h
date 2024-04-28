/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "QGCToolbox.h"
#include "QmlObjectListModel.h"
#include "ADSBVehicle.h"

#include <QTcpSocket>
#include <QThread>
#include <QTimer>

namespace ADSB {
    Q_NAMESPACE
}

class ADSBVehicleManager : public QGCTool {
    Q_OBJECT

public:
    ADSBVehicleManager(QGCApplication* app, QGCToolbox* toolbox);
    virtual ~ADSBVehicleManager();

    Q_PROPERTY(QmlObjectListModel* adsbVehicles READ adsbVehicles CONSTANT)

    QmlObjectListModel* adsbVehicles() { return &_adsbVehicles; }

    // QGCTool overrides
    void setToolbox(QGCToolbox* toolbox) final;

public slots:
    void adsbVehicleUpdate  (ADSBVehicle::ADSBVehicleInfo_t const& vehicleInfo);

private slots:
    void _cleanupStaleVehicles();

private:
    QmlObjectListModel           _adsbVehicles;
    QMap<uint32_t, ADSBVehicle*> _adsbICAOMap;
    QTimer                       _adsbVehicleCleanupTimer;
};

// Local Variables:
// c-basic-offset: 4
// End:
