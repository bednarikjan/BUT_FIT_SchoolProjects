/*******************************************************************************
 * IMS
 * Project Simulation of analog filter (Butterworth filter)
 * 
 * @file: waveData.cc
 * @author Jan Bednarik (xbedna45)
 * @author Martin Janys (xjanys00)
 * @date 6.12.2012
 *
 * Implementation of class WaveData used for processing input WAV file.
 ******************************************************************************/
 
#include "wavedata.h"

/**
 * Constructor.
 * Read wav file. 
 */
WaveData::WaveData() {
    
}

/**
 * Loads wav file
 * @param name file name
 */
void WaveData::load(string name) {
    FILE* f = fopen(name.c_str(), "rb");
    if (!f) {
        cerr << "An error occurred during opening the file" << endl;
        exit(1);
    }
    
    /* RIFF chunk descriptor */
  fread(&this->chunkID, 1,    4, f); // "RIFF"
  fread(&this->chunkSize, 1,  4, f); // in bytes
  fread(&this->format, 1,     4, f); // "WAVE"
  
  /* Sub chunk 1 */
  
  fread(&this->subchunk_1_ID, 1,    4, f); // "fmt "
  fread(&this->subchunk_1_Size, 1,  4, f); // number of bytes
  fread(&this->audioFormat, 1,      2, f); // 1 PCM (Pulse Code Modulation).
  fread(&this->numChannels, 1,      2, f); // 1 = mono, 2 = stereo
  fread(&this->sampleRate, 1,       4, f); // 11025 (phone), 22050 (hi-fi), 44100 (CD) (I'll be there for you 44100)
  fread(&this->byteRate, 1,         4, f); // nAvgBytesPerSec - for PCM useless
  fread(&this->blockAlign, 1,       2, f); // 
  fread(&this->bitsPerSample, 1,    2, f); //
  
  this->extraFormatSize = this->subchunk_1_Size - 16; // 16 prev. data of chunk
  this->extraFormat = new char[extraFormatSize];
  fread(this->extraFormat, 1, extraFormatSize, f);
  
  /* skip to sub chunk 2 */
  char buffer[4];
  while(fread(&buffer[0], 1, 1, f) && !feof(f)) {
    
    if (buffer[0] != 'd') // D
      continue;

    if (fread(&buffer[1], 1, 1, f) && buffer[1] != 'a') // A
      continue;
    else if (fread(&buffer[2], 1, 1, f) && buffer[2] != 't') // T
      continue;
    else if (fread(&buffer[3], 1, 1, f) && buffer[3] != 'a') // A
      continue;
    else {
      memcpy(&this->subchunk_2_ID, buffer, 4);
      break;
    }
  }

  /* Sub chunk 2 */
  
  //fread(this->subchunk_2_ID,    4); // "data"
  fread(&this->subchunk_2_Size, 1,  4, f); // number of bytes
  
  /* Data */
  
  this->data = new char[this->subchunk_2_Size];
  fread(this->data, 1, subchunk_2_Size, f);
  
  /* aux */
  this->bytesPerSample = 0;
  do {
    this->bytesPerSample++;
  } while (this->bytesPerSample * 8 < bitsPerSample);
  
  this->channel = 0;
  /* TODO: Validate header */
}

/**
 * Save wav file.
 */
int WaveData::save(string name) {

  FILE* f = fopen(name.c_str(), "wb");
  if (!f) {
    cerr << "An error occurred opening the output file" << endl;
    return 0;
  }
  
  /* RIFF chunk descriptor */
  fwrite(&this->chunkID, 1,    4, f); // "RIFF"
  fwrite(&this->chunkSize, 1,  4, f); // in bytes
  fwrite(&this->format, 1,     4, f); // "WAVE"
  
  /* Sub chunk 1 */
  
  fwrite(&this->subchunk_1_ID, 1,    4, f); // "fmt "
  fwrite(&this->subchunk_1_Size, 1,  4, f); // number of bytes
  fwrite(&this->audioFormat, 1,      2, f); // 1 PCM (Pulse Code Modulation).
  fwrite(&this->numChannels, 1,      2, f); // 1 = mono, 2 = stereo
  fwrite(&this->sampleRate, 1,       4, f); // 11025 (phone), 22050 (hi-fi), 44100 (CD) (I'll be there for you 44100)
  fwrite(&this->byteRate, 1,         4, f); // nAvgBytesPerSec - for PCM useless
  fwrite(&this->blockAlign, 1,       2, f); // 
  fwrite(&this->bitsPerSample, 1,    2, f); //
  fwrite(this->extraFormat, 1, this->extraFormatSize, f); // 
  
  /* Sub chunk 2 */
  
  fwrite(&this->subchunk_2_ID, 1, 4, f);     // "data"
  fwrite(&this->subchunk_2_Size, 1,  4, f); // number of bytes
  
  /* Data */
  
  fwrite(this->data, 1, subchunk_2_Size, f);
  
  return 1;
}

/**
 * Prints .wav header
 */
