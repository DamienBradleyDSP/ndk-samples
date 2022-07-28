/*
  ==============================================================================

    MidiMessage.cpp
    Created: 28 Jul 2022 6:11:05pm
    Author:  Damien

  ==============================================================================
*/

#include "MidiMessage.h"

void dbMidiMessage::setTimeStamp(double t)
{
    timeStamp = t;
}

void dbMidiMessage::setMidiChannel(int channel)
{
    midiChannel = channel;
}

void dbMidiMessage::setNoteNumber(int note)
{
    noteNumber = note;
}

void dbMidiMessage::setVelocity(float v)
{
    velocity = v;
}

int dbMidiMessage::getNoteNumber()
{
    return noteNumber;
}

double dbMidiMessage::getTimeStamp()
{
    return timeStamp;
}

dbMidiMessage dbMidiMessage::noteOn(int channel, int noteNumber, float velocity)
{
    auto message = dbMidiMessage();
    message.setMidiChannel(channel);
    message.setMessageType(dbMidiMessage::messageType::noteOn);
    message.setNoteNumber(noteNumber);
    message.setVelocity(velocity);
    return message;
}

dbMidiMessage dbMidiMessage::noteOff(int channel, int noteNumber)
{
    auto message = dbMidiMessage();
    message.setMidiChannel(channel);
    message.setMessageType(dbMidiMessage::messageType::noteOff);
    message.setNoteNumber(noteNumber);
    return message;
}

void dbMidiMessage::setByteMessage(uint8_t* dataBuffer, int)
{
    if (type == messageType::noteOff)
    {
        dataBuffer[0] = 0x80+midiChannel;
        dataBuffer[1] = noteNumber;
        dataBuffer[2] = (uint8_t)(velocity * 127.0f);
    }
    else if (type == messageType::noteOn)
    {
        dataBuffer[0] = 0x90+midiChannel;
        dataBuffer[1] = noteNumber;
        dataBuffer[2] = (uint8_t)(velocity * 127.0f);
    }
}
