/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "SBS1Decoder.h"

#include "QGCLoggingCategory.h"

namespace ADSB {

    namespace { // utilities (declarations)

        using Result = ADSBVehicle::ADSBVehicleInfo_t;

        using CompletionHandler = std::function<void(Result&&)>;

        void _parseLine(QString&&, CompletionHandler);

        void _parseAndEmitCallsign(QStringList const&, Result&&, CompletionHandler);

        void _parseAndEmitLocation(QStringList const&, Result&&, CompletionHandler);

        void _parseAndEmitHeading(QStringList const&, Result&&, CompletionHandler);
    }

    SBS1Decoder::SBS1Decoder(QObject* parent)
        : QObject {parent}
    {
    }

    SBS1Decoder::~SBS1Decoder()
    {
    }

    void SBS1Decoder::decode(QByteArray line)
    {
        _parseLine(QString::fromLocal8Bit(line), [this] (Result&& result) {
            emit decoded(result);
        });
    }

    namespace { // utilities (definitions)

        void _parseLine(QString&& line, CompletionHandler ch)
        {
            if (line.startsWith(QStringLiteral("MSG"))) {
                bool icaoOk = false;
                int msgType = line.at(4).digitValue();
                if (msgType == -1) {
                    qCDebug(ADSBVehicleManagerLog) << "ADSB Invalid message type " << line.at(4);
                    return;
                }
                // Skip unsupported mesg types to avoid parsing
                if (msgType == 2 || msgType > 6) {
                    return;
                }
                qCDebug(ADSBVehicleManagerLog) << " ADSB SBS-1 " << line;
                QStringList values = line.split(QChar(','));
                uint32_t icaoAddress = values[4].toUInt(&icaoOk, 16);

                if (!icaoOk) {
                    return;
                }

                Result result;
                result.icaoAddress = icaoAddress;

                switch (msgType) {
                case 1: [[fallthrough]];
                case 5: [[fallthrough]];
                case 6:
                    _parseAndEmitCallsign(values, std::move(result), std::move(ch));
                    break;
                case 3:
                    _parseAndEmitLocation(values, std::move(result), std::move(ch));
                    break;
                case 4:
                    _parseAndEmitHeading(values, std::move(result), std::move(ch));
                    break;
                }
            }
        }

        void _parseAndEmitCallsign(QStringList const& values,
                                   Result&& result,
                                   CompletionHandler ch)
        {
            QString callsign = values[10].trimmed();
            if (callsign.isEmpty()) {
                return;
            }

            result.callsign = callsign;
            result.availableFlags = ADSBVehicle::CallsignAvailable;
            if (ch) {
                ch(std::move(result));
            }
        }

        void _parseAndEmitLocation(QStringList const& values,
                                   Result&& result,
                                   CompletionHandler ch)
        {
            bool altOk = false, latOk = false, lonOk = false;

            QString altitudeStr = values[11];
            // Altitude is either Barometric - based on pressure, in ft
            // or HAE - as reported by GPS - based on WGS84 Ellipsoid, in ft
            // If altitude ends with H, we have HAE
            // There's a slight difference between Barometric alt and HAE, but it would require
            // knowledge about Geoid shape in particular Lat, Lon. It's not worth complicating the code
            if (altitudeStr.endsWith('H')) {
                altitudeStr.chop(1);
            }
            int modeCAltitude = modeCAltitude = altitudeStr.toInt(&altOk);

            double lat = values[14].toDouble(&latOk);
            double lon = values[15].toDouble(&lonOk);
            int alert = values[19].toInt();

            if (!altOk || !latOk || !lonOk) {
                return;
            }
            if (lat == 0 && lon == 0) {
                return;
            }

            double altitude = modeCAltitude * 0.3048;
            QGeoCoordinate location(lat, lon);
            result.location = location;
            result.altitude = altitude;
            result.alert = alert == 1;
            result.availableFlags = ADSBVehicle::LocationAvailable | ADSBVehicle::AltitudeAvailable | ADSBVehicle::AlertAvailable;

            if (ch) {
                ch(std::move(result));
            }
        }

        void _parseAndEmitHeading (QStringList const& values,
                                   Result&& result,
                                   CompletionHandler ch)
        {
            bool headingOk = false;
            double heading = values[13].toDouble(&headingOk);
            if (!headingOk) {
                return;
            }

            result.heading = heading;
            result.availableFlags = ADSBVehicle::HeadingAvailable;

            if (ch) {
                ch(std::move(result));
            }
        }

    }

}

// Local Variables:
// c-basic-offset: 4
// End:
