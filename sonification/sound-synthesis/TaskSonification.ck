public class TaskSonification
{
    static UGen @out;
    static UGen @rel_out;
}

UGen out @=> TaskSonification.out;
UGen rel_out @=> TaskSonification.rel_out;


Envelope volume_bridge;
rel_out => volume_bridge;
volume_bridge => out => dac;

5::ms => volume_bridge.duration;
0.1 => out.gain;

fun void sonifyTaskClock()
{
    OscIn oscIn;
    OscMsg oscMsg;
    OscAddress.PORT => oscIn.port;
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

sonifyTaskClock();