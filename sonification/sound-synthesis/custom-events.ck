fun void waitForMessage(string address, float freq)
{
    SqrOsc osc => ADSR env => TaskSonification.out;
    1 => osc.gain;
    freq => osc.freq;
    (1::ms, 20::ms, 0, 1::ms) => env.set;

    OscIn oscIn;
    OscMsg oscMsg;

    OscAddress.PORT => oscIn.port;
    oscIn.addAddress(address);

    while(true)
    {
        oscIn => now;

        while(oscIn.recv(oscMsg) != 0)
        {
            1 => env.keyOn;
        }
    }
}

spork ~ waitForMessage("/custom1", Std.mtof(70));
waitForMessage("/custom2", Std.mtof(77));