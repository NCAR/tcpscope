#include "AScopeReader.h"
#include <cerrno>
#include <radar/iwrf_functions.hh>
using namespace std;

// Note that the timer interval for QtTSReader is 0

AScopeReader::AScopeReader(const string &host,
                           int port,
                           const string &fmqPath,
                           AScope &scope,
                           int debugLevel):
        _debugLevel(debugLevel),
        _serverHost(host),
        _serverPort(port),
        _serverFmq(fmqPath),
        _scope(scope),
        _pulseCount(0),
        _tsSeqNum(0)
{
  
  // this are required in order to send structured data types
  // via a qt signal
  qRegisterMetaType<AScope::TimeSeries>();
  
  // start timer for checking socket every 50 msecs
  _dataTimerId = startTimer(50);

  // pulse mode

  _channelMode = CHANNEL_MODE_HV_SIM;

  // pulse reader

  if (_serverFmq.size() > 0) {
    _pulseReader = new IwrfTsReaderFmq(_serverFmq.c_str());
  } else {
    _pulseReader = new IwrfTsReaderTcp(_serverHost.c_str(), _serverPort);
  }
  _haveChan1 = false;

}

AScopeReader::~AScopeReader()

{

  if (_pulseReader) {
    delete _pulseReader;
  }

}

//////////////////////////////////////////////////////////////
// respond to timer events
  
void AScopeReader::timerEvent(QTimerEvent *event)
{

  if (event->timerId() == _dataTimerId) {

    if (_debugLevel > 0) {
      cerr << "Servicing socket timer event" << endl;
    }

    // read data from server, until enough data is gathered

    if (_readData() == 0) {
      _sendDataToAScope();
    }

  } // if (event->timerId() == _dataTimerId)
    
}

/////////////////////////////
// read data from the server
// returns 0 on succes, -1 on failure (not enough data)

int AScopeReader::_readData()

{

  // read data until nSamples pulses have been gathered
  
  _nSamples = _scope.getBlockSize();
  
  MemBuf buf;
  while (true) {
    
    // read in a pulse
    
    IwrfTsPulse *pulse = _getNextPulse();
    _pulseCount++;

    // add to vector based on H/V flag
    
    int hvFlag = pulse->get_hv_flag();
    if (hvFlag) {
      _pulses.push_back(pulse);
    } else {
      _pulsesV.push_back(pulse);
    }

    // check we have enough data
    
    if ((int) _pulses.size() >= _nSamples) {
      
      if (_pulsesV.size() == 0) {
        // H data only
        _channelMode = CHANNEL_MODE_HV_SIM;
        return 0;
      } else if ((int) _pulsesV.size() >= _nSamples) {
        // Equal number H and V implies alternating data
        _channelMode = CHANNEL_MODE_ALTERNATING;
        return 0;
      }

    } else if ((int) _pulsesV.size() >= _nSamples) {
      
      if (_pulses.size() == 0) {
        // V data only
        _channelMode = CHANNEL_MODE_V_ONLY;
        return 0;
      } else if ((int) _pulses.size() >= _nSamples) {
        // Equal number H and V implies alternating data
        _channelMode = CHANNEL_MODE_ALTERNATING;
        return 0;
      }

    }

  } // while 

  return -1;

}

///////////////////////////////////////////////////////
// get next pulse
//
// returns NULL on failure

IwrfTsPulse *AScopeReader::_getNextPulse()

