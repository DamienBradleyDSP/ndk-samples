//
// Created by Damien on 11/07/2022.
//

#include "MidiEngine.h"

void MidiEngine::initialise(double s, int b) {
    sampleRate = s;
    bufferLength = b;

    populateParameterMap();
    sequencer = std::unique_ptr<Sequencer>(new Sequencer(layout.getParameterMap()));

    sequencer->initialise(sampleRate, bufferLength);
}

bool MidiEngine::generateMidi(uint8_t* midiByteArray, int numberOfBytes) {

    clearMidiArray(midiByteArray, numberOfBytes);

    currentPosition.bpm = ((double)bufferLength*20.0)/60.0;
    currentPosition.ppqPosition = 0;                    // INSERT TESTING SCRIPT
    currentPosition.ppqPositionOfLastBarStart = 0;

    // TESTING__________________________________________________________________________________
    currentPosition.bpm = 120;

    auto TESTbpm = 120;				// test BPM - STRUCT
    double beatspersecond = TESTbpm / 60.0;		// 2 beats per second
    double barspersecond = beatspersecond / 4.0;	// 0.5 bars per second

    double barspersample = barspersecond / sampleRate;		// <<<<1 bars per sample

    auto samplesperquarternote = sampleRate / beatspersecond;	// 24000 samples per quarter note

    auto quarternotespersample = 1 / samplesperquarternote;	// 1/24000 quarter notes per sample

    currentPosition.ppqPosition = quarternotespersample * frameCounter;	// calculates current number of quarter notes - this will be done by the struct
    currentPosition.ppqPositionOfLastBarStart = floor(frameCounter * barspersample) * 4;	// calculates number of quarter notes at start of last bar, 4/4 time
    frameCounter+=bufferLength;

    // TESTING______________________________________________________________________________

    std::list<dbMidiMessage> midiMessages;
    sequencer->generateMidi(midiMessages, currentPosition);

    bool messageSent = false;
    for (auto message : midiMessages) {
        message.setByteMessage(midiByteArray, 3);
        messageSent = true;
    }

    return messageSent;
}

void MidiEngine::shutDown() {



}

void MidiEngine::clearMidiArray(std::uint8_t * byteArray, int numberOfBytes) {
    for(int i =0; i<numberOfBytes; i++) byteArray[i] = 0;
}
void MidiEngine::populateParameterMap() {
// Sequencer / Global Parameters

    layout.add(std::string("loadSaveState"), 0);
    layout.add(std::string("numberOfModules"), ProjectSettings::startingNumberOfRhythmModules);
    layout.add(std::string("midiNoteNumber"),ProjectSettings::midiNote);
    layout.add(std::string("resetLoop"),0);
    layout.add(std::string("barOffset"),0);

    // Rhythm Specific Parameters

    for (int i = 0; i < ProjectSettings::maxNumberOfRhythmModules; i++) // +1 for parameter move operations on gui side
    {
        layout.add(std::string("numberOfBeats" + std::to_string(i)),ProjectSettings::startingNumberOfBeats);
        layout.add(std::string("numberOfBars" + std::to_string(i)),ProjectSettings::startingNumberOfBars);
        layout.add(std::string("selectionOfBeats" + std::to_string(i)), ProjectSettings::startingNumberOfBeats);

        auto onState = i >= ProjectSettings::startingNumberOfRhythmModules ? 0 : 1;
        layout.add(std::string("moduleTurnedOn" + std::to_string(i)), onState);
        layout.add(std::string("moduleReadOrder" + std::to_string(i)),i);
    }

    // Beat Specific Parameters

    for (int i = 0; i < ProjectSettings::maxNumberOfRhythmModules; i++) // +1 for parameter move operations on gui side
    {
        for (int j = 1; j <= ProjectSettings::maxNumberOfBeats; j++)
        {
            layout.add(std::string("BeatOnOffR" + std::to_string(i) + "B" + std::to_string(j)),1);
            layout.add(std::string("VelocityR" + std::to_string(i) + "B" + std::to_string(j)),0.8);
            layout.add("noteLengthR" + std::to_string(i) + "B" + std::to_string(j), 0.5);
            layout.add(std::string("SemitoneR" + std::to_string(i) + "B" + std::to_string(j)), 0);
            layout.add("OctaveR" + std::to_string(i) + "B" + std::to_string(j), 0);
            layout.add(std::string("SustainR" + std::to_string(i) + "B" + std::to_string(j)), 0);
        }
    }
}