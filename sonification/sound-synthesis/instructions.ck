SpinningOsc3 ipcSpinner => TriOsc ipcOsc => TaskSonification.rel_out;
2 => ipcOsc.sync; // FM

Step constOneStep;
1 => constOneStep.next;

constOneStep => Envelope spinnerBaseFreqEnv;
10::ms => spinnerBaseFreqEnv.duration;
constOneStep => Envelope randomGainEnv;
10::ms => randomGainEnv.duration;

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
                // <<< "IPC:", IPC >>>;
            }
        }
    }
}

fun void refreshRandomGain()
{
    randomGainEnv => blackhole;
    while(true)
    {
        randomGainEnv.last() => ipcSpinner.randomGain;
        1::samp => now;
    }
}

fun void sonifyL1ICacheMisses()
{
    OscIn oscIn;
    OscMsg oscMsg;
    OscAddress.PORT => oscIn.port;
    oscIn.addAddress("/cache/L1I/misses");

    1 => ipcSpinner.randomGain;

    spork ~ refreshRandomGain();

    while(true)
    {
        oscIn => now;
        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getFloat(0) => float misses;
            // <<< "I$-Misses per Instruction:", misses * 100, "%" >>>;
            misses * 5000 => randomGainEnv.target;
        }
    }
}

spork ~ sonifyIPC();
spork ~ sonifyL1ICacheMisses();

1::week => now;