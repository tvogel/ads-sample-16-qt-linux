#include "Sample16Dlg.h"
#include "ui_Sample16Dlg.h"
#include <QMessageBox>
#include <QPainter>
#include <QIcon>

//////////////////////////////////////////////////////////////////////////////
typedef enum ADSGETDYNSYMBOLTYPE
{
  ADSDYNSYM_GET_NEXT    = 1,
  ADSDYNSYM_GET_SIBLING  = 2,
  ADSDYNSYM_GET_CHILD    = 3,
  ADSDYNSYM_GET_PARENT    = 4,
} ADSGETDYNSYMBOLTYPE;

////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  quint32    nSymbols;
  quint32    nSymSize;
  quint32    nDatatypes;
  quint32    nDatatypeSize;
  quint32    nMaxDynSymbols;
  quint32    nUsedDynSymbols;
} AdsSymbolUploadInfo2, *PAdsSymbolUploadInfo2;


#define ADSIGRP_SYM_DT_UPLOAD        0xF00E
#define ADSIGRP_SYM_UPLOADINFO2      0xF00F

/////////////////////////////////////////////////////////////////////////////
// CSample16Dlg dialog

CSample16Dlg::CSample16Dlg(const char * targetIp, const AmsNetId & targetNetId, quint16 targetPort, QWidget * parent)
    : QDialog(parent),
      m_adsDevice(targetIp, targetNetId, targetPort),
      m_strName(""),
      m_strFullName(""),
      m_strComment(""),
      m_nIndexGroup(0),
      m_nIndexOffset(0),
      m_nSize(0),
      m_strType(""),
      m_nAdsType(0),
      m_pDynSymbols(nullptr),
      m_pCurSubSymbol(nullptr),
      m_ui(new Ui::Sample16Dialog)
{
    m_ui->setupUi(this);
    setWindowIcon(QIcon(":/resources/mainframe.ico"));
    initDialog();

    connect(m_ui->buttonLoad, &QPushButton::clicked, this, &CSample16Dlg::onButtonLoad);
    connect(m_ui->buttonParent, &QPushButton::clicked, this, &CSample16Dlg::onButtonParent);
    connect(m_ui->buttonSibling, &QPushButton::clicked, this, &CSample16Dlg::onButtonSibling);
    connect(m_ui->buttonChild, &QPushButton::clicked, this, &CSample16Dlg::onButtonChild);
    connect(m_ui->buttonNext, &QPushButton::clicked, this, &CSample16Dlg::onButtonNext);
}

CSample16Dlg::~CSample16Dlg()
{
    if (m_pDynSymbols)
        delete m_pDynSymbols;
    if (m_pCurSubSymbol)
        delete m_pCurSubSymbol;
}

void CSample16Dlg::onButtonLoad()
{
    long nErr = adsSetFirstDynSymbol(true);
    if (nErr != ADSERR_NOERR)
    {
        QMessageBox::critical(this, "Error", QString("Failed to load dynamic symbols (error = 0x%1).").arg(nErr, 0, 16));
    }
}

void CSample16Dlg::onButtonParent()
{
    handleNavigation(ADSDYNSYM_GET_PARENT, "Failed to get parent");
}

void CSample16Dlg::onButtonSibling()
{
    handleNavigation(ADSDYNSYM_GET_SIBLING, "Failed to get sibling");
}

void CSample16Dlg::onButtonChild()
{
    handleNavigation(ADSDYNSYM_GET_CHILD, "Failed to get child");
}

void CSample16Dlg::onButtonNext()
{
    handleNavigation(ADSDYNSYM_GET_NEXT, "Failed to get next");
}

void CSample16Dlg::handleNavigation(long navType, const QString& errorMessage)
{
    long nErr = adsGetNextDynSymbol(navType, m_strName, m_strFullName, m_strType, m_strComment, m_nAdsType,
                                    m_nSize, m_nIndexGroup, m_nIndexOffset);

    if (nErr == ADSERR_NOERR)
    {
        m_ui->editName->setText(m_strName);
        m_ui->editFullName->setText(m_strFullName);
        m_ui->editComment->setText(m_strComment);
        m_ui->editType->setText(m_strType);
        m_ui->editADSType->setText(QString::number(m_nAdsType));
        m_ui->editSize->setText(QString::number(m_nSize));
        m_ui->editIndexGroup->setText(QString::number(m_nIndexGroup));
        m_ui->editIndexOffset->setText(QString::number(m_nIndexOffset));
    }
    else
    {
        QMessageBox::critical(this, "Error", QString("%1 (error = 0x%2).").arg(errorMessage).arg(nErr, 0, 16));
    }
}