{
  
  IwrfTsPulse *pulse = NULL;
  
  while (pulse == NULL) {
    pulse = _pulseReader->getNextPulse(false);
    if (pulse == NULL) {
      if (_pulseReader->getTimedOut()) {
	cout << "# NOTE: read timed out, retrying ..." << endl;
	continue;
      }
      if (_pulseReader->endOfFile()) {
	cout << "# NOTE: end of file encountered" << endl;
      }
      return NULL;
    }
  }

  if (_pulseReader->endOfFile()) {
    cout << "# NOTE: end of file encountered" << endl;
  }
  if (pulse->getIq1() != NULL) {
    _haveChan1 = true;
  } else {
    _haveChan1 = false;
  }

  return pulse;

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
  
  for (size_t ii = 0; ii < _pulsesV.size(); ii++) {
    const IwrfTsPulse *pulse = _pulsesV[ii];
    int nGatesPulse = pulse->getNGates();
    if (nGatesPulse > nGates) {
      nGates = nGatesPulse;
    }
    int nChannelsPulse = pulse->getNChannels();
    if (nChannelsPulse > nChannels) {
      nChannels = nChannelsPulse;
    }
  } // ii

  if (_channelMode == CHANNEL_MODE_HV_SIM) {

    // load H chan 0, send to scope

    AScope::FloatTimeSeries tsChan0;
    _loadTs(nGates, 0, _pulses, 0, tsChan0);
    emit newItem(tsChan0);

    // load H chan 1, send to scope

    AScope::FloatTimeSeries tsChan1;
    _loadTs(nGates, 1, _pulses, 1, tsChan1);
    emit newItem(tsChan1);

    // load burst, send to scope as chan 2

    AScope::FloatTimeSeries tsChan2;
    _loadBurst(_pulseReader->getBurst(), 2, tsChan2);
    emit newItem(tsChan2);

  } else if (_channelMode == CHANNEL_MODE_V_ONLY) {

    // load V chan 0, send to scope
    
    AScope::FloatTimeSeries tsChan0;
    _loadTs(nGates, 0, _pulsesV, 0, tsChan0);
    emit newItem(tsChan0);

    // load V chan 1, send to scope

    AScope::FloatTimeSeries tsChan1;
    _loadTs(nGates, 1, _pulsesV, 1, tsChan1);
    emit newItem(tsChan1);
    
    // load V burst, send to scope as chan 3
    
    AScope::FloatTimeSeries tsChan3;
    _loadBurst(_pulseReader->getBurst(), 3, tsChan3);
    emit newItem(tsChan3);

  } else {

    // alternating mode, 4 channels

    // load H chan 0, send to scope

    AScope::FloatTimeSeries tsChan0;
    _loadTs(nGates, 0, _pulses, 0, tsChan0);
    emit newItem(tsChan0);

    // load H chan 1, send to scope
    
    AScope::FloatTimeSeries tsChan1;
    _loadTs(nGates, 1, _pulses, 1, tsChan1);
    emit newItem(tsChan1);

    // load V chan 0, send to scope as chan 2

    AScope::FloatTimeSeries tsChan2;
    _loadTs(nGates, 0, _pulsesV, 2, tsChan2);
    emit newItem(tsChan2);

    // load V chan 1, send to scope as chan 3

    AScope::FloatTimeSeries tsChan3;
    _loadTs(nGates, 1, _pulsesV, 3, tsChan3);
    emit newItem(tsChan3);
    
  }

  // free up the pulses
  
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    delete _pulses[ii];
  }
  _pulses.clear();

  for (size_t ii = 0; ii < _pulsesV.size(); ii++) {
    delete _pulsesV[ii];
  }
  _pulsesV.clear();

}

///////////////////////////////////////////////
// load up time series object

void AScopeReader::_loadTs(int nGates,
                           int channelIn,
                           const vector<IwrfTsPulse *> &pulses,
                           int channelOut,
                           AScope::FloatTimeSeries &ts)

{

  if (pulses.size() < 2) return;

  // set header

  ts.gates = nGates;
  ts.chanId = channelOut;
  ts.sampleRateHz = 1.0 / pulses[0]->get_prt();
  
  // set sequence number

  size_t *seq0 = new size_t;
  *seq0 = _tsSeqNum;
  ts.handle = seq0;
  if (_debugLevel > 1) {
    cerr << "Creating ts data, seq num: " << _tsSeqNum << endl;
  }
  _tsSeqNum++;
  
  // load IQ data

  for (size_t ii = 0; ii < pulses.size(); ii++) {
    
    const IwrfTsPulse* pulse = pulses[ii];
    int nGatesPulse = pulse->getNGates();
    
    fl32 *iq = new fl32[nGates * 2];
    memset(iq, 0, nGates * 2 * sizeof(fl32));
    if (channelIn == 0) {
      memcpy(iq, pulse->getIq0(), nGatesPulse * 2 * sizeof(fl32));
    } else if (channelIn == 1) {
      if (pulse->getIq1() != NULL) {
        memcpy(iq, pulse->getIq1(), nGatesPulse * 2 * sizeof(fl32));
      }
    } else if (channelIn == 2) {
      if (pulse->getIq2() != NULL) {
        memcpy(iq, pulse->getIq2(), nGatesPulse * 2 * sizeof(fl32));
      }
    }
    ts.IQbeams.push_back(iq);
    
  } // ii
  
}
    
///////////////////////////////////////////////
// load up burst data

void AScopeReader::_loadBurst(const IwrfTsBurst &burst,
                              int channelOut,
                              AScope::FloatTimeSeries &ts)

{
  
  if (burst.getNSamples() < 2) return;

  // set header

  ts.gates = burst.getNSamples();
  ts.chanId = channelOut;
  ts.sampleRateHz = burst.getSamplingFreqHz();
  
  // set sequence number

  size_t *seq0 = new size_t;
  *seq0 = _tsSeqNum;
  ts.handle = seq0;
  if (_debugLevel > 1) {
    cerr << "Creating burst data, seq num: " << _tsSeqNum << endl;
  }
  _tsSeqNum++;
  
  // load IQ data

  fl32 *iq = new fl32[burst.getNSamples() * 2];
  memcpy(iq, burst.getIq(), burst.getNSamples() * 2 * sizeof(fl32));
  ts.IQbeams.push_back(iq);
  
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

