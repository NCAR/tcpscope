#include "AScopeReader.h"
#include <QMetaType>
#include <cerrno>
#include <radar/iwrf_functions.hh>
using namespace std;

Q_DECLARE_METATYPE(AScope::TimeSeries)
  
// Note that the timer interval for QtTSReader is 0; we
// are accepting all DDS samples.
  AScopeReader::AScopeReader(const string &host,
                             int port,
                             AScope &scope,
                             int debugLevel):
         _serverHost(host),
          _serverPort(port),
          _scope(scope),
          _lastTryConnectTime(0),
          _debugLevel(debugLevel),
          _pulseCount(0)
{
  // this are required in order to send structured data types
  // via a qt signal
  qRegisterMetaType<AScope::TimeSeries>();

  // start timer for checking socket every 50 msecs
  _sockTimerId = startTimer(50);

}

AScopeReader::~AScopeReader() {

  if (_sock.isOpen()) {
    if (_debugLevel) {
      cerr << "Closing socket to IWRF data server" << endl;
    }
    _sock.close();
  }

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

    cerr << "Servicing socket timer event" << endl;

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

    // read data from server

    if (_readFromServer()) {
    }

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

/////////////////////////////
// read data from the server

int AScopeReader::_readFromServer()

{

  // read data
  
  MemBuf buf;
  while (true) {
    
    // read packet from time series server server

    int packetId, packetLen;
    if (_readTcpPacket( packetId, packetLen, buf)) {
      cerr << "ERROR - AScopeReader::_readFromServer" << endl;
      return -1;
    }
    if (_timedOut) {
      return 0;
    }
    
    // handle packet types

    if (packetId == IWRF_PULSE_HEADER_ID) {
      
      // pulse header and data
      
      // iwrf_pulse_header_t &pHdr = *((iwrf_pulse_header_t *) buf.getPtr());
      
      // scale data as applicable
      
//       if(_params.apply_scale) {
//         iwrf_pulse_scale_data(buf.getPtr(), buf.getLen(),
//                               _params.scale,_params.bias);
//       }

      // add to message

//       _msg.addPart(packetId, packetLen, buf.getPtr());

      // delay as needed

//       _doReadDelay(pHdr.packet);

    } else if (packetId == IWRF_RADAR_INFO_ID) {
      
      // radar info - make local copy
      
      iwrf_radar_info_t *radar = (iwrf_radar_info_t *) buf.getPtr();
      _tsRadarInfo = *radar;
      
      // add to FMQ if shmem is not active

//       if (_sysconShmem != NULL) {
//         _msg.addPart(packetId, packetLen, buf.getPtr());
//       }

    } else if (packetId == IWRF_SCAN_SEGMENT_ID) {

      // scan segment - make local copy

      iwrf_scan_segment_t *scan = (iwrf_scan_segment_t *) buf.getPtr();
      _tsScanSegment = *scan;

      // add to FMQ if shmem is not active

//       if (_sysconShmem != NULL) {
//         _msg.addPart(packetId, packetLen, buf.getPtr());
//       }

    } else if (packetId == IWRF_TS_PROCESSING_ID) {
      
      // ts processing - make local copy

      iwrf_ts_processing_t *proc = (iwrf_ts_processing_t *) buf.getPtr();
      _tsTsProcessing = *proc;
      
      // add to FMQ if shmem is not active

//       if (_sysconShmem != NULL) {
//         _msg.addPart(packetId, packetLen, buf.getPtr());
//       }

    } else {

      // other packet type
      // add to outgoing message
      
//       _msg.addPart(packetId, packetLen, buf.getPtr());

    }

    // if the message is large enough, write to the FMQ
    
//     _writeToFmq();
    
  } // while (true)

  return 0;

}

///////////////////////////////////////////////////////////////////
// Read in next packet, set id and load buffer.
// Returns 0 on success, -1 on failure

int AScopeReader::_readTcpPacket(int &id, int &len, MemBuf &buf)

{
  bool haveGoodHeader = false;
  si32 packetId;
  si32 packetLen;
  si32 packetTop[2];
  _timedOut = false;

  do {

    // peek at the first 8 bytes
    if (_peekAtBuffer(packetTop, sizeof(packetTop))) {
      cerr << "ERROR - AScopeReader::_readTcpPacket" << endl;
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
      cerr << "ERROR - AScopeReader::_readPacket" << endl;
      cerr << " Incoming data stream out of sync" << endl;
      cerr << " Closing socket" << endl;
      _sock.close();
      return -1;
      // read bytes to re-synchronize data stream
      // if (_reSync(sock)) {
      //  cerr << "ERROR - AScopeReader::_readPacket" << endl;
      //  cerr << " Cannot re-sync incoming data stream from socket";
      //  cerr << endl;
      //  return -1;
      // }
    } else {
      haveGoodHeader = true;
      id = packetId;
      len = packetLen;
    }
  } while (!haveGoodHeader);
    
  // read it in
  buf.reserve(packetLen);

  if (_sock.readBuffer(buf.getPtr(), packetLen)) {
    if (_sock.getErrNum() == Socket::TIMED_OUT) {
      _timedOut = true;
      return 0;
    } else {
      cerr << "ERROR - AScopeReader::_readTcpPacket" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }
  }
  
  if (_debugLevel > 1 &&
      id != IWRF_PULSE_HEADER_ID &&
      id != IWRF_RVP8_PULSE_HEADER_ID) {
    cerr << "Read in TCP packet, id, len: " << iwrf_packet_id_to_str(id)
	 << ", " << packetLen << endl;

    if(_pulseCount > 0 && id == IWRF_SYNC_ID) {
      cerr << "Read " << _pulseCount
           << " Pulse packets since last sync" << endl;
      _pulseCount = 0;  
    }
  } else {
    _pulseCount++;  // Keep track of pulse packets
    if (_debugLevel > 2) {
      cerr << "Read in TCP packet, id, len: " << iwrf_packet_id_to_str(id)
           << ", " << packetLen << endl;
    }
  }
  
  if (_debugLevel > 1 && 
      id != IWRF_PULSE_HEADER_ID && id != IWRF_SYNC_ID) {
    iwrf_packet_print(stderr, buf.getPtr(), buf.getLen());
  }
  
  if(_debugLevel > 2 && 
     (id == IWRF_PULSE_HEADER_ID || id == IWRF_SYNC_ID))   {
    iwrf_packet_print(stderr, buf.getPtr(), buf.getLen());
  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// Peek at buffer from socket
// Returns 0 on success, -1 on failure
// _timedOut set in case of timeout

int AScopeReader::_peekAtBuffer(void *buf, int nbytes)

{

  _timedOut = false;

  if (_sock.peek(buf, nbytes, 0) == 0) {
    return 0;
  } else {
    if (_sock.getErrNum() == Socket::TIMED_OUT) {
      _timedOut = true;
      return 0;
    } else {
      cerr << "ERROR - AScopeReader::_peekAtBuffer" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
    }
  }

  return -1;

}

#ifdef NOTNOW

///////////////////////////////////////////
// re-sync the data stream
// returns 0 on success, -1 on error

int AScopeReader::_reSync(Socket &sock)
  
{

  int sync_count = 0;
  
  if (_debugLevel) {
    cerr << "Trying to resync ....." << endl;
  }
  
  unsigned int check[2];

  while (true) {
    
    // peek at the next 8 bytes
    
    if (_peekAtBuffer(sock, check, sizeof(check))) {
      cerr << "ERROR - AScopeReader::_reSync" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      return -1;
    }

//     float *fc1,*fc2;    // Cast to floats for diagnostic output
//     fc1 = (float*) check;
//     fc2 = (float *)  fc1 +1;

//     if(sync_count < 4000 && sync_count%8 == 0) {
//       fprintf(stderr,"DEBUG: %d bytes, %X %X   %d %d   %f %f\n",sync_count,
//               check[0],check[1],
//               check[0],check[1],
//               *fc1,*fc2);
//     }

    if((check[0] == IWRF_RADAR_INFO_ID &&
        check[1] == sizeof(iwrf_radar_info_t)) ||
       (check[0] == IWRF_XMIT_SAMPLE_ID &&
        check[1] == sizeof(iwrf_xmit_sample_t))) {
      return 0; // We've found a legitimate IWRF packet header
    } 

    // Search for the sync packet 
    if (check[0] == IWRF_SYNC_VAL_00 && check[1] == IWRF_SYNC_VAL_01) {
      // These are "sync packet" bytes read the 8 sync bytes and move on
      if (_debugLevel) {
	cerr << "Found sync packet, back in sync" << endl;
      }
      if (sock.readBufferHb(check, sizeof(check), sizeof(check),
			    NULL, 1000)) {
	cerr << "ERROR - AScopeReader::_reSync" << endl;
	cerr << "  " << sock.getErrStr() << endl;
	return -1;
      }
      return 0;
    }
    
    // no sync yet, read 1 byte and start again

    char byteVal;
    if (sock.readBufferHb(&byteVal, 1, 1, NULL, 1000)) {
      cerr << "ERROR - AScopeReader::_reSync" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      return -1;
    }
    sync_count++;

  } // while

  return -1;

}

#endif

