/*
 * Copyright (C) 2018-2019 QuasarApp.
 * Distributed under the lgplv3 software license, see the accompanying
 * Everyone is permitted to copy and distribute verbatim copies
 * of this license document, but changing it is not allowed.
 */

#include "extracter.h"
#include "deploycore.h"
#include "quasarapp.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <configparser.h>

//QString DeployCore::qtDir = "";
//QStringList DeployCore::extraPaths = QStringList();


const DeployConfig* DeployCore::_config = nullptr;

QtModuleEntry DeployCore::qtModuleEntries[] = {
    { QtBluetoothModule, "bluetooth", "Qt5Bluetooth", nullptr },
    { QtConcurrentModule, "concurrent", "Qt5Concurrent", "qtbase" },
    { QtCoreModule, "core", "Qt5Core", "qtbase" },
    { QtDeclarativeModule, "declarative", "Qt5Declarative", "qtquick1" },
    { QtDesignerModule, "designer", "Qt5Designer", nullptr },
    { QtDesignerComponents, "designercomponents", "Qt5DesignerComponents", nullptr },
    { QtEnginioModule, "enginio", "Enginio", nullptr },
    { QtGamePadModule, "gamepad", "Qt5Gamepad", nullptr },
    { QtGuiModule, "gui", "Qt5Gui", "qtbase" },
    { QtHelpModule, "qthelp", "Qt5Help", "qt_help" },
    { QtMultimediaModule, "multimedia", "Qt5Multimedia", "qtmultimedia" },
    { QtMultimediaWidgetsModule, "multimediawidgets", "Qt5MultimediaWidgets", "qtmultimedia" },
    { QtMultimediaQuickModule, "multimediaquick", "Qt5MultimediaQuick_p", "qtmultimedia" },
    { QtNetworkModule, "network", "Qt5Network", "qtbase" },
    { QtNfcModule, "nfc", "Qt5Nfc", nullptr },
    { QtOpenGLModule, "opengl", "Qt5OpenGL", nullptr },
    { QtPositioningModule, "positioning", "Qt5Positioning", nullptr },
    { QtPrintSupportModule, "printsupport", "Qt5PrintSupport", nullptr },
    { QtQmlModule, "qml", "Qt5Qml", "qtdeclarative" },
    { QtQmlToolingModule, "qmltooling", "qmltooling", nullptr },
    { QtQuickModule, "quick", "Qt5Quick", "qtdeclarative" },
    { QtQuickParticlesModule, "quickparticles", "Qt5QuickParticles", nullptr },
    { QtQuickWidgetsModule, "quickwidgets", "Qt5QuickWidgets", nullptr },
    { QtScriptModule, "script", "Qt5Script", "qtscript" },
    { QtScriptToolsModule, "scripttools", "Qt5ScriptTools", "qtscript" },
    { QtSensorsModule, "sensors", "Qt5Sensors", nullptr },
    { QtSerialPortModule, "serialport", "Qt5SerialPort", "qtserialport" },
    { QtSqlModule, "sql", "Qt5Sql", "qtbase" },
    { QtSvgModule, "svg", "Qt5Svg", nullptr },
    { QtTestModule, "test", "Qt5Test", "qtbase" },
    { QtWebKitModule, "webkit", "Qt5WebKit", nullptr },
    { QtWebKitWidgetsModule, "webkitwidgets", "Qt5WebKitWidgets", nullptr },
    { QtWebSocketsModule, "websockets", "Qt5WebSockets", nullptr },
    { QtWidgetsModule, "widgets", "Qt5Widgets", "qtbase" },
    { QtWinExtrasModule, "winextras", "Qt5WinExtras", nullptr },
    { QtXmlModule, "xml", "Qt5Xml", "qtbase" },
    { QtXmlPatternsModule, "xmlpatterns", "Qt5XmlPatterns", "qtxmlpatterns" },
    { QtWebEngineCoreModule, "webenginecore", "Qt5WebEngineCore", nullptr },
    { QtWebEngineModule, "webengine", "Qt5WebEngine", "qtwebengine" },
    { QtWebEngineWidgetsModule, "webenginewidgets", "Qt5WebEngineWidgets", nullptr },
    { Qt3DCoreModule, "3dcore", "Qt53DCore", nullptr },
    { Qt3DRendererModule, "3drenderer", "Qt53DRender", nullptr },
    { Qt3DQuickModule, "3dquick", "Qt53DQuick", nullptr },
    { Qt3DQuickRendererModule, "3dquickrenderer", "Qt53DQuickRender", nullptr },
    { Qt3DInputModule, "3dinput", "Qt53DInput", nullptr },
    { Qt3DAnimationModule, "3danimation", "Qt53DAnimation", nullptr },
    { Qt3DExtrasModule, "3dextras", "Qt53DExtras", nullptr },
    { QtLocationModule, "geoservices", "Qt5Location", nullptr },
    { QtWebChannelModule, "webchannel", "Qt5WebChannel", nullptr },
    { QtTextToSpeechModule, "texttospeech", "Qt5TextToSpeech", nullptr },
    { QtSerialBusModule, "serialbus", "Qt5SerialBus", nullptr },
    { QtWebViewModule, "webview", "Qt5WebView", nullptr }
};

