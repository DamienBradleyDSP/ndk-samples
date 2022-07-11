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

package com.example.nativemidi;

import android.media.midi.MidiDevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;
import android.media.midi.MidiInputPort;
import android.content.Context;
import android.content.res.AssetManager;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;

import java.util.ArrayList;

public class AppMidiManager {
    private static final String TAG = AppMidiManager.class.getName();
    private MidiManager mMidiManager;
    private MidiDevice mSendDevice; // an "Input" device is one we will SEND data TO
    private boolean mUseRunningStatus = true;

    public AppMidiManager(MidiManager midiManager) {
        mMidiManager = midiManager;
    }

    public MidiManager GetMidiManager() {
        return mMidiManager;
    }

    public void ScanMidiDevices(ArrayList<MidiDeviceInfo> sendDevices) {
        sendDevices.clear();
        MidiDeviceInfo[] devInfos = mMidiManager.getDevices();
        for(MidiDeviceInfo devInfo : devInfos) {
            int numInPorts = devInfo.getInputPortCount();
            String deviceName =
                    devInfo.getProperties().getString(MidiDeviceInfo.PROPERTY_NAME);
            if (deviceName == null) {
                continue;
            }
            if (numInPorts > 0) {
                sendDevices.add(devInfo);
            }
        }
    }

    public class OpenMidiSendDeviceListener implements MidiManager.OnDeviceOpenedListener {
        @Override
        public void onDeviceOpened(MidiDevice device) {
            mSendDevice = device;
            startWritingMidi(mSendDevice, 0/*mPortNumber*/);
        }
    }

    public void openSendDevice(MidiDeviceInfo devInfo) {
        mMidiManager.openDevice(devInfo, new OpenMidiSendDeviceListener(), null);
    }

    public void closeSendDevice() {
        if (mSendDevice != null) {
            // Native API
            mSendDevice = null;
        }
    }
    // OPENSL ES SETUP
    void createOpenSLEngine(Context context)
    {
        createEngine();

        int sampleRate = 0;
        int bufSize = 0;

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            AudioManager myAudioMgr = (AudioManager) context.getSystemService(context.AUDIO_SERVICE);
            String nativeParam = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            sampleRate = Integer.parseInt(nativeParam);
            nativeParam = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
            bufSize = Integer.parseInt(nativeParam);
        }
        createBufferQueueAudioPlayer(sampleRate, bufSize);
    }

    public static void loadNativeAPI() {
        System.loadLibrary("native_midi");
    }
    public native void startWritingMidi(MidiDevice sendDevice, int portNumber);
    public native void stopWritingMidi();

    public static native void createEngine();
    public static native void createBufferQueueAudioPlayer(int sampleRate, int samplesPerBuf);
    public static native void shutdown();
    
}
