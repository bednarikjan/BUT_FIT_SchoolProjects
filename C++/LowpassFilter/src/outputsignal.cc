/*******************************************************************************
 * IMS
 * Project Simulation of analog filter (Butterworth filter)
 * 
 * @file: outputsignal.cc
 * @author Jan Bednarik (xbedna45)
 * @author Martin Janys (xjanys00)
 * @date 6.12.2012
 *
 * Implementation of class OutputSignal used for frequency response analysis.
 ******************************************************************************/

#include "outputsignal.h"
#include <iostream>

/*
 * Initializes the object properties
 */
void OutputSignal::init(double _stepsPerPeriod) {
    stepsPerPeriod = _stepsPerPeriod;    
    inState = outState = sample1;        
    inAmplitude = outAmplitude = 0.0;
    inAmplitudeIdx = outAmplitudeIdx = 0.0;
}


/*
 * Counts one step to get amplitude and phase shift of output signal.
 */
void OutputSignal::step(double in, double out) {
    oneStep(in, inAmplitude, inState);
    oneStep(out, outAmplitude, outState);
    if(inState  != done) { inAmplitudeIdx++;  }
    if(outState != done) { outAmplitudeIdx++; }
}

void OutputSignal::oneStep(double sig, double& amplitude, sigState& state) {
    switch(state) {
        // first sample (signal might be increasing or decreasing)
        case sample1:
            amplitude = sig;
            state = sample2;
            break;
            
        // second sample (now we find out if signal is increasing/decreasing)
        case sample2:
            state = (sig < amplitude) ? decr : incr;                             
            amplitude = sig; 
            break;

        // increasing function
        case incr:
            if(sig > amplitude) {
                amplitude = sig;
            } else {
                state = done;
            }
            break;
         
        // decreasing function
        case decr:
            if(sig > amplitude) { state = incr; }
            amplitude = sig;
            break;
            
        // amplitude already found
        case done:
            break;
    }
}

/*
 * Returns phase shift of output signal in degrees.
 */
double OutputSignal::getPhase() {
    return (inAmplitudeIdx - outAmplitudeIdx) * (360.0 / stepsPerPeriod);    
}