class SpringGain extends Chugen
{
    fun float tick(float x)
    {
        (1 - x * 0.9) => float y;
        return y * y + 0.1;
    }
}

public class Spring extends Chugraph
{
    SinOsc osc => UGen multiply => outlet;
    3 => multiply.op;
    Phasor lfo => SpringGain sg => LPF lpf => multiply;

    400 => lpf.freq;
    80 => lfo.freq;

    1200 => osc.freq;
    2 => osc.sync; // FM
    SinOsc lfo2 => osc;
    200 => lfo2.gain;
    10 => lfo2.freq;
}