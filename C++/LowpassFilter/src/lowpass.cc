/*******************************************************************************
 * IMS
 * Project Simulation of analog filter (Butterworth filter)
 * 
 * @file: lowpass.cc 
 * @author Jan Bednarik (xbedna45)
 * @author Martin Janys (xjanys00)
 * @date 6.12.2012
 *
 * Main source file. Includes Butterworth filter scheme. Calculates
 * frequency response and processes input WAV file.
 ******************************************************************************/

#include "simlib.h"
#include "lowpass.h"
#include "wavedata.h"
#include "outputsignal.h"
#include <iostream>

using namespace std;

// Need to skip couple of periods before the signal gets stable
double startSampleTime = 0.0;

/* mode o operation:
 *      freqAnalysis - Calculates frequency response of lowpass filter
 *      wavFiltering - Runs WAV file through filter
 */
enum ESampleMode {freqAnalysis, wavFiltering} sampleMode;

SinWave sw;     // input sinus signal (for frequency response measurement)
OutputSignal os;// ouput sinus signal amplitude and phase shift
WaveData wd;    // input WAV file

class LowpassFilter {  
  public:
    
    // Resistors
    Parameter R1;
    Parameter R2;
    
    // Capacitors
    Parameter C1;
    Parameter C3;
    Parameter C5;
    
    // Inductors
    Parameter L2;
    Parameter L4;
      
    InputSignal U;
    Integrator X, Y, Z, I2, I3;
    Expression I1, I4;
    
  LowpassFilter(double _R1, double _R2, 
                double _C1, double _C3, double _C5, 
                double _L2, double _L4):    
    R1(_R1), R2(_R2),
    C1(_C1), C3(_C3), C5(_C5),
    L2(_L2), L4(_L4),
                
    X((I1-I2)/C1),
    Y((I2-I3)/C3),
    Z((I3-I4)/C5),
  
    I1((U-X)/R1),
    I2((X-Y)/L2),
    I3((Y-Z)/L4),
    I4(Z/R2)        
    {}

    double Uout() { return I4.Value() * R2.Value(); }
    double Uin()  { return U.Value(); }                    
};

////////////////////////////////////////////////////////////////////////////
// Values for Butterworth filter

LowpassFilter o(1000.0 /* Ohm */  , 1000.0 /* Ohm */,
                106e-9 /* Farad */, 344e-9 /* Farad */, 106e-9 /* Farad */,
                278e-3 /* Henry */, 278e-3 /* Henry */);
////////////////////////////////////////////////////////////////////////////

void Sample() { 
    switch (sampleMode) {
        case freqAnalysis:
            if (T.Value() >= startSampleTime) {
                os.step(o.Uin(), o.Uout());
            }
            break;
            
        case wavFiltering:
            wd.set((int)(T.Value() * wd.sampleRate), o.Uout());            
            break;
    }        
}

Sampler S(Sample); // sampling object

/**
 * Calculates and prints data necessary for ploting the 
 * bode plot (gain and phase response).
 * Otput format(each line):
 * 
 * frequency gain(V) gain(dBV) phase(degrees)
 * 
 * @param fMin frequency range minimum limit
 * @param fMax frequency range maximum limit
 * @param fStep step while increasing frequency
 * @param amplitude input sin wave amplitude
 * @param accuracy sample steps per sin wave period
 */
void bodePlotData(double fMin, 
                  double fMax, 
                  double fStep, 
                  double amplitude,
                  double accuracy) {    
    
    double period;      // current signal period
    double phase = 0.0;
    double lastPhase = 0.0;
    
    ((SinWave*)(o.U.In))->setSinAmplitude(amplitude);  // set input sin singal amplitude
    
    for(double freq = fMin; freq <= fMax; freq += fStep) {
        cout << "\r" << "freq: " << freq << flush;
        period = 1.0/freq;
        startSampleTime = period*160.0;         // skip 160 periods until signal is stable        
        ((SinWave*)(o.U.In))->setSinFreq(freq); // set input signal frequency
        os.init(accuracy);                      // initialization of output signal object        
        SetStep(period/10000.0, period/10.0);   // simulation step  
        SetAccuracy(1e-6, 0.001);               // simulation accuracy
        S.SetStep(period/accuracy);             // sampling step
        Init(0, period*165);                    // simulation time range        
        Run();
        
        phase = os.getPhase();
        
        // adjusting the phase value so that it would continuously decrease
        if(freq != fMin) {            
            while(phase > lastPhase) { phase -= 360.0; }
        }
        
        lastPhase = phase;
        
        Print("%5g %14g %10g %8g\n", freq, 
                               os.getAmplitude(), 
                               20*log10(os.getAmplitude()/amplitude), 
                               phase);
    }
}

/**
 * Run input wav file through simluator and save the result
 * 
 * @param name File name
 */
void wavFilter(string name) {
    double timeStep; 
    string outName = name;
        
    wd.load(name);    
    timeStep = wd.getPeriod();
    
    for(int i = 0; i < wd.channelsNum(); i++) {
        wd.setChannel(i);
        SetStep(timeStep/100.0,timeStep);// simulation step  
        SetAccuracy(1e-6, 0.001);        // simulation accuracy
        S.SetStep(timeStep);             // sampling step
        Init(0, wd.timeLen());           // simulation time range
        Run();                
    }
    
    outName.insert(outName.length()-4,"-out");
    
    wd.save(outName);
}

int main() {     
    // run options
    bool freqRes = true;
    bool wavFilt = true;    
    
if(freqRes) {
    /* Frequency response */
    /**************************************************************************/
    
    sampleMode = freqAnalysis;
    o.U.In = &sw;    
    
    double fMin = 1.0;     
    double fMax = 20000.0; 
    double fStep = 1.0;    
    double amplitude = 10.0;
    double stepsPerPeriod = 100.0;      // accuracy - samples per sinus period                    
    
    SetOutput("bode_plot.dat");            // output file   
    
    bodePlotData(fMin, fMax, fStep, amplitude, stepsPerPeriod);            
}
    
if(wavFilt) {
    /* WAV file filtering */
    /**************************************************************************/
    
    sampleMode = wavFiltering;        
    o.U.In = &wd;
    
    string inputFileName = "../wav/drums.wav";
    
    wavFilter(inputFileName);
}   
    return 0;
}