#include "TelemetrySimulator.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("telemetry-sim"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Sends simulated telemetry packets over UDP."));
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption addressOption({QStringLiteral("a"), QStringLiteral("address")},
                                           QStringLiteral("Destination address (multicast group or unicast)."),
                                           QStringLiteral("address"), QStringLiteral("239.255.42.1"));
    const QCommandLineOption portOption({QStringLiteral("p"), QStringLiteral("port")},
                                        QStringLiteral("Destination UDP port."),
                                        QStringLiteral("port"), QStringLiteral("45454"));
    const QCommandLineOption rateOption({QStringLiteral("r"), QStringLiteral("rate")},
                                        QStringLiteral("Packets per second (1-1000)."),
                                        QStringLiteral("hz"), QStringLiteral("50"));
    const QCommandLineOption dropOption({QStringLiteral("d"), QStringLiteral("drop")},
                                        QStringLiteral("Percentage of packets to skip, to simulate loss."),
                                        QStringLiteral("percent"), QStringLiteral("0"));
    parser.addOptions({addressOption, portOption, rateOption, dropOption});
    parser.process(app);

    SimulatorConfig config;
    config.address = QHostAddress(parser.value(addressOption));

    bool portOk = false;
    bool rateOk = false;
    bool dropOk = false;
    config.port = parser.value(portOption).toUShort(&portOk);
    config.rateHz = parser.value(rateOption).toInt(&rateOk);
    config.dropPercent = parser.value(dropOption).toDouble(&dropOk);

    QTextStream err(stderr);
    if (config.address.isNull()) {
        err << "Invalid address: " << parser.value(addressOption) << '\n';
        return 1;
    }
    if (!portOk || config.port == 0) {
        err << "Invalid port: " << parser.value(portOption) << '\n';
        return 1;
    }
    if (!rateOk || config.rateHz < 1 || config.rateHz > 1000) {
        err << "Rate must be between 1 and 1000 Hz\n";
        return 1;
    }
    if (!dropOk || config.dropPercent < 0.0 || config.dropPercent > 100.0) {
        err << "Drop percentage must be between 0 and 100\n";
        return 1;
    }

    TelemetrySimulator simulator(config);
    QString error;
    if (!simulator.start(&error)) {
        err << "Could not open a UDP socket: " << error << '\n';
        return 1;
    }

    QTextStream(stdout) << "Sending to " << config.address.toString() << ':' << config.port << " at "
                        << config.rateHz << " Hz, dropping " << config.dropPercent
                        << "% of packets. Press Ctrl+C to stop.\n";

    return app.exec();
}
