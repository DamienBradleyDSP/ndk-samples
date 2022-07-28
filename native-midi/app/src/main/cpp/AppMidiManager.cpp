/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <atomic>
#include <inttypes.h>
#include <stdio.h>
#include <string>
#include <chrono>
#include <thread>

#include <jni.h>

#include <pthread.h>

#define LOG_TAG "AppMidiManager-JNI"
#include "AndroidDebug.h"

#include <amidi/AMidi.h>

#include "MidiSpec.h"
#include "MidiEngine.h"
#include <jni.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>


static AMidiDevice* sNativeSendDevice = NULL;
static AMidiInputPort* sMidiInputPort = NULL;

//                  SL ES SETUP
// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interface
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
static const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;


// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

static SLmilliHertz bqPlayerSampleRate = 0;
static jint   bqPlayerBufSize = 0;

static pthread_mutex_t  engineMutexLock = PTHREAD_MUTEX_INITIALIZER;

static short *nextBuffer;

static MidiEngine midiEngine;
static uint8_t* byteArray;
static double maximumMidiBitsPerBuffer;
static int maximumMidiBytesPerBuffer;
const int midiBaudRate = 31250;

//
// JNI Functions
//
extern "C" {

void midiEngineCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);

    //uint8_t* byteArray = new uint8_t[3]{0x90, 0x3C, 0x60};

    midiEngine.generateMidi(byteArray, 12);
    AMidiInputPort_send(sMidiInputPort, byteArray, 12);

    // what happens in the case of a midi device change or diconnection?

    (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, bqPlayerBufSize);
}

void Java_com_example_nativemidi_AppMidiManager_startWritingMidi(
        JNIEnv* env, jobject, jobject midiDeviceObj, jint portNumber) {

    media_status_t status;
    status = AMidiDevice_fromJava(env, midiDeviceObj, &sNativeSendDevice);
    // int32_t deviceType = AMidiDevice_getType(sNativeReceiveDevice);
    // ssize_t numPorts = AMidiDevice_getNumInputPorts(sNativeSendDevice);

    AMidiInputPort *inputPort;
    status = AMidiInputPort_open(sNativeSendDevice, portNumber, &inputPort);
    // sMidiInputPort.store(inputPort);
    sMidiInputPort = inputPort;
}

void Java_com_example_nativemidi_AppMidiManager_stopWritingMidi(JNIEnv*, jobject) {
    /*media_status_t status =*/ AMidiDevice_release(sNativeSendDevice);
    sNativeSendDevice = NULL;
}




JNIEXPORT void JNICALL
Java_com_example_nativemidi_AppMidiManager_createEngine(JNIEnv*,jclass)
{
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
    result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
            outputMixEnvironmentalReverb, &reverbSettings);
    (void)result;
    }
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example
}

JNIEXPORT void JNICALL Java_com_example_nativemidi_AppMidiManager_createBufferQueueAudioPlayer(JNIEnv, jclass, jint sampleRate, jint bufSize)
{
    SLresult result;
    if (sampleRate >= 0 && bufSize >= 0 ) {
        bqPlayerSampleRate = sampleRate * 1000;
        bqPlayerBufSize = bufSize;
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    /*
     * Enable Fast Audio when possible:  once we set the same rate to be the native, fast audio path
     * will be triggered
     */
    if(bqPlayerSampleRate) {
        format_pcm.samplesPerSec = bqPlayerSampleRate;       //sample rate in mili second
    }
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ };

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                bqPlayerSampleRate? 2 : 3, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, midiEngineCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    short* buffer = new short[bqPlayerBufSize];
    for(int i = 0; i<bqPlayerBufSize; i++) buffer[i] = 0;

    nextBuffer = buffer;
    (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, bqPlayerBufSize);
    (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, bqPlayerBufSize);

    midiEngine.initialise(sampleRate,bqPlayerBufSize);
    maximumMidiBitsPerBuffer = (double)midiBaudRate/((double)sampleRate/(double)bqPlayerBufSize);
    maximumMidiBytesPerBuffer = (int)(maximumMidiBitsPerBuffer/8.0);

    byteArray = new uint8_t[maximumMidiBytesPerBuffer];

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
}

JNIEXPORT void JNICALL
Java_com_example_nativemidi_AppMidiManager_shutdown(JNIEnv*, jclass) {
// destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
    (*bqPlayerObject)->Destroy(bqPlayerObject);
    bqPlayerObject = NULL;
    bqPlayerPlay = NULL;
    bqPlayerBufferQueue = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
    (*outputMixObject)->Destroy(outputMixObject);
    outputMixObject = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
    (*engineObject)->Destroy(engineObject);
    engineObject = NULL;
    engineEngine = NULL;
    }

    delete nextBuffer;
    delete byteArray;

    midiEngine.shutDown();

    pthread_mutex_destroy(&engineMutexLock);
}

} // extern "C"

