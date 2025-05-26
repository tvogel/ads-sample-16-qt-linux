#pragma once

#include <QDialog>
#include <QString>
#include "AdsDevice.h"
#include "AdsParseSymbols.h"

typedef struct ADSDYNSYM_SUBINFO
{
    CAdsSymbolInfo infoParent;
    long nSub;
    ADSDYNSYM_SUBINFO* pParent;
} ADSDYNSYM_SUBINFO, *PADSDYNSYM_SUBINFO;

namespace Ui {
  class Sample16Dialog;
}

class CSample16Dlg : public QDialog
{
    Q_OBJECT

public:
    explicit CSample16Dlg(const char * targetIp, const AmsNetId & targetNetId, quint16 targetPort, QWidget * parent = nullptr);
    ~CSample16Dlg();

private slots:
    void onButtonLoad();
    void onButtonParent();
    void onButtonSibling();
    void onButtonChild();
    void onButtonNext();

private:
    bool initDialog();
    void handleNavigation(long navType, const QString& errorMessage);

    AdsDevice m_adsDevice;

    QString m_strName;
    QString m_strFullName;
    QString m_strComment;
    long m_nIndexGroup;
    long m_nIndexOffset;
    long m_nSize;
    QString m_strType;
    long m_nAdsType;

    CAdsParseSymbols* m_pDynSymbols;
    long m_nCurDynSymbol;
    ADSDYNSYM_SUBINFO* m_pCurSubSymbol;
    long m_nNextNavType;

    Ui::Sample16Dialog * m_ui;

    long adsSetFirstDynSymbol(bool bForceReload);
    long adsGetNextDynSymbol(long navType, QString& strName, QString& strFullName,
                             QString& strType, QString& strComment, long& adsType,
                             long& cbSymbolSize, long& nIndexGroup, long& IndexOffset);
};
