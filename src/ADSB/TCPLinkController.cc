/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "TCPLinkController.h"
#include "ADSBTCPLink.h"

#include "QGCLoggingCategory.h"

namespace ADSB {

    TCPLinkController::TCPLinkController(QString hostAddress, int port, ByteOriented, QObject* parent)
        : QObject       {parent}
        , _hostAddress  {std::move(hostAddress)}
        , _port         {port}
    {
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link controller -- constructed (byte-oriented)";
    }

    TCPLinkController::TCPLinkController(QString hostAddress, int port, LineOriented, QObject* parent)
        : QObject       {parent}
        , _hostAddress  {std::move(hostAddress)}
        , _port         {port}
        , _lineOriented {true}
    {
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link controller -- constructed (line-oriented)";
    }

    TCPLinkController::~TCPLinkController()
    {
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link controller -- destroyed";
    }

    void TCPLinkController::establishLink()
    {
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link controller -- establishing link";
        _newLink();
    }

    void TCPLinkController::_newLink()
    {
        TCPLink* link = nullptr;
        _link = link = new TCPLink(this);
        connect(link, &TCPLink::linkUp, this, &Self::_linkUp);
        connect(link, &TCPLink::linkDown, this, &Self::_linkDown);
        link->start(_hostAddress, _port);
    }

    void TCPLinkController::_linkUp()
    {
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link controller -- link up";
        if (auto link = dynamic_cast<TCPLink*>(_link)) {
            disconnect(link, &TCPLink::linkUp, this, &Self::_linkUp);
            connect(link->_socket, &QTcpSocket::readyRead, this, &Self::_readBytes);
        }
    }

    void TCPLinkController::_linkDown()
    {
        qCInfo(ADSBVehicleManagerLog) << "ADSB TCP link controller -- link down";
        if (auto link = dynamic_cast<TCPLink*>(_link)) {
            disconnect(link, &TCPLink::linkDown, this, &Self::_linkDown);
            disconnect(link->_socket, &QTcpSocket::readyRead, this, &Self::_readBytes);
        }
        _link->setParent(nullptr);
        delete _link;
        _newLink();
    }

    void TCPLinkController::_readBytes()
    {
        auto link = dynamic_cast<TCPLink*>(_link);
        if (link->_socket == nullptr) return;
        auto& socket = *link->_socket;
        if (_lineOriented) {
            while(socket.canReadLine()) {
                emit readLine(socket.readLine());
            }
        } else {
            emit readBytes(socket.readAll());
        }
    }

}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
