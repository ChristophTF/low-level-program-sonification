public class SpinningOsc extends Chugraph
{
    inlet => blackhole;
    Modulate modulator => UGen modulationMultiplier => UGen freqAdder => outlet;
    inlet => Gain modulationMultiplierGain => modulationMultiplier;
    1 / 60.0 => modulationMultiplierGain.gain;
    inlet => freqAdder;

    1 => freqAdder.op; // Sum inputs
    3 => modulationMultiplier.op; // Multiply inputs

    1 => modulator.vibratoGain;
    2 => modulator.randomGain;

    fun void refreshVibratoRate()
    {
        while(true)
        {
            Math.log(inlet.last() + 1) * 4 - 20 => float vibratoRate => modulator.vibratoRate;
            1::samp => now;
        }
    }

    spork ~ refreshVibratoRate();
}