///////////////////////////////////////////////////////////////////////////////
// CSample16Dlg message handlers

bool CSample16Dlg::initDialog()
{
  if( m_adsDevice.GetLocalPort() == 0 )
  {
    QMessageBox::critical(this, "Error", "Unable to open ads port");
    return false;
  }
  return true;  // return true  unless you set the focus to a control
}

long CSample16Dlg::adsSetFirstDynSymbol(bool bForceReload)
{
  long nResult=ADSERR_NOERR;

  if ( bForceReload )
  {
    if ( m_pDynSymbols != NULL )
      delete m_pDynSymbols;
    m_pDynSymbols    = NULL;
  }
  m_nCurDynSymbol  = -1;
  m_nNextNavType    = ADSDYNSYM_GET_SIBLING;
  while ( m_pCurSubSymbol )
  {
    PADSDYNSYM_SUBINFO tmp = m_pCurSubSymbol->pParent;
    delete m_pCurSubSymbol;
    m_pCurSubSymbol = tmp;
  }

  if ( m_pDynSymbols == NULL )
  {
    m_pDynSymbols = CAdsParseSymbols::fromCache();
    if (m_pDynSymbols)
    {
      qDebug() << "Loaded symbols from cache";
      return ADSERR_NOERR;
    }

    AdsSymbolUploadInfo2 info;
     nResult = m_adsDevice.ReadReqEx2(ADSIGRP_SYM_UPLOADINFO2, 0, sizeof(info), &info, 0);
    if ( nResult == ADSERR_NOERR )
    {
      char * pSym = new char[info.nSymSize];
      if ( pSym )
      {
        nResult = m_adsDevice.ReadReqEx2(ADSIGRP_SYM_UPLOAD, 0, info.nSymSize, pSym, 0);
        if ( nResult == ADSERR_NOERR )
        {
          qDebug() << "Symbol count: " << info.nSymbols;
          qDebug() << "Datatype count: " << info.nDatatypes;
          qDebug() << "Datatype size: " << info.nDatatypeSize;

          char * pDT = new char[info.nDatatypeSize];
          if ( pDT )
          {
            nResult = m_adsDevice.ReadReqEx2(ADSIGRP_SYM_DT_UPLOAD, 0, info.nDatatypeSize, pDT, 0);
            if ( nResult == ADSERR_NOERR )
            {
              m_pDynSymbols = new CAdsParseSymbols(pSym, info.nSymSize, pDT, info.nDatatypeSize);
              if ( m_pDynSymbols == NULL )
                nResult = ADSERR_DEVICE_NOMEMORY;
              else
                m_pDynSymbols->writeCache();
            }
            else
            {
              qDebug() << "Failed to read datatypes (error = 0x" << nResult << ")";
            }
            delete[] pDT;
          }
        }
        delete[] pSym;
      }
    }
  }

  return nResult;
}

