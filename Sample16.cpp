// Sample16.cpp : Defines the class behaviors for the application.
//

#include "Sample16Dlg.h"
#include <QApplication>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QString targetIp = app.arguments().value(1, qgetenv("ADS_TARGET_IP"));
  AmsNetId targetNetId = AmsNetId(app.arguments().value(2, qgetenv("ADS_TARGET_NETID")).toStdString());
  quint16 targetPort = app.arguments().value(3, qgetenv("ADS_TARGET_PORT")).toUShort();

  if (!targetNetId)
  {
    qWarning("Usage: %s <targetIp> <targetNetId> [<targetPort>]", qPrintable(app.arguments().at(0)));
    qInfo() << "Or set environment variables:\n"
            << "ADS_TARGET_IP\n"
            << "ADS_TARGET_NETID\n"
            << "ADS_TARGET_PORT (optional, defaults to 851)";

    return -1;
  }

  if (targetPort == 0)
  {
    targetPort = AMSPORT_R0_PLC_TC3;
  }

  CSample16Dlg dlg(qPrintable(targetIp), targetNetId, targetPort);

  dlg.show();

  return app.exec();
}