DeployCore::QtModule DeployCore::getQtModule(const QString& path) {
    auto priority = DeployCore::getLibPriority(path);

    if (priority != QtLib) {
        return DeployCore::QtModule::NONE;
    }

    int modulesCount = sizeof (qtModuleEntries) / sizeof (QtModuleEntry);

    auto lIbName = QFileInfo(path).fileName();

    for (int i = 0; i < modulesCount; ++i) {
        if (lIbName.contains(qtModuleEntries[i].libraryName)) {            
            return static_cast<DeployCore::QtModule>(qtModuleEntries[i].module);
        }
    }

    return DeployCore::QtModule::NONE;
}

void DeployCore::addQtModule(DeployCore::QtModule &module, const QString &path) {

    QuasarAppUtils::Params::verboseLog("current module " + QString::number(module),
                                       QuasarAppUtils::Info);

    auto mod = getQtModule(path);
    QuasarAppUtils::Params::verboseLog("add new module from path " + path  +
                                       " module value " + QString::number(mod),
                                       QuasarAppUtils::Info);


    module = static_cast<DeployCore::QtModule>(
                static_cast<quint64>(module) | static_cast<quint64>(mod));


}

LibPriority DeployCore::getLibPriority(const QString &lib) {

    if (!QFileInfo(lib).isFile()) {
        return NotFile;
    }

    if (isQtLib(lib)) {
        return QtLib;
    }

    if (isExtraLib(lib)) {
        return ExtraLib;
    }

    return SystemLib;
}

void DeployCore::verboseLog(const QString &str) {
    if (QuasarAppUtils::Params::isEndable("verbose")) {
        qDebug() << str;
    }
}

#define C(X) QuasarAppUtils::Params::isEndable(X)
RunMode DeployCore::getMode() {
    if (C("help") || C("h") || C("v") || C("version")) {
        return RunMode::Info;
    }

    if (C("bin") || C("binDir")) {
        return RunMode::Deploy;
    }

    if (C("clear") || C("force-clear")) {
        return RunMode::Clear;
    }

    return RunMode::Deploy;
}

