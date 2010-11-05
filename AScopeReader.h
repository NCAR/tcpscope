#ifndef ASCOPEREADER_H_
#define ASCOPEREADER_H_

#include "AScope.h"

/// A Time series reader for the AScope. It reads IWRF data and translates
/// DDS samples to AScope::TimeSeries.

class AScopeReader:
{
  Q_OBJECT
public:
    
  /// Constructor
  /// @param subscriber The DDS subscriber
  /// @param topicName The DDS topic name
    AScopeReader(const string &host, int port);

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

protected:
  /// Re-implemented from super class. This function will be  called
  /// when new samples are available on the topic. If it is time to
  /// accept a new sample (_capture is true), then the newItem()
  /// signal will be emitted for one item. Any remaining available items
  /// will be returned.
  virtual void notify();

private:

  string _serverHost;
  int _serverPort;

};


#endif /*ASCOPEREADER_H_*/
