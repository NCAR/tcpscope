#ifndef ASCOPEREADER_H_
#define ASCOPEREADER_H_

#include "AScope.h"
#include <string>
#include <QObject>
#include <toolsa/Socket.hh>

/// A Time series reader for the AScope. It reads IWRF data and translates
/// DDS samples to AScope::TimeSeries.

class AScopeReader : public QObject
{

  Q_OBJECT

public:
    
  /// Constructor
  /// @param host The server host
  /// @param port The server port
    AScopeReader(const std::string &host, int port, AScope &scope);

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
  int _sockTimerId;

};


#endif /*ASCOPEREADER_H_*/
