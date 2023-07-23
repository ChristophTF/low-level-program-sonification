fun void sonifyIoSyscalls(string address, float frequency)
{
    SinOsc osc => ADSR env => Gain gain => TaskSonification.out;
    frequency => osc.freq;
    (1::ms, 5::ms, 0, 1::ms) => env.set;

    OscIn oscIn;
    OscMsg oscMsg;

    OscAddress.PORT => oscIn.port;
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

spork ~ sonifyIoSyscalls("/io/syscalls/read", 4400);
sonifyIoSyscalls("/io/syscalls/write", 3200);