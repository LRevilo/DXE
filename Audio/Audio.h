#pragma once
#include "DXE.h"
#include "Logger.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <immintrin.h>
#include <audioclient.h>
#include <iostream>
#include <thread>
#include <functional>
#include <fstream>
#include <algorithm>
#include <atomic>


namespace DXE
{
   bool DXE_API LoadWavFile(const std::string& filename, std::vector<int16_t>& audioData);
   // free functions
   void DXE_API GenerateSineWave(int16_t* buffer, UINT32 frames);

   void DXE_API ConvertFloatToPCM16_AVX2(const float* input, int16_t* output, size_t sampleCount);

   void DXE_API ConvertPCM16ToFloat_AVX2(const int16_t* input, float* output, size_t sampleCount);

   // Copies float samples directly (assuming output is float*), for N channels
   void DXE_API ConvertFloatToFloat(const float* input, BYTE* output, size_t frameCount, int channels);
   // Converts float samples to 16-bit PCM (using your AVX2 function)
   void DXE_API ConvertFloatToPCM16(const float* input, BYTE* output, size_t frameCount, int channels);


   // Dummy fallback if unsupported format (fills silence)
   void DXE_API ConvertSilence(const float* input, BYTE* output, size_t frameCount, int channels);


    class DXE_API Audio {
    public:
        using AudioCallback = void(*)(float* buffer, UINT32 frames);
        using ConversionFunc = void(*)(const float* input, BYTE* output, size_t frameCount, int channels);


        Audio() : device(nullptr), audioClient(nullptr), renderClient(nullptr),
            isRunning(false), userCallback(nullptr) {
        }

        bool Initialise(uint32_t _bufferSize, bool exclusive = false) {
            CoInitialize(nullptr);

            // Get default audio device
            IMMDeviceEnumerator* enumerator = nullptr;
            HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
            if (FAILED(hr)) return Error("Failed to create device enumerator");

            hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
            enumerator->Release();
            if (FAILED(hr)) return Error("Failed to get default audio endpoint");

            // Activate IAudioClient
            hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient);
            if (FAILED(hr)) return Error("Failed to activate audio client");

            WAVEFORMATEX* formatToUse = nullptr;
            std::string mode_string = exclusive ? "Exclusive" : "Shared";
            AUDCLNT_SHAREMODE mode = exclusive ? AUDCLNT_SHAREMODE_EXCLUSIVE : AUDCLNT_SHAREMODE_SHARED;

            // Get system format (will be used as fallback and for shared mode)
            WAVEFORMATEX* nativeFormat = nullptr;
            hr = audioClient->GetMixFormat(&nativeFormat);
            if (FAILED(hr)) return Error("Failed to get device format");

            // Try to use 16-bit PCM format if in exclusive mode
            WAVEFORMATEXTENSIBLE pcmFormat = {};
            pcmFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
            pcmFormat.Format.nChannels = 2;
            pcmFormat.Format.nSamplesPerSec = 48000;
            pcmFormat.Format.wBitsPerSample = 16;
            pcmFormat.Format.nBlockAlign = pcmFormat.Format.nChannels * pcmFormat.Format.wBitsPerSample / 8;
            pcmFormat.Format.nAvgBytesPerSec = pcmFormat.Format.nSamplesPerSec * pcmFormat.Format.nBlockAlign;
            pcmFormat.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
            pcmFormat.Samples.wValidBitsPerSample = 16;
            pcmFormat.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
            pcmFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

            // Check if it's supported
            if (exclusive) {
                hr = audioClient->IsFormatSupported(mode, (WAVEFORMATEX*)&pcmFormat, nullptr);
                if (hr == S_OK) {
                    std::cout << "16-bit PCM supported (Exclusive Mode)" << std::endl;
                    formatToUse = (WAVEFORMATEX*)&pcmFormat;
                }
                else {
                    std::cout << "16-bit PCM NOT supported. Falling back to system format (Exclusive Mode)." << std::endl;
                    formatToUse = nativeFormat;
                }
            }
            else {
                // Always use system format in shared mode
                formatToUse = nativeFormat;
            }

            // Calculate buffer duration
            REFERENCE_TIME bufferDuration = (_bufferSize * 10'000'000LL) / formatToUse->nSamplesPerSec;

            hr = audioClient->Initialize(
                mode,
                AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                bufferDuration,
                exclusive ? bufferDuration : 0,  // Only set bufferDuration2 if exclusive
                formatToUse,
                nullptr
            );

            if (FAILED(hr)) {
                std::string error_string = "Failed to initialize audio client in " + mode_string + " Mode";
                return Error(error_string.c_str(), hr);
            }

