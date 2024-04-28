/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include <QObject>
#include <QTcpSocket>

namespace ADSB {

    class TCPLinkController;

    class TCPLink : public QObject
    {
        Q_OBJECT

    public:
        explicit TCPLink(QObject* parent = nullptr);
        virtual ~TCPLink();
        void start(QString hostAddress, quint16 port);

    private slots:
        void connected();
        void disconnected();
        void timeout();
        void stateChanged(QTcpSocket::SocketState);

    signals:
        void linkUp();
        void linkDown();

    private:
        QTcpSocket* _socket = nullptr;

        friend class TCPLinkController;
    };

}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
