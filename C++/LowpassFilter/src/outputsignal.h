/*******************************************************************************
 * IMS
 * Project Simulation of analog filter (Butterworth filter)
 * 
 * @file: outputsignal.h
 * @author Jan Bednarik (xbedna45)
 * @author Martin Janys (xjanys00)
 * @date 6.12.2012
 *
 * Declaration of class OutputSignal used for frequency response analysis.
 ******************************************************************************/

#ifndef OUTPUTSIGNAL_H
#define	OUTPUTSIGNAL_H

/*
 * Class representing the properties output signal. It calculates 
 * output signal amplitude and the phase shift.
 */
class OutputSignal {
public:
    enum sigState {sample1, sample2, incr, decr, done};
    
    OutputSignal() :
        stepsPerPeriod(0),
        inAmplitude(0.0),
        outAmplitude(0.0),
        inState(sample1),
        outState(sample1),
        shift360(0),
        negativeShift(true) {}
    void init(double _timeStep);
    void step(double in, double out);
    double getAmplitude () { return outAmplitude; }
    double getPhase ();  
            
private:
    int stepsPerPeriod;
    
    double inAmplitude; // input signal amplitude
    double outAmplitude;// output signal amplitude    
    
    sigState inState;
    sigState outState;            
    
    int inAmplitudeIdx; // sample index of amplitude
    int outAmplitudeIdx;// sample index of amplitude
    
    int shift360;       // how many 360 degree shifts signal phase has
    bool negativeShift; // negative value of phase shift -> true
    
    void oneStep(double sig, double& amplitude, sigState& state);
};

#endif	/* OUTPUTSIGNAL_H */

