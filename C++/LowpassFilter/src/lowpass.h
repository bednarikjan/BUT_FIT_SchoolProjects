/*******************************************************************************
 * IMS
 * Project Simulation of analog filter (Butterworth filter)
 * 
* @file: lowpass.h 
 * @author Jan Bednarik (xbedna45)
 * @author Martin Janys (xjanys00)
 * @date 6.12.2012
 *
 * Declaration of class InputSignal and SinWave.
 ******************************************************************************/

#ifndef LOWPASS_H
#define	LOWPASS_H

#include "simlib.h"
#include "math.h"

const double PI = 3.141592653589793;

/**
 * Wrapper for input signal
 */
class InputSignal: public aContiBlock {
public:
    aContiBlock *In;
    
    double Value() {
        return In->Value();
    }
};

/**
 * Sinus signal input
 */
class SinWave : public aContiBlock {
private:
    double a;   // amplitude
    double f;   // frequency

public:
    // constructors
    SinWave(): a(0), f(0) {}
    SinWave(double _a, double _f): a(_a), f(_f) {}
    
    // sets amplitude
    void setSinAmplitude(double _a) { a = _a; }
    // sets frequency
    void setSinFreq(double _f) { f = _f; }
    // returns sin value according to time
    double Value() { return a * sin(2 * PI * f * T.Value()); }
};

#endif	/* LOWPASS_H */

