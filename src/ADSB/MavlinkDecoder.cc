/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "MavlinkDecoder.h"

#include "ADSBVehicle.h"
#include "QGCLoggingCategory.h"

#include <mavlink.h>

namespace ADSB {

    namespace { // utilities (declarations)

        using Result = ADSBVehicle::ADSBVehicleInfo_t;

        using CompletionHandler = std::function<void(Result&&)>;

        void _handleMavlinkMessage(mavlink_message_t&&, CompletionHandler);

        void _handleMavlinkAdsbVehicle(mavlink_adsb_vehicle_t&&, CompletionHandler);

        constexpr auto
        emitterCategory(uint8_t const emitter_type) noexcept
            -> ADSBVehicle::EmitterCategory;

        constexpr auto
        to_string(ADSBVehicle::EmitterCategory ecat) noexcept
            -> char const *;
    }

    struct MavlinkDecoder::State
    {
        mavlink_message_t message = mavlink_message_t();
        mavlink_status_t  status  = mavlink_status_t();
    };

    MavlinkDecoder::MavlinkDecoder(QObject* parent)
        : QObject {parent}
        , state {new State}
    {
    }

    MavlinkDecoder::~MavlinkDecoder()
    {
    }

    void MavlinkDecoder::decode(QByteArray bytes)
    {
        mavlink_message_t message;
        for (uint8_t byte: bytes) {
            if (1 == mavlink_frame_char_buffer(&state->message, &state->status, byte, &message, nullptr)) {
                _handleMavlinkMessage(std::move(message), [this] (Result&& result) {
                    emit decoded(std::move(result));
                });
                message = mavlink_message_t();
            }
        }
    }

    namespace { // utilities (definitions)

        void _handleMavlinkMessage(mavlink_message_t&& msg, CompletionHandler ch)
        {
            switch (msg.msgid) {
            case MAVLINK_MSG_ID_ADSB_VEHICLE: {
                mavlink_adsb_vehicle_t payload;
                mavlink_msg_adsb_vehicle_decode(&msg, &payload);
                _handleMavlinkAdsbVehicle(std::move(payload), std::move(ch));
            }
                break;

            default: // ignore other message types
                break;
            }
        }

        void _handleMavlinkAdsbVehicle(mavlink_adsb_vehicle_t&& adsbVehicle, CompletionHandler ch)
        {
            if (adsbVehicle.ICAO_address == 0) {
                return;
            }

            Result result;

            result.icaoAddress = adsbVehicle.ICAO_address;
            QString msgAddress = QStringLiteral(" #%1").arg(result.icaoAddress, 6, 16, QChar('0'));
            QString msgCallSign;
            QString msgLocation;
            QString msgAltitude;
            QString msgHeading;
            QString msgSquawk;
            QString msgCategory;

            if (adsbVehicle.flags & ADSB_FLAGS_VALID_CALLSIGN) {
                QString cs = QString(adsbVehicle.callsign).trimmed();
                if (!cs.isEmpty()) {
                    result.callsign = std::move(cs);
                    result.availableFlags |= ADSBVehicle::CallsignAvailable;
                    msgCallSign = QStringLiteral(" [%1]").arg(result.callsign, -8);
                }
            }

            if (adsbVehicle.flags & ADSB_FLAGS_VALID_COORDS) {
                auto const lat_deg = double(adsbVehicle.lat) * 1E-7;
                auto const lon_deg = double(adsbVehicle.lon) * 1E-7;
                result.location = QGeoCoordinate(lat_deg, lon_deg);
                result.availableFlags |= ADSBVehicle::LocationAvailable;
                msgLocation = QStringLiteral(" (LAT:%1 / LON:%2)/deg[WGS84]").arg(lat_deg).arg(lon_deg);
            }

            if (adsbVehicle.flags & ADSB_FLAGS_VALID_ALTITUDE) {
                if (adsbVehicle.altitude_type != ADSB_ALTITUDE_TYPE_GEOMETRIC) {
                    // TODO: Obtain QNH
                    // qCWarning(ADSBVehicleManagerLog) << "QNH not available; assuming 1013.25 hPa";
                }
                auto const alt_m = double(adsbVehicle.altitude) * 1E-3;
                result.altitude = alt_m;
                result.availableFlags |= ADSBVehicle::AltitudeAvailable;
                msgAltitude = QStringLiteral(" HAE:%1/m").arg(alt_m);
            }

            if (adsbVehicle.flags & ADSB_FLAGS_VALID_HEADING) {
                result.heading = double(adsbVehicle.heading) * 1E-2;
                result.availableFlags |= ADSBVehicle::HeadingAvailable;
                msgHeading = QStringLiteral(" HDG:%1/deg").arg(result.heading);
            }

            if (adsbVehicle.flags & ADSB_FLAGS_VALID_SQUAWK) {
                msgSquawk = QStringLiteral(" SQ:%1").arg(adsbVehicle.squawk, 4, 8, QChar('0'));
                static constexpr uint16_t const ssrModeAMask         = 07777;
                static constexpr uint16_t const ssrModeAEmergency    = 07700;
                static constexpr uint16_t const ssrModeARadioFailure = 07600;
                static constexpr uint16_t const ssrModeAHijack       = 07500;
                static constexpr uint16_t const ssrModeALostLink     = 07400;
                switch (adsbVehicle.squawk & ssrModeAMask) {
                case ssrModeAEmergency:    // [fallthrough];
                case ssrModeARadioFailure: // [fallthrough];
                case ssrModeAHijack:       // [fallthrough];
                case ssrModeALostLink:     // [fallthrough];
                    result.alert = true;
                    result.availableFlags |= ADSBVehicle::AlertAvailable;
                    break;
                default:
                    result.alert = false;
                    result.availableFlags |= ADSBVehicle::AlertAvailable;
                }
            }

            auto ecat = emitterCategory(adsbVehicle.emitter_type);
            if (ecat != ADSBVehicle::EmitterCategory::NoInformation) {
                result.category = ecat;
                result.availableFlags |= ADSBVehicle::CategoryAvailable;
                msgCategory = QStringLiteral(" CAT:%1").arg(to_string(ecat));
            }

            qCInfo(ADSBVehicleManagerLog)
                << (QStringLiteral("ADSB MAVLINK:")
                    % msgAddress % msgSquawk % msgCallSign % msgCategory
                    % msgLocation % msgAltitude % msgHeading);

            if (ch) {
                ch(std::move(result));
            }
        }


