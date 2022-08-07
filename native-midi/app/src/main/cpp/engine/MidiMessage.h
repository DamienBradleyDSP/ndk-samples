/*
  ==============================================================================

    MidiMessage.h
    Created: 28 Jul 2022 6:11:05pm
    Author:  Damien

  ==============================================================================
*/

#pragma once
#include <cstdint>
#include <list>

/*
namespace midiSpec
{
	// DATA VS STATUS BYTE
	uint8_t dataByte = 0x00;
	uint8_t statusByte = 0x80;

	// System Real Time Messages
	uint8_t timingClock = 0xF8;
	uint8_t start = 0xFA;
	uint8_t continuePlaying = 0xFB;
	uint8_t stop = 0xFC;
	uint8_t activeSensing = 0xFE;
	uint8_t systemReset = 0xFF;

	// System Common Messages // These all have data bytes
	uint8_t timeCodeQuarterFrame = 0xF1; // 1 data byte
	uint8_t songPositionPointer = 0xF2;
	uint8_t songSelect = 0xF3;
	uint8_t TuneRequest = 0xF2;
	uint8_t EOXFlag = 0xF2;

	// Channel Voice Status Messages
	uint8_t noteOff = 0x80; // 0x7F noteNumber + 0x7F velocity
	uint8_t noteOn = 0x90; // 0x7F noteNumber + 0x7F velocity
	uint8_t polyKeyPressure = 0xA0; // 0x7F keypressure + 0x7F pressure value
	uint8_t ControlChange = 0xB0; // 0x7F control change + 0x7F control value
	uint8_t programChange = 0xC0; // 0x7F program number
	uint8_t channelPressure = 0xD0; // 0x7F pressure value
	uint8_t pitchBend = 0xE0; // 0x7F pitch bend change LSB + 0x7F pitch bend change MSB

}
*/

class dbMidiMessage
{
public:
    enum class messageType
    {
        noteOn = 0,
        noteOff = 1,
    };

    void setTimeStamp(double timeStamp);
    void setMidiChannel(int channel);
    void setNoteNumber(int note);
    void setVelocity(float velocity);
    void inline setMessageType(messageType t) { type = t; };
    int getNoteNumber();
	double getTimeStamp();

    static dbMidiMessage noteOn(int channel, int noteNumber, float velocity);
    static dbMidiMessage noteOff(int channel, int noteNumber) ;

	std::list<uint8_t> get3ByteMessage();

private:

	double timeStamp = 0;
	int midiChannel = 1;
	int noteNumber = 72;
	messageType type = messageType::noteOff;
	float velocity = 1.0;
};