QString DeployCore::help() {

    QStringList help = {
    { "CQtDeployer version: " + getAppVersion()},
    { "Usage: cqtdeployer <-bin    [params]> [options]"},
    { "" },
    { "Options:" },
    { "   help / h                 : Shows help." },
    { "   noOverwrite              : Prevents replacing existing files." },
    { "   -bin    [list, params]   : Deployable file or folder." },
    { "                            | For example -bin /my/project/bin/,/my/project/bin.exe" },
    { "   -binDir [params]         : A folder which includes deployable files (recursive search)." },
    { "                            | WARNING: this flag supports 'so', 'dll' and 'exe' files only." },
    { "                            | Use '-bin' flag if you want to deploy linux binary files" },
    { "   -confFile [params]       : Path to json file with all deploy config" },
    { "                            | Using this file, you can add the necessary options, " },
    { "                            | thereby simplifying the command call in the console. "},
    { "                            | However, the parameters in Kansol have a higher priority than in the file. "},
    { "   -qmlDir [params]         : Qml data dir. For example -qmlDir ~/my/project/qml" },
    { "   deploySystem             : Deploys all libs" },
    { "   noLibc                   : Skip Deploys libc and ld-linux libs" },
    { "   -qmake  [params]         : Qmake path." },
    { "                            | For example -qmake ~/Qt/5.11.1/gcc_64/bin/qmake" },
    { "   -ignore [list,params]    : The list of libs to ignore." },
    { "                            | For example -ignore libicudata.so.56,libicudata2.so.56" },
    { "   -ignoreEnv [list,params] : The list of the environment to ignore" },
    { "                            | For example -ignoreEnv /bad/dir,/my/bad/Dir" },
    { "   clear                    : Deletes deployable files of the previous session." },
    { "   force-clear              : Deletes the destination directory before deployment." },
    { "   allQmlDependes           : Extracts all the qml libraries." },
    { "                            | (not recommended, as it takes great amount of computer memory)" },
    { "   -libDir [list,params]    : Sets additional paths for extra libs of an app." },
    { "                            | For example -libDir /myLib,/newLibs " },
    { "   -extraPlugin[list,params]: Sets an additional path to extraPlugin of an app" },
    { "   -recursiveDepth [params] : Sets the Depth of recursive search of libs (default 0)" },
    { "   -targetDir [params]      : Sets target directory(by default it is the path to the first deployable file)" },
    { "   noStrip                  : Skips strip step" },
    { "   noTranslations           : Skips the translations files." },
    { "                            | It doesn't work without qmake and inside a snap package" },
    { "   v / version              : Shows compiled version" },
    { "   verbose [1-3]            : Shows debug log" },

    { "" },
    { "" },
    { "Example: cqtdeployer -bin myApp -qmlDir ~/Qt/5.14.0/gcc_64/qml -qmake ~/Qt/5.14.0/gcc_64/bin/qmake clear" },
    { "Example (only C libs): cqtdeployer -bin myApp clear" }};

    QuasarAppUtils::Params::showHelp(help);

    return help.join(" ");
}

QStringList DeployCore::helpKeys() {
    return {
        "help",
        "noOverwrite",
        "bin",
        "binDir",
        "qmlDir",
        "deploySystem",
        "noLibc",
        "qmake",
        "ignore",
        "ignoreEnv",
        "clear",
        "force-clear",
        "allQmlDependes",
        "libDir",
        "extraPlugin",
        "recursiveDepth",
        "targetDir",
        "noStrip",
        "noTranslations",
        "version",
        "verbose"
    };
}

QStringList DeployCore::extractTranslation(const QStringList &libs) {
    QSet<QString> res;
    const size_t qtModulesCount = sizeof(qtModuleEntries) / sizeof(QtModuleEntry);

    for (auto &&lib: libs) {
        for (size_t i = 0; i < qtModulesCount; ++i) {
            if (lib.contains(qtModuleEntries[i].libraryName) &&
                    qtModuleEntries[i].translation) {
                res.insert(qtModuleEntries[i].translation);
            }
        }
    }
    return res.toList();
}

QString DeployCore::getAppVersion() {
    return APP_VERSION;
}

QString DeployCore::getQtVersion() {
#ifdef QT_VERSION_STR
    return QT_VERSION_STR;
#else
    return "without qt";
#endif
}

void DeployCore::printVersion() {
    qInfo() << "CQtDeployer: " + getAppVersion();
    qInfo() << "Qt: " +  getQtVersion();
}

