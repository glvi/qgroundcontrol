/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "ADSBVehicle.h"

#include <QObject>

namespace ADSB {

    class SBS1Decoder : public QObject
    {
        Q_OBJECT

    public:
        Q_INVOKABLE explicit SBS1Decoder(QObject* parent = nullptr);
        virtual ~SBS1Decoder();

    public slots:
        void decode(QByteArray);

    signals:
        void decoded(ADSBVehicle::ADSBVehicleInfo_t const&);
    };
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
