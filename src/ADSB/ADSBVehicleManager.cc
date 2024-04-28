/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "ADSBVehicleManager.h"
#include "ADSBVehicleManagerSettings.h"
#include "SettingsManager.h"

#include "MavlinkDecoder.h"
#include "SBS1Decoder.h"
#include "TCPLinkController.h"

#include "QGCApplication.h"
#include "QGCLoggingCategory.h"

#include <QGeoCoordinate>
#include <QDebug>

ADSBVehicleManager::ADSBVehicleManager(QGCApplication* app, QGCToolbox* toolbox)
    : QGCTool(app, toolbox)
{
}

ADSBVehicleManager::~ADSBVehicleManager()
{
}

void ADSBVehicleManager::setToolbox(QGCToolbox* toolbox)
{
    QGCTool::setToolbox(toolbox);

    struct ADSBSettings {
        ADSBVehicleManagerSettings* settings;
        auto enabled()       { return settings->adsbServerConnectEnabled()->rawValue().toBool(); }
        auto hostname()      { return settings->adsbServerHostAddress()->rawValue().toString(); }
        auto port()          { return settings->adsbServerPort()->rawValue().toInt(); }
        auto messageFormat() { return settings->adsbMessageFormat()->rawValue().toInt(); }
    } adsbSettings {
        qgcApp()->toolbox()->settingsManager()->adsbVehicleManagerSettings(),
    };

    if (!adsbSettings.enabled()) {
        return;
    }

    connect(&_adsbVehicleCleanupTimer, &QTimer::timeout, this, &ADSBVehicleManager::_cleanupStaleVehicles);
    _adsbVehicleCleanupTimer.setSingleShot(false);
    _adsbVehicleCleanupTimer.start(1000);

    if (adsbSettings.enabled()) {
        switch (adsbSettings.messageFormat()) {
        case 1: {
            auto decoder = new ADSB::MavlinkDecoder(this);
            connect(decoder, &ADSB::MavlinkDecoder::decoded,
                    this, &ADSBVehicleManager::adsbVehicleUpdate,
                    Qt::QueuedConnection);
            auto linkctrl =
                new ADSB::TCPLinkController(adsbSettings.hostname(), adsbSettings.port(),
                                            ADSB::TCPLinkController::ByteOriented(), this);
            connect(linkctrl, &ADSB::TCPLinkController::readBytes,
                    decoder, &ADSB::MavlinkDecoder::decode);
            linkctrl->establishLink();
        }
            break;

        case 0: // [fallthrough];
        default: {
            auto linkctrl =
                new ADSB::TCPLinkController(adsbSettings.hostname(), adsbSettings.port(),
                                            ADSB::TCPLinkController::LineOriented(), this);
            auto decoder = new ADSB::SBS1Decoder(this);
            connect(linkctrl, &ADSB::TCPLinkController::readLine,
                    decoder, &ADSB::SBS1Decoder::decode);
            connect(decoder, &ADSB::SBS1Decoder::decoded,
                    this, &ADSBVehicleManager::adsbVehicleUpdate,
                    Qt::QueuedConnection);
            linkctrl->establishLink();
        }
            break;
        }
    }
}

void ADSBVehicleManager::_cleanupStaleVehicles()
{
    // Remove all expired ADSB vehicles
    for (int i=_adsbVehicles.count()-1; i>=0; i--) {
        ADSBVehicle* adsbVehicle = _adsbVehicles.value<ADSBVehicle*>(i);
        if (adsbVehicle->expired()) {
            qCDebug(ADSBVehicleManagerLog) << "Expired " << QStringLiteral("%1").arg(adsbVehicle->icaoAddress(), 0, 16);
            _adsbVehicles.removeAt(i);
            _adsbICAOMap.remove(adsbVehicle->icaoAddress());
            adsbVehicle->deleteLater();
        }
    }
}

void ADSBVehicleManager::adsbVehicleUpdate(const ADSBVehicle::ADSBVehicleInfo_t& vehicleInfo)
{
    uint32_t icaoAddress = vehicleInfo.icaoAddress;

    if (_adsbICAOMap.contains(icaoAddress)) {
        _adsbICAOMap[icaoAddress]->update(vehicleInfo);
    } else {
        if (vehicleInfo.availableFlags & ADSBVehicle::LocationAvailable) {
            ADSBVehicle* adsbVehicle = new ADSBVehicle(vehicleInfo, this);
            _adsbICAOMap[icaoAddress] = adsbVehicle;
            _adsbVehicles.append(adsbVehicle);
            qCDebug(ADSBVehicleManagerLog) << "Added " << QStringLiteral("%1").arg(adsbVehicle->icaoAddress(), 0, 16);
        }
    }
}

// Local Variables:
// c-basic-offset: 4
// End:
