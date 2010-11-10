#include "AScopeReader.h"
#include <cerrno>
#include <radar/iwrf_functions.hh>
using namespace std;

// Note that the timer interval for QtTSReader is 0

AScopeReader::AScopeReader(const string &host,
                           int port,
                           AScope &scope,
                           int debugLevel):
        _debugLevel(debugLevel),
        _serverHost(host),
        _serverPort(port),
        _scope(scope),
        _lastTryConnectTime(0),
        _pulseCount(0),
        _pulseCountSinceSync(0),
        _tsSeqNum(0)
{
  
  // this are required in order to send structured data types
  // via a qt signal
  qRegisterMetaType<AScope::TimeSeries>();
  
  // start timer for checking socket every 50 msecs
  _sockTimerId = startTimer(50);

}

AScopeReader::~AScopeReader()

{

  // close socket if open

  if (_sock.isOpen()) {
    if (_debugLevel) {
      cerr << "Closing socket to IWRF data server" << endl;
    }
    _sock.close();
  }

}

//////////////////////////////////////////////////////////////
// respond to timer events
  
void AScopeReader::timerEvent(QTimerEvent *event)
{

  if (event->timerId() == _sockTimerId) {

    if (_debugLevel > 0) {
      cerr << "Servicing socket timer event" << endl;
    }

    if (!_sock.isOpen()) {
      // try opening, once per second
      time_t now = time(NULL);
      if (now == _lastTryConnectTime) {
        return;
      }
      _lastTryConnectTime = now;
      if (_sock.open(_serverHost.c_str(), _serverPort)) {
        int errNum = errno;
        cerr << "ERROR AScopeReader::timerEvent" << endl;
        cerr << "  Cannot open socket to IWRF data server" << endl;
        cerr << "  host: " << _serverHost << endl;
        cerr << "  port: " << _serverPort << endl;
        cerr << "  " << strerror(errNum) << endl;
        return;
      }
      if (_debugLevel) {
        cerr << "INFO - AScopeReader::timerEvent" << endl;
        cerr << "Opened socket to IWRF data server" << endl;
        cerr << "  host: " << _serverHost << endl;
        cerr << "  port: " << _serverPort << endl;
      }
    }

    // read data from server, until enough data is gathered

    if (_readFromServer() == 0) {
      _sendDataToAScope();
    }
    
  } // if (event->timerId() == _sockTimerId)
    
}

/////////////////////////////
// read data from the server
// returns 0 on succes, -1 on failure (not enough data)

int AScopeReader::_readFromServer()

{

  // read data until nSamples pulses have been gathered
  
  size_t nSamples = _scope.getBlockSize();
  // if (nSamples < 16) nSamples = 16;

  MemBuf buf;
  while (_pulses.size() < nSamples) {
    
    // read packet from time series server server
    
    int packetId, packetLen;
    if (_readPacket(packetId, packetLen, buf)) {
      cerr << "ERROR - AScopeReader::_readFromServer" << endl;
      return -1;
    }
    if (_timedOut) {
      return -1;
    }
    
    // handle packet types

    if (packetId == IWRF_PULSE_HEADER_ID) {
    
      // add pulse to vector

      _addPulse(buf);

    } else {

      // set the ops info appropriately

      _info.setFromBuffer(buf.getPtr(), buf.getLen());

    }

  } // while 

  return 0;

}

///////////////////////////////////////////////////////////////////
// Read in next packet, set id and load buffer.
// Returns 0 on success, -1 on failure

int AScopeReader::_readPacket(int &id, int &len, MemBuf &buf)

{
  bool haveGoodHeader = false;
  si32 packetId;
  si32 packetLen;
  si32 packetTop[2];
  _timedOut = false;

  while (!haveGoodHeader) {

    // peek at the first 8 bytes

    if (_peekAtBuffer(packetTop, sizeof(packetTop))) {
      cerr << "ERROR - AScopeReader::_readPacket" << endl;
      return -1;
    }

    if (_timedOut) {
      return 0;
    }
    
    // check ID for packet, and get its length
    
    packetId = packetTop[0];
    packetLen = packetTop[1];

    if (iwrf_check_packet_id(packetId, packetLen)) {

      // out of order, so close and return error
      // this will force a reconnection and resync

      cerr << "ERROR - AScopeReader::_readPacket" << endl;
      cerr << " Incoming data stream out of sync" << endl;
      cerr << " Closing socket" << endl;
      _sock.close();
      return -1;

    } else {

      haveGoodHeader = true;
      id = packetId;
      len = packetLen;

    }

  } // while (!haveGoodHeader)
    
  // read packet in

  buf.reserve(packetLen);
  if (_sock.readBuffer(buf.getPtr(), packetLen)) {
    if (_sock.getErrNum() == Socket::TIMED_OUT) {
      _timedOut = true;
      return 0;
    } else {
      cerr << "ERROR - AScopeReader::_readPacket" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }
  }
  
  if (id == IWRF_PULSE_HEADER_ID) {
    _pulseCount++;
    _pulseCountSinceSync++;
  }
    
  if (_debugLevel > 2) {

    iwrf_packet_print(stderr, buf.getPtr(), buf.getLen());

  } else if (_debugLevel > 1) {

    if (id == IWRF_PULSE_HEADER_ID) {
      cerr << "Read in PULSE packet, id, len, count: " << ", "
           << iwrf_packet_id_to_str(id) << ", "
           << packetLen << ", "
           << _pulseCount << endl;
    } else {
      cerr << "Read in TCP packet, id, len: "
           << iwrf_packet_id_to_str(id) << ", "
           << packetLen << endl;
      if(id == IWRF_SYNC_ID) {
        if(_pulseCount > 0) {
          cerr << "N pulses since last sync: " << _pulseCountSinceSync << endl;
          _pulseCountSinceSync = 0;  
        }
      } else {
        iwrf_packet_print(stderr, buf.getPtr(), buf.getLen());
      }
      
    } // if (id == IWRF_PULSE_HEADER_ID)

  } // if (_debugLevel > 2) 
    
  return 0;

}