            // Continue with setup
            audioClient->GetBufferSize(&bufferSize);
            DXE_INFO("Buffer size: ", bufferSize);

            hr = audioClient->GetService(__uuidof(IAudioRenderClient), (void**)&renderClient);
            if (FAILED(hr)) return Error("Failed to get render client");

            bufferEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (!bufferEvent) return Error("Failed to create buffer event");

            hr = audioClient->SetEventHandle(bufferEvent);
            if (FAILED(hr)) return Error("Failed to set event handle");

            // Free the system mix format if we didn't use it directly
            if (formatToUse != nativeFormat) {
                CoTaskMemFree(nativeFormat);
            }
            outputFormat = formatToUse;


            PrintWaveFormat(outputFormat);
            sampleRate = outputFormat->nSamplesPerSec;
            numChannels = outputFormat->nChannels;
            floatBuffer.resize(bufferSize * outputFormat->nChannels);
            SetConversionFunction();

            ringBufferSize = 48000 * numChannels; // 1 second buffer (you can tweak this)
            softwareRingBuffer.resize(ringBufferSize, 0.0f);
            ringBufferWritePos = 0;
            ringBufferReadPos = 0;


            return true;
        }

        void Start() {
            if (!audioClient) return;
            isRunning = true;
            std::thread audioThread(&Audio::AudioLoop, this);
            audioThread.detach();
            HRESULT hr = audioClient->Start();
            if (FAILED(hr)) { Error("Failed to start audio client"); isRunning = false; }
        }

        void Stop() {
            isRunning = false;
            if (audioClient) audioClient->Stop();
        }

        void SetConversionFunction() {
             int channels = outputFormat->nChannels;
             if (outputFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
                 converter = &DXE::ConvertFloatToFloat;
             }
             else if (outputFormat->wFormatTag == WAVE_FORMAT_PCM) {
                 converter = &DXE::ConvertFloatToPCM16;
             }
             else if (outputFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
                 const WAVEFORMATEXTENSIBLE* ext = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(outputFormat);
                 if (ext->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
                     converter = &DXE::ConvertFloatToFloat;
                 }
                 else if (ext->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
                     converter = &DXE::ConvertFloatToPCM16;
                 }
                 else {
                     converter = &DXE::ConvertSilence;  // Unsupported format fallback
                 }
             }
             else {
                 converter = &DXE::ConvertSilence;  // Unsupported format fallback
             }
         
        }

        // Set the callback function dynamically
        void SetCallback(AudioCallback callback) {
            userCallback = callback;
        }

