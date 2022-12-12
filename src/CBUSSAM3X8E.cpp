
/*

  Copyright (C) Duncan Greenwood 2021 (duncan_greenwood@hotmail.com)

  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

#include <SPI.h>
#include <Streaming.h>
#include "CBUSSAM3X8E.h"

// CBUS configuration object, declared externally
extern CBUSConfig config;

// forward declaration
void format_message(CANFrame *msg);

//
/// constructor
//

CBUSSAM3X8E::CBUSSAM3X8E() {
  eventhandler = NULL;
  framehandler = NULL;
  _instance = 0;
  _can = &Can0;
}

CBUSSAM3X8E::CBUSSAM3X8E(CBUSConfig *the_config) : CBUSbase(the_config) {
  eventhandler = NULL;
  framehandler = NULL;
  _instance = 0;
  _can = &Can0;
}

//
/// set the CAN controller peripheral instance, there are two, default is zero
//

void CBUSSAM3X8E::setControllerInstance(byte instance) {

  // Serial << "> setting CAN controller instance to " << instance << endl;
  _instance = instance;
  _can = (_instance == 0) ? &Can0 : &Can1;
}

//
/// initialise the CAN controller and buffers, and attach the ISR
//

bool CBUSSAM3X8E::begin(bool poll, SPIClass spi) {

  uint32_t init_ret;
  int init_watch;

  _numMsgsSent = 0;
  _numMsgsRcvd = 0;

  // Serial << "> initialising CAN controller peripheral, instance = " << _instance << endl;

  // init CAN instance
  init_ret = _can->begin(CAN_BPS_125K, 255);

  if (!init_ret) {
    Serial << "> CAN error from begin(), ret = " << init_ret << endl;
    return false;
  }

  // set filter to permissive
  init_watch = _can->watchFor();

  if (init_watch == -1) {
    Serial << "> CAN error from watchFor(), ret = " << init_watch << endl;
    return false;
  }

  // Serial << "> CAN controller instance = " << _instance << " initialised successfully" << endl;
  return true;
}

//
/// check for unprocessed messages in the receive buffer
//

bool CBUSSAM3X8E::available(void) {

  return _can->available();
}

//
/// get next unprocessed message from the buffer
//

CANFrame CBUSSAM3X8E::getNextMessage(void) {

  uint32_t ret;
  CAN_FRAME cf;

  ret = _can->read(cf);

  if (ret != CAN_MAILBOX_TRANSFER_OK) {
    // Serial << "> CAN read did not return CAN_MAILBOX_TRANSFER_OK, instance = " << _instance << ", ret = " << ret << endl;
  } else {
    // Serial << "> received CAN message ok, instance = " << _instance << endl;
  }

  _msg.id = cf.id;
  _msg.len = cf.length;
  _msg.rtr = cf.rtr;
  _msg.ext = cf.extended;

  memcpy(_msg.data, cf.data.byte, cf.length);

  // format_message(&_msg);
  return _msg;
}

//
/// send a CBUS message
//

bool CBUSSAM3X8E::sendMessage(CANFrame *msg, bool rtr, bool ext, byte priority) {

  bool ret;
  CAN_FRAME cf;                         // library-specific CAN message structure

  makeHeader(msg, priority);            // set the CBUS header - CANID and priority bits
  // format_message(msg);

  cf.id = msg->id;
  cf.length = msg->len;
  cf.rtr = rtr;
  cf.extended = ext;

  memcpy(cf.data.bytes, msg->data, msg->len);

  ret = _can->sendFrame(cf);

  if (!ret) {
    // Serial << "> error sending CAN message, instance = " << _instance << ", ret = " << ret << endl;
  }

  return ret;
}

//
/// display the CAN bus status instrumentation
//

void CBUSSAM3X8E::printStatus(void) {

  return;
}

//
/// reset the CAN driver
//

void CBUSSAM3X8E::reset(void) {

}

//
/// set the TX and RX pins
//

void CBUSSAM3X8E::setPins(byte txPin, byte rxPin) {

  return;
}

//
/// set the depth of the TX and RX queues
//

void CBUSSAM3X8E::setNumBuffers(byte num) {

  return;
}

//
/// format and display CAN message
//

void format_message(CANFrame *msg) {

  char mbuff[80], dbuff[8];

  sprintf(mbuff, "[%03ld] [%d] [", (msg->id & 0x7f), msg->len);

  for (byte i = 0; i < msg->len; i++) {
    sprintf(dbuff, " %02x", msg->data[i]);
    strcat(mbuff, dbuff);
  }

  strcat(mbuff, " ]");
  Serial << "> " << mbuff << endl;

  return;
}

