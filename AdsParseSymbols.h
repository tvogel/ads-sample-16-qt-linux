#pragma once

#include "AdsLib.h"
#include "AdsDatatypeEntry.h"
#include <QString>

class CAdsSymbolInfo
{
public:
  CAdsSymbolInfo() : m_pEntry(NULL) {};
  quint32 iGrp;     //
  quint32 iOffs;    //
  quint32 size;     // size of datatype ( in bytes )
  quint32 offs;     // offs of dataitem in parent datatype ( in bytes )
  quint32 dataType; // adsDataType of symbol (if alias)
  quint32 flags;    //
  QString name;
  QString fullname;
  QString type;
  QString comment;
  AdsDatatypeEntry *  m_pEntry;
};

class CAdsParseSymbols
{
public:
  CAdsParseSymbols(void *  pSymbols, unsigned int nSymSize, void *  pDatatypes=NULL, unsigned int nDTSize=0);
  virtual ~CAdsParseSymbols();

  static CAdsParseSymbols * fromCache();

  void writeCache() const;

  virtual  unsigned int  SymbolCount()
    { return m_nSymbols; }
  virtual  unsigned int  DatatypeCount()
    { return m_nDatatypes; }
  virtual  AdsSymbolEntry *  Symbol(unsigned int sym)
    { return (sym < m_nSymbols) ? m_ppSymbolArray[sym] : NULL; }
  virtual  bool  Symbol(unsigned int sym, CAdsSymbolInfo& info);
  virtual  const char *  SymbolName(unsigned int sym)
    { return (sym < m_nSymbols) ? SymbolName(m_ppSymbolArray[sym]) : NULL; }
  virtual  const char *  SymbolType(unsigned int sym)
    { return (sym < m_nSymbols) ? SymbolType(m_ppSymbolArray[sym]) : NULL; }
  virtual  const char *  SymbolComment(unsigned int sym)
    { return (sym < m_nSymbols) ? SymbolComment(m_ppSymbolArray[sym]) : NULL; }
  virtual  unsigned int  SubSymbolCount(unsigned int sym);
  virtual  unsigned int  SubSymbolCount(const char * sType);
  virtual  unsigned int  SubSymbolCount(AdsDatatypeEntry *  pEntry);
  virtual  bool  SubSymbolInfo(CAdsSymbolInfo main, unsigned int sub, CAdsSymbolInfo& info);
protected:
  virtual  AdsDatatypeEntry *  GetTypeByName(QString sType);

  char *              m_pSymbols;
  char *              m_pDatatypes;
  unsigned int        m_nSymbols;
  unsigned int        m_nDatatypes;
  unsigned int        m_nSymSize;
  unsigned int        m_nDTSize;
  AdsSymbolEntry **   m_ppSymbolArray;
  AdsDatatypeEntry ** m_ppDatatypeArray;
  char                m_bufGetTypeByNameBuffer[300];

private:
  static const char *  SymbolName(const AdsSymbolEntry * p)  {
    return reinterpret_cast<const char *>(p+1);
  }
  static const char *  SymbolType(const AdsSymbolEntry * p)  {
    return SymbolName(p) + p->nameLength + 1;
  }
  static const char *  SymbolComment(const AdsSymbolEntry * p)  {
    return SymbolType(p) + p->typeLength + 1;
  }
  static const AdsSymbolEntry * NextSymbolEntry(const AdsSymbolEntry * pEntry) {
    return *reinterpret_cast<const quint32 *>(reinterpret_cast<const char *>(pEntry) + pEntry->entryLength)
      ? reinterpret_cast<const AdsSymbolEntry *>(reinterpret_cast<const char *>(pEntry) + pEntry->entryLength)
      : nullptr;
  }
};
