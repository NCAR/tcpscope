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
#include <radar/IwrfTsBurst.hh>
#include <radar/IwrfTsReader.hh>

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
  /// @param fmqPath - set in FMQ mode
    AScopeReader(const std::string &host, int port,
                 const std::string &fmqPath,
                 bool simulMode,
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
  std::string _serverFmq;
  bool _simulMode;

  AScope &_scope;
  
  // read in data

  IwrfTsReader *_pulseReader;
  bool _haveChan1;
  int _dataTimerId;

  // pulse stats

  int _nSamples;
  int _pulseCount;
  
  // info and pulses

  vector<IwrfTsPulse *> _pulses; // SIM mode, or when H/V flag is 1
  vector<IwrfTsPulse *> _pulsesV; // when H/V flag is 0

  // xmit mode

  typedef enum {
    CHANNEL_MODE_HV_SIM,
    CHANNEL_MODE_V_ONLY,
    CHANNEL_MODE_ALTERNATING
  } channelMode_t;
  channelMode_t _channelMode;

  // sequence number for time series to ascope

  size_t _tsSeqNum;

  // methods
  
  int _readData();
  IwrfTsPulse *_getNextPulse();
  void _sendDataToAScope();
  int _loadTs(int nGates,
              int channelIn,
              const vector<IwrfTsPulse *> &pulses,
              int channelOut,
              AScope::FloatTimeSeries &ts);
  int _loadBurst(const IwrfTsBurst &burst,
                 int channelOut,
                 AScope::FloatTimeSeries &ts);

};


#endif /*ASCOPEREADER_H_*/