        static void Shutdown() {

        }
        int GetBufferSize() { return bufferSize; }
        double GetSampleRate() { return sampleRate; }
        int GetNumChannels() { return numChannels; }
        void PrintWaveFormat(const WAVEFORMATEX* format) {
            std::cout << "=== Chosen Audio Format ===" << std::endl;
            std::cout << "Format Tag       : 0x" << std::hex << format->wFormatTag << std::dec << std::endl;
            std::cout << "Channels         : " << format->nChannels << std::endl;
            std::cout << "Sample Rate      : " << format->nSamplesPerSec << " Hz" << std::endl;
            std::cout << "Bits Per Sample  : " << format->wBitsPerSample << std::endl;
            std::cout << "Block Align      : " << format->nBlockAlign << std::endl;
            std::cout << "Avg Bytes/Sec    : " << format->nAvgBytesPerSec << std::endl;

            if (format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
                const WAVEFORMATEXTENSIBLE* ext = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(format);
                std::cout << "Valid Bits/Sample: " << ext->Samples.wValidBitsPerSample << std::endl;
                std::cout << "Channel Mask     : 0x" << std::hex << ext->dwChannelMask << std::dec << std::endl;

                if (ext->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
                    std::cout << "SubFormat        : PCM" << std::endl;
                else if (ext->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
                    std::cout << "SubFormat        : IEEE Float" << std::endl;
                else
                    std::cout << "SubFormat        : Unknown GUID" << std::endl;
            }

            std::cout << "============================" << std::endl;
        }

        ~Audio() {
            if (renderClient) { renderClient->Release(); }
            if (audioClient) {
                audioClient->Stop();
                audioClient->Release();
            }
            if (device) { device->Release(); }
            CloseHandle(bufferEvent);
            CoUninitialize();
        }

    private:
        AudioCallback userCallback = nullptr;
        ConversionFunc converter = nullptr;


        IMMDevice* device;
        IAudioClient* audioClient;
        IAudioRenderClient* renderClient;
        HANDLE bufferEvent;
        uint32_t bufferSize = 256;
        double sampleRate = 48000.0;
        uint32_t numChannels = 2;
        bool isRunning;



        WAVEFORMATEX* outputFormat = nullptr;
        std::vector<float> floatBuffer;

        public:
        std::vector<float> softwareRingBuffer;
        size_t ringBufferWritePos = 0;
        size_t ringBufferReadPos = 0;
        size_t ringBufferSize = 48000 * numChannels; // 1 second of audio, for example

        void FillRingBuffer() {
            const size_t chunkFrames = 2048;
            const size_t chunkSamples = chunkFrames * numChannels;

            if (!userCallback) return;

            // Only write if there's room
            size_t spaceAvailable = (ringBufferReadPos + ringBufferSize - ringBufferWritePos - 1) % ringBufferSize;
            if (spaceAvailable < chunkSamples) return;

            std::vector<float> tempBuffer(chunkSamples);
            userCallback(tempBuffer.data(), chunkFrames);

            for (size_t i = 0; i < chunkSamples; ++i) {
                softwareRingBuffer[ringBufferWritePos] = tempBuffer[i];
                ringBufferWritePos = (ringBufferWritePos + 1) % ringBufferSize;
            }
        }

        struct Sound {
            std::vector<int16_t> data;
            size_t position = 0;
            bool looping = false;
            float volume = 1.0f;
            std::function<void(int16_t*, size_t)> effect = nullptr;
        };
        std::unordered_map<int, Sound> sounds;


        void AudioLoop() {
            while (isRunning) {
                WaitForSingleObject(bufferEvent, INFINITE);
                FillAudioBuffer();
            }
        }

        void FillAudioBuffer() {
            if (!audioClient || !renderClient || !outputFormat || !converter) return;

            HRESULT hr;
            UINT32 paddingFrames = 0, bufferFrameCount = 0;
            BYTE* data = nullptr;

            hr = audioClient->GetCurrentPadding(&paddingFrames);
            if (FAILED(hr)) return;

            hr = audioClient->GetBufferSize(&bufferFrameCount);
            if (FAILED(hr)) return;

            UINT32 availableFrames = bufferFrameCount - paddingFrames;
            if (availableFrames == 0) return;

            hr = renderClient->GetBuffer(availableFrames, &data);
            if (FAILED(hr)) return;

            size_t numSamples = availableFrames * numChannels;

            // 1. Fill temporary float buffer with user audio or silence
            if (userCallback) {
                userCallback(floatBuffer.data(), availableFrames);
            }
            else {
                std::fill(floatBuffer.begin(), floatBuffer.begin() + numSamples, 0.0f);
            }

            // 2. Convert floatBuffer to device format using the pre-set converter function
            converter(floatBuffer.data(), data, availableFrames, numChannels);

            hr = renderClient->ReleaseBuffer(availableFrames, 0);
            if (FAILED(hr)) return;
        }
        void FillAudioBufferEx() {
            if (!audioClient || !renderClient || !outputFormat || !converter) return;

            HRESULT hr;
            UINT32 paddingFrames = 0, bufferFrameCount = 0;
            BYTE* data = nullptr;

            hr = audioClient->GetCurrentPadding(&paddingFrames);
            if (FAILED(hr)) return;

            hr = audioClient->GetBufferSize(&bufferFrameCount);
            if (FAILED(hr)) return;

            UINT32 availableFrames = bufferFrameCount - paddingFrames;
            if (availableFrames == 0) return;

            hr = renderClient->GetBuffer(availableFrames, &data);
            if (FAILED(hr)) return;

            const size_t numSamples = availableFrames * numChannels;

            size_t dataAvailable = (ringBufferWritePos + ringBufferSize - ringBufferReadPos) % ringBufferSize;
            if (dataAvailable < numSamples) {
                // Not enough data, fill output buffer with silence
                std::fill(floatBuffer.begin(), floatBuffer.begin() + numSamples, 0.0f);
            }
            else {
                for (size_t i = 0; i < numSamples; ++i) {
                    floatBuffer[i] = softwareRingBuffer[ringBufferReadPos];
                    ringBufferReadPos = (ringBufferReadPos + 1) % ringBufferSize;
                }
            }

            // Then convert floatBuffer to device format using converter
            converter(floatBuffer.data(), data, availableFrames, numChannels);

            hr = renderClient->ReleaseBuffer(availableFrames, 0);
            if (FAILED(hr)) return;
        }
        bool Error(const char* msg, HRESULT hr = S_OK) {
            std::cerr << msg << "\n";
            if (hr != S_OK) {
                std::cerr << "Error Code: " << std::hex << hr << std::dec << std::endl;
            }
            return false;
        }

        void LoadSound(const std::string& filename, int soundID) {
            std::vector<int16_t> audioData;
            if (!LoadWavFile(filename, audioData)) {
                return;
            }
        }
    };




}