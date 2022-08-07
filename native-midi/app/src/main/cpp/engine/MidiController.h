/*
  ==============================================================================

    MidiController.h
    Created: 24 Oct 2021 4:19:29pm
    Author:  Damien

  ==============================================================================
*/

#pragma once
#include <map>
#include <atomic>
#include <string>
#include <memory>
#include <list>
#include <cmath>
#include "ProjectSettings.h"
#include "MidiMessage.h"

// This handles the buffer portion of maths, that is to say, the sample locations fall within a particular buffer length
// the rest of the sequencer should have no knowledge of the buffer, only specific sample locations

// The sample location will be calculated and then each will be checked every buffer
// if the sample location is within the buffer, the midimessage is loaded and a new sample location is calculated for that beat

class MidiController
{
public:
    MidiController() = delete;
    MidiController(std::map<std::string, std::atomic<float>*>& parameters);
    ~MidiController();

    struct playPositionInformation
    {
        double bpm;
        double ppqPosition;
        double ppqPositionOfLastBarStart;
        std::int64_t timeInSamples;
        double 	timeInSeconds;
        int frameRate; //30fps
    };

    // Sequencer Interface
    void initialise(double sampleRate, int bufferSize);
    void calculateBufferSamples(playPositionInformation& currentpositionstruct, bars totalNumberOfBars);
    void applyMidiMessages(std::list<dbMidiMessage>& buffer);

    // Beat Interface
    void addMidiMessage(dbMidiMessage& noteOnMessage,bars noteOnPosition, dbMidiMessage& noteOffMessage, bars noteOffPosition, bool sustain=false);

private:

    void checkNoteOffMessages();
    int getLocation(bars barPosition);

    double sampleRate;
    int bufferLength;
    samples samplesFromRhythmStart;
    bars totalNumberOfBars;
    samples oneBarSampleLength;
    samples totalSampleLength;

    std::pair<int, int> bufferSpan;
    std::pair<samples, samples> sampleSpan;
    std::pair<samples, samples> sampleSpanOverspill;
    bool sampleOverspill = false;

    std::list<dbMidiMessage> bufferMidiMessages;

    std::list<std::pair<dbMidiMessage, double>> noteOffMessages;
    std::list<dbMidiMessage> sustainedMidiMessages;

    bool sustainToNextNote = false;

    std::atomic<float>* resetLoop;
};