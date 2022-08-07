//
// Created by Damien on 11/07/2022.
//
#include <cstdint>
#include <stack>
#include "engine/Sequencer.h"
#include "parameterLayout.h"

#ifndef NATIVE_MIDI_MIDIENGINE_H
#define NATIVE_MIDI_MIDIENGINE_H


class MidiEngine {
public:
    void initialise(double sampleRate, int bufferLength, int maxBytesPerBuffer);

    int generateMidi(uint8_t* ,  MidiController::playPositionInformation& currentPosition);
    void shutDown();

private:
    void populateParameterMap();
    void generateClockSignal(uint8_t* , MidiController::playPositionInformation& currentPosition);
    void generateMtcQuarterFrame(uint8_t* , MidiController::playPositionInformation& currentPosition);

    double sampleRate;
    int bufferLength;
    int maxBytesPerBuffer;
    double midiClock = 0;
    int maxArrayLocation = 0;

    parameterLayout layout;

    std::map<std::string, std::atomic<float>*> parameterMap;
    std::unique_ptr<Sequencer> sequencer;
};


#endif //NATIVE_MIDI_MIDIENGINE_H
