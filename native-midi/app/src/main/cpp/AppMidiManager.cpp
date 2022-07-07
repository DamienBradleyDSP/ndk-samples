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

static AMidiDevice* sNativeSendDevice = NULL;
static AMidiInputPort* sMidiInputPort = NULL;

static pthread_t sWriteThread;
static std::atomic<bool> sWriting(false);

#if 0
// unblock this method if logging of the midi messages is required.
/**
 * Formats a midi message set and outputs to the log
 * @param   timestamp   The timestamp for when the message(s) was received
 * @param   dataBytes   The MIDI message bytes
 * @params  numDataBytew    The number of bytes in the MIDI message(s)
 */
static void logMidiBuffer(int64_t timestamp, uint8_t* dataBytes, size_t numDataBytes) {
#define DUMP_BUFFER_SIZE    1024
    char midiDumpBuffer[DUMP_BUFFER_SIZE];
    memset(midiDumpBuffer, 0, sizeof(midiDumpBuffer));
    int pos = snprintf(midiDumpBuffer, DUMP_BUFFER_SIZE,
            "%" PRIx64 " ", timestamp);
    for (uint8_t *b = dataBytes, *e = b + numDataBytes; b < e; ++b) {
        pos += snprintf(midiDumpBuffer + pos, DUMP_BUFFER_SIZE - pos,
                "%02x ", *b);
    }
    LOGD("%s", midiDumpBuffer);
}
#endif

//
// JNI Functions
//
extern "C" {

static void* writeThreadRoutine(void * context) {
    (void)context;

    uint8_t* byteArray = new uint8_t[3]{0x90, 0x3C, 0x60};
    bool test = false;
    while (sWriting)
    {
        // Poll the midi buffer here, copy and flush - record the time

        // GET TIME OUTSIDE THIS LOOP - then increment a set amount each time
        //auto next = std::chrono::system_clock::now();
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        test = !test;
        if(test) now = now + 100;
        /*ssize_t numSent =*/ AMidiInputPort_sendWithTimestamp(sMidiInputPort, byteArray, 3,now);
        // sleep until recorded time + buffer length

        struct timespec sleepTime;
        struct timespec returnTime;
        sleepTime.tv_sec = 1;
        sleepTime.tv_nsec = 0;
        nanosleep(&sleepTime, &returnTime);
    }
    delete[] byteArray;
    return NULL;
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


    sWriting = true;
    pthread_create(&sWriteThread, NULL, writeThreadRoutine, NULL);


}

void Java_com_example_nativemidi_AppMidiManager_stopWritingMidi(JNIEnv*, jobject) {
    /*media_status_t status =*/ AMidiDevice_release(sNativeSendDevice);
    sNativeSendDevice = NULL;
}


} // extern "C"
