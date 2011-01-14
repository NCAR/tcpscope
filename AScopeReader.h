#ifndef ASCOPEREADER_H_
#define ASCOPEREADER_H_

#include <QObject>
#include <QMetaType>

#include <string>
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <radar/iwrf_data.h>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>

#include "AScope.h"

/// A Time series reader for the AScope. It reads IWRF data and translates
/// DDS samples to AScope::TimeSeries.

Q_DECLARE_METATYPE(AScope::TimeSeries)
  
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
  
  // respond to timer events
  
  void timerEvent(QTimerEvent *event);
    
protected:

private:

  int _debugLevel;

  std::string _serverHost;
  int _serverPort;

  AScope &_scope;

  // communication via socket

  Socket _sock;
  time_t _lastTryConnectTime;
  int _sockTimerId;
  bool _timedOut;
  
  // pulse stats

  int _nSamples;
  int _pulseCount;
  int _pulseCountSinceSync;
  
  // info and pulses

  IwrfTsInfo _info;
  vector<IwrfTsPulse *> _pulsesH; // when H/V flag is 1
  vector<IwrfTsPulse *> _pulsesV; // when H/V flag is 0

  // xmit mode

  typedef enum {
    XMIT_MODE_H_ONLY,
    XMIT_MODE_V_ONLY,
    XMIT_MODE_ALTERNATING
  } xmitMode_t;
  xmitMode_t _xmitMode;

  // sequence number for time series to ascope

  size_t _tsSeqNum;

  // methods
  
  int _readFromServer();
  int _readPacket(int &id, int &len, MemBuf &buf);
  int _peekAtBuffer(void *buf, int nbytes);
  void _addPulse(const MemBuf &buf);
  void _sendDataToAScope();
  void _loadTs(int nGates,
               int channelIn,
               const vector<IwrfTsPulse *> &pulses,
               int channelOut,
               AScope::FloatTimeSeries &ts);


};


#endif /*ASCOPEREADER_H_*/
