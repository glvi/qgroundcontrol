/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include <QObject>

namespace ADSB {

    class TCPLinkController : public QObject
    {
        Q_OBJECT

        using Self = TCPLinkController;

    public:
        enum class ByteOriented {};
        enum class LineOriented {};

        explicit TCPLinkController(QString hostAddress, int port, ByteOriented = {}, QObject* parent = nullptr);
        explicit TCPLinkController(QString hostAddress, int port, LineOriented, QObject* parent = nullptr);
        ~TCPLinkController();

        void establishLink();

    signals:
        void readLine(QByteArray);
        void readBytes(QByteArray);
        void error(QString const& errorMsg);

    private:
        void _hardwareConnect();
        void _newLink();

        QString  _hostAddress = "127.0.0.1";
        int      _port = 0;
        bool     _lineOriented = false;
        QObject* _link;

    private slots:
        void _linkUp();
        void _linkDown();
        void _readBytes();
    };

}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
