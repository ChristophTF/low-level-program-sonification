UGen out => dac => WvOut waveOut => blackhole;
UGen rel_out => UGen volume_bridge => out;
3 => volume_bridge.op; // Multiply
Step overall_volume => LPF overall_volume_lpf => volume_bridge;
0 => overall_volume.next;
overall_volume_lpf => WvOut volumeWv => blackhole;
"volume.wav" => volumeWv.wavFilename;
100 => overall_volume_lpf.freq;
0.1 => overall_volume_lpf.Q;

//out => NRev reverb => dac;
//0.1 => reverb.mix;
"out.wav" => waveOut.wavFilename;

9000 => int PORT;



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

    PORT => oscIn.port;
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
    PORT => oscIn.port;
    oscIn.addAddress("/task-clock");

    while(true)
    {
        oscIn => now;
        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getFloat(0) => float utilization;
            // <<< "Utilization:", utilization >>>;
            utilization => overall_volume.next;
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
    PORT => oscIn.port;
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
                // <<< "IPC:", IPC, "frequency: ", frequency >>>;
            }
        }
    }
}

fun void sonifyL1ICacheMisses()
{
    OscIn oscIn;
    OscMsg oscMsg;
    PORT => oscIn.port;
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

fun void sonifyIoSyscalls(string address, float frequency)
{
    SinOsc osc => ADSR env => Gain gain => out;
    frequency => osc.freq;
    (1::ms, 5::ms, 0, 1::ms) => env.set;

    OscIn oscIn;
    OscMsg oscMsg;

    PORT => oscIn.port;
    oscIn.addAddress(address);

    while(true)
    {
        oscIn => now;
        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getInt(0) => int syscalls;

            if (syscalls > 0)
            {
                float loudness;
                100 => int MAX_SYSCALLS;
                if (syscalls > MAX_SYSCALLS)
                {
                    1 => loudness;
                }
                else
                {
                    syscalls => float syscalls_float;
                    syscalls_float / MAX_SYSCALLS => loudness;
                }

                Math.sqrt(loudness) => gain.gain;
                1 => env.keyOn;
            }
        }
    }
}

spork ~ sonifyCacheMisses("/cache/L3/misses", Std.mtof(48));
spork ~ sonifyCacheMisses("/cache/L2/misses", Std.mtof(52));
spork ~ sonifyCacheMisses("/cache/L1D/misses", Std.mtof(55));

spork ~ sonifyTaskClock();
spork ~ sonifyIPC();
spork ~ sonifyL1ICacheMisses();

spork ~ sonifyIoSyscalls("/io/syscalls/read", 4400);
spork ~ sonifyIoSyscalls("/io/syscalls/write", 3200);

1::week => now;