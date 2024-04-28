/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtCore/QElapsedTimer>
#include <QtCore/QLoggingCategory>
#include <QtPositioning/QGeoCoordinate>

Q_DECLARE_LOGGING_CATEGORY(ADSBVehicleManagerLog)

class ADSBVehicle : public QObject
{
    Q_OBJECT

public:
    enum {
        CategoryAvailable = 1 << 0,
        CallsignAvailable = 1 << 1,
        LocationAvailable = 1 << 2,
        AltitudeAvailable = 1 << 3,
        HeadingAvailable  = 1 << 4,
        AlertAvailable    = 1 << 5,
    };

    /*
      Emitter category encoding as defined in DO-260C/ED-102B (ADS-B v3).
     */
    enum class EmitterCategory {
        NoInformation           =  0,
        MTOW_lt_15500_lbs       =  1,
        MTOW_lt_75000_lbs       =  3,
        MTOW_lt_300000_lbs      =  5,
        MTOW_ge_300000_lbs      =  7,
        Rotorcraft              = 10,
        Glider                  = 11,
        Sailplane               = 11,
        LighterThanAir          = 12,
        Ultralight              = 15,
        Hangglider              = 15,
        Paraglider              = 15,
        SurfaceEmergencyVehicle = 20,
        SurfaceServiceVehicle   = 21,
        PointObstacle           = 22,
        ClusterObstacle         = 23,
        LineObstacle            = 24,
        // Encodings that were defined for previous versions of ADS-B,
        // but have been deprecated in ADS-B v3.
        Light [[deprecated("use \"MTOW_lt_15500_lbs\" instead")]]  = MTOW_lt_15500_lbs,
        Small [[deprecated("use \"MTOW_lt_75000_lbs\" instead")]]  = MTOW_lt_75000_lbs,
        Large [[deprecated("use \"MTOW_lt_300000_lbs\" instead")]] = MTOW_lt_300000_lbs,
        Heavy [[deprecated("use \"MTOW_ge_300000_lbs\" instead")]] = MTOW_ge_300000_lbs,
        HighVortexLarge         [[deprecated]] = 6,
        HighPerformance         [[deprecated]] = 8,
        UAV                     [[deprecated]] = 13,
        Spacecraft              [[deprecated]] = 14,
        TransatmosphericVehicle [[deprecated]] = 14,
        Parachutist             [[deprecated]] = 16,
        Skydiver                [[deprecated]] = 16,
    };

    struct ADSBVehicleInfo_t {
        uint32_t        icaoAddress;    // Required
        QString         callsign;
        QGeoCoordinate  location;
        double          altitude;
        double          heading;
        bool            alert;
        EmitterCategory category;
        uint32_t        availableFlags;
    };

    ADSBVehicle(const ADSBVehicleInfo_t & vehicleInfo, QObject* parent);

    Q_PROPERTY(int              icaoAddress READ icaoAddress    CONSTANT)
    Q_PROPERTY(QString          callsign    READ callsign       NOTIFY callsignChanged)
    Q_PROPERTY(QGeoCoordinate   coordinate  READ coordinate     NOTIFY coordinateChanged)
    Q_PROPERTY(double           altitude    READ altitude       NOTIFY altitudeChanged)     // NaN for not available
    Q_PROPERTY(double           heading     READ heading        NOTIFY headingChanged)      // NaN for not available
    Q_PROPERTY(bool             alert       READ alert          NOTIFY alertChanged)        // aircraft emergency status
    Q_PROPERTY(EmitterCategory  category    READ category       NOTIFY categoryChanged)

    int             icaoAddress () const { return static_cast<int>(_icaoAddress); }
    QString         callsign    () const { return _callsign; }
    QGeoCoordinate  coordinate  () const { return _coordinate; }
    double          altitude    () const { return _altitude; }
    double          heading     () const { return _heading; }
    bool            alert       () const { return _alert; }
    EmitterCategory category    () const { return _category; }

    void update(const ADSBVehicleInfo_t & vehicleInfo);

    /// check if the vehicle is expired and should be removed
    bool expired();

signals:
    void coordinateChanged  ();
    void callsignChanged    ();
    void altitudeChanged    ();
    void headingChanged     ();
    void alertChanged       ();
    void categoryChanged    ();

private:
    uint32_t        _icaoAddress;
    QString         _callsign;
    QGeoCoordinate  _coordinate;
    double          _altitude;
    double          _heading;
    bool            _alert;
    EmitterCategory _category;

    QElapsedTimer   _lastUpdateTimer;

    static constexpr qint64 expirationTimeoutMs = 120000;   ///< timeout with no update in ms after which the vehicle is removed.
};

Q_DECLARE_METATYPE(ADSBVehicle::ADSBVehicleInfo_t)

// Local Variables:
// c-basic-offset: 4
// End:
