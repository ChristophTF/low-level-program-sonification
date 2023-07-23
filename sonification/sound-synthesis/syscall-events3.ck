fun void sonifyIoSyscalls(string address, float frequency)
{
    Impulse impulse => dac;

    OscIn oscIn;
    OscMsg oscMsg;

    OscAddress.PORT() => oscIn.port;
    oscIn.addAddress(address);

    0 => int index;

    while(true)
    {
        oscIn => now;
        while(oscIn.recv(oscMsg) != 0)
        {
            oscMsg.getInt(0) => int count;
            oscMsg.getInt(1) => int bytes;
            <<< address, ":", bytes, "/", count >>>;

            if (count > 0)
            {
                count => impulse.next;
            }
        }
    }
}

spork ~ sonifyIoSyscalls("/io/syscalls/read", 4400);
sonifyIoSyscalls("/io/syscalls/write", 4400 / 1.5);