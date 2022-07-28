/*
  ==============================================================================

    Sequencer.h
    Created: 17 Oct 2021 12:18:48pm
    Author:  Damien

  ==============================================================================
*/

#pragma once
#include <map>
#include <atomic>
#include <string>
#include <memory>
#include <list>
#include <vector>
#include "ProjectSettings.h"
#include "RhythmModule.h"
#include "MidiMessage.h"

// current position struct use INT64 samples from timeline start with BPM
// Sequencer needs to be exactly sample accurate and should not stray unreasonably through time

class Sequencer
{
public:
    Sequencer() = delete;
    Sequencer(std::map<std::string, std::atomic<float>*>& parameters);
    ~Sequencer();
    
    void initialise(double sampleRate, int bufferSize);
    void generateMidi(std::list<dbMidiMessage>& buffer, MidiController::playPositionInformation& playhead);

private:

    std::unique_ptr<MidiController> midiController;

    std::vector<std::atomic<float>*> rhythmModuleOrder;
    std::vector<std::unique_ptr<RhythmModule>> rhythmModules;

    std::atomic<float>* numberOfModulesSelected;
    std::atomic<float>* barOffset;


};
