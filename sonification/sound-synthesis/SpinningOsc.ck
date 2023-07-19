public class SpinningOsc extends Chugraph
{
    inlet => Gain modulatorFreqGain => SinOsc modulator => UGen modulationMultiplier => UGen freqAdder => outlet;
    inlet => Gain modulationMultiplierGain => modulationMultiplier;
    1 / 60.0 => modulationMultiplierGain.gain;
    inlet => freqAdder;
    60.0 / 4186.01 => modulatorFreqGain.gain;

    2 => modulator.sync; // FM
    1 => freqAdder.op; // Sum inputs
    0 => modulator.freq;
    3 => modulationMultiplier.op; // Multiply inputs
}