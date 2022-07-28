/*
  ==============================================================================

    Beat.h
    Created: 25 Oct 2021 3:55:25pm
    Author:  Damien

  ==============================================================================
*/

#pragma once
#include <map>
#include <atomic>
#include <string>
#include <memory>
#include <list>
#include "ProjectSettings.h"
#include "MidiController.h"
#include "MidiMessage.h"

class Beat
{
public:
    Beat()=delete;
    Beat(std::map<std::string, std::atomic<float>*>& parameters, int rhythmNumber, int beatNumber);
    ~Beat();

    void setBarPositionLength(bars barPosition, bars length);

    void applyMidiMessages(std::unique_ptr<MidiController>& midiController, bars lastModuleBarEnding);
private:

    const int rhythmNumber; // 
    const int beatNumber;

    bars beatPosition = 0; // beatPosition - integers are bars and fractional amounts are positions within the bar
    bars beatLength = 0;

    dbMidiMessage noteOn;
    dbMidiMessage noteOff;
    
    // User params
    std::atomic<float>* onState;
    std::atomic<float>* velocity;
    std::atomic<float>* noteLength;
    std::atomic<float>* midiNote;
    std::atomic<float>* midiSemitone;
    std::atomic<float>* midiOctave;
    std::atomic<float>* sustain;
};