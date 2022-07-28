/*
  ==============================================================================

    ProjectSettings.h
    Created: 15 Mar 2020 12:00:08am
    Author:  damob

  ==============================================================================
*/

#pragma once

/*
APVTS Names

User Settings:        


*/

namespace ProjectSettings
{
	//Functional Settings 

	constexpr int startingNumberOfBeats = 4;
	constexpr int startingNumberOfBars = 1;
	constexpr int startingSemitone = 0;
	constexpr int startingOctave = 0;
	constexpr int startingNumberOfRhythmModules = 1;
	constexpr int minNumberOfRhythmModules = 1;
	constexpr int maxNumberOfRhythmModules = 32;
	constexpr int minNumberOfBeats = 1;
	constexpr int maxNumberOfBeats = 32;
	constexpr int minNumberOfBars = 1;
	constexpr int maxNumberOfBars = 8;


	constexpr int midiChannel = 1;
	constexpr int midiNote = 36;
	constexpr float midiVelocity = 0.5;

	
};

typedef double samples; // Numerical value representing samples, can be decimal
typedef double bars; // Numerical value representing number of bars, can be decimal