SpinningOsc3 ipcSpinner => TriOsc ipcOsc => TaskSonification.rel_out;
2 => ipcOsc.sync; // FM


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
    OscAddress.PORT => oscIn.port;
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
                <<< "IPC:", IPC >>>;
            }
        }
    }
}

fun void sonifyL1ICacheMisses()
{
    OscIn oscIn;
    OscMsg oscMsg;
    OscAddress.PORT => oscIn.port;
    oscIn.addAddress("/cache/L1I/misses");

    1 => ipcSpinner.randomGain;

    while(true)
    {
        oscIn => now;
        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getFloat(0) => float misses;
            misses * 10000 => float randomGain;
            // <<< "randomGain:", randomGain >>>;
            randomGain => ipcSpinner.randomGain;
        }
    }
}

spork ~ sonifyIPC();
spork ~ sonifyL1ICacheMisses();

1::week => now;