int DeployCore::find(const QString &str, const QStringList &list) {
    for (int i = 0 ; i < list.size(); ++i) {
        if (list[i].contains(str))
            return i;
    }
    return -1;
}

bool DeployCore::isLib(const QFileInfo &file) {
    return file.completeSuffix().contains("so", Qt::CaseInsensitive)
            || file.completeSuffix().contains("dll", Qt::CaseInsensitive);
}

bool DeployCore::isPath(const QString &path) {
    return path.contains('/') || path.contains('\\');
}

MSVCVersion DeployCore::getMSVC(const QString &_qmake) {
    QFileInfo qmake(_qmake);

    int res = MSVCVersion::MSVC_Unknown;

    if (!qmake.isFile()) {
        QuasarAppUtils::Params::verboseLog("qmake is wrong!");
        return static_cast<MSVCVersion>(res);
    }

    QDir dir = qmake.absoluteDir();

    if (!dir.cdUp()) {
        QuasarAppUtils::Params::verboseLog("is not standart qt repo");
        return static_cast<MSVCVersion>(res);
    }

    auto msvcPath = dir.absolutePath();

    if (!(dir.cdUp() && dir.cdUp())) {
        QuasarAppUtils::Params::verboseLog("is not standart qt repo");
        return static_cast<MSVCVersion>(res);
    }

    if (!msvcPath.contains("msvc")) {
        QuasarAppUtils::Params::verboseLog("vcredis not defined");
        return static_cast<MSVCVersion>(res);
    }

    auto base = msvcPath.mid(msvcPath.indexOf("msvc"), 11);
    auto version = base.mid(4 , 4);
    auto type = base.right(2);

    if (version == "2013") {
        res |= MSVC_13;
    }
    else if (version == "2015") {
        res |= MSVC_15;
    }
    else if (version == "2017") {
        res |= MSVC_17;
    }
    else if (version == "2019") {
        res |= MSVC_19;
    }

    if (type == "32") {
        res |= MSVC_x32;
    }
    else if (type == "64") {
        res |= MSVC_x64;
    }

    return static_cast<MSVCVersion>(res);
}

QString DeployCore::getVCredist(const QString &_qmake) {
    auto msvc = getMSVC(_qmake);

    QFileInfo qmake(_qmake);

    QDir dir = qmake.absoluteDir();

    if (!(dir.cdUp() && dir.cdUp() && dir.cdUp() && dir.cd("vcredist"))) {
        QuasarAppUtils::Params::verboseLog("redist not findet!");
        return "";
    }

    auto infoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    auto name = getMSVCName(msvc);
    auto version = getMSVCVersion(msvc);

    for (auto &&info: infoList) {
        auto file = QFileInfo(info).fileName();
        if (file.contains(name) && file.contains(version)) {
            return info.absoluteFilePath();
        }
    }

    return "";
}

QString DeployCore::getMSVCName(MSVCVersion msvc) {
    if (msvc | MSVCVersion::MSVC_13) {
        return "msvc2013";
    } else if (msvc | MSVCVersion::MSVC_15) {
        return "msvc2015";
    } else if (msvc | MSVCVersion::MSVC_17) {
        return "msvc2017";
    } else if (msvc | MSVCVersion::MSVC_19) {
        return "msvc2019";
    }

    return "";
}

QString DeployCore::getMSVCVersion(MSVCVersion msvc) {
    if (msvc | MSVCVersion::MSVC_x32) {
        return "x86";
    } else if (msvc | MSVCVersion::MSVC_x64) {
        return "x64";
    }

    return "";
}

bool DeployCore::isQtLib(const QString &lib) {
    QFileInfo info(lib);
    return !_config->qtDir.isEmpty() && info.absoluteFilePath().contains(_config->qtDir);

}

bool DeployCore::isExtraLib(const QString &lib) {
    QFileInfo info(lib);

    for (auto i : _config->extraPaths) {
        if (info.absoluteFilePath().contains(i)) {
            return true;
        }
    }

    return false;
}
