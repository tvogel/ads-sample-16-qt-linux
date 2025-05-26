#include "AdsParseSymbols.h"
#include <QString>
#include <cassert>
#include <QMessageBox>
#include <QLocale>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextCodec>
#include <QFile>
#include <cstdint>

CAdsParseSymbols::CAdsParseSymbols(void *  pSymbols, unsigned int nSymSize, void *  pDatatypes, unsigned int nDTSize)
{
  memset(m_bufGetTypeByNameBuffer, 0, sizeof(m_bufGetTypeByNameBuffer));
  m_pSymbols = new char[nSymSize+sizeof(uint32_t)];
  QJsonArray typeList;

  if ( m_pSymbols )
  {
    m_nSymSize = nSymSize;
    if ( nSymSize )
      memcpy(m_pSymbols, pSymbols, nSymSize);
    *(uint32_t *)&m_pSymbols[nSymSize] = 0;
    m_nSymbols = 0;
    unsigned int offs = 0;
    while (*(uint32_t *)&m_pSymbols[offs])
    {
      m_nSymbols++;
      offs += *(uint32_t *)&m_pSymbols[offs];
    }
    assert(offs==nSymSize);
    m_ppSymbolArray = new AdsSymbolEntry *[m_nSymbols];
    m_nSymbols = offs = 0;
    while (*(uint32_t *)&m_pSymbols[offs])
    {
      m_ppSymbolArray[m_nSymbols++] = reinterpret_cast<AdsSymbolEntry *>(&m_pSymbols[offs]);
      offs += *(uint32_t *)&m_pSymbols[offs];
    }
    assert(offs==nSymSize);
  }
  m_pDatatypes = new char[nDTSize+sizeof(uint32_t)];
  if ( m_pDatatypes )
  {
    m_nDTSize = nDTSize;
    if ( nDTSize )
      memcpy(m_pDatatypes, pDatatypes, nDTSize);
    *(uint32_t *)&m_pDatatypes[nDTSize] = 0;
    m_nDatatypes = 0;
    unsigned int offs = 0;
    while (*(uint32_t *)&m_pDatatypes[offs])
    {
      m_nDatatypes++;
      offs += *(uint32_t *)&m_pDatatypes[offs];
    }
    assert(offs==nDTSize);
    m_ppDatatypeArray = new AdsDatatypeEntry *[m_nDatatypes];
    m_nDatatypes = offs = 0;
    while (*(uint32_t *)&m_pDatatypes[offs])
    {
      auto adsDatatypeEntry = (AdsDatatypeEntry *)&m_pDatatypes[offs];
      typeList << adsDatatypeEntry->toJson();
      m_ppDatatypeArray[m_nDatatypes++] = adsDatatypeEntry;
      offs += *(uint32_t *)&m_pDatatypes[offs];
    }
    assert(offs==nDTSize);
  }
  QFile typeListFile("datatypes.json");
  typeListFile.open(QFile::WriteOnly);
  typeListFile.write(QJsonDocument(typeList).toJson());
}

CAdsParseSymbols::~CAdsParseSymbols()
{
  delete m_pSymbols;
  delete m_pDatatypes;
  delete m_ppSymbolArray;
  delete m_ppDatatypeArray;
}

CAdsParseSymbols * CAdsParseSymbols::fromCache()
{
  QFile symbolsFile("symbols.cache");
  if ( !symbolsFile.open(QIODevice::ReadOnly) )
  {
    qDebug() << "Failed to open symbols.cache for reading";
    return 0;
  }
  QDataStream in(&symbolsFile);
  QByteArray symbolsData, datatypesData;
  in >> symbolsData >> datatypesData;
  if ( in.status() != QDataStream::Ok )
  {
    qDebug() << "Failed to read symbols.cache" << in.status();
    return 0;
  }
  return new CAdsParseSymbols(symbolsData.data(), symbolsData.size(), datatypesData.data(), datatypesData.size());
}

void CAdsParseSymbols::writeCache() const
{
  QFile symbolsFile("symbols.cache");
  if ( !symbolsFile.open(QIODevice::WriteOnly) )
  {
    qDebug() << "Failed to open symbols.cache for writing";
    return;
  }
  QDataStream out(&symbolsFile);
  out << QByteArray::fromRawData(m_pSymbols, m_nSymSize)
      << QByteArray::fromRawData(m_pDatatypes, m_nDTSize);
  if ( out.status() != QDataStream::Ok )
  {
    qDebug() << "Failed to write symbols.cache" << out.status();
    return;
  }
}

static int CompareDTByName( const void* p1, const void* p2 )
{
  return strcmp( (char*)((*(AdsDatatypeEntry **)p1)+1), (char*)((*(AdsDatatypeEntry **)p2)+1) );
}

AdsDatatypeEntry * CAdsParseSymbols::GetTypeByName(QString sType)
{
  qDebug() << "GetTypeByName: " << sType;
  AdsDatatypeEntry * pKey = (AdsDatatypeEntry *)m_bufGetTypeByNameBuffer;
  strcpy((char *)(pKey+1), sType.toUtf8().constData());

  // serach data type by name
  AdsDatatypeEntry **  ppEntry = (AdsDatatypeEntry **)bsearch(&pKey, m_ppDatatypeArray, m_nDatatypes,
    sizeof(*m_ppDatatypeArray), CompareDTByName);

  if ( ppEntry )
    return *ppEntry;
  else
    return NULL;
}

