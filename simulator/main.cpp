#include "TelemetryPacket.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QHostAddress>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("telemetry-sim"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Sends simulated telemetry packets over UDP."));
    parser.addHelpOption();

    const QCommandLineOption addressOption({QStringLiteral("a"), QStringLiteral("address")},
                                           QStringLiteral("Destination address (multicast group or unicast)."),
                                           QStringLiteral("address"), QStringLiteral("239.255.42.1"));
    const QCommandLineOption portOption({QStringLiteral("p"), QStringLiteral("port")},
                                        QStringLiteral("Destination UDP port."),
                                        QStringLiteral("port"), QStringLiteral("45454"));
    const QCommandLineOption rateOption({QStringLiteral("r"), QStringLiteral("rate")},
                                        QStringLiteral("Packets per second."),
                                        QStringLiteral("hz"), QStringLiteral("50"));
    const QCommandLineOption dropOption({QStringLiteral("d"), QStringLiteral("drop")},
                                        QStringLiteral("Percentage of packets to skip, to simulate loss."),
                                        QStringLiteral("percent"), QStringLiteral("0"));
    parser.addOptions({addressOption, portOption, rateOption, dropOption});
    parser.process(app);

    const QHostAddress address(parser.value(addressOption));
    bool portOk = false;
    bool rateOk = false;
    bool dropOk = false;
    const quint16 port = parser.value(portOption).toUShort(&portOk);
    const int rate = parser.value(rateOption).toInt(&rateOk);
    const double dropPercent = parser.value(dropOption).toDouble(&dropOk);

    if (address.isNull() || !portOk || port == 0 || !rateOk || rate <= 0 || rate > 1000
        || !dropOk || dropPercent < 0.0 || dropPercent > 100.0) {
        QTextStream(stderr) << "Invalid arguments. Run with --help for usage.\n";
        return 1;
    }

    QTextStream(stdout) << "Sending to " << address.toString() << ':' << port << " at " << rate
                        << " Hz, dropping " << dropPercent << "% of packets\n";

    // TODO:
    //  - Create a QUdpSocket. For multicast on the same machine, set
    //    MulticastLoopbackOption to 1 so a local receiver sees the packets.
    //  - Start a QTimer with interval 1000 / rate ms.
    //  - On each tick, build a TelemetrySample: sequence++, current time in ms,
    //    three sine-plus-noise channels (e.g. ~25 °C, ~101 kPa, ~3.3 V).
    //  - With probability dropPercent, skip the send but still increment the
    //    sequence, so the receiver sees a gap.
    //  - Otherwise writeDatagram(TelemetryPacket::encode(sample), address, port).

    return app.exec();
}