///////////////////////////////////////////////////////////////////
// Peek at buffer from socket
// Returns 0 on success, -1 on failure
// _timedOut set in case of timeout

int AScopeReader::_peekAtBuffer(void *buf, int nbytes)

{

  _timedOut = false;

  // peek with no wait
  if (_sock.peek(buf, nbytes, 0) == 0) {
    return 0;
  } else {
    if (_sock.getErrNum() == Socket::TIMED_OUT) {
      // no data available
      _timedOut = true;
      return 0;
    } else {
      cerr << "ERROR - AScopeReader::_peekAtBuffer" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
    }
  }

  return -1;

}

///////////////////////////////////////////////////////
// add pulse to queue

void AScopeReader::_addPulse(const MemBuf &buf)

{
  
  // create a new pulse
  
  IwrfTsPulse *pulse = new IwrfTsPulse(_info);
  
  // set the data on the pulse, as floats

  pulse->setFromBuffer(buf.getPtr(), buf.getLen(), true);

  // add to the pulse vector
  
  _pulses.push_back(pulse);

}
      
///////////////////////////////////////////////////////
// send data to the AScope

void AScopeReader::_sendDataToAScope()

{
  
  // compute max gates and channels
  
  int nGates = 0;
  int nChannels = 0;
  
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    const IwrfTsPulse *pulse = _pulses[ii];
    int nGatesPulse = pulse->getNGates();
    if (nGatesPulse > nGates) {
      nGates = nGatesPulse;
    }
    int nChannelsPulse = pulse->getNChannels();
    if (nChannelsPulse > nChannels) {
      nChannels = nChannelsPulse;
    }
  } // ii
  
  // create channel 0 time series objects for AScope
  
  AScope::FloatTimeSeries tsChan0;
  tsChan0.gates = nGates;
  tsChan0.chanId = 0;
  tsChan0.sampleRateHz = 1.0 / _pulses[0]->get_prt();
  
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    
    const IwrfTsPulse *pulse = _pulses[ii];
    int nGatesPulse = _pulses[ii]->getNGates();
    
    fl32 *iq = new fl32[nGates * 2];
    memset(iq, 0, nGates * 2 * sizeof(fl32));
    memcpy(iq, pulse->getIq0(), nGatesPulse * 2 * sizeof(fl32));
    
    tsChan0.IQbeams.push_back(iq);
    
  } // ii
  
  // set sequence number

  size_t *seq0 = new size_t;
  *seq0 = _tsSeqNum;
  tsChan0.handle = seq0;
  if (_debugLevel > 1) {
    cerr << "Creating ts data for chan 0, seq num: " << _tsSeqNum << endl;
  }
  _tsSeqNum++;

  // send the time series to the display
  
  emit newItem(tsChan0);
    
  if (nChannels > 1) {
    
    // create channel 1 time series objects for AScope
    
    AScope::FloatTimeSeries tsChan1;
    tsChan1.gates = nGates;
    tsChan1.chanId = 1;
    tsChan1.sampleRateHz = 1.0 / _pulses[0]->get_prt();
    tsChan1.handle = (void*) this;
    
    for (size_t ii = 0; ii < _pulses.size(); ii++) {
      
      const IwrfTsPulse *pulse = _pulses[ii];
      int nGatesPulse = _pulses[ii]->getNGates();
      
      fl32 *iq = new fl32[nGates * 2];
      memset(iq, 0, nGates * 2 * sizeof(fl32));
      
      if (pulse->getIq1() != NULL) {
        memcpy(iq, pulse->getIq1(), nGatesPulse * 2 * sizeof(fl32));
      }
      
      tsChan1.IQbeams.push_back(iq);
      
    } // ii
    
    // set sequence number
    
    size_t *seq1 = new size_t;
    *seq1 = _tsSeqNum;
    tsChan1.handle = seq1;
    if (_debugLevel > 1) {
      cerr << "Creating ts data for chan 1, seq num: " << _tsSeqNum << endl;
    }
    _tsSeqNum++;

    // send the time series to the display
    
    emit newItem(tsChan1);
    
  } // if (nChannels > 1)
  
  // free up the pulses
  
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    delete _pulses[ii];
  }
  _pulses.clear();

}

      
//////////////////////////////////////////////////////////////////////////////
// Clean up when iq data is returned from display

void AScopeReader::returnItemSlot(AScope::TimeSeries ts)

{

  size_t *seqNum = (size_t *) ts.handle;
  if (_debugLevel > 1) {
    cerr << "--->> Freeing ts data, seq num: " << *seqNum << endl;
  }
  delete seqNum;
  
  for (size_t ii = 0; ii < ts.IQbeams.size(); ii++) {
    delete[] (fl32 *) ts.IQbeams[ii];
  }
  
}