unsigned int  CAdsParseSymbols::SubSymbolCount(unsigned int sym)
{
  if ( sym < m_nSymbols )
    return SubSymbolCount(SymbolType(sym));
  else
    return 0;
}

unsigned int  CAdsParseSymbols::SubSymbolCount(const char * sType)
{
  AdsDatatypeEntry * pEntry = GetTypeByName(sType);
  if ( pEntry )
    return SubSymbolCount(pEntry);
  else
    return 0;
}

unsigned int  CAdsParseSymbols::SubSymbolCount(AdsDatatypeEntry *  pEntry)
{
  unsigned int cnt=0;
  if ( pEntry )
  {
    if ( pEntry->subItems )
    {
      cnt += pEntry->subItems;
    }
    else if ( pEntry->arrayDim )
    {
      cnt = 1;
      const AdsDatatypeArrayInfo * pAI = pEntry->ArrayInfo();
      for ( unsigned short i=0; i < pEntry->arrayDim; i++ )
        cnt *= pAI[i].elements;
    }
  }

  return cnt;
}

bool  CAdsParseSymbols::Symbol(unsigned int sym, CAdsSymbolInfo& info)
{
  auto codec = QTextCodec::codecForName("Windows-1252");
  auto pEntry = Symbol(sym);
  if ( pEntry == NULL )
    return false;
  info.iGrp    = pEntry->iGroup;
  info.iOffs    = pEntry->iOffs;
  info.size    = pEntry->size;
  info.dataType  = pEntry->dataType;
  info.flags    = pEntry->flags;
  info.name    = codec->toUnicode(SymbolName(pEntry));
  info.fullname  = codec->toUnicode(SymbolName(pEntry));
  info.type    = codec->toUnicode(SymbolType(pEntry));
  info.comment  = codec->toUnicode(SymbolComment(pEntry));
  return true;
}

bool  CAdsParseSymbols::SubSymbolInfo(CAdsSymbolInfo main, unsigned int sub, CAdsSymbolInfo & info)
{
  if ( !main.m_pEntry )
    main.m_pEntry = GetTypeByName(main.type);
  AdsDatatypeEntry * pEntry = main.m_pEntry;
  if ( pEntry )
  {
    auto codec = QTextCodec::codecForName("Windows-1252");
    if ( pEntry->subItems )
    {
      AdsDatatypeEntry *  pSEntry = AdsDatatypeStructItem(pEntry, sub);
      if ( pSEntry )
      {
        info.iGrp    = main.iGrp;
        info.iOffs    = main.iOffs + pSEntry->offs;
        info.size    = pSEntry->size;
        info.dataType  = pSEntry->dataType;
        info.flags    = pSEntry->flags;
        info.name    = codec->toUnicode(pSEntry->name());
        info.fullname = QString("%1.%2").arg(main.fullname, info.name);
        info.type    = codec->toUnicode(pSEntry->type());
        info.comment  = codec->toUnicode(pSEntry->comment());
        return true;
      }
    }
    else if ( pEntry->arrayDim )
    {
      unsigned int x[10]={0}, baseSize=pEntry->size;
      x[pEntry->arrayDim] = 1;
      const AdsDatatypeArrayInfo * pAI = pEntry->ArrayInfo();
      for ( int i=pEntry->arrayDim-1; i >= 0 ; i-- )
      {
        x[i] = x[i+1]*pAI[i].elements;
        if ( pAI[i].elements )
          baseSize /= pAI[i].elements;
      }
      if ( sub == 0 && x[0] > 1000 )
      {
        QString msg;
        if ( QLocale::system().language() == QLocale::German )
          msg = QString("Das Array '%1' beinhaltet %2 Elemente. Es kann sehr lange dauern diese anzuzeigen!\nSollen die Elemente �bersprungen werden")
          .arg(
            main.fullname, x[0]);
        else
          msg = QString("The array '%1' contains %2 elements. It may take a long time to show each element!\nSkip these elements")
          .arg(
            main.fullname, x[0]);
        if ( QMessageBox::question(0, "Question", msg) == QMessageBox::Yes )
          return false;
      }
      if ( sub < x[0] )
      {
        info.iGrp    = main.iGrp;
        info.iOffs    = main.iOffs;
        info.size    = baseSize;
        info.dataType  = pEntry->dataType;
        info.flags    = pEntry->flags;
        info.type    = codec->toUnicode(pEntry->type());
        info.comment  = codec->toUnicode(pEntry->comment());
        QString arr("["), tmp;
        for ( int i = 0; i < pEntry->arrayDim; i++ )
        {
          tmp = QString("%1").arg(pAI[i].lBound + sub / x[i+1]);
          arr  += tmp;
          arr  += (i==pEntry->arrayDim-1) ? ']' : ',';
          info.iOffs  += baseSize * x[i+1] * (sub/x[i+1]);
          sub %= x[i+1];
        }
        info.name    = main.name + arr;
        info.fullname  = main.fullname + arr;
        return true;
      }
    }
  }
  return false;
}
