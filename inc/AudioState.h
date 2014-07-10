//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_AUDIOSTATE_H
#define INCLUDED_AUDIOSTATE_H

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------
/*
struct AudioEngine {
  AudioEngine();
  
  static std::unique_ptr<char[]> oggToPcm(unsigned int uiOggSize, void* pvOggBuffer);
  SLEngineItf& getEngine();
  SLObjectItf& getOutputMix();

//private:
  SLObjectItf engineObject_;
  SLObjectItf outputMixObject_;

  SLEngineItf engine_;
};

//------------------------------------------------------------------------------

struct AudioPlayer {
  typedef std::function<void(void)> Callback;

  AudioPlayer(AudioEngine engine, 
              SLuint32 numBuffers    = 2,
              SLuint32 numChannels   = 1,
              SLuint32 sampleRate    = AudioPlayer::rate8k,
              SLuint32 bitsPerSample = AudioPlayer::pcm16,
              SLuint32 endianness    = AudioPlayer::littleEndian);
  void setVolume(float vol);
  void mute(bool muted);

  // XXX replace w/ generic enqueue
  void pew(unsigned f = 10);
  void music();

  void loop(char* pBuffer, size_t length, bool first = true);
  void stop();

  static void callback(SLAndroidSimpleBufferQueueItf queue, 
                       void*                         pContext);

  SLObjectItf                   playerObject_;
  SLPlayItf                     player_;
  SLSeekItf                     seeker_;
  SLAndroidSimpleBufferQueueItf queue_;
  SLVolumeItf                   volume_;
  Callback                      bufferCallback_;

  int   minVol;
  int   maxVol;
  char* pBuffer_;

  static const SLuint32 pcm          = SL_DATAFORMAT_PCM;
  static const SLuint32 rate8k       = SL_SAMPLINGRATE_8;
  static const SLuint32 rate48k      = SL_SAMPLINGRATE_48;
  static const SLuint32 pcm16        = SL_PCMSAMPLEFORMAT_FIXED_16;
  static const SLuint32 littleEndian = SL_BYTEORDER_LITTLEENDIAN;
  static const SLuint32 bigEndian    = SL_BYTEORDER_BIGENDIAN;
};
*/
//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
