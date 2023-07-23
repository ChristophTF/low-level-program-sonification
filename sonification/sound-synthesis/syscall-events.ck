fun void hitADSR(ADSR adsr, dur sustainT)
{
    1 => adsr.keyOn;
    sustainT => now;
    0 => adsr.keyOn;
}

fun void sonifySyscalls(string address, float frequency)
{
    SinOsc osc;
    0.5 => osc.gain;
    UGen out => BPF hpf => Dyno limiter => TaskSonification.out;
    frequency => hpf.freq;
    4 => hpf.Q;

    ADSR env[100];
    for (0 => int i; i < env.size(); i++)
    {
        frequency => osc.freq;
        (1::ms, 1::ms, 1, 10::ms) => env[i].set;
        osc => env[i] => out;
    }


    OscIn oscIn;
    OscMsg oscMsg;

    OscAddress.PORT => oscIn.port;
    oscIn.addAddress(address);

    0 => int index;

    while(true)
    {
        oscIn => now;

        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getInt(0) => int count;
            oscMsg.getInt(1) => int bytes;
            // <<< address, count, "bytes:", bytes >>>;

            Math.sqrt(count) => env[index].gain;
            spork ~ hitADSR(env[index], (bytes / count / 8192)::samp + 1::samp);
            (index + 1) % env.size() => index;
        }
    }
}

spork ~ sonifySyscalls("/io/syscalls/read", Std.mtof(60));
sonifySyscalls("/io/syscalls/write", Std.mtof(53));