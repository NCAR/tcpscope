#ifndef ASCOPEREADER_H_
#define ASCOPEREADER_H_

#include "AScope.h"
#include <string>
#include <QObject>

/// A Time series reader for the AScope. It reads IWRF data and translates
/// DDS samples to AScope::TimeSeries.

class AScopeReader : QObject
{

  Q_OBJECT

public:
    
  /// Constructor
  /// @param subscriber The DDS subscriber
  /// @param topicName The DDS topic name
    AScopeReader(const std::string &host, int port);

  /// Destructor
  virtual ~AScopeReader();
  
signals:

/// This signal provides an item that falls within
/// the desired bandwidth specification.
/// @param pItem A pointer to the item. It must be returned
/// via returnItem().

  void newItem(AScope::TimeSeries pItem);

public slots:

/// Use this slot to return an item
/// @param pItem the item to be returned.
  void returnItemSlot(AScope::TimeSeries pItem);
  
/// notify() which will be called whenever new samples
/// are available for plotting on AScope.
/// Send the sample to clients via a signal.
/// The clients must release the sample via returnItem()
/// when they are finished with it.
  void notify();

protected:

private:

  std::string _serverHost;
  int _serverPort;

};


#endif /*ASCOPEREADER_H_*/
