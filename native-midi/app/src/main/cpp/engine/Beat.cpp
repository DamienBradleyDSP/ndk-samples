/*
  ==============================================================================

    Beat.cpp
    Created: 25 Oct 2021 3:55:25pm
    Author:  Damien

  ==============================================================================
*/

#include "Beat.h"

Beat::Beat(std::map<std::string, std::atomic<float>*>& parameters, int rnum, int bnum)
    : rhythmNumber(rnum), beatNumber(bnum)
{
    onState = parameters["BeatOnOffR" + std::to_string(rhythmNumber) + "B" + std::to_string(beatNumber)];
    velocity = parameters["VelocityR" + std::to_string(rhythmNumber) + "B" + std::to_string(beatNumber)];
    noteLength = parameters["noteLengthR" + std::to_string(rhythmNumber) + "B" + std::to_string(beatNumber)];
    midiNote = parameters["midiNoteNumber"];
    midiSemitone = parameters["SemitoneR" + std::to_string(rhythmNumber) + "B" + std::to_string(beatNumber)];
    midiOctave = parameters["OctaveR" + std::to_string(rhythmNumber) + "B" + std::to_string(beatNumber)];
    sustain = parameters["SustainR" + std::to_string(rhythmNumber) + "B" + std::to_string(beatNumber)];

    noteOn = dbMidiMessage::noteOn(ProjectSettings::midiChannel, midiNote->load(), (float)ProjectSettings::midiVelocity);
    noteOff = dbMidiMessage::noteOff(ProjectSettings::midiChannel, midiNote->load());
}

Beat::~Beat()
{
}

void Beat::setBarPositionLength(bars barPosition, bars length)
{
    beatPosition = barPosition;
    beatLength = length;
}

void Beat::applyMidiMessages(std::unique_ptr<MidiController>& midiController, bars lastModuleBarEnding)
{
    if (!onState->load()) return;

    auto noteNumber = midiNote->load() + (midiOctave->load() * 12) + midiSemitone->load();
    noteOn.setNoteNumber(noteNumber);
    noteOff.setNoteNumber(noteNumber);

    noteOn.setVelocity(velocity->load());

    midiController->addMidiMessage(noteOn, lastModuleBarEnding + beatPosition,noteOff, lastModuleBarEnding + beatPosition + beatLength * noteLength->load(),sustain->load());
}