void WaveData::printHead() {
  cout << "WAV Header" << endl;
  cout << "ID   " << "HEX\tvalue" << endl; 
  cout << "---- --------\t-----" << endl; 
  cout << "RIFF " << hex << chunkID << (memcmp(&chunkID, "RIFF", 4) == 0 ? "\tOK" : "\tBAD") << endl;    // "RIFF"
  cout << "size " << hex << chunkSize << "\t" << dec << chunkSize << endl;  // in bytes
  cout << "WAVE " << hex << format << (memcmp(&format, "WAVE", 4) == 0 ? "\tOK" : "\tBAD") << endl;     // "WAVE"

  /* Sub chunk 1 */

  cout << "fmt  " << hex << subchunk_1_ID << (memcmp(&subchunk_1_ID, "fmt ", 4) == 0 ? "\tOK" : "\tBAD") << endl;    // "fmt "
  cout << "size " << hex << subchunk_1_Size << "\t\t" << dec << subchunk_1_Size << endl;  // number of bytes
  cout << "audi " << hex << audioFormat << "\t\t" << dec << audioFormat << endl;      // 1 PCM (Pulse Code Modulation).
  cout << "chan " << hex << numChannels << "\t\t" << dec << numChannels << endl;      // 1 = mono, 2 = stereo
  cout << "srat " << hex << sampleRate << "\t" << dec << sampleRate << endl;       // 11025 (phone), 22050 (hi-fi), 44100 (CD) (I'll be there for you 44100)
  cout << "brat " << hex << byteRate << "\t" << dec << byteRate << endl;         // nAvgBytesPerSec - for PCM useless
  cout << "alig " << hex << blockAlign << "\t\t" << dec << blockAlign << endl;       // 
  cout << "bpse " << hex << bitsPerSample << "\t\t" << bitsPerSample << endl;    // 
  //cout <<  extraFormat << endl;      // skip
  
  /* Sub chunk 2 */

  cout << "data " << hex << subchunk_2_ID  << (memcmp(&subchunk_2_ID, "data", 4) == 0 ? "\tOK" : "\tBAD") << endl;    // "data"
  cout << "size " << hex << subchunk_2_Size << "\t" << dec << subchunk_2_Size << endl;  // number of bytes
}

/**
 * Returns sample specifed by index i as double.
 */
double WaveData::get(int i) {
  
  /* index of sample in channel */
  i = this->bytesPerSample * (this->numChannels * i + channel);
  
  int8_t sample[this->bytesPerSample];
  
  memcpy(sample, this->data + i, this->bytesPerSample);
  
  long long result = 0;
  int8_t byte;
  
  /* binary to number */
  for (int j = 0; j < this->bytesPerSample; j++) {
    byte = sample[j];
    result += byte << (8*j);
  }
  
  return (double)result;
  
  /*
  switch (bytesPerSample) {
    case 1:
      return (double)*((int8_t*)(this->data) + i);
      break;
    case 2:
      return (double)*((int16_t*)(this->data) + i);
      break;
    case 3:
    case 4:
      return (double)*((int32_t*)(this->data) + i);
      break;
  }
  */
}

/**
 * Set sample specifed by index i to value val.
 */
void WaveData::set(int i, double val) {
  
  /* index of sample in channel */
  i = this->bytesPerSample * (this->numChannels * i + channel);
  
  long long var = val;
  int8_t sample[this->bytesPerSample];
  int8_t byte;
  
  /* number to binary */
  for (int j = 0; j < this->bytesPerSample; j++) {
    byte = var & 0xff;
    var >>= 8;
    sample[j] = byte;
  }
  
  memcpy((this->data) + i, sample, this->bytesPerSample);
  
  /*
  switch (bytesPerSample) {
    case 1:
      val = min((double)0xff, val);
      *((int8_t*)(this->data) + i * this->channel) = val;
      break;
    case 2:
      val = min((double)0xffff, val); 
      *((int16_t*)(this->data) + i * this->channel) = val;
      break;
    case 3:
    case 4:
      val = min((double)0xffffffff, val); 
      *((int32_t*)(this->data) + i * this->channel) = val;
      break;
  }
  */
}

/**
 * Multiple all samples.
 */
void WaveData::multiple(double m) {
  
  for (int i = 0; i < this->data_size / numChannels; i += this->bytesPerSample) {
    this->set(i/bytesPerSample, this->get(i/bytesPerSample) * m);
  }
}

double WaveData::Value() {
  double x0, y0, x1, y1, x;
  
  x0 = (int)(T.Value() * this->sampleRate);
  x1 = x0 + 1;
  
  y0 = this->get(x0);
  y1 = this->get(x1);
  
  x = (T.Value() * this->sampleRate);
  
  double result = y0 + (x - x0) * ((y1 - y0)/(x1 - x0));
  
  return result;
}

//int main (int argc, char** argv) {
//  //WaveData wd("godzilla.wav");
//  WaveData wd("FriendsOriginal.wav");
//  //WaveData wd("laugh.wav");
//  wd.printHead();
//  wd.multiple(0.1);
//  //wd.setChannel(1);
//  //wd.multiple(0.8);
//  wd.save("out.wav");
//}

