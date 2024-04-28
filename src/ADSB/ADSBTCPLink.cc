/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "ADSBTCPLink.h"

#include "QGCLoggingCategory.h"

#include <QTimer>

namespace ADSB {

    TCPLink::TCPLink(QObject* parent)
        : QObject {parent}
        , _socket {new QTcpSocket}
    {
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Constructed";
        connect(_socket, &QTcpSocket::stateChanged, this, &TCPLink::stateChanged);
        connect(_socket, &QTcpSocket::connected, this, &TCPLink::connected);
        connect(_socket, &QTcpSocket::disconnected, this, &TCPLink::disconnected);
    }

    TCPLink::~TCPLink()
    {
        _socket->disconnectFromHost();
        _socket->deleteLater();
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Destroyed";
    }

    void TCPLink::start(QString hostname, quint16 port)
    {
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Started";
        _socket->connectToHost(hostname, port);
        QTimer::singleShot(3000, this, &TCPLink::timeout);
    }

    void TCPLink::connected()
    {
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Connected";
        disconnect(_socket, &QTcpSocket::connected, this, &TCPLink::connected);
        emit linkUp();
    }

    void TCPLink::disconnected()
    {
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Disconnected";
        emit linkDown();
    }

    void TCPLink::timeout()
    {
        if (_socket->state() == QTcpSocket::ConnectedState) {
            return;
        }
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Connection attempt timed out";
        emit linkDown();
    }

    void TCPLink::stateChanged(QTcpSocket::SocketState newState)
    {
        switch (newState) {
        case QTcpSocket::UnconnectedState:
            qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Socket unconnected";
            break;
        case QTcpSocket::HostLookupState:
            qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Host lookup";
            break;
        case QTcpSocket::ConnectingState:
            qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Socket connecting";
            break;
        case QTcpSocket::ConnectedState:
            qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Socket connected";
            break;
        case QTcpSocket::BoundState:
            qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Socket bound";
            break;
        case QTcpSocket::ListeningState:
            qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Socket listening";
            break;
        case QTcpSocket::ClosingState:
            qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link -- Socket closing";
            break;
        }
    }

}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
