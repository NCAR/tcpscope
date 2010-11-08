#ifndef ASCOPEREADER_H_
#define ASCOPEREADER_H_

#include "AScope.h"
#include <string>
#include <QObject>
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <radar/iwrf_data.h>

/// A Time series reader for the AScope. It reads IWRF data and translates
/// DDS samples to AScope::TimeSeries.

class AScopeReader : public QObject
{

  Q_OBJECT

public:
    
  /// Constructor
  /// @param host The server host
  /// @param port The server port
    AScopeReader(const std::string &host, int port,
                 AScope &scope, int debugLevel);

  /// Destructor
  virtual ~AScopeReader();
  
  signals:

  /// This signal provides an item that falls within
  /// the desired bandwidth specification.
  /// @param pItem A pointer to the item.
  /// It must be returned via returnItem().
    
  void newItem(AScope::TimeSeries pItem);

public slots:

  /// Use this slot to return an item
  /// @param pItem the item to be returned.

  void returnItemSlot(AScope::TimeSeries pItem);
  
  /// Run continuously, reading data from the server

  void readFromServer();

  // respond to timer events
  
  void timerEvent(QTimerEvent *event);
    
protected:

private:

  std::string _serverHost;
  int _serverPort;
  AScope &_scope;
  Socket _sock;
  time_t _lastTryConnectTime;
  int _sockTimerId;
  int _debugLevel;
  int _pulseCount;
  bool _timedOut;

  // radar info etc from time series
  
  iwrf_radar_info _tsRadarInfo;
  iwrf_scan_segment _tsScanSegment;
  iwrf_ts_processing _tsTsProcessing;
  
  int _readFromServer();
  int _readTcpPacket(int &id, int &len, MemBuf &buf);
  int _peekAtBuffer(void *buf, int nbytes);

#ifdef NOTNOW
  int _reSync();
#endif

};


#endif /*ASCOPEREADER_H_*/
