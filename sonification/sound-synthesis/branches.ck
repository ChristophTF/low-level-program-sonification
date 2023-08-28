Spring spring => Envelope volumeEnv => TaskSonification.rel_out;
10::ms => volumeEnv.duration;
1 => volumeEnv.target;

OscIn oscIn;
OscMsg oscMsg;

OscAddress.PORT => oscIn.port;
oscIn.addAddress("/branches");

while(true)
{
    oscIn => now;

    while(oscIn.recv(oscMsg) != 0)
    {
        oscMsg.getFloat(0) => float branches;
        oscMsg.getFloat(1) => float branch_misses;

        if (branches != 0)
        {
            branch_misses / branches => float miss_rate;
            miss_rate * 8 => volumeEnv.target;
            // <<< "Branch misses:", (miss_rate * 100), "%" >>>;
        }
    }
}