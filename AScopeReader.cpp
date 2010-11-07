#include "AScopeReader.h"
#include <QMetaType>

Q_DECLARE_METATYPE(AScope::TimeSeries)
  
// Note that the timer interval for QtTSReader is 0; we
// are accepting all DDS samples.
  AScopeReader::AScopeReader(const std::string &host,
                             int port,
                             AScope &scope):
          _serverHost(host),
          _serverPort(port),
          _scope(scope)
{
  // this are required in order to send structured data types
  // via a qt signal
  qRegisterMetaType<AScope::TimeSeries>();

  // start timer for checking socket every 50 msecs
  _sockTimerId = startTimer(50);

}

AScopeReader::~AScopeReader() {
  
}

//////////////////////////////////////////////////////////////////////////////
void
  AScopeReader::returnItemSlot(AScope::TimeSeries pItem) {
  

}

////////////////////////////////////////////////////////////////////////
/// Run continuously, reading data from the server

void AScopeReader::readFromServer() 

{

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

//////////////////////////////////////////////////////////////
// respond to timer events
  
void AScopeReader::timerEvent(QTimerEvent *event)
{

  if (event->timerId() == _sockTimerId) {

    std::cerr << "Servicing socket timer event" << std::endl;

    // get all available beams
    
    //     while (true) {
    
    // get the next ray from the reader queue
    // responsibility for this ray memory passes to
    // this (the master) thread
    
    //       RadxVol vol;
    //       const RadxRay *ray = _reader->getNextRay(vol);
    //       if (ray == NULL) {
    //         break; // no pending rays
    //       }
    
    // draw the beam
    
    //       _drawBeam(vol, ray);
    
    // delete the ray
    
    // delete ray;
    
    // } // while

  }
    
}