        constexpr auto
        emitterCategory(uint8_t const emitter_type) noexcept
            -> ADSBVehicle::EmitterCategory
        {
            using ECat = ADSBVehicle::EmitterCategory;
            switch (emitter_type) {
            case ADSB_EMITTER_TYPE_LIGHT              : return ECat::MTOW_lt_15500_lbs;
            case ADSB_EMITTER_TYPE_SMALL              : return ECat::MTOW_lt_75000_lbs;
            case ADSB_EMITTER_TYPE_LARGE              : return ECat::MTOW_lt_300000_lbs;
            case ADSB_EMITTER_TYPE_HIGH_VORTEX_LARGE  : return ECat::HighVortexLarge;
            case ADSB_EMITTER_TYPE_HEAVY              : return ECat::MTOW_ge_300000_lbs;
            case ADSB_EMITTER_TYPE_HIGHLY_MANUV       : return ECat::HighPerformance;
            case ADSB_EMITTER_TYPE_ROTOCRAFT          : return ECat::Rotorcraft;
            case ADSB_EMITTER_TYPE_GLIDER             : return ECat::Glider;
            case ADSB_EMITTER_TYPE_LIGHTER_AIR        : return ECat::LighterThanAir;
            case ADSB_EMITTER_TYPE_PARACHUTE          : return ECat::Parachutist;
            case ADSB_EMITTER_TYPE_ULTRA_LIGHT        : return ECat::Ultralight;
            case ADSB_EMITTER_TYPE_UAV                : return ECat::UAV;
            case ADSB_EMITTER_TYPE_SPACE              : return ECat::Spacecraft;
            case ADSB_EMITTER_TYPE_EMERGENCY_SURFACE  : return ECat::SurfaceEmergencyVehicle;
            case ADSB_EMITTER_TYPE_SERVICE_SURFACE    : return ECat::SurfaceServiceVehicle;
            case ADSB_EMITTER_TYPE_POINT_OBSTACLE     : return ECat::PointObstacle;
                // cases below are specified in DO-260C/ED-102B, but not in MAVLINK
                // case ADSB_EMITTER_TYPE_CLUSTER_OBSTACLE: [[fallthrough]];
                // case ADSB_EMITTER_TYPE_LINE_OBSTACLE   : [[fallthrough]];
            default                                   : return ECat::NoInformation;
            }
        }

        constexpr auto
        to_string(ADSBVehicle::EmitterCategory const ecat) noexcept
            -> char const *
        {
            using ECat = ADSBVehicle::EmitterCategory;
            switch (ecat) {
            case ECat::MTOW_lt_15500_lbs       : return "MTOW < 15500 lbs";
            case ECat::MTOW_lt_75000_lbs       : return "MTOW < 75000 lbs";
            case ECat::MTOW_lt_300000_lbs      : return "MTOW < 300000 lbs";
            case ECat::MTOW_ge_300000_lbs      : return "MTOW >= 300000 lbs";
            case ECat::Rotorcraft              : return "rotorcraft";
            case ECat::Glider                  : return "glider or sailplane";
            case ECat::LighterThanAir          : return "lighter-than-air";
            case ECat::Ultralight              : return "ultralight or hangglider or paraglider";
            case ECat::SurfaceEmergencyVehicle : return "surface emergency vehicle";
            case ECat::SurfaceServiceVehicle   : return "surface service vehicle";
            case ECat::PointObstacle           : return "point obstacle or tethered balloon";
            case ECat::ClusterObstacle         : return "cluster obstacle";
            case ECat::LineObstacle            : return "line obstacle";
            case ECat::HighVortexLarge         : return "high vortex large";
            case ECat::HighPerformance         : return "high performance";
            case ECat::UAV                     : return "uncrewed aircraft";
            case ECat::Spacecraft              : return "spacecraft or transatmospheric vehicle";
            case ECat::Parachutist             : return "parachutist or skydiver";
            default                            : return "no information";
            }
        }
    }

}

// Local Variables:
// c-basic-offset: 4
// End:
