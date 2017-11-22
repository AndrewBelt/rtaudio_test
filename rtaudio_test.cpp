#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include "RtAudio.h"


int mini(int a, int b) {
	return a < b ? a : b;
}

void debug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(stderr, "[debug] ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	fflush(stderr);
	va_end(args);
}


static float phase = 0.0;


struct StreamData {
	int outChannels;
	int inChannels;
	int sampleRate;
};


int callback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData) {
	const float *input = (float*) inputBuffer;
	float *output = (float*) outputBuffer;
	StreamData *streamData = (StreamData*) userData;

	for (size_t i = 0; i < nFrames; i++) {
		phase += 440.0 / streamData->sampleRate;
		if (phase > 1.0)
			phase -= 1.0;
		float v = sinf(2*3.141592653589793 * phase);

		for (int c = 0; c < streamData->outChannels; c++) {
			output[i*streamData->outChannels + c] = v;
		}
	}
	return 0;
}


int main() {
	RtAudio stream;
	StreamData streamData;

	int deviceCount = stream.getDeviceCount();
	debug("deviceCount: %d", deviceCount);

	for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
		RtAudio::DeviceInfo deviceInfo = stream.getDeviceInfo(deviceId);
		debug("Stream %d: probed: %d, name: %s outputs: %d, inputs: %d, duplex: %d", deviceId, deviceInfo.probed, deviceInfo.name.c_str(), deviceInfo.outputChannels, deviceInfo.inputChannels, deviceInfo.duplexChannels);
		fflush(stderr);

		RtAudio::StreamParameters inParameters;
		inParameters.deviceId = deviceId;
		inParameters.nChannels = mini(deviceInfo.inputChannels, 8);

		RtAudio::StreamParameters outParameters;
		outParameters.deviceId = deviceId;
		outParameters.nChannels = mini(deviceInfo.outputChannels, 8);

		unsigned int sampleRate = 44100;
		unsigned int bufferFrames = 256;

		RtAudio::StreamOptions options;

		try {
			if (outParameters.nChannels == 0 && inParameters.nChannels == 0)
				continue;
			stream.openStream(
				outParameters.nChannels > 0 ? &outParameters : NULL,
				inParameters.nChannels > 0 ? &inParameters : NULL,
				RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &callback, &streamData, &options, NULL);
		}
		catch ( RtAudioError& e ) {
			debug("%s\n", e.what());
		}

		streamData.inChannels = inParameters.nChannels;
		streamData.outChannels = outParameters.nChannels;
		streamData.sampleRate = stream.getStreamSampleRate();

		debug("\toutputs: %d, inputs: %d, blockSize: %d, sampleRate: %d", deviceId, streamData.outChannels, streamData.inChannels, bufferFrames, streamData.sampleRate);

		try {
			stream.startStream();
			sleep(5);
			stream.closeStream();
		}
		catch ( RtAudioError& e ) {
			debug("%s", e.what());
		}
	}
}
