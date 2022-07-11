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

package com.example.nativemidi; // Namespace and program package - see folder name

import android.app.Activity;
import android.content.Context;

import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;

import android.os.Bundle;

import android.view.View;

import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import android.os.Handler;

import java.util.ArrayList;

/**
 * Application MainActivity handles UI and Midi device hotplug event from
 * native side.
 */
public class MainActivity extends Activity // Main entry point for app
 {

    private static final String TAG = MainActivity.class.getName();;

    private AppMidiManager mAppMidiManager;

    // Connected devices - List format - will be populated?
    private ArrayList<MidiDeviceInfo> mReceiveDevices = new ArrayList<MidiDeviceInfo>();
    private ArrayList<MidiDeviceInfo> mSendDevices = new ArrayList<MidiDeviceInfo>();

    // Force to load the native library
    static {
        AppMidiManager.loadNativeAPI();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) { // Entry point on app opening
        super.onCreate(savedInstanceState); // Running parent class onCreates
        setContentView(R.layout.activity_main); // Setting content view to our resource file

        MidiManager midiManager = (MidiManager) getSystemService(Context.MIDI_SERVICE);
        midiManager.registerDeviceCallback(new MidiDeviceCallback(), new Handler());

        mAppMidiManager = new AppMidiManager(midiManager);

        ScanMidiDevices();

        mAppMidiManager.createOpenSLEngine(this);

    }

    private class MidiDeviceCallback extends MidiManager.DeviceCallback {
        @Override
        public void onDeviceAdded(MidiDeviceInfo device) {
            ScanMidiDevices();
        }

        @Override
        public void onDeviceRemoved(MidiDeviceInfo device) {
            ScanMidiDevices();
        }
    }

    private void ScanMidiDevices() {
        mAppMidiManager.ScanMidiDevices(mSendDevices);
        if(mSendDevices.size()>0) mAppMidiManager.openSendDevice(mSendDevices.get(0));
    }
}
