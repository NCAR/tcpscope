#ifndef ASCOPEREADER_H_
#define ASCOPEREADER_H_

#include "AScope.h"
#include "QtKaTSReader.h"

/// A DDS reader for the AScope. It is derived from QtTSReader, and translates
/// DDS samples to AScope::TimeSeries. It will emit a newTSItemSlot(RadarDDS::TimeSeriesSequence*)
/// when new data arrives.
class AScopeReader: public QtKaTSReader 
{
		Q_OBJECT
public:

	/// Constructor
	/// @param subscriber The DDS subscriber
	/// @param topicName The DDS topic name
	AScopeReader(DDSSubscriber& subscriber,
			std::string topicName);

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

};


#endif /*ASCOPEREADER_H_*/
