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

    class MavlinkDecoder : public QObject
    {
        Q_OBJECT

    public:
        Q_INVOKABLE explicit MavlinkDecoder(QObject* parent = nullptr);
        virtual ~MavlinkDecoder();

    public slots:
        void decode(QByteArray);

    signals:
        void decoded(ADSBVehicle::ADSBVehicleInfo_t const&);

    private:
        struct State;
        std::unique_ptr<State> state;
    };
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
