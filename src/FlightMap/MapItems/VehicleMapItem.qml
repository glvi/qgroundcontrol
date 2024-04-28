/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Effects
import QtLocation
import QtPositioning

import QGroundControl
import QGroundControl.ScreenTools
import QGroundControl.Vehicle
import QGroundControl.Controls

/// Marker for displaying a vehicle location on the map
MapQuickItem {

    id: _root

    property var    vehicle                                                         /// Vehicle object, undefined for ADSB vehicle
    property var    map
    property double altitude:       Number.NaN                                      ///< NAN to not show
    property string callsign:       ""                                              ///< Vehicle callsign
    property int    category:       0                                               ///< Aircraft category
    property double heading:        vehicle ? vehicle.heading.value : Number.NaN    ///< Vehicle heading, NAN for none
    property real   size:           ScreenTools.defaultFontPixelHeight * 3          /// Default size for icon, most usage overrides this
    property bool   alert:          false                                           /// Collision alert

    anchorPoint.x:  vehicleItem.width  / 2
    anchorPoint.y:  vehicleItem.height / 2
    visible:        coordinate.isValid

    property var    _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property var    _map:           map
    property bool   _multiVehicle:  QGroundControl.multiVehicleManager.vehicles.count > 1

    sourceItem: Item {
        id:         vehicleItem
        width:      vehicleIcon.width
        height:     vehicleIcon.height
        opacity:    !vehicle || vehicle === _activeVehicle ? 1.0 : 0.5

        MultiEffect {
            source: vehicleIcon
            shadowEnabled: vehicleIcon.visible && _adsbVehicle
            shadowColor: Qt.rgba(0.94,0.91,0,1.0)
            shadowVerticalOffset: 4
            shadowHorizontalOffset: 4
            shadowBlur: 1.0
            shadowOpacity: 0.5
            shadowScale: 1.3
            blurMax: 32
          blurMultiplier: .1
        }

        Image {
            id:                 vehicleIcon
            source:             {
                if (vehicle) return vehicle.vehicleImageOpaque
                switch (category) {
                    case 1: return "/qmlimages/LightAircraft.svg"
                    case 3: return "/qmlimages/SmallAircraft.svg"
                    case 5: return "/qmlimages/LargeAircraft.svg"
                    case 6: return "/qmlimages/LargeAircraft.svg"
                    case 7: return "/qmlimages/HeavyAircraft.svg"
                    default: return "/qmlimages/AwarenessAircraft.svg"
              }
            }
            mipmap:             true
            width:              _root.size
            sourceSize.width:   _root.size
            fillMode:           Image.PreserveAspectFit
            transform: Rotation {
                origin.x:       vehicleIcon.width  / 2
                origin.y:       vehicleIcon.height / 2
                angle:          isNaN(heading) ? 0 : heading
            }
        }

        QGCMapLabel {
            id:                         vehicleLabel
            anchors.top:                parent.bottom
            anchors.horizontalCenter:   parent.horizontalCenter
            map:                        _map
            text:                       vehicleLabelText
            font.pointSize:             vehicle ? ScreenTools.smallFontPointSize : ScreenTools.defaultFontPointSize
            visible:                    vehicle ? _multiVehicle : !isNaN(altitude)
            property string vehicleLabelText: {
                if (!visible) {
                    return ""
                }
                if (!vehicle) {
                    const unitValue = arg => QGroundControl.unitsConversion.metersToAppSettingsVerticalDistanceUnits(arg).toFixed(0)
                    const unitString = QGroundControl.unitsConversion.appSettingsVerticalDistanceUnitsString
                    return unitValue(altitude) + " " + unitString + "\n" + callsign
                }
                if (!_multiVehicle) {
                    return ""
                }
                return qsTr("Vehicle %1").arg(vehicle.id)
            }
        }
    }
}
