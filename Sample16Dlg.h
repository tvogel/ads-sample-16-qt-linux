#pragma once

#include <QDialog>
#include <QString>
#include "AdsDevice.h"
#include "AdsParseSymbols.h"

typedef struct ADSDYNSYM_SUBINFO
{
    CAdsSymbolInfo infoParent;
    uint32_t nSub;
    ADSDYNSYM_SUBINFO* pParent;
} ADSDYNSYM_SUBINFO, *PADSDYNSYM_SUBINFO;

namespace Ui {
  class Sample16Dialog;
}

class CSample16Dlg : public QDialog
{
    Q_OBJECT

public:
    explicit CSample16Dlg(const char * targetIp, const AmsNetId & targetNetId, uint16_t targetPort, QWidget * parent = nullptr);
    ~CSample16Dlg();

private slots:
    void onButtonLoad();
    void onButtonParent();
    void onButtonSibling();
    void onButtonChild();
    void onButtonNext();

private:
    bool initDialog();
    void handleNavigation(uint32_t navType, const QString & errorMessage);

    AdsDevice m_adsDevice;

    QString m_strName;
    QString m_strFullName;
    QString m_strComment;
    uint32_t m_nIndexGroup;
    uint32_t m_nIndexOffset;
    uint32_t m_nSize;
    QString m_strType;
    uint32_t m_nAdsType;

    CAdsParseSymbols* m_pDynSymbols;
    uint32_t m_nCurDynSymbol;
    ADSDYNSYM_SUBINFO* m_pCurSubSymbol;
    uint32_t m_nNextNavType;

    Ui::Sample16Dialog * m_ui;

    uint32_t adsSetFirstDynSymbol(bool bForceReload);
    uint32_t adsGetNextDynSymbol(uint32_t navType, QString & strName, QString & strFullName,
                             QString & strType, QString & strComment, uint32_t & adsType,
                             uint32_t & cbSymbolSize, uint32_t & nIndexGroup, uint32_t & IndexOffset);
};
