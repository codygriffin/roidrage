//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

/*

#include <RoidRage.h>

#include <AudioState.h>
#include <Log.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <ivorbisfile.h>
#include <ivorbiscodec.h>

#include <stdlib.h>
#include <stdexcept>
#include <cmath>

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;

//------------------------------------------------------------------------------

#define SAWTOOTH_FRAMES 950
// XXX legacy
short                         sawtoothBuffer[SAWTOOTH_FRAMES];
short                         musicBuffer[48000];

AudioEngine::AudioEngine() {
  SLresult result;

  // create audio engine
  result = slCreateEngine(&engineObject_, 0, NULL, 0, NULL, NULL);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error creating audio engine object");
  }

  // realize the engine
  result = (*engineObject_)->Realize(engineObject_, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error realizing audio engine");
  }

  // get the engine interface, which is needed in order to create other objects
  result = (*engineObject_)->GetInterface(engineObject_, SL_IID_ENGINE, &engine_);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error retrieving audio interface");
  }

  // create output mix
  result = (*engine_)->CreateOutputMix(engine_, &outputMixObject_, 0, 0, 0);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error creating output mix");
  }

  // realize the output mix
  result = (*outputMixObject_)->Realize(outputMixObject_, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error realizing output mix");
  }
}

SLEngineItf& AudioEngine::getEngine() {
  return engine_;
}

SLObjectItf& AudioEngine::getOutputMix() {
  return outputMixObject_;
}


AudioPlayer::AudioPlayer(AudioEngine engine, 
                         SLuint32 numBuffers,
                         SLuint32 numChannels,
                         SLuint32 sampleRate,
                         SLuint32 bitsPerSample,
                         SLuint32 endianness) {
  SLresult result;
  minVol = -4000;
  maxVol = 200;

  SLDataLocator_AndroidSimpleBufferQueue bufferQueue = {
    SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, // locator type (matches the datatype)
    2                                        // number of buffers to allocate
  };

  SLDataFormat_PCM pcmFormat = {
    pcm,                      // format type
    numChannels,              // num channels
    sampleRate,               // sample rate (in mHz)
    bitsPerSample,            // bits per sample
    bitsPerSample,            // container size 
    SL_SPEAKER_FRONT_CENTER,  // channel mask
    endianness                // endiandess
  };

  SLDataSource source = {
    &bufferQueue, // source location
    &pcmFormat    // source format
  };

  // configure audio sink
  SLDataLocator_OutputMix mix = {
    SL_DATALOCATOR_OUTPUTMIX, // locator type (matches the datatype)
    engine.getOutputMix()     // output mix object
  };

  SLDataSink sink = {
    &mix, // sink location
    0           // sink format (ignored for output mix)
  };

  // create audio player
  const SLInterfaceID ids[2] = {
    SL_IID_BUFFERQUEUE, 
    SL_IID_VOLUME
  };
  const SLboolean req[2] = {
    SL_BOOLEAN_TRUE, 
    SL_BOOLEAN_TRUE,
  };

  result = (*engine.getEngine())->CreateAudioPlayer(  
    engine.getEngine(), 
    &playerObject_, 
    &source, 
    &sink,
    2, // require only bufferqueue and volume for now 
    ids, 
    req
  );
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error creating audio player");
  }

  // realize the player
  result = (*playerObject_)->Realize(playerObject_, SL_BOOLEAN_FALSE);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error realizing audio player");
  }

  // get the play interface
  result = (*playerObject_)->GetInterface(playerObject_, SL_IID_PLAY, &player_);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error getting audio player's play interface");
  }

  // get the buffer queue interface
  result = (*playerObject_)->GetInterface(playerObject_, SL_IID_BUFFERQUEUE,
          &queue_);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error getting audio player's buffer queue interface");
  }

  // register callback on the buffer queue
  result = (*queue_)->RegisterCallback(queue_, AudioPlayer::callback, this);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error registering audio player's buffer queue callback");
  }

  // get the volume interface
  result = (*playerObject_)->GetInterface(playerObject_, SL_IID_VOLUME, &volume_);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error getting audio player's volume interface");
  }

  // get the volume interface
  //result = (*playerObject_)->GetInterface(playerObject_, SL_IID_SEEK, &seeker_);
  //if (result != SL_RESULT_SUCCESS) //{
  //  throw std::runtime_error("error getting audio player's seeker interface");
  //}
}

void AudioPlayer::callback(SLAndroidSimpleBufferQueueItf queue, void *context) {
  AudioPlayer* pPlayer = reinterpret_cast<AudioPlayer*>(context);

  if (queue != pPlayer->queue_) {
    throw std::runtime_error("random-ass buffer queue in callback...");
  }

  if (pPlayer->bufferCallback_) {
    pPlayer->bufferCallback_();
  }
}

void
AudioPlayer::setVolume(float vol) {
  SLresult result;

  auto volume = minVol + (vol * (maxVol - minVol));
  if (volume < minVol + 100) {
    volume = -8000;
  }

  // get the volume interface
  result = (*volume_)->SetVolumeLevel(volume_, volume);
  Log::info("volume = %dBm [min=%dBm, max=%dBm)", volume, minVol, maxVol);
  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error setting audio player's volume");
  }
}

void
AudioPlayer::pew(unsigned f) {
  SLresult result;

  const float pi = 3.14159265f;
  for (unsigned i = 0; i < SAWTOOTH_FRAMES; ++i) {
    float  env = std::max(
      (float)(0.5f * cos(-((float)(SAWTOOTH_FRAMES)/3.0f) + 3.0f*pi*i/float(2*SAWTOOTH_FRAMES)) + 0.5f), 
      0.0f
    );

    //f *= abs(sin(i));
    unsigned dev = ((i % std::max(unsigned(1), unsigned(env*f)) * 20));
    dev = dev * env;
    sawtoothBuffer[i] = 32768 - dev;
  }

  result = (*queue_)->Enqueue(queue_, sawtoothBuffer, sizeof(sawtoothBuffer));
  if (result != SL_RESULT_SUCCESS) {
    //throw std::runtime_error("error enqueing buffer");
    return;
  }

  // set the player's state to playing
  result = (*player_)->SetPlayState(player_, SL_PLAYSTATE_PLAYING);
  if (result != SL_RESULT_SUCCESS) {
    //Log::warn( "error setting audio player's play state");
    return;
  }
}

void
AudioPlayer::music() {
  SLresult result;
  static int offset = 0;
  memcpy(musicBuffer, pRoidRage->music.get() + offset, sizeof(musicBuffer));
  offset += sizeof(musicBuffer);
  if (offset > 15600385) offset = 0;

  result = (*queue_)->Enqueue(queue_, 
                              musicBuffer, 
                              sizeof(musicBuffer));

  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error enqueing buffer");
    return;
  }

  // set the player's state to playing
  result = (*player_)->SetPlayState(player_, SL_PLAYSTATE_PLAYING);
  if (result != SL_RESULT_SUCCESS) {
    //Log::warn( "error setting audio player's play state");
    return;
  }
}

//------------------------------------------------------------------------------

void
AudioPlayer::loop(char* pBuffer, size_t length, bool first) {
  SLresult result;
  static int offset = 0;
  
  if (first) {
    if (pBuffer_ == pBuffer) {
      return;
    }

    pBuffer_ = pBuffer;

    bufferCallback_ = [this, pBuffer, length]() {
      loop(pBuffer, length, false);
    };

    // set the player's state to stopped
    result = (*player_)->SetPlayState(player_, SL_PLAYSTATE_STOPPED);

    if (result != SL_RESULT_SUCCESS) {
      throw std::runtime_error("error stopping player");
      return;
    }

    // clear
    result = (*queue_)->Clear(queue_);

    if (result != SL_RESULT_SUCCESS) {
      throw std::runtime_error("error clearing buffer");
      return;
    }

    offset = 0;
  }

  memcpy(musicBuffer, pBuffer + offset, sizeof(musicBuffer));
  offset += sizeof(musicBuffer);
  
  int fullchunks = length / sizeof(musicBuffer);
  int partial    = length % sizeof(musicBuffer);

  if (offset > fullchunks * sizeof(musicBuffer)) {
    offset = 0;
    length = partial;
  } else {
    length = sizeof(musicBuffer);
  }

  result = (*queue_)->Enqueue(queue_, 
                              musicBuffer, 
                              length);

  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error enqueing buffer");
    return;
  }

  // set the player's state to playing
  result = (*player_)->SetPlayState(player_, SL_PLAYSTATE_PLAYING);

  if (result != SL_RESULT_SUCCESS) {
    //Log::warn( "error setting audio player's play state");
    return;
  }
}

void 
AudioPlayer::stop() {
  SLresult result;
  // set the player's state to stopped
  result = (*player_)->SetPlayState(player_, SL_PLAYSTATE_STOPPED);

  if (result != SL_RESULT_SUCCESS) {
    throw std::runtime_error("error stopping player");
    return;
  }
}

//------------------------------------------------------------------------------


static unsigned int suiCurrPos = 0;
static unsigned int suiSize = 0;

static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
{
    unsigned int uiBytes = std::min(suiSize - suiCurrPos, (unsigned int)nmemb * (unsigned int)size);
    memcpy(ptr, (unsigned char*)datasource + suiCurrPos, uiBytes);
    suiCurrPos += uiBytes;

    return uiBytes;
}

static int seek_func(void* datasource, ogg_int64_t offset, int whence)
{
    if (whence == SEEK_SET)
        suiCurrPos = (unsigned int)offset;
    else if (whence == SEEK_CUR)
        suiCurrPos = suiCurrPos + (unsigned int)offset;
    else if (whence == SEEK_END)
        suiCurrPos = suiSize;

    return 0;
}

static int close_func(void* datasource)
{
    return 0;
}

static long tell_func(void* datasource)
{
    return (long)suiCurrPos;
}

std::unique_ptr<char[]>
AudioEngine::oggToPcm(unsigned int uiOggSize, void* pvOggBuffer)
{
    // Register callbacks
    OggVorbis_File vf;
    ov_callbacks callbacks;
    callbacks.read_func  = &read_func;
    callbacks.seek_func  = &seek_func;
    callbacks.close_func = &close_func;
    callbacks.tell_func  = &tell_func;

    // Init Tremor
    suiCurrPos = 0;
    suiSize = uiOggSize;
    int iRet = ov_open_callbacks(pvOggBuffer, &vf, NULL, 0, callbacks);

    // Get sample info
    vorbis_info* vi = ov_info(&vf, -1);
    unsigned int uiPCMSamples = (unsigned int)ov_pcm_total(&vf, -1);

    auto bufferSize  = uiPCMSamples * vi->channels * sizeof(short);
    auto pvPCMBuffer = std::unique_ptr<char[]>(new char[bufferSize]);

    // Decode!
    int current_section = 0;
    long iRead = 0;
    unsigned int uiCurrPos = 0;
    do
    {
        iRead = ov_read(&vf, (char*)pvPCMBuffer.get() + uiCurrPos, 4096, &current_section);
        uiCurrPos += (unsigned int)iRead;
    }
    while (iRead != 0);

    // Cleanup
    ov_clear(&vf);
    
    return pvPCMBuffer;
}

*/
