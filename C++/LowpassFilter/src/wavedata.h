/*******************************************************************************
 * IMS
 * Project Simulation of analog filter (Butterworth filter)
 * 
 * @file: waveData.h
 * @author Jan Bednarik (xbedna45)
 * @author Martin Janys (xjanys00)
 * @date 6.12.2012
 *
 * Declaration of class WaveData used for processing input WAV file.
 ******************************************************************************/

#ifndef WAVE_H
#define	WAVE_H

//#define FMT_CHUNK_SIZE 24


///////

#include "simlib.h"
 
#include <fstream>
#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define data_size subchunk_2_Size

using namespace std;

class WaveData : public aContiBlock {
  public:
    WaveData();
    void load(string name);
    int save(string name);
    void printHead();
    void multiple(double m);
    void set(int i, double val);
    double get(int i);
    void setChannel(int _num) { channel = _num; }
    double getPeriod() { return 1.0 / sampleRate; }
    int channelsNum() { return numChannels; }   // returns number of channels
    //double timeLen() { return data_size/numChannels/bitsPerSample/8.0/sampleRate; } // returns time length of wav signal
    double timeLen() { return data_size / (sampleRate * numChannels * (bitsPerSample/8.0));}   
    double Value();
    
    int32_t chunkID;    // "RIFF"
    int32_t chunkSize;  // in bytes
    int32_t format;     // "WAVE"

    /* Sub chunk 1 */

    int32_t subchunk_1_ID;    // "fmt "
    int32_t subchunk_1_Size;  // number of bytes
    int16_t audioFormat;      // 1 PCM (Pulse Code Modulation).
    int16_t numChannels;      // 1 = mono, 2 = stereo
    int32_t sampleRate;       // 11025 (phone), 22050 (hi-fi), 44100 (CD) (I'll be there for you 44100)
    int32_t byteRate;         // nAvgBytesPerSec - for PCM useless
    int16_t blockAlign;       // 
    int16_t bitsPerSample;    // 
    char* extraFormat;        // skip
    
    /* Sub chunk 2 */

    int32_t subchunk_2_ID;    // "data"
    int32_t subchunk_2_Size;  // number of bytes

    /* Data */

    char* data;
    
    /* aux */
    int extraFormatSize;
    int bytesPerSample;
    int channel;
};


#endif