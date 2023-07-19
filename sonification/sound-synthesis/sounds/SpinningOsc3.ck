public class SpinningOsc3 extends Chugraph
{
    Modulate modulator => Gain modulationMultiplier => UGen freqAdder => outlet;
    1 => freqAdder.op; // Sum inputs
    Step baseFreqStep => freqAdder;

    1 => modulator.vibratoGain;
    0 => modulator.randomGain;

    fun void baseFreq(float baseFreq)
    {
        baseFreq => baseFreqStep.next;
        baseFreq / 60.0 => modulationMultiplier.gain;
        Math.log(baseFreq + 1) * 4 - 20 => float vibratoRate;
        vibratoRate => modulator.vibratoRate;
    }

    fun void randomGain(float randomGain)
    {
        randomGain => modulator.randomGain;
    }
}