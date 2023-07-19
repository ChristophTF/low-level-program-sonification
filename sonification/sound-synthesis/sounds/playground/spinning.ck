Impulse spinningFreq => SpinningOsc spinner => SinOsc osc => dac;
2 => osc.sync; // FM

SawOsc cpiOsc => blackhole;
0.1 => cpiOsc.freq;
0.75 => cpiOsc.phase;

while(true)
{
    Math.pow(10, (cpiOsc.last() - 1) / 2) * 4186.01 => float f;
    f => spinningFreq.next;
    1::samp => now;
}

1::week => now;