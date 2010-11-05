#include "AScopeReader.h"
#include <QMetaType>

Q_DECLARE_METATYPE(AScope::TimeSeries)
  
// Note that the timer interval for QtTSReader is 0; we
// are accepting all DDS samples.
  AScopeReader::AScopeReader(const std::string &host,
                             int port):
          _serverHost(host),
          _serverPort(port)
{
  // this are required in order to send structured data types
  // via a qt signal
  qRegisterMetaType<AScope::TimeSeries>();
}

AScopeReader::~AScopeReader() {
  
}

//////////////////////////////////////////////////////////////////////////////
void
  AScopeReader::returnItemSlot(AScope::TimeSeries pItem) {
  

}

////////////////////////////////////////////////////////////////////////
/// notify() which will be called whenever new samples
/// are available for plotting on AScope.
/// Send the sample to clients via a signal.
/// The clients must release the sample via returnItem()
/// when they are finished with it.
void AScopeReader::notify(){

  while (true) {

    int tsLength  = 10;
    
    // copy required metadata

    AScope::ShortTimeSeries pItem;
//     pItem.gates = ddsItem->tsList[0].hskp.gates;
//     pItem.chanId = ddsItem->chanId;
//     pItem.sampleRateHz = 1.0/ddsItem->tsList[0].hskp.prt1;
//     pItem.handle = (void*) ddsItem;
    
    // copy the pointers to the beam data
    pItem.IQbeams.resize(tsLength);
    for (int i = 0; i < tsLength; i++) {
      // pItem.IQbeams[i] = &ddsItem->tsList[i].data[0];
    }
    
    // send the sample to our clients
    emit newItem(pItem);
  }

}
