//
// Created by Damien on 11/07/2022.
//
#include <cstdint>
#include "engine/Sequencer.h"
#include "parameterLayout.h"

#ifndef NATIVE_MIDI_MIDIENGINE_H
#define NATIVE_MIDI_MIDIENGINE_H


class MidiEngine {
public:
    void initialise(double sampleRate, int bufferLength);

    bool generateMidi(uint8_t* , int);
    void shutDown();

private:
    void clearMidiArray(std::uint8_t* , int);
    void populateParameterMap();

    double sampleRate;
    int bufferLength;

    int frameCounter = 0; // testing

    MidiController::playPositionInformation currentPosition;

    parameterLayout layout;

    std::map<std::string, std::atomic<float>*> parameterMap;
    std::unique_ptr<Sequencer> sequencer;
};


#endif //NATIVE_MIDI_MIDIENGINE_H
