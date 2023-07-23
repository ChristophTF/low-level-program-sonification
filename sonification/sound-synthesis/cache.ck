fun void sonifyCacheMisses(string address, float frequency)
{
    SqrOsc osc => Envelope env => TaskSonification.rel_out;
    10::ms => env.duration;
    3 => osc.gain;
    frequency => osc.freq;

    OscIn oscIn;
    OscMsg oscMsg;

    OscAddress.PORT => oscIn.port;
    oscIn.addAddress(address);

    while(true)
    {
        oscIn => now;

        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getFloat(0) => float intensity;
            intensity => env.target;
        }
    }
}

spork ~ sonifyCacheMisses("/cache/L3/misses", Std.mtof(48));
spork ~ sonifyCacheMisses("/cache/L2/misses", Std.mtof(51));
spork ~ sonifyCacheMisses("/cache/L1D/misses", Std.mtof(54));

1::week => now;