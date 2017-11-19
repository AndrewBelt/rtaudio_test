#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include "rtaudio/RtAudio.h"


int mini(int a, int b) {
	return a < b ? a : b;
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
	fprintf(stderr, "deviceCount: %d\n", deviceCount);

	for (int deviceId = 0; deviceId < deviceCount; deviceId++) {
		RtAudio::DeviceInfo info = stream.getDeviceInfo(deviceId);
		fprintf(stderr, "%d: probed: %d, name: %s output: %d, input: %d, duplex: %d\n", deviceId, info.probed, info.name.c_str(), info.outputChannels, info.inputChannels, info.duplexChannels);

		RtAudio::StreamParameters inParameters;
		inParameters.deviceId = deviceId;
		inParameters.nChannels = mini(info.inputChannels, 8);

		RtAudio::StreamParameters outParameters;
		outParameters.deviceId = deviceId;
		outParameters.nChannels = mini(info.outputChannels, 8);

		unsigned int sampleRate = 44100;
		unsigned int bufferFrames = 256;

		RtAudio::StreamOptions options;

		try {
			stream.openStream(
				outParameters.nChannels > 0 ? &outParameters : NULL,
				inParameters.nChannels > 0 ? &inParameters : NULL,
				RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &callback, &streamData, &options, NULL);
		}
		catch ( RtAudioError& e ) {
			fprintf(stderr, "%s\n", e.what());
		}

		streamData.inChannels = inParameters.nChannels;
		streamData.outChannels = outParameters.nChannels;
		streamData.sampleRate = stream.getStreamSampleRate();

		try {
			fprintf(stderr, "Starting stream %d\n", deviceId);
			stream.startStream();
			sleep(3);
			fprintf(stderr, "Stopping stream %d\n", deviceId);
			stream.closeStream();
		}
		catch ( RtAudioError& e ) {
			fprintf(stderr, "%s\n", e.what());
		}
	}
}
