//
// Created by Damien on 11/07/2022.
//

#include "MidiEngine.h"

void MidiEngine::initialise(double s, int b, int maxBytes) {
    sampleRate = s;
    bufferLength = b;
    maxBytesPerBuffer = maxBytes;

    populateParameterMap();
    sequencer = std::unique_ptr<Sequencer>(new Sequencer(layout.getParameterMap()));

    sequencer->initialise(sampleRate, bufferLength);
}

int MidiEngine::generateMidi(uint8_t* midiByteArray, MidiController::playPositionInformation& currentPosition) {

    maxArrayLocation=0;
    std::list<dbMidiMessage> midiMessages;
    sequencer->generateMidi(midiMessages, currentPosition);

    generateClockSignal(midiByteArray, currentPosition);
    generateMtcQuarterFrame(midiByteArray,currentPosition);
    if((int)midiMessages.size()*3>(maxBytesPerBuffer)) return 0; // If too many messages return

    // Add midi messages to buffer
    auto arrayLocation=0;
    for (auto message : midiMessages) {
        auto messageBytes(message.get3ByteMessage());
        for(auto byte : messageBytes)
        {
            while(midiByteArray[arrayLocation]!=0xFD) arrayLocation++;
            midiByteArray[arrayLocation] = byte;
            if(arrayLocation>maxArrayLocation) maxArrayLocation = arrayLocation;
            arrayLocation++;
        }
    }
    return maxArrayLocation+1;
}

void MidiEngine::shutDown() {
}

void MidiEngine::generateClockSignal(uint8_t * midiByteArray, MidiController::playPositionInformation &currentPosition) {

    if(midiClock > 1.0)
    {
        midiClock-=1.0;
        return;
    }

    auto buffersPerSecond = (double)sampleRate/(double)bufferLength;
    double midiClocksPerSecond = (currentPosition.bpm/60)*24;
    auto buffersPerMidiClock = buffersPerSecond / midiClocksPerSecond;

    auto currentMidiClock = midiClock;
    midiClock += buffersPerMidiClock;

    uint8_t clockSignal = 0xF8;
    auto clockLocation = (int)(currentMidiClock * (double)maxBytesPerBuffer);
    if(clockLocation == maxBytesPerBuffer) clockLocation = maxBytesPerBuffer-1;
    midiByteArray[clockLocation] = clockSignal;
    if(clockLocation>maxArrayLocation) maxArrayLocation = clockLocation;
}

void MidiEngine::generateMtcQuarterFrame(uint8_t* , MidiController::playPositionInformation &currentPosition ) {

    double framesPerSecond = currentPosition.frameRate;
    double secondsPerFrame = 1/framesPerSecond;
    auto buffersPerSecond = (double)sampleRate/(double)bufferLength;
    double buffersPerFrame = buffersPerSecond/secondsPerFrame;



    buffersPerFrame = 0;

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