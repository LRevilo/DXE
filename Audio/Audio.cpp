#include "pch.h"
#include "Audio.h"
namespace DXE
{

    bool LoadWavFile(const std::string& filename, std::vector<int16_t>& audioData) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Could not open WAV file: " << filename << std::endl;
            return false;
        }

        struct WavHeader {
            char riff[4];
            uint32_t chunkSize;
            char wave[4];
            char fmt[4];
            uint32_t subchunk1Size;
            uint16_t audioFormat;
            uint16_t numChannels;
            uint32_t sampleRate;
            uint32_t byteRate;
            uint16_t blockAlign;
            uint16_t bitsPerSample;
            char dataHeader[4];
            uint32_t dataSize;
        };

        WavHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));

        if (std::string(header.riff, 4) != "RIFF" || std::string(header.wave, 4) != "WAVE") {
            std::cerr << "Error: Invalid WAV file format." << std::endl;
            return false;
        }
        // 1 = PCM, 
        // 3 = IEEE Float – 32-bit floating point audio
        if (header.audioFormat != 1 || header.bitsPerSample != 16) {
            std::cerr << "Error: Only PCM 16-bit WAV files are supported." << std::endl;
            return false;
        }

        audioData.resize(header.dataSize / sizeof(int16_t));
        file.read(reinterpret_cast<char*>(audioData.data()), header.dataSize);
        return true;
    }

    void ConvertFloatToPCM16_AVX2(const float* input, int16_t* output, size_t sampleCount) {
        constexpr float scale = 32767.0f;
        const __m256 mulFactor = _mm256_set1_ps(scale);
        const __m256 minVal = _mm256_set1_ps(-32768.0f);
        const __m256 maxVal = _mm256_set1_ps(32767.0f);

        size_t i = 0;
        size_t vectorizedSamples = sampleCount & ~7; // Multiple of 8

        for (; i < vectorizedSamples; i += 8) {
            __m256 floatSamples = _mm256_loadu_ps(&input[i]);       // Load 8 floats
            floatSamples = _mm256_mul_ps(floatSamples, mulFactor);  // Scale to PCM16 range
            floatSamples = _mm256_max_ps(minVal, floatSamples);     // Clip to min
            floatSamples = _mm256_min_ps(maxVal, floatSamples);     // Clip to max

            __m256i intSamples = _mm256_cvtps_epi32(floatSamples);  // Convert to int32
            __m128i packedSamples = _mm_packs_epi32(
                _mm256_extractf128_si256(intSamples, 0),
                _mm256_extractf128_si256(intSamples, 1)
            ); // Pack 32-bit ints into 16-bit shorts

            _mm_storeu_si128(reinterpret_cast<__m128i*>(&output[i]), packedSamples); // Store result
        }

        // Handle remaining samples (non-multiple of 8)
        for (; i < sampleCount; ++i) {
            output[i] = static_cast<int16_t>(std::clamp(input[i] * scale, -32768.0f, 32767.0f));
        }
    }

    void ConvertPCM16ToFloat_AVX2(const int16_t* input, float* output, size_t sampleCount) {
        constexpr float scale = 1.0f / 32768.0f;
        const __m256 mulFactor = _mm256_set1_ps(scale); // Set scale factor to convert from 16-bit to float

        size_t i = 0;
        size_t vectorizedSamples = sampleCount & ~7; // Multiple of 8

        for (; i < vectorizedSamples; i += 8) {
            // Load 8 16-bit PCM samples
            __m128i pcmSamplesLow = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[i]));
            __m128i pcmSamplesHigh = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[i + 4]));

            // Convert to 32-bit integers (sign-extended from 16-bit PCM)
            __m256i intSamplesLow = _mm256_cvtepi16_epi32(pcmSamplesLow); // Convert 16-bit to 32-bit (low)
            __m256i intSamplesHigh = _mm256_cvtepi16_epi32(pcmSamplesHigh); // Convert 16-bit to 32-bit (high)

            // Merge low and high into a 256-bit vector of 32-bit integers
            __m256i intSamples = _mm256_packus_epi32(intSamplesLow, intSamplesHigh);

            // Convert 32-bit ints to floats by scaling
            __m256 floatSamples = _mm256_cvtepi32_ps(intSamples);  // Convert 32-bit ints to floats
            floatSamples = _mm256_mul_ps(floatSamples, mulFactor);  // Scale to range of -1.0f to 1.0f

            // Store the resulting floats back to output
            _mm256_storeu_ps(&output[i], floatSamples);
        }

        // Handle remaining samples (non-multiple of 8)
        for (; i < sampleCount; ++i) {
            output[i] = static_cast<float>(input[i]) * scale;
        }
    }

    // Copies float samples directly (assuming output is float*), for N channels
    void ConvertFloatToFloat(const float* input, BYTE* output, size_t frameCount, int channels) {
        size_t sampleCount = frameCount * channels;
        memcpy(output, input, sampleCount * sizeof(float));
    }

    // Converts float samples to 16-bit PCM (using your AVX2 function)
    void ConvertFloatToPCM16(const float* input, BYTE* output, size_t frameCount, int channels) {
        size_t sampleCount = frameCount * channels;
        ConvertFloatToPCM16_AVX2(input, reinterpret_cast<int16_t*>(output), sampleCount);
    }

    // Dummy fallback if unsupported format (fills silence)
    void ConvertSilence(const float* input, BYTE* output, size_t frameCount, int channels) {
        size_t byteCount = frameCount * channels * sizeof(int16_t); // Assuming silence in 16-bit PCM format
        memset(output, 0, byteCount);
    }

    void GenerateSineWave(int16_t* buffer, UINT32 frames) {
        static double phase = 0.0;
        static const double frequency = 440.0;
        static const double phaseIncrement = (2.0 * 3.14159265358979323846 * frequency) / 48000.0;

        for (UINT32 i = 0; i < frames; i++) {
            double sample = sin(phase) * 0.005; // Generate sine wave
            phase += phaseIncrement;
            if (phase >= 2.0 * 3.14159265358979323846) phase -= 2.0 * 3.14159265358979323846;

            int16_t sampleValue = static_cast<int16_t>(sample * 32767);
            buffer[i * 2] = sampleValue;      // Left channel
            buffer[i * 2 + 1] = sampleValue;  // Right channel
        }
    }
}