///////////////////////////////////////////////////////////////////////////////
long CSample16Dlg::adsGetNextDynSymbol(long navType, QString &strName, QString &strFullName,
                            QString &strType, QString &strComment, long &adsType,
                            long &cbSymbolSize, long &nIndexGroup, long &nIndexOffset)
{
  long nResult      = ADSERR_NOERR;
  long internalNayType = navType;

  if ( navType == ADSDYNSYM_GET_NEXT )
    internalNayType = m_nNextNavType;

  if ( m_pDynSymbols == NULL )
    nResult = adsSetFirstDynSymbol(true);

  if ( m_pDynSymbols && nResult == ADSERR_NOERR )
  {
    nResult = ADSERR_DEVICE_SYMBOLNOTFOUND;
    CAdsSymbolInfo info;
    switch ( internalNayType )
    {
    case ADSDYNSYM_GET_SIBLING:
      if ( m_pCurSubSymbol )
      {
        if ( m_pDynSymbols->SubSymbolInfo(m_pCurSubSymbol->infoParent, ++m_pCurSubSymbol->nSub, info) )
          nResult = ADSERR_NOERR;
      }
      else
      {
        if ( m_pDynSymbols->Symbol(++m_nCurDynSymbol, info) )
          nResult = ADSERR_NOERR;
      }
      break;
    case ADSDYNSYM_GET_CHILD:
      if ( m_pCurSubSymbol )
      {
        CAdsSymbolInfo main;
        if ( m_pDynSymbols->SubSymbolInfo(m_pCurSubSymbol->infoParent, m_pCurSubSymbol->nSub, main) )
        {
          if ( m_pDynSymbols->SubSymbolInfo(main, 0, info) )
          {
            ADSDYNSYM_SUBINFO* pTmp = m_pCurSubSymbol;
            m_pCurSubSymbol = new ADSDYNSYM_SUBINFO;
            if ( m_pCurSubSymbol )
            {
              m_pCurSubSymbol->infoParent  = main;
              m_pCurSubSymbol->nSub      = 0;
              m_pCurSubSymbol->pParent    = pTmp;
              nResult = ADSERR_NOERR;
            }
            else
            {
              m_pCurSubSymbol  = pTmp;
              nResult        = ADSERR_DEVICE_NOMEMORY;
            }
          }
        }
      }
      else
      {
        CAdsSymbolInfo main;
        if ( m_pDynSymbols->Symbol(m_nCurDynSymbol, main) )
        {
          if ( m_pDynSymbols->SubSymbolInfo(main, 0, info) )
          {
            m_pCurSubSymbol = new ADSDYNSYM_SUBINFO;
            if ( m_pCurSubSymbol )
            {
              m_pCurSubSymbol->infoParent  = main;
              m_pCurSubSymbol->nSub      = 0;
              m_pCurSubSymbol->pParent    = NULL;
              nResult = ADSERR_NOERR;
            }
            else
              nResult = ADSERR_DEVICE_NOMEMORY;
          }
        }
      }
      break;
    case ADSDYNSYM_GET_PARENT:
      if ( m_pCurSubSymbol )
      {
        ADSDYNSYM_SUBINFO* pTmp = m_pCurSubSymbol;
        m_pCurSubSymbol = m_pCurSubSymbol->pParent;
        delete pTmp;
        if ( m_pCurSubSymbol )
        {
          if ( m_pDynSymbols->SubSymbolInfo(m_pCurSubSymbol->infoParent, ++m_pCurSubSymbol->nSub, info) )
            nResult = ADSERR_NOERR;
        }
        else
        {
          if ( m_pDynSymbols->Symbol(++m_nCurDynSymbol, info) )
            nResult = ADSERR_NOERR;
        }
      }
      break;
    }
    if ( nResult == ADSERR_NOERR )
    {
      strName = info.name;
      strFullName = info.fullname;
      strType = info.type;
      strComment = info.comment;
      adsType      = info.dataType;
      cbSymbolSize  = info.size;
      nIndexGroup  = info.iGrp;
      nIndexOffset  = info.iOffs;
    }
  }

  if ( navType == ADSDYNSYM_GET_NEXT )
  {
    if ( nResult != ADSERR_NOERR )
    {
      switch (internalNayType)
      {
      case ADSDYNSYM_GET_CHILD:
        m_nNextNavType = ADSDYNSYM_GET_SIBLING;
        break;
      case ADSDYNSYM_GET_SIBLING:
      case ADSDYNSYM_GET_PARENT:
        m_nNextNavType = ADSDYNSYM_GET_PARENT;
        break;
      }
      if ( m_nNextNavType != ADSDYNSYM_GET_PARENT || m_pCurSubSymbol != NULL )
        nResult = adsGetNextDynSymbol(navType, strName, strFullName, strType, strComment, adsType,
          cbSymbolSize, nIndexGroup, nIndexOffset);
    }
    else
    {
      switch (internalNayType)
      {
      case ADSDYNSYM_GET_SIBLING:
      case ADSDYNSYM_GET_PARENT:
        m_nNextNavType = ADSDYNSYM_GET_CHILD;
        break;
      }
    }
  }

  return nResult;
}

