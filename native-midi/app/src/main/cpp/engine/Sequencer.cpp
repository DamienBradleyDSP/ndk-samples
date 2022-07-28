/*
  ==============================================================================

    Sequencer.cpp
    Created: 17 Oct 2021 12:18:48pm
    Author:  Damien

  ==============================================================================
*/

#include "Sequencer.h"

Sequencer::Sequencer(std::map<std::string, std::atomic<float>*>& parameters)
    : midiController(new MidiController(parameters))//, draggingHandler(draggingelementhandler)
{

    for (int i = 0; i < ProjectSettings::maxNumberOfRhythmModules; i++)
    {
        rhythmModules.push_back(std::unique_ptr<RhythmModule>(new RhythmModule(parameters, i)));
        rhythmModuleOrder.push_back(parameters["moduleReadOrder" + std::to_string(i)]);
    } 

    numberOfModulesSelected = parameters["numberOfModules"];
    barOffset = parameters["barOffset"];
}

Sequencer::~Sequencer()
{
}

void Sequencer::initialise(double sampleRate, int bufferSize)
{
    midiController->initialise(sampleRate, bufferSize);
}

void Sequencer::generateMidi(std::list<dbMidiMessage>& buffer, MidiController::playPositionInformation& playhead)
{
    // For module, if its turned on, add the bars to the total number of bars - ORDER SENSITIVE
    
    bars totalNumberOfBars = barOffset->load();
    for (auto&& index : rhythmModuleOrder)
    {
        rhythmModules[index->load()]->getNumberOfBars(totalNumberOfBars); // order sensitive
    }
       
    //for (auto&& rmodule : rhythmModules) rmodule->getNumberOfBars(totalNumberOfBars);

    // calculate current sample range for buffer and get bar location
    midiController->calculateBufferSamples(playhead, totalNumberOfBars);

    for (auto&& rModule : rhythmModules) rModule->generateMidi(midiController);

    midiController->applyMidiMessages(buffer);
}

