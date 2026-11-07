#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>
#include <QtQml/QQmlExtensionPlugin>

#include <cstdlib>

// The QML module is a static library; this pulls its plugin into the executable.
Q_IMPORT_QML_PLUGIN(TelemetryMonitorPlugin)

int main(int argc, char *argv[])
{
    // 4x multisampling gives the chart lines smooth edges.
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("Telemetry Monitor"));
    QGuiApplication::setApplicationVersion(QStringLiteral("1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Real-time UDP telemetry monitor."));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption startOption({QStringLiteral("s"), QStringLiteral("start")},
                                         QStringLiteral("Start listening as soon as the window opens."));
    const QCommandLineOption addressOption({QStringLiteral("a"), QStringLiteral("address")},
                                           QStringLiteral("Multicast group to join, or local address to listen on."),
                                           QStringLiteral("address"));
    const QCommandLineOption portOption({QStringLiteral("p"), QStringLiteral("port")},
                                        QStringLiteral("UDP port."), QStringLiteral("port"));
    parser.addOptions({startOption, addressOption, portOption});
    parser.process(app);

    QVariantMap initialProperties;
    initialProperties.insert(QStringLiteral("autoStart"), parser.isSet(startOption));
    if (parser.isSet(addressOption))
        initialProperties.insert(QStringLiteral("initialAddress"), parser.value(addressOption));
    if (parser.isSet(portOption))
        initialProperties.insert(QStringLiteral("initialPort"), parser.value(portOption).toInt());

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
    engine.setInitialProperties(initialProperties);
    engine.loadFromModule("TelemetryMonitor", "Main");

    return app.exec();
}
