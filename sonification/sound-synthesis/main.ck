UGen out => dac;
UGen rel_out => Envelope volume_bridge => out;
5::ms => volume_bridge.duration;
0.1 => out.gain;

SpinningOsc3 ipcSpinner => TriOsc ipcOsc => rel_out;
2 => ipcOsc.sync; // FM




fun void sonifyCacheMisses(string address, float frequency)
{
    SqrOsc osc => Envelope env => rel_out;
    10::ms => env.duration;
    2 => osc.gain;
    frequency => osc.freq;

    OscIn oscIn;
    OscMsg oscMsg;

    OscAddress.PORT() => oscIn.port;
    oscIn.addAddress(address);

    while(true)
    {
        oscIn => now;

        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getFloat(0) => float intensity;
            // <<< address, "Intensity:", intensity >>>;
            intensity => env.target;
        }
    }
}

fun void sonifyTaskClock()
{
    OscIn oscIn;
    OscMsg oscMsg;
    OscAddress.PORT() => oscIn.port;
    oscIn.addAddress("/task-clock");

    while(true)
    {
        oscIn => now;
        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getFloat(0) => float utilization;
            // <<< "Utilization:", utilization >>>;
            utilization => volume_bridge.target;
        }
    }
}

Step spinnerBaseFreqStep => Envelope spinnerBaseFreqEnv;
1 => spinnerBaseFreqStep.next;
50::ms => spinnerBaseFreqEnv.duration;

fun void refreshSpinnerFrequency()
{
    spinnerBaseFreqEnv => blackhole;
    while(true)
    {
        spinnerBaseFreqEnv.last() => ipcSpinner.baseFreq;
        // <<< "frequency: ", spinnerBaseFreqEnv.last() >>>;
        1::samp => now;
    }
}

fun void sonifyIPC()
{
    OscIn oscIn;
    OscMsg oscMsg;
    OscAddress.PORT() => oscIn.port;
    oscIn.addAddress("/IPC");

    spork ~ refreshSpinnerFrequency();

    while(true)
    {
        oscIn => now;
        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getFloat(0) => float cycles;
            oscMsg.getFloat(1) => float instructions;

            instructions / cycles => float IPC;
            IPC * 1000 => float frequency;

            // Exclude NaN and inf
            if (frequency == frequency && 1.0 / frequency != 0)
            {
                frequency => spinnerBaseFreqEnv.target;
                // ipcSpinner.baseFreq;
                <<< "IPC:", IPC, "frequency: ", frequency >>>;
            }
        }
    }
}

fun void sonifyL1ICacheMisses()
{
    OscIn oscIn;
    OscMsg oscMsg;
    OscAddress.PORT() => oscIn.port;
    oscIn.addAddress("/cache/L1I/misses");

    1 => ipcSpinner.randomGain;

    while(true)
    {
        oscIn => now;
        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getFloat(0) => float misses;
            misses * 3000 => float randomGain;
            // <<< "randomGain:", randomGain >>>;
            randomGain => ipcSpinner.randomGain;
        }
    }
}


spork ~ sonifyCacheMisses("/cache/L3/misses", Std.mtof(48));
spork ~ sonifyCacheMisses("/cache/L2/misses", Std.mtof(52));
spork ~ sonifyCacheMisses("/cache/L1D/misses", Std.mtof(55));

spork ~ sonifyTaskClock();
spork ~ sonifyIPC();
spork ~ sonifyL1ICacheMisses();

1::week => now;