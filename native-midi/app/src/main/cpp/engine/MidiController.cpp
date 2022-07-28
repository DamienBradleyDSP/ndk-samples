/*
  ==============================================================================

    MidiController.cpp
    Created: 24 Oct 2021 4:19:29pm
    Author:  Damien

  ==============================================================================
*/

#include "MidiController.h"

MidiController::MidiController(std::map<std::string, std::atomic<float>*>& parameters)
{
    resetLoop = parameters["resetLoop"];
}

MidiController::~MidiController()
{
}

void MidiController::initialise(double s, int b)
{
    sampleRate = s;
    bufferLength = b;
    samplesFromRhythmStart = 0;
    sampleSpanOverspill.first = -1;
    sampleSpanOverspill.second = -1;
    bufferSpan.first = 0;
    bufferSpan.second = bufferLength;
    totalNumberOfBars = 0;
    totalSampleLength = 0;
    sampleOverspill = false;
}

void MidiController::calculateBufferSamples(playPositionInformation& currentpositionstruct, bars totalNumOfBars)
{
    // THE SAMPLE COUNT SHOULD PROBABLY RESET OR BE CHECKED AT THE BAR LINE
    // 
    // Track the current bar using the current position struct.
    // When the bar ticks over we should be recalibrating our sample locations
    // 
    // We need to switch from a completely sample based engine here to a hybrid,
    // where It tracks bar locations and then map samples on to it
    // 
    // calculating sample ranges that 

    totalNumberOfBars = totalNumOfBars;
    double barsPerMinute = (double)currentpositionstruct.bpm / 4.0;
    oneBarSampleLength = ((4.0 * 60.0) / (double)currentpositionstruct.bpm) * sampleRate;
    totalSampleLength = (totalNumberOfBars / barsPerMinute) * 60.0 * sampleRate; //length in samples of the total number of bars spanned by the given rhythm modules

    /// RESET LOOP ADDITION
    if (resetLoop->load())
    {
        double bufferLengthInSeconds = (double)bufferLength / sampleRate;
        double bps = (double)currentpositionstruct.bpm/60.0;
        double ppqPerBuffer = bps * bufferLengthInSeconds;
        double ppqOverSpill = (double)currentpositionstruct.ppqPosition - (double)currentpositionstruct.ppqPositionOfLastBarStart + ppqPerBuffer;
        double ppqFromReset = 4.0 -((double)currentpositionstruct.ppqPosition - (double)currentpositionstruct.ppqPositionOfLastBarStart);
        auto samplesLeft = ((ppqFromReset / 4.0) / barsPerMinute) * 60.0 * sampleRate;
        if (ppqOverSpill >= 4.0)
        {
            samplesFromRhythmStart = totalSampleLength - samplesLeft;
            resetLoop->store(false);
        }
    }
    /// _____________________________________________

    if ((samplesFromRhythmStart + (double)bufferLength) > totalSampleLength)
    {
        sampleSpan.first = samplesFromRhythmStart;
        sampleSpan.second = totalSampleLength;
        sampleSpanOverspill.first = 0;
        sampleSpanOverspill.second = samplesFromRhythmStart + bufferLength - totalSampleLength;
        sampleOverspill = true;

        samplesFromRhythmStart = 0 + sampleSpanOverspill.second;
    }
    else
    {
        sampleSpan.first = samplesFromRhythmStart;
        sampleSpan.second = samplesFromRhythmStart+bufferLength;
        sampleSpanOverspill.first = -1;
        sampleSpanOverspill.second = -1;
        sampleOverspill = false;
        samplesFromRhythmStart += bufferLength;
    }
    // Bar location at start of buffer
    //return (sampleSpan.first / totalSampleLength) * totalNumberOfBars;

}

void MidiController::applyMidiMessages(std::list<dbMidiMessage>& buffer)
{
    checkNoteOffMessages();

    for (auto&& message : bufferMidiMessages)
    {
        buffer.push_back(message);
    }
    bufferMidiMessages.clear();
}

void MidiController::addMidiMessage(dbMidiMessage& noteOnMessage, bars noteOnPosition, dbMidiMessage& noteOffMessage, bars noteOffPosition, bool sustain)
{
    // Currently checks twice but why
    auto noteOnSampleLocation = getLocation(noteOnPosition);
    if (noteOnSampleLocation == -1) return;

    noteOnMessage.setTimeStamp(noteOnSampleLocation);
    bufferMidiMessages.push_back(noteOnMessage);

    if (sustainToNextNote)
    {
        for (auto&& message : sustainedMidiMessages)
        {
            message.setTimeStamp(noteOnSampleLocation);
            if (message.getNoteNumber() != noteOnMessage.getNoteNumber()) bufferMidiMessages.push_back(message);
        }
        sustainedMidiMessages.clear();
        sustainToNextNote = false;
    }
   
    if (sustain)
    {
        sustainedMidiMessages.push_back(noteOffMessage);
        sustainToNextNote = true;
    }
    else noteOffMessages.push_back(std::make_pair(noteOffMessage, noteOffPosition));
}

void MidiController::checkNoteOffMessages()
{
    auto copyOfMessages{noteOffMessages};
    noteOffMessages.clear();
    for (auto&& message : copyOfMessages)
    {
        auto sampleLocation = getLocation(message.second);
        if (sampleLocation == -1) noteOffMessages.push_back(message);
        else
        {
            message.first.setTimeStamp(sampleLocation);
            bufferMidiMessages.push_back(message.first);
        }
    }
}

int MidiController::getLocation(bars barPosition)
{
    // THIS NEEDS RETHINK - WHY ARE ALL YOUR SAMPLE LOCATIONS FLOATS?

    // fractional part is the location within a bar
    // whole part is the bar number

    auto sampleLocation = (barPosition * totalSampleLength) / totalNumberOfBars;

    if (sampleLocation >= sampleSpan.first && sampleLocation < sampleSpan.second)
    {
        return std::round(sampleLocation - sampleSpan.first);
    }
    if (sampleLocation >= sampleSpanOverspill.first && sampleLocation < sampleSpanOverspill.second)
    {
        return std::round(sampleLocation);
    }
    else return -1;
}
