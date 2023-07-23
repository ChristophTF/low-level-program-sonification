SndBuf spring => LPF lpf => Envelope volumeEnv => TaskSonification.rel_out;
"synth_spring.wav" => spring.read;
10::ms => volumeEnv.duration;
1 => volumeEnv.target;

2 => lpf.Q;
1 => spring.loop;
1.5 => spring.freq;

OscIn oscIn;
OscMsg oscMsg;

OscAddress.PORT => oscIn.port;
oscIn.addAddress("/branches");

Step constOne => Envelope lpfFreqEnv => blackhole;
1 => constOne.next;
10::ms => lpfFreqEnv.duration;

fun void refreshLpfFreq()
{
    while(true)
    {
        lpfFreqEnv.last() => lpf.freq;
        1::samp => now;
    }
}

spork ~ refreshLpfFreq();

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
            miss_rate * 2000 => lpfFreqEnv.target;
            miss_rate * 5 => volumeEnv.target;
            // <<< "Branch misses:", (miss_rate * 100), "%" >>>;
        }
